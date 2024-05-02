#include <crawler/config.hpp>

namespace se {

namespace crawler {

void Config::load(const std::string& path) {
    se::utils::Config::load_impl(path);
    name_ = config_["crawler"]["name"].as<std::string>();
    max_resource_size_ = from_service<size_t>("processor", "max_resource_size");
}

std::string Config::name() noexcept {
    return name_;
}

size_t Config::max_resource_size() noexcept {
    return max_resource_size_;
}

std::string Config::logging_message_pattern(const std::string& key) {
    return get<std::string>(
        (boost::format{ "crawler.logging.log_formats.%1%" } % key).str()            
    );
}

std::string Config::logging_time_pattern(const std::string& key) {
    return get<std::string>(
        (boost::format{ "crawler.logging.time_formats.%1%" } % key).str()            
    );
}

DbConfig Config::db_config() {
    return DbConfig{
        boost::url{ connection_string("crawler.db") }
    };
}
    
} // namespace crawler

} // namespace se