#pragma once

#include <string>
#include <chrono>
#include <boost/url.hpp>
#include <seutils/config/config.hpp>
#include <logger/db/db_config.hpp>
#include <logger/config/logging_options.hpp>

namespace se {

namespace logger {

class Config : public se::utils::Config {
private:
    using se::utils::Config::config_;

public:
    Config() = default;
    Config(const YAML::Node&);
    explicit Config(const std::filesystem::path& path); 
    
    LoggingOptions options() const;
    DbConfig db_config() const;
};
    
} // namespace logger

} // namespace se