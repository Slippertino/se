#pragma once

#include <string>

namespace se {

namespace utils {
    
struct QueryData final {
    std::string query;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar & query;
    }
};

} // namespace utils

} // namespace se