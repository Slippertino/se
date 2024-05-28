#pragma once

#include <string>
#include <concepts>
#include <filesystem>
#include <type_traits>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/node/convert.h>
#include <seutils/amqp/amqp_config.hpp>
#include <seutils/dtos/transmission_options.hpp>

namespace se {

namespace utils {

class Config {
public:
    Config() = default;
    Config(const YAML::Node&);
    explicit Config(const std::filesystem::path& path); 

    template<typename T>
    T get(const std::string& path = "") const {
        return node_by_path(path).as<T>();
    }

    AMQPBusConfig bus_config(const std::string& path = "") const;
    size_t thread_pool(const std::string& path = "") const;
    std::string connection_string(const std::string& path = "") const;
    std::string logging_message_pattern(const std::string& key, const std::string& logging_path = "") const;
    std::string logging_time_pattern(const std::string& key, const std::string& logging_path = "") const;

protected:
    template<typename Successor>
    static void extract_optional_field(const YAML::Node& root, const std::string& name, Successor&& handler) {
        auto node = root[name];
        if (node.IsDefined())
            handler(node);
    }

    template<typename Target, typename Field>
    static void extract_optional_field(const YAML::Node& root, const std::string& name, Target& target, const Field& field) {
        auto& value = std::invoke(field, target);
        extract_optional_field(root, name, [&value](const YAML::Node& node) {
            value = node.as<std::decay_t<decltype(value)>>();
        });
    }

    YAML::Node node_by_path(const std::string& path) const;

protected:
    YAML::Node config_;
};

template<std::derived_from<Config> ConfigType>
struct GlobalConfig {
    static inline ConfigType config;
    static inline void load_global_config(const std::filesystem::path& path) {
        config = ConfigType(path);
    }
};

} // namespace utils

} // namespace se