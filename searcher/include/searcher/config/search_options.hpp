#pragma once

#include <string>

namespace se {

namespace searcher {

struct SearchOptions {
    size_t max_query_languages_count;
    float language_threshold;
    float resource_rank_threshold;
    float resource_static_rank_component_rate;
    float resource_dynamic_rank_component_rate;
    std::string default_language;
};

} // namespace searcher

} // namespace se