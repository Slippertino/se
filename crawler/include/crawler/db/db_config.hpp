#pragma once

#include <boost/url.hpp>

namespace se {

namespace crawler {

struct DbConfig {
    boost::url connection_url;
};

} // namespace crawler

} // namespace se