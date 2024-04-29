#pragma once

#include <boost/url.hpp>

namespace crawler {

struct DbConfig {
    boost::url connection_url;
};

} // namespace crawler