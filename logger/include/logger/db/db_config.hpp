#pragma once

#include <boost/url.hpp>

namespace se {

namespace logger {

struct DbConfig {
    boost::url connection_url;
    size_t pool_size;
};

} // namespace logger

} // namespace se