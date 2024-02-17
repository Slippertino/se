#pragma once

#include <string>
#include <optional>
#include <variant>

namespace crawler {

namespace sitemap {

enum PageChangeFrequencyType {
    always, 
    hourly,
    daily,
    weekly,
    monthly,
    yearly,
    never
};

struct SitemapRoot {
    std::string url;
};

struct SitemapUrl {
    std::string loc;
    std::optional<std::string> lastModified = std::nullopt;
    std::optional<PageChangeFrequencyType> changeFreq = std::nullopt;
    std::optional<float> priority = 0.5; 
};
using SitemapUrlset = std::vector<SitemapUrl>;

struct Sitemap {
    std::string loc;
    std::optional<std::string> lastModified = std::nullopt;
};
using SitemapIndex = std::vector<Sitemap>;

using SitemapAny = std::variant<
    SitemapRoot, 
    SitemapIndex, 
    SitemapUrlset
>;

} // namespace sitemap

} // namespace crawler