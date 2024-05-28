#include <crawler/handlers/http_resource_handler.hpp>
#include <crawler/core/resource_processor.hpp>

namespace se {

namespace crawler {

HttpResourceHandler::HttpResourceHandler(HttpResourceHandler::private_token) : 
    ResourceHandler({}),
    se::utils::ThreadedMemoryBuffer<resource_handling_tag>{ se::utils::GlobalConfig<Config>::config.max_resource_size() }
{ } 

HttpResourceHandler::~HttpResourceHandler() { }

bool HttpResourceHandler::check_permissions() const {
    const auto& domain = *resource_->header.domain;
    auto& robots_cache = RobotsCache::instance();
    if (!robots_cache.contains(domain))
        return false;
    const auto& state = robots_cache[domain];
    if (!state.robots.has_value())
        return false;
    const auto& robots = state.robots.value();
    if (!robots.allowed(se::utils::GlobalConfig<Config>::config.name(), resource_->url())) {
        LOG_TRACE_L1_WITH_TAGS(logging::handler_category, "Permission denied to {}", resource_->url().c_str());
        return false;
    }
    return true;
} 

void HttpResourceHandler::handle_no_permissions() {
    const auto& domain = *resource_->header.domain;
    auto& robots_cache = RobotsCache::instance();
    if (!robots_cache.contains(domain))
        robots_cache.upload(domain, RobotState{});
    const auto& state = robots_cache[domain];
    if (state.type == RobotStateType::Loaded)
        return;
    set_handling_status(HandlingStatus::partly_handled);
    if (state.type == RobotStateType::Loading)
        return;
    std::vector<ResourcePtr> resources;
    resources.push_back(Resource::create_from_url<RobotsTxt, Resource>(resource_->header.url()));
    robots_cache.modify(domain, [](RobotState& state) {
        state.type = RobotStateType::Loading;
    });
    if (auto proc = processor_.lock())
        proc->handle_new_resources(std::move(resources));
}

void HttpResourceHandler::handle_failed_load() { }

void HttpResourceHandler::handle_resource(ResourceLoader::ResourceLoadResults results) {
    if (!results.success) {
        LOG_ERROR_WITH_TAGS(
            logging::handler_category, 
            "Failed to get {} with error: {}.", 
            resource_->url().c_str(),
            results.error_message
        );
        handle_failed_load();
        return;   
    }
    if (results.content.size() > get_thread_resource().size()) {
        LOG_WARNING_WITH_TAGS(
            logging::handler_category, 
            "The size of the resource {} exceeds the allowed limit.", 
            resource_->url().c_str()
        );
        handle_failed_load();
        return;
    }
    boost::beast::http::response_parser<boost::beast::http::string_body> parser;
    boost::beast::error_code ec;
    parser.put(boost::asio::buffer(results.metadata), ec);
    if (ec) {
        LOG_ERROR_WITH_TAGS(
            logging::handler_category, 
            "Couldn't read metadata of {} with error: {}.", 
            resource_->url().c_str(),
            ec.message()
        );
        handle_failed_load();
        return;
    }
    auto& headers = parser.get();
    auto code = headers.result();
    if (code_handlers_.contains(code))
        code_handlers_.at(code)(this, results, headers);
    else 
        handle_unknown_code(results, headers);
}

void HttpResourceHandler::handle_ok(ResourceLoader::ResourceLoadResults& results, headers_t& headers) {
    if (auto proc = processor_.lock())
        proc->handle_resource_received(resource_);
    HttpResults http_results { 
        std::move(headers), 
        std::move(results.content)
    };
    try {
        handle_http_resource(http_results);
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            logging::handler_category, 
            "Error occured while processing resource {}: {}", 
            resource_->url().c_str(),
            ex.what()
        );
    }
}

void HttpResourceHandler::handle_redirect(ResourceLoader::ResourceLoadResults& results, headers_t& headers) {
    auto proc = processor_.lock();
    if (proc)
        proc->handle_resource_received(resource_);
    auto loc = boost::url(headers["Location"]);
    loc.set_scheme(*resource_->header.type);
    LOG_WARNING_WITH_TAGS(
        logging::handler_category, 
        "HTTP redirection ({}) from {} to {}.", 
        headers.result_int(),
        resource_->url().c_str(),
        loc.c_str()
    );
    std::vector<ResourcePtr> resources;
    resources.push_back(
        Resource::create_from_url<Page, Resource>(loc, loc.path() + loc.query())
    );
    if (proc)
        proc->handle_new_resources(std::move(resources));
    handle_failed_load();
}

void HttpResourceHandler::handle_bad_request(ResourceLoader::ResourceLoadResults& results, headers_t& headers) {
    if (auto proc = processor_.lock())
        proc->handle_resource_received(resource_);
    LOG_WARNING_WITH_TAGS(
        logging::handler_category, 
        "Bad HTTP response code {} while fetching resource {}.", 
        headers.result_int(),
        resource_->url().c_str()
    );
    handle_failed_load();
}

void HttpResourceHandler::handle_requests_overflow(ResourceLoader::ResourceLoadResults& results, headers_t& headers) {
    auto delay_sec = std::atoi(headers["Retry-After"].data());
    set_handling_status(HandlingStatus::partly_handled);
    if (auto proc = processor_.lock())
        proc->handle_resource_received(resource_, true, std::chrono::seconds(delay_sec));    
    LOG_WARNING_WITH_TAGS(
        logging::handler_category, 
        "HTTP requests overflow occured while fetching {}, delay is {} seconds.", 
        resource_->url().c_str(),
        delay_sec
    );
    handle_failed_load();
}

void HttpResourceHandler::handle_unknown_code(ResourceLoader::ResourceLoadResults& results, headers_t& headers) {
    if (auto proc = processor_.lock())
        proc->handle_resource_received(resource_);
    LOG_WARNING_WITH_TAGS(
        logging::handler_category, 
        "Unhandled HTTP {} response code occurs while fetching resource {}.", 
        headers.result_int(),
        resource_->url().c_str()
    );
    handle_failed_load();
}

} // namespace crawler

} // namespace se