#include <robotstxt/robots.h>
#include <crawler/parsers/robots/extended_robots_matcher.hpp>
#include <crawler/parsers/robots/robots.hpp>

namespace crawler {

namespace parsers {

Robots::Robots(const std::string& content) :
    content_{ content }
{ }

bool Robots::allowed(const std::string& agent, const boost::url& url) const {
    return ExtendedRobotsMatcher{ false }.OneAgentAllowedByRobots(
        content_, 
        agent, 
        std::string{ url.c_str() }
    );
}

std::vector<std::string> Robots::extract_sitemaps(const std::string& agent) const {
    auto matcher = ExtendedRobotsMatcher{ true };
    matcher.OneAgentAllowedByRobots(content_, agent, "");
    return matcher.Sitemaps();
}

} // namespace parsers

} // namespace crawler

