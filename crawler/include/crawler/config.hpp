#pragma once

#include <string>
#include <chrono>
#include <boost/format.hpp>
#include <boost/url.hpp>
#include <seutils/config/config.hpp>
#include <seutils/amqp/amqp_config.hpp>
#include <crawler/db/db_config.hpp>

namespace se {

namespace crawler {

class Config : public se::utils::Config {
private:
    using se::utils::Config::config_;

public:
    Config() = default;
    Config(const YAML::Node&);
    explicit Config(const std::filesystem::path&); 

    std::string name() const noexcept;
    size_t max_resource_size() const noexcept;

    template<typename T>
    T from_bus_message(const std::string& key, const std::string& path) const {
        return get<T>(
            (boost::format{ "crawler.bus.messages.%1%.%2%" } % key % path).str()
        );
    }

    template<typename T>
    T from_service(const std::string& name, const std::string& path) const {
        return get<T>(
            (boost::format{ "crawler.services.%1%.%2%" } % name % path).str()
        );    
    }

    DbConfig db_config() const;

private:
    void init_cached_data();

private:
    std::string name_;
    size_t max_resource_size_;
};
    
} // namespace crawler

} // namespace se