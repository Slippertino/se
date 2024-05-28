#include <logger/config/config.hpp>

namespace se {

namespace logger {

Config::Config(const YAML::Node& root) : se::utils::Config(root)
{ }

Config::Config(const std::filesystem::path& path) : se::utils::Config(path)
{ }

LoggingOptions Config::options() const {
    const auto opts_root = node_by_path("logger.options");
    LoggingOptions opts;
    opts.memory_limit = opts_root["memory_limit"].as<size_t>();
    opts.reduce_ratio = opts_root["reduce_ratio"].as<double>();
    opts.periodicity = std::chrono::milliseconds(opts_root["periodicity_ms"].as<size_t>());
    return opts;
}

DbConfig Config::db_config() const {
    return DbConfig{
        boost::url{ connection_string("logger.db") },
        get<size_t>("logger.db.pool_size")
    };
}

} // namespace logger

} // namespace se