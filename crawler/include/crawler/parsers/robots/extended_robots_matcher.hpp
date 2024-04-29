#pragma once

#include <vector>
#include <boost/url.hpp>
#include <robotstxt/robots.h>

namespace crawler {

namespace parsers {

class ExtendedRobotsMatcher final : public googlebot::RobotsMatcher {
public:
    ExtendedRobotsMatcher(bool handling_sitmaps);

    std::vector<std::string> Sitemaps() const noexcept;

protected:
    void HandleSitemap(int line_num, absl::string_view value) override;

private:
    bool handling_sitemaps_;
    std::vector<std::string> sitemaps_;
};

} // namespace parsers

} // namespace crawler

