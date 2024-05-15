#include <logger/db/logs_provider.hpp>

namespace se {

namespace logger {

LogsProvider::LogsProvider(const DbConfig& config) : 
    config_ { config },
    connections_{ config.pool_size, config.connection_url }
{ 
    connections_.add_prepared_query(
        "add_logs",    
        "CALL add_logs($1)"            
    );
}

bool LogsProvider::enabled() {
    auto conn_holder = connections_.get_connection();
    auto& conn = conn_holder.connection();
    auto res = conn.is_open();
    return res;
}

void LogsProvider::load_logs_batch(const std::vector<se::utils::LogData>& logs) {
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        pqxx::work session{ con };
        session.exec_prepared("add_logs", logs);
        session.commit();
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }    
}

void LogsProvider::validate_connection(pqxx::connection& con) {
    auto res = con.is_open();
    if (!res)
        throw std::runtime_error("Database connection is not estabilished.");
}

} // namespace logger

} // namespace se