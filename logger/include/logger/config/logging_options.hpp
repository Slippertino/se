#pragma once

#include <chrono>

namespace se {

namespace logger {

struct LoggingOptions {
    size_t memory_limit;
    double reduce_ratio;
    std::chrono::milliseconds periodicity; 
};

} // namespace logger

} // namespace se