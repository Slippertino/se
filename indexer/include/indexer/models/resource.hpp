#pragma once

#include <string>
#include <boost/url.hpp>

namespace se {

namespace indexer {

using resource_id_t = uint64_t;

struct Resource {
    boost::url url;
    std::string title;
    std::string compression_type;
    std::string content;
    size_t size;
};

} // namespace indexer

} // namespace se