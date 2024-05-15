#include <seutils/config/config.hpp>
#include <iostream>

namespace se {

namespace utils {

AMQPBusConfig Config::bus_config(const std::string& path) {
    auto bus_root = node_by_path(path);
    se::utils::AMQPBusConfig config;
    config.pool_size = bus_root["pool_size"].as<size_t>();
    config.url = boost::url { 
        bus_root["connection_string"].as<std::string>() 
    };
    for(const auto& cur : bus_root["messages"]) {
        auto key = cur.first.as<std::string>();
        auto& fields = cur.second;
        se::utils::AMQPBusMessageConfig msg {
            fields["enabled"].as<bool>(),
            fields["exchange"].as<std::string>(),
            fields["routing_key"].as<std::string>()
        };
        config.messages.insert({ std::move(key), std::move(msg) });
    }
    for(const auto& cur : bus_root["queues"]) {
        auto key = cur.first.as<std::string>();
        auto name = cur.second.as<std::string>();
        config.queues.insert({ std::move(key), std::move(name) });
    }
    return config;
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

void Config::load_impl(const std::string& path) {
    config_ = YAML::LoadFile(path);
}

YAML::Node Config::node_by_path(const std::string& path) {
    std::vector<std::string> keys;
    boost::split(keys, path, boost::is_any_of("./"));
    YAML::Node cur = config_;
    for(const auto& k : keys)
        cur.reset(cur[k]);    
    return cur;
}
    
} // namespace utils

} // namespace se