#include <logger/db/logs_provider.hpp>

namespace se {

namespace logger {

LogsProvider::LogsProvider(const DbConfig& config) : 
    config_ { config },
    connections_{ config.connection_url }
{ 
    connections_.add_prepared_query(
        "add_logs",    
        "CALL add_logs($1)"            
    );
}

bool LogsProvider::enabled() {
    auto conn = connections_.create_thread_resource<pqxx::connection>();
    auto res = conn.is_open();
    if (conn.is_open())
        conn.close();
    return res;
}

void LogsProvider::load_logs_batch(const std::vector<se::utils::LogData>& logs) {
    auto& con = get_connection();
    try {
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

auto LogsProvider::get_connection() -> pqxx::connection& {
    auto& con = connections_.get_thread_resource();
    test_connection(con);
    return con;
}

bool LogsProvider::test_connection(pqxx::connection& con) {
    auto res = con.is_open();
    if (!res)
        LOG_ERROR_WITH_TAGS(se::utils::logging::db_category, "Database connection is not estabilished.");
    return res;
}

} // namespace logger

} // namespace se