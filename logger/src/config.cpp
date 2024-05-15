#include <logger/config.hpp>

namespace se {

namespace logger {

void Config::load(const std::string& path) {
    se::utils::Config::load_impl(path);
}

DbConfig Config::db_config() {
    return DbConfig{
        boost::url{ connection_string("logger.db") },
        get<size_t>("logger.db.pool_size")
    };
}
    
} // namespace logger

} // namespace se