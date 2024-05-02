#include <crawler/parsers/robots/extended_robots_matcher.hpp>

namespace se {

namespace crawler {

namespace parsers {

ExtendedRobotsMatcher::ExtendedRobotsMatcher(bool handling_sitmaps) : 
    googlebot::RobotsMatcher(),
    handling_sitemaps_{ handling_sitmaps }
{ }

std::vector<std::string> ExtendedRobotsMatcher::Sitemaps() const noexcept {
    return sitemaps_;
}

void ExtendedRobotsMatcher::HandleSitemap(int line_num, absl::string_view value) {
    if (handling_sitemaps_)
        sitemaps_.emplace_back(value);
}

} // namespace parsers

} // namespace crawler

} // namespace se