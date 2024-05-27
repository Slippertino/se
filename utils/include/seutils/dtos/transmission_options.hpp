#pragma once

#include <string>

namespace se {

namespace utils {

struct TransmissionOptions {
    size_t max_batch_size;
    size_t max_batch_volume;
    std::string compression_type;

    TransmissionOptions() :
        max_batch_size{ 1 },
        max_batch_volume{ 10240 },
        compression_type{ "" }
    { }
};

} // namespace utils

} // namespace se