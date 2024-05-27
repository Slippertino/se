#include <searcher/config/config.hpp>

namespace se {

namespace searcher {

Config::Config(const YAML::Node& root) : se::utils::Config(root)
{ }

Config::Config(const std::filesystem::path& path) : se::utils::Config(path)
{ }

SearchOptions Config::options(const std::string& options_path) const {
    auto cfg_root = node_by_path(options_path);
    SearchOptions options;
    options.max_query_languages_count = cfg_root["max_query_languages_count"].as<size_t>();
    options.language_threshold = cfg_root["language_threshold"].as<float>();
    options.resource_rank_threshold = cfg_root["resource_rank_threshold"].as<float>();
    options.resource_static_rank_component_rate = cfg_root["resource_static_rank_component_weight"].as<float>();
    options.resource_dynamic_rank_component_rate = cfg_root["resource_dynamic_rank_component_weight"].as<float>();
    options.default_language = cfg_root["default_language"].as<std::string>();
    return options;
}

} // namespace searcher

} // namespace se