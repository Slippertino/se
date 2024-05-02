#pragma once

#include <vector>
#include <crawler/core/resource.hpp>

namespace se {

namespace crawler {

class IDataProvider {
public:
    virtual bool enabled() = 0;
    virtual std::vector<ResourceHeader> get_unhandled_headers_top(int limit) = 0;
    virtual std::vector<IndexingResourcePtr> get_unhandled_top_by_header(const ResourceHeader& header, int limit) = 0;
    virtual IndexingResourcePtr get_resource(const ResourceHeader& header, const std::string& uri) = 0;
    virtual void upload_resource(const IndexingResource& resource) = 0;
    virtual void finalize() = 0;
    virtual ~IDataProvider() { }
};

} // namespace crawler

} // namespace se