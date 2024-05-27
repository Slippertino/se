#pragma once

#include <string>
#include <chrono>
#include <seutils/config/config.hpp>
#include <seutils/amqp/amqp_config.hpp>
#include <searcher/config/search_options.hpp>

namespace se {

namespace searcher {

class Config : public se::utils::Config {
private:
    using se::utils::Config::config_;

public:
    Config() = default;
    Config(const YAML::Node&);
    explicit Config(const std::filesystem::path& path); 
    SearchOptions options(const std::string& options_path = "") const;
};
    
} // namespace searcher

} // namespace se