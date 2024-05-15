#pragma once

#include <vector>
#include <string>
#include <pqxx/pqxx>
#include <boost/url.hpp>
#include "connections_pool.hpp"

namespace se {

namespace utils {

class PostgresConnectionsPool final : public ConnectionsPool<PostgresConnectionsPool, pqxx::connection> {
public:
    PostgresConnectionsPool () = delete;
    PostgresConnectionsPool(size_t size, const boost::url& server);

    std::unique_ptr<pqxx::connection> create_connection() const;
    PostgresConnectionsPool& add_prepared_query(const std::string& alias, const std::string& query);

private:
    boost::url server_url_;
    std::vector<std::pair<std::string, std::string>> prepared_queries_;
};

} // namespace utils

} // namespace se