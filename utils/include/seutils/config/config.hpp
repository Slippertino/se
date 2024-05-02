#pragma once

#include <string>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/node/convert.h>
#include <seutils/amqp/amqp_config.hpp>

namespace se {

namespace utils {

class Config {
public:
    template<typename T>
    static T get(const std::string& path) {
        return node_by_path(path).as<T>();
    }

    static AMQPBusConfig bus_config(const std::string& path);
    static size_t thread_pool(const std::string& path);
    static std::string connection_string(const std::string& path);

protected:
    static void load_impl(const std::string& path);
    static YAML::Node node_by_path(const std::string& path);

protected:
    static inline YAML::Node config_;
};

} // namespace utils

} // namespace se