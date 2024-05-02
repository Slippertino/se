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
    static void load(const std::string& path);
    static std::string name() noexcept;
    static size_t max_resource_size() noexcept;

    template<typename T>
    static T from_bus_message(const std::string& key, const std::string& path) {
        return get<T>(
            (boost::format{ "crawler.bus.messages.%1%.%2%" } % key % path).str()
        );
    }

    template<typename T>
    static T from_service(const std::string& name, const std::string& path) {
        return get<T>(
            (boost::format{ "crawler.services.%1%.%2%" } % name % path).str()
        );    
    }

    static std::string logging_message_pattern(const std::string& key);
    static std::string logging_time_pattern(const std::string& key);
    static DbConfig db_config();

private:
    static inline std::string name_;
    static inline size_t max_resource_size_;
};
    
} // namespace crawler

} // namespace se