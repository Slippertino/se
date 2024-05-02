#pragma once

#include <vector>
#include <boost/url.hpp>

namespace se {

namespace crawler {

namespace parsers {

class Robots {
public:
    Robots(const std::string& content);
    bool allowed(const std::string& agent, const boost::url& url) const;
    std::vector<std::string> extract_sitemaps(const std::string& agent) const;

private:
    std::string content_;
};

} // namespace parsers

} // namespace crawler

} // namespace se