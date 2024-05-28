#pragma once

#include <string>
#include <unordered_map>
#include <boost/url.hpp>
#include <seutils/dtos/transmission_options.hpp>
#include <seutils/bus/bus_config.hpp>

namespace se {

namespace utils {

struct AMQPBusMessageConfig {
    bool enabled;
    std::string exchange;
    std::string routing_key;
    TransmissionOptions transmission_options;
};

struct AMQPBusConfig {
    size_t pool_size;
    boost::url url;
    std::unordered_map<std::string, AMQPBusMessageConfig> messages;
    std::unordered_map<std::string, std::string> queues;

    BusConfig to_bus_config() const {
        BusConfig cfg;
        for(const auto& msg : messages)
            cfg.routes_options.insert({ msg.first, msg.second.transmission_options });
        return cfg;
    }
};

} // namespace utils

} // namespace se