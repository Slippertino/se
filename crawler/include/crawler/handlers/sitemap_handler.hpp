#pragma once

#include <regex>
#include <crawler/parsers/sitemap/parser.hpp>
#include <seutils/compression_helper.hpp>
#include "http_resource_handler.hpp"

namespace se {

namespace crawler {

class SitemapHandler final : public HttpResourceHandler {
public:
    SitemapHandler(private_token);

protected:
    void handle_http_resource(HttpResults& results) override;

private:
    static void extract_resources(
        ResourcePtr& resource,
        std::string content, 
        std::vector<ResourcePtr>& out
    );

private:
    static inline const std::regex raw_sitemap_pattern_ = std::regex(R"(.*\.xml$)");
};

} // namespace crawler

} // namespace se