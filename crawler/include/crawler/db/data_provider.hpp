#pragma once

#include <vector>
#include <crawler/core/resource.hpp>
#include <crawler/caches/connections_pools/postgres_connections_pool.hpp>

namespace crawler {

class IDataProvider {
public:
    virtual std::vector<ResourceHeader> get_unhandled_headers_top(int limit) = 0;
    virtual std::vector<IndexingResourcePtr> get_unhandled_top_by_header(const ResourceHeader& header, int limit) = 0;
    virtual IndexingResourcePtr get_resource(const ResourceHeader& header, const std::string& uri) = 0;
    virtual void upload_resource(const IndexingResource& resource) = 0;
    virtual void finalize() = 0;
    virtual ~IDataProvider() { }
};

} // namespace crawler