#pragma once

#include <string>

namespace se {

namespace utils {
    
struct LogData final {
    uint64_t timestamp;
    std::string component;
    std::string category;
    std::string lvl;
    std::string message;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & timestamp;
        ar & component;
        ar & category;
        ar & lvl;
        ar & message;
    }
};

} // namespace utils

} // namespace se