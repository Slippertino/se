#include <crawler/handlers/http_resource_handler.hpp>
#include <crawler/core/resource_processor.hpp>

namespace se {

namespace crawler {

HttpResourceHandler::HttpResourceHandler(HttpResourceHandler::private_token) : 
    ResourceHandler({}),
    se::utils::ThreadedMemoryBuffer<resource_handling_tag>{ Config::max_resource_size() }
{ } 

HttpResourceHandler::~HttpResourceHandler() { }

bool HttpResourceHandler::check_permissions(
    const ResourcePtr& resource,
    std::shared_ptr<ResourceProcessor>& processor
) const {
    const auto& domain = *resource->header.domain;
    auto& robots_cache = RobotsCache::instance();
    if (!robots_cache.contains(domain))
        return false;
    const auto& state = robots_cache[domain];
    if (!state.robots.has_value())
        return false;
    const auto& robots = state.robots.value();
    if (!robots.allowed(Config::name(), resource->url())) {
        LOG_TRACE_L1_WITH_TAGS(logging::handler_category, "Permission denied to {}", resource->url().c_str());
        return false;
    }
    return true;
} 

void HttpResourceHandler::handle_no_permissions(
    const ResourcePtr& resource,
    std::shared_ptr<class ResourceProcessor>& processor
) {
    const auto& domain = *resource->header.domain;
    auto& robots_cache = RobotsCache::instance();
    if (!robots_cache.contains(domain))
        robots_cache.upload(domain, RobotState{});
    const auto& state = robots_cache[domain];
    if (state.type == RobotStateType::Loaded)
        return;
    std::vector<ResourcePtr> resources;
    if (state.type == RobotStateType::None && state.load_attempts_last) {
        resources.push_back(Resource::create_from_url<RobotsTxt, Resource>(resource->header.url()));
        robots_cache.modify(domain, [](RobotState& state){
            state.type = RobotStateType::Loading;
        });
    }
    resources.push_back(resource->clone());
    processor->handle_new_resources(std::move(resources));    
}

void HttpResourceHandler::handle_failed_load(        
    ResourcePtr& resource, 
    std::shared_ptr<ResourceProcessor>& processor
) { }

void HttpResourceHandler::handle_resource(
    ResourcePtr& resource, 
    std::shared_ptr<ResourceProcessor> processor, 
    ResourceLoader::ResourceLoadResults results
) {
    processor->handle_resource_received(resource);
    if (!results.success) {
        LOG_ERROR_WITH_TAGS(
            logging::handler_category, 
            "Failed to get {} with error: {}.", 
            resource->url().c_str(),
            results.error_message
        );
        handle_failed_load(resource, processor);
        return;   
    }
    if (results.content.size() > get_thread_resource().size()) {
        LOG_WARNING_WITH_TAGS(
            logging::handler_category, 
            "The size of the resource {} exceeds the allowed limit.", 
            resource->url().c_str()
        );
        handle_failed_load(resource, processor);
        return;
    }

    boost::beast::http::response_parser<boost::beast::http::string_body> parser;
    boost::beast::error_code ec;
    parser.put(boost::asio::buffer(results.metadata), ec);
    if (ec) {
        LOG_ERROR_WITH_TAGS(
            logging::handler_category, 
            "Couldn't read metadata of {} with error: {}.", 
            resource->url().c_str(),
            ec.message()
        );
        handle_failed_load(resource, processor);
        return;
    }
    auto& headers = parser.get();
    auto code = headers.result_int();
    std::vector<ResourcePtr> resources;
    if (code == 200) {
        HttpResults http_results{ 
            std::move(headers), 
            std::move(results.content)
        };
        try {
            handle_http_resource(resource, processor, http_results);
            return;
        } catch(const std::exception& ex) {
            LOG_ERROR_WITH_TAGS(
                logging::handler_category, 
                "Error occured while processing resource {}: {}", 
                resource->url().c_str(),
                ex.what()
            );
        }
    }
    else if (code == 301 || code == 302) {
        auto loc = boost::url(headers["Location"]);
        loc.set_scheme(*resource->header.type);
        resources.push_back(
            Resource::create_from_url<Page, Resource>(loc, loc.path() + loc.query())
        );
        processor->handle_new_resources(std::move(resources));
    }
    else if (code >= 400) {
        LOG_WARNING_WITH_TAGS(
            logging::handler_category, 
            "Bad HTTP response code {} while fetching resource {}.", 
            code,
            resource->url().c_str()
        );
    }
    else {
        LOG_WARNING_WITH_TAGS(
            logging::handler_category, 
            "Unhandled HTTP {} response code occurs while fetching resource {}.", 
            code,
            resource->url().c_str()
        );
    }
    handle_failed_load(resource, processor);
}

} // namespace crawler

} // namespace se