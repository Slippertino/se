#include <seutils/connections_pools/postgres_connections_pool.hpp>

namespace se {

namespace utils {

PostgresConnectionsPool::PostgresConnectionsPool(const boost::url& server) :
    server_url_{ server }
{ }

PostgresConnectionsPool& PostgresConnectionsPool::add_prepared_query(const std::string& alias, const std::string& query) {
    prepared_queries_.push_back({ alias, query });
    return *this;
}

} // namespace utils

} // namespace se