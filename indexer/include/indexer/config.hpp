#pragma once

#include <string>
#include <chrono>
#include <boost/format.hpp>
#include <boost/url.hpp>
#include <seutils/config/config.hpp>
#include <seutils/amqp/amqp_config.hpp>
#include <indexer/db/db_config.hpp>
#include <indexer/indexing_options.hpp>

namespace se {

namespace indexer {

class Config : public se::utils::Config {
private:
    using se::utils::Config::config_;

public:
    static void load(const std::string& path);
    static std::string name() noexcept;

    template<typename T>
    static T from_bus_message(const std::string& key, const std::string& path) {
        return get<T>(
            (boost::format{ "indexer.bus.messages.%1%.%2%" } % key % path).str()
        );
    }
    
    static std::string logging_message_pattern(const std::string& key);
    static std::string logging_time_pattern(const std::string& key);

    static IndexingOptions options();
    static DbConfig db_config();
};
    
} // namespace indexer

} // namespace se