#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>

namespace se {

namespace indexer {

struct PrimaryIndexerOptions {
    size_t min_lexem_size;
    size_t max_lexem_size;
    std::string resource_compression_type;
    std::vector<std::string> resource_skip_tags;
    std::unordered_map<std::string, double> resource_tag_weights; 
};

struct LoaderOptions {
    size_t batch_size;
    std::chrono::duration<double, std::ratio<1>> speed;
};

struct ResourcesRankingOptions {
    size_t rank_estimation_iterations_count;
    double teleport_probability;
};

struct ChampionListsOptions {
    size_t size;
    double threshold;
};

struct IndexingOptions {
    size_t resources_capacity;
    PrimaryIndexerOptions primary_options;  
    LoaderOptions loader_options;
    ResourcesRankingOptions ranking_options;
    ChampionListsOptions champ_lists_options;
};

} // namespace indexer

} // namespace se