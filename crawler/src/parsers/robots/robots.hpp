#pragma once

#include <vector>
#include <robotstxt/robots.h>

namespace crawler {

namespace robots {

class ExtendedRobotsMatcher final : public googlebot::RobotsMatcher {
public:
    const std::vector<std::string>& Sitemaps() const {
        return sitemaps_;
    }

    void ClearSitemaps() {
        sitemaps_.clear();
    }

protected:
    void HandleSitemap(int line_num, absl::string_view value) override final {
        sitemaps_.push_back(std::string{value});
    }

private:
    std::vector<std::string> sitemaps_;
};

} // namespace robots

} // namespace crawler

