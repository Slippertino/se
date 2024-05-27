#include <searcher/tools/relevance_estimator.hpp>

namespace se {

namespace searcher {

float RelevanceEstimator::operator()(const ResourceRank& rank, const SearchOptions& opts) {
    return (
        rank.dynamic_rank * opts.resource_dynamic_rank_component_rate + 
        rank.static_rank * opts.resource_static_rank_component_rate
    ) / (opts.resource_dynamic_rank_component_rate + opts.resource_static_rank_component_rate);        
}

} // namespace searcher

} // namespace se