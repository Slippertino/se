#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/error.hpp>
#include <crawler/caches/robots_cache.hpp>
#include <crawler/caches/memory_buffer_tags.hpp>
#include <seutils/threaded_memory_buffer.hpp>
#include "resource_handler.hpp"

namespace se {

namespace crawler {

class HttpResourceHandler : 
    public ResourceHandler, 
    public se::utils::ThreadedMemoryBuffer<resource_handling_tag> {

public:
    struct HttpResults {
       boost::beast::http::message<
            false, 
            boost::beast::http::string_body, 
            boost::beast::http::fields
        > headers;
        std::string body;
    };

public:
    HttpResourceHandler(private_token);

    virtual ~HttpResourceHandler();

protected:
    bool check_permissions(
        const ResourcePtr& resource,
        std::shared_ptr<ResourceProcessor>& processor
    ) const override final;

    void handle_no_permissions(
        const ResourcePtr& resource,
        std::shared_ptr<class ResourceProcessor>& processor
    ) override final;

    void handle_resource(
        ResourcePtr& resource, 
        std::shared_ptr<ResourceProcessor> processor, 
        ResourceLoader::ResourceLoadResults results
    ) override final;

    virtual void handle_failed_load(        
        ResourcePtr& resource, 
        std::shared_ptr<ResourceProcessor>& processor
    );

    virtual void handle_http_resource(
        ResourcePtr& resource, 
        std::shared_ptr<ResourceProcessor>& processor,
        HttpResults& results
    ) = 0;
};

} // namespace crawler

} // namespace se