#pragma once

#include <vector>
#include <string>
#include <pqxx/pqxx>
#include <boost/url.hpp>
#include "../threaded_resource.hpp"

namespace se {

namespace utils {

class PostgresConnectionsPool final : public ThreadedResource<PostgresConnectionsPool, pqxx::connection> {
public:
    PostgresConnectionsPool () = delete;
    PostgresConnectionsPool (const boost::url& server);

    template<std::same_as<pqxx::connection> R>
    pqxx::connection create_thread_resource() {
        auto conn = pqxx::connection{ server_url_.c_str() };
        for(const auto& pq : prepared_queries_)
            conn.prepare(pq.first, pq.second);
        return conn;
    }

    PostgresConnectionsPool& add_prepared_query(const std::string& alias, const std::string& query);

private:
    boost::url server_url_;
    std::vector<std::pair<std::string, std::string>> prepared_queries_;
};

} // namespace utils

} // namespace se