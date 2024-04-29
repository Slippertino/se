#include <crawler/handlers/robotstxt_handler.hpp>
#include <crawler/core/resource_processor.hpp>

namespace crawler {

RobotsTxtHandler::RobotsTxtHandler(RobotsTxtHandler::private_token) : HttpResourceHandler({})
{ }

void RobotsTxtHandler::handle_failed_load(
    ResourcePtr& resource, 
    std::shared_ptr<ResourceProcessor>& processor        
) {
    const auto& domain = *resource->header.domain;
    auto& robots_cache = RobotsCache::instance();
    if (!robots_cache.contains(domain))
        return;
    const auto& state = robots_cache[domain];
    if (state.type == RobotStateType::Loading) {
        robots_cache.modify(domain, [](RobotState& st){
            st.type = RobotStateType::None;
            if (st.load_attempts_last)
                --st.load_attempts_last;
        });
    }
}

void RobotsTxtHandler::handle_http_resource(
    ResourcePtr& resource, 
    std::shared_ptr<ResourceProcessor>& processor,
    HttpResults& results
) {
    auto& robots = RobotsCache::instance();
    const auto& domain = *resource->header.domain;
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
    robots.modify(domain, [&rb](RobotState& st){
        st.type = RobotStateType::Loaded;
        st.robots = std::move(rb);
    });
    processor->handle_new_resources(std::move(sitemaps));
    confirm_success_handling();
    LOG_INFO_WITH_TAGS(
        logging::handler_category, 
        "Successfully handled ROBOTS file with URL: {}", 
        resource->url().c_str()
    );
}

} // namespace crawler