#pragma once

#include <searcher/config/search_options.hpp>
#include <searcher/models/resource.hpp>

namespace se {

namespace searcher {

struct RelevanceEstimator {
    float operator()(const ResourceRank& rank, const SearchOptions& opts);
};

} // namespace searcher

} // namespace se