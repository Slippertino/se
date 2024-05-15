#include <indexer/config.hpp>
#include <iostream>

namespace se {

namespace indexer {

void Config::load(const std::string& path) {
    se::utils::Config::load_impl(path);
}

std::string Config::logging_message_pattern(const std::string& key) {
    return get<std::string>(
        (boost::format{ "indexer.logging.log_formats.%1%" } % key).str()            
    );
}

std::string Config::logging_time_pattern(const std::string& key) {
    return get<std::string>(
        (boost::format{ "indexer.logging.time_formats.%1%" } % key).str()            
    );
}

IndexingOptions Config::options() {
    auto cfg_root = node_by_path("indexer.options");
    IndexingOptions options;
    options.resources_capacity = cfg_root["resources_capacity"].as<size_t>();

    auto pi = cfg_root["primary_indexer"];
    auto& po = options.primary_options;
    po.resource_compression_type = pi["compression_type"].as<std::string>();
    po.min_lexem_size = pi["min_lexem_size"].as<size_t>();
    po.max_lexem_size = pi["max_lexem_size"].as<size_t>();
    auto tags = pi["tags"];
    for(const auto& cur : tags["skip"]) {
        auto key = cur.as<std::string>();
        po.resource_skip_tags.push_back(key);
    }
    for(const auto& cur : tags["weights"]) { 
        auto name = cur.first.as<std::string>();
        auto weight = cur.second.as<double>();
        po.resource_tag_weights.insert({ std::move(name), weight });        
    }

    auto ldr = cfg_root["loader"];
    auto& lo = options.loader_options;
    lo.batch_size = ldr["batch_size"].as<size_t>();
    lo.speed = decltype(lo.speed)(ldr["speed"].as<double>());

    auto rr = cfg_root["resources_ranking"];
    auto& rro = options.ranking_options;
    rro.teleport_probability = rr["teleport_probability"].as<double>();
    rro.rank_estimation_iterations_count = rr["ranking_iterations_count"].as<size_t>();

    auto chlst = cfg_root["champion_lists"];
    auto& chlstopt = options.champ_lists_options;
    chlstopt.size = chlst["size"].as<size_t>();
    chlstopt.threshold = chlst["threshold"].as<double>();

    return options;
}

DbConfig Config::db_config() {
    return DbConfig {
        boost::url{ connection_string("indexer.db") },
        get<size_t>("indexer.db.pool_size")
    };
}

} // namespace indexer

} // namespace se