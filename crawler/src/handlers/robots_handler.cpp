#include <crawler/handlers/robotstxt_handler.hpp>
#include <crawler/core/resource_processor.hpp>

namespace se {

namespace crawler {

RobotsTxtHandler::RobotsTxtHandler(RobotsTxtHandler::private_token) : HttpResourceHandler({})
{ }

void RobotsTxtHandler::handle_failed_load() {
    const auto& domain = *resource_->header.domain;
    auto& robots_cache = RobotsCache::instance();
    if (!robots_cache.contains(domain))
        robots_cache.upload(domain, RobotState{});
    const auto& state = robots_cache[domain];
    if (state.load_attempts_last) {
        set_handling_status(HandlingStatus::partly_handled);
        robots_cache.modify(domain, [](RobotState& st) {
            --st.load_attempts_last;
        });
    }
    else if (state.type == RobotStateType::Loading) {
        robots_cache.modify(domain, [](RobotState& st) {
            st.type = RobotStateType::Loaded;
            st.robots = std::nullopt;
        });        
    }
}

void RobotsTxtHandler::handle_http_resource(HttpResults& results) {
    auto& robots = RobotsCache::instance();
    const auto& domain = *resource_->header.domain;
    if (!robots.contains(domain))
        robots.upload(domain, RobotState{});
    auto rb = parsers::Robots{ results.body };
    auto raw_sitemaps = rb.extract_sitemaps(Config::name());
    std::vector<ResourcePtr> sitemaps;
    sitemaps.reserve(raw_sitemaps.size());
    for(auto&& sm : raw_sitemaps) {
        boost::url url{ sm };
        sitemaps.push_back(Resource::create_from_url<Sitemap>(url, url.path() + url.query()));
    }
    robots.modify(domain, [&rb](RobotState& st) {
        st.type = RobotStateType::Loaded;
        st.robots = std::move(rb);
    });
    if (auto proc = processor_.lock())
        proc->handle_new_resources(std::move(sitemaps));
    set_handling_status(HandlingStatus::handled_success);
    LOG_INFO_WITH_TAGS(
        logging::handler_category, 
        "Successfully handled ROBOTS file with URL: {}", 
        resource_->url().c_str()
    );
}

} // namespace crawler

} // namespace se