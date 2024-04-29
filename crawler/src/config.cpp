#include <crawler/config.hpp>

namespace crawler {

void Config::load(const std::string& path) {
    config_ = YAML::LoadFile(path);
    name_ = config_["crawler"]["name"].as<std::string>();
    max_resource_size_ = from_service<size_t>("processor", "max_resource_size");
}

std::string Config::name() noexcept {
    return name_;
}

size_t Config::max_resource_size() noexcept {
    return max_resource_size_;
}

size_t Config::thread_pool(const std::string& path) {
    return get<size_t>(
        (boost::format{ "%1%.thread_pool" } % path).str()
    );    
}

std::string Config::connection_string(const std::string& path) {
    return get<std::string>(
        (boost::format{ "%1%.connection_string" } % path).str()
    );
}

std::string Config::logging_pattern(const std::string& key) {
    return get<std::string>(
        (boost::format{ "crawler.logging.formats.%1%" } % key).str()            
    );
}

AMQPBusConfig Config::bus_config() {
    auto bus_root = node_by_path("crawler.bus");
    AMQPBusConfig config;
    config.url = boost::url { 
        bus_root["connection_string"].as<std::string>() 
    };
    for(const auto& cur : bus_root["messages"]) {
        auto key = cur.first.as<std::string>();
        auto& fields = cur.second;
        AMQPBusMessageConfig msg {
            fields["enabled"].as<bool>(),
            fields["exchange"].as<std::string>(),
            fields["routing_key"].as<std::string>()
        };
        config.messages.insert({ std::move(key), std::move(msg) });
    }
    return config;
}

DbConfig Config::db_config() {
    return DbConfig{
        boost::url{ connection_string("crawler.db") }
    };
}

YAML::Node Config::node_by_path(const std::string& path) {
    std::vector<std::string> keys;
    boost::split(keys, path, boost::is_any_of("./"));
    YAML::Node cur = config_;
    for(const auto& k : keys)
        cur.reset(cur[k]);    
    return cur;
}
    
} // namespace crawler