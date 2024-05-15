#pragma once

#include <boost/url.hpp>

namespace se {

namespace indexer {

struct DbConfig {
    boost::url connection_url;
    size_t pool_size;
};

} // namespace indexer

} // namespace se