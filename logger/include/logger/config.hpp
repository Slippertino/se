#pragma once

#include <string>
#include <chrono>
#include <boost/url.hpp>
#include <seutils/config/config.hpp>
#include <seutils/amqp/amqp_config.hpp>
#include <logger/db/db_config.hpp>

namespace se {

namespace logger {

class Config : public se::utils::Config {
private:
    using se::utils::Config::config_;

public:
    static void load(const std::string& path);
    static DbConfig db_config();
};
    
} // namespace logger

} // namespace se