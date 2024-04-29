#pragma once

#include <string>
#include <boost/url.hpp>

namespace crawler {

struct AMQPBusMessageConfig {
    bool enabled;
    std::string exchange;
    std::string routing_key;
};

struct AMQPBusConfig {
    boost::url url;
    std::unordered_map<std::string, AMQPBusMessageConfig> messages;
};

} // namespace crawler