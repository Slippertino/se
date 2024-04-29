#pragma once

#include <string>
#include <optional>
#include <variant>

namespace crawler {

namespace parsers {

enum class SitemapType {
    INDEX,
    URLSET
};

enum PageChangeFrequencyType {
    always, 
    hourly,
    daily,
    weekly,
    monthly,
    yearly,
    never
};

struct SitemapAny {
    SitemapType type;
    std::string loc;
    std::optional<std::string> last_modified = std::nullopt;
    std::optional<PageChangeFrequencyType> change_freq = std::nullopt;
    std::optional<float> priority = 0.5; 
};

} // namespace parsers

} // namespace crawler