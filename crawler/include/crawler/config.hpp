#pragma once

#include <string>
#include <chrono>
#include <boost/format.hpp>
#include <boost/url.hpp>
#include <boost/algorithm/string.hpp>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/node/convert.h>
#include <crawler/bus/amqp/amqp_config.hpp>
#include <crawler/db/db_config.hpp>

namespace crawler {

class Config {
public:
    static void load(const std::string& path);
    static std::string name() noexcept;
    static size_t max_resource_size() noexcept;

    template<typename T>
    static T get(const std::string& path) {
        return node_by_path(path).as<T>();
    }

    template<typename T>
    static T from_bus_message(const std::string& key, const std::string& path) {
        return get<T>(
            (boost::format{ "crawler.bus.messages.%1%.%2%" } % key % path).str()
        );
    }

    template<typename T>
    static T from_service(const std::string& name, const std::string& path) {
        return get<T>(
            (boost::format{ "crawler.services.%1%.%2%" } % name % path).str()
        );    
    }

    static size_t thread_pool(const std::string& path);

    static std::string connection_string(const std::string& path);
    static std::string logging_pattern(const std::string& key);

    static AMQPBusConfig bus_config();
    static DbConfig db_config();

private:
    static YAML::Node node_by_path(const std::string& path);

private:
    static inline std::string name_;
    static inline size_t max_resource_size_;
    static inline YAML::Node config_;
};
    
} // namespace crawler