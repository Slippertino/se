#pragma once

#include <regex>
#include <crawler/parsers/sitemap/parser.hpp>
#include <crawler/utils/compression_helper.hpp>
#include "http_resource_handler.hpp"

namespace crawler {

class SitemapHandler final : public HttpResourceHandler {
public:
    SitemapHandler(private_token);

protected:
    void handle_http_resource(
        ResourcePtr& resource, 
        std::shared_ptr<ResourceProcessor>& processor,
        HttpResults& results
    ) override;

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