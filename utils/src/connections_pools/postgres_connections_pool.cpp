#include <seutils/connections_pools/postgres_connections_pool.hpp>

namespace se {

namespace utils {

PostgresConnectionsPool::PostgresConnectionsPool(size_t size, const boost::url& server) :
    ConnectionsPool(size),
    server_url_{ server }
{ }

std::unique_ptr<pqxx::connection> PostgresConnectionsPool::create_connection() const {
    auto conn = std::make_unique<pqxx::connection>(server_url_.c_str());
    for(const auto& pq : prepared_queries_)
        conn->prepare(pq.first, pq.second);
    return conn;
}

PostgresConnectionsPool& PostgresConnectionsPool::add_prepared_query(const std::string& alias, const std::string& query) {
    prepared_queries_.push_back({ alias, query });
    return *this;
}

} // namespace utils

} // namespace se