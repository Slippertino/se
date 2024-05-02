#pragma once

#include <boost/url.hpp>

namespace se {

namespace logger {

struct DbConfig {
    boost::url connection_url;
};

} // namespace logger

} // namespace se