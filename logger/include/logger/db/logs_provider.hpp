#pragma once

#include <seutils/models/log_data.hpp>
#include <seutils/connections_pools/postgres_connections_pool.hpp>
#include <seutils/logging/logging.hpp>
#include "conversions.hpp"
#include "db_config.hpp"

namespace se {

namespace logger {

class LogsProvider {
public:
    LogsProvider(const DbConfig& config);

    bool enabled();
    void load_logs_batch(const std::vector<se::utils::LogData>& logs);

private:
    auto get_connection() -> pqxx::connection&;
    bool test_connection(pqxx::connection& con);

private:
    DbConfig config_;
    se::utils::PostgresConnectionsPool connections_;
};

} // namespace logger

} // namespace se