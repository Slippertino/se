#pragma once

#include <string>
#include <unordered_map>
#include <seutils/dtos/transmission_options.hpp>

namespace se {

namespace utils {

struct BusConfig {
    std::unordered_map<std::string, TransmissionOptions> routes_options;    
};

} // namespace utils

} // namespace se