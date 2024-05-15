#pragma once

#include <boost/url.hpp>

namespace se {

namespace crawler {

struct DbConfig {
    boost::url connection_url;
    size_t pool_size;
};

} // namespace crawler

} // namespace se