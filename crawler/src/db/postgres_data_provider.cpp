#include <crawler/db/postgres_data_provider.hpp>

namespace se {

namespace crawler {

PostgresDataProvider::PostgresDataProvider(const DbConfig& config) : 
    config_ { config },
    connections_{ config.pool_size, config.connection_url }
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
    auto conn_holder = connections_.get_connection();
    auto& conn = conn_holder.connection();
    auto res = conn.is_open();
    return res;
}

std::vector<ResourceHeader> PostgresDataProvider::get_unhandled_headers_top(int limit) {
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        validate_connection(con);
        pqxx::read_transaction session{ con };
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
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        validate_connection(con);
        pqxx::read_transaction session{ con };
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
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        validate_connection(con);
        pqxx::read_transaction session{ con };
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
    try {        
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        validate_connection(con);
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
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        validate_connection(con);
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

void PostgresDataProvider::validate_connection(pqxx::connection& con) {
    auto res = con.is_open();
    if (!res)
        throw std::runtime_error("Database connection is not estabilished.");
}

} // namespace crawler

} // namespace se