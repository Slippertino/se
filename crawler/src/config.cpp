#include <crawler/config.hpp>

namespace se {

namespace crawler {

Config::Config(const YAML::Node& root) : se::utils::Config(root) { 
    init_cached_data();
}

Config::Config(const std::filesystem::path& path) : se::utils::Config(path) { 
    init_cached_data();
}

std::string Config::name() const noexcept {
    return name_;
}

size_t Config::max_resource_size() const noexcept {
    return max_resource_size_;
}

DbConfig Config::db_config() const {
    return DbConfig{
        boost::url{ connection_string("crawler.db") },
        get<size_t>("crawler.db.pool_size")
    };
}

void Config::init_cached_data() {
    name_ = config_["crawler"]["name"].as<std::string>();
    max_resource_size_ = from_service<size_t>("processor", "max_resource_size");
}
    
} // namespace crawler

} // namespace se