#include <seutils/config/config.hpp>

namespace se {

namespace utils {

Config::Config(const YAML::Node& root) : 
    config_{ root }
{ }

Config::Config(const std::filesystem::path& path) : Config{ YAML::LoadFile(path) }
{ }

AMQPBusConfig Config::bus_config(const std::string& path) const {
    const auto bus_root = node_by_path(path);
    se::utils::AMQPBusConfig config;
    extract_optional_field(bus_root, "pool_size", config, &AMQPBusConfig::pool_size);
    extract_optional_field(bus_root, "connection_string", [&config](const YAML::Node& node) {
        config.url = boost::url(node.as<std::string>());
    });
    extract_optional_field(bus_root, "messages", [&config](const YAML::Node& messages_node) {
        for(const auto& cur : messages_node) {
            const auto& fields = cur;

            auto key = fields["name"].as<std::string>();

            TransmissionOptions opts;
            extract_optional_field(fields, "max_batch_size", opts, &TransmissionOptions::max_batch_size);
            extract_optional_field(fields, "max_batch_volume", opts, &TransmissionOptions::max_batch_volume);
            extract_optional_field(fields, "compression_type", opts, &TransmissionOptions::compression_type);

            se::utils::AMQPBusMessageConfig msg {
                fields["enabled"].as<bool>(),
                fields["exchange"].as<std::string>(),
                fields["routing_key"].as<std::string>(),
                std::move(opts)
            };

            config.messages.insert({ std::move(key), std::move(msg) });
        }
    });
    extract_optional_field(bus_root, "queues", [&config](const YAML::Node& queues_node) {
        for(const auto& cur : queues_node) {
            auto key = cur.first.as<std::string>();
            auto name = cur.second.as<std::string>();
            config.queues.insert({ std::move(key), std::move(name) });
        }
    });
    return config;
}

size_t Config::thread_pool(const std::string& path) const {
    return get<size_t>(
        (boost::format{ "%1%.thread_pool" } % path).str()
    );    
}

std::string Config::connection_string(const std::string& path) const {
    return get<std::string>(
        (boost::format{ "%1%.connection_string" } % path).str()
    );
}

std::string Config::logging_message_pattern(const std::string& key, const std::string& logging_path) const {
    return get<std::string>(
        (boost::format{ "%1%.log_formats.%2%" } % logging_path % key).str()            
    );
}

std::string Config::logging_time_pattern(const std::string& key, const std::string& logging_path) const {
    return get<std::string>(
        (boost::format{ "%1%.time_formats.%2%" } % logging_path % key).str()            
    ); 
}

YAML::Node Config::node_by_path(const std::string& path) const {
    if (path.empty())
        return config_;
    std::vector<std::string> keys;
    boost::split(keys, path, boost::is_any_of("./"));
    YAML::Node cur = config_;
    for(const auto& k : keys)
        if (k.empty())
            continue;
        else
            cur.reset(cur[k]);    
    return cur;
}
    
} // namespace utils

} // namespace se