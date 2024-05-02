#include <crawler/db/postgres_data_provider.hpp>

namespace se {

namespace crawler {

PostgresDataProvider::PostgresDataProvider(const DbConfig& config) : 
    config_ { config },
    connections_{ config.connection_url }
{ 
    connections_
    .add_prepared_query(
        "get_unhandled_domains_top",                
        "SELECT * FROM get_unhandled_domains_top($1)"
    )
    .add_prepared_query(
        "get_unhandled_resources_top_by_domain",    
        "SELECT * FROM get_unhandled_resources_top_by_domain($1, $2, $3)"
    )
    .add_prepared_query(
        "get_resource",                             
        "SELECT * FROM get_resource($1, $2, $3)"
    )
    .add_prepared_query(
        "upload_resource",                          
        "CALL upload_resource($1, $2, $3, $4, $5)"
    );
}

bool PostgresDataProvider::enabled() {
    auto conn = connections_.create_thread_resource<pqxx::connection>();
    auto res = conn.is_open();
    if (conn.is_open())
        conn.close();
    return res;
}

std::vector<ResourceHeader> PostgresDataProvider::get_unhandled_headers_top(int limit) {
    auto& con = get_connection();
    try {
        pqxx::work session{ con };
        auto result = session.exec_prepared("get_unhandled_domains_top", limit);
        std::vector<ResourceHeader> out;
        out.reserve(result.size());
        for(const auto& row : result) {
            const auto&[method, domain, count] = row.as<std::string, std::string, uint64_t>();
            boost::url url;
            url.set_scheme(method);
            url.set_host(domain);
            out.push_back(ResourceHeader::create_from_url(std::move(url)));
        }
        return out;
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }
    return {};
}

std::vector<IndexingResourcePtr> PostgresDataProvider::get_unhandled_top_by_header(const ResourceHeader& header, int limit) {
    auto& con = get_connection();
    try {
        pqxx::work session{ con };
        auto result = session.exec_prepared("get_unhandled_resources_top_by_domain", *header.type, *header.domain, limit);
        std::vector<IndexingResourcePtr> out;
        out.reserve(result.size());
        auto url = header.url();
        for(const auto& row : result) {
            const auto&[uri, priority, checksum] = row.as<std::string, double, std::string>();
            auto res = Resource::create_from_url<Page, IndexingResource>(url, uri, priority, checksum);
            out.push_back(std::move(res));
        }
        return out;    
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
           se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }
    return {};
}

IndexingResourcePtr PostgresDataProvider::get_resource(const ResourceHeader& header, const std::string& uri) {
    auto& con = get_connection();
    try {
        pqxx::work session{ con };
        auto result = session.exec_prepared("get_resource", *header.type, *header.domain, uri);  
        if (result.empty())
            return nullptr;
        auto data = result.front();
        return Resource::create_from_url<Page, IndexingResource>(
            header.url(), 
            uri,
            data[3].as<double>(), 
            data[4].as<std::string>(),
            data[5].as<bool>()
        );
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category,  
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }
    return nullptr;
}

void PostgresDataProvider::upload_resource(const IndexingResource& resource) {
    auto& con = get_connection();
    try {
        pqxx::work session{ con };
        session.exec_prepared("upload_resource", 
            *resource.header.type, 
            *resource.header.domain, 
            resource.uri, 
            resource.priority,
            resource.checksum
        );
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

void PostgresDataProvider::finalize() {
    auto& con = get_connection();
    try {
        pqxx::work session{ con };
        session.exec("CALL reset()");
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

auto PostgresDataProvider::get_connection() -> pqxx::connection& {
    auto& con = connections_.get_thread_resource();
    test_connection(con);
    return con;
}

bool PostgresDataProvider::test_connection(pqxx::connection& con) {
    auto res = con.is_open();
    if (!res)
        LOG_ERROR_WITH_TAGS(se::utils::logging::db_category, "Database connection is not estabilished.");
    return res;
}

} // namespace crawler

} // namespace se