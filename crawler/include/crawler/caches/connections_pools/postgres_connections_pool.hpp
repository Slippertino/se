#pragma once

#include <pqxx/pqxx>
#include <boost/url.hpp>
#include <crawler/utils/threaded_resource.hpp>

namespace crawler {

class PostgresConnectionsPool final : public utils::ThreadedResource<PostgresConnectionsPool, pqxx::connection> {
public:
    PostgresConnectionsPool () = delete;
    PostgresConnectionsPool (const boost::url& server) :
        server_url_{ server }
    { }

    template<std::same_as<pqxx::connection> R>
    pqxx::connection create_thread_resource() {
        auto conn = pqxx::connection{ server_url_.c_str() };
        conn.prepare(
            "get_unhandled_domains_top",                
            "SELECT * FROM get_unhandled_domains_top($1)"
        );
        conn.prepare(
            "get_unhandled_resources_top_by_domain",    
            "SELECT * FROM get_unhandled_resources_top_by_domain($1, $2, $3)"
        );
        conn.prepare(
            "get_resource",                             
            "SELECT * FROM get_resource($1, $2, $3)"
        );
        conn.prepare(
            "upload_resource",                          
            "CALL upload_resource($1, $2, $3, $4, $5)"
        );
        return conn;
    }

private:
    boost::url server_url_;
};

} // namespace crawler