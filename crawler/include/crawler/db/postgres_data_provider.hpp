#pragma once

#include "data_provider.hpp"
#include "db_config.hpp"
#include <seutils/connections_pools/postgres_connections_pool.hpp>
#include <crawler/logging/logging.hpp>

namespace se {

namespace crawler {

class PostgresDataProvider : public IDataProvider {
public:
    PostgresDataProvider(const DbConfig& config);

    bool enabled() override;
    std::vector<ResourceHeader> get_unhandled_headers_top(int limit) override;
    std::vector<IndexingResourcePtr> get_unhandled_top_by_header(const ResourceHeader& header, int limit) override;
    IndexingResourcePtr get_resource(const ResourceHeader& header, const std::string& uri) override;
    void upload_resource(const IndexingResource& resource) override;
    void finalize() override;

private:
    auto get_connection() -> pqxx::connection&;
    bool test_connection(pqxx::connection& con);

private:
    DbConfig config_;
    se::utils::PostgresConnectionsPool connections_;
};

} // namespace crawler

} // namespace se