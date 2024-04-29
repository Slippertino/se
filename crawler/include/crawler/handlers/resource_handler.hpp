#pragma once

#include <functional>
#include <memory>
#include <crawler/core/resource.hpp>
#include <crawler/loaders/loaders_factory.hpp>
#include <crawler/logging/logging.hpp>

namespace crawler {

class ResourceHandler : public std::enable_shared_from_this<ResourceHandler> {
protected:
    struct private_token{};

public:
    ResourceHandler(private_token);

    template<typename Handler>
    static std::shared_ptr<Handler> create() {
        return std::make_shared<Handler>(private_token{});
    }

    void confirm_success_handling();

    void handle(
        ResourcePtr resource, 
        std::shared_ptr<class ResourceProcessor> processor
    );

    virtual ~ResourceHandler();

protected:
    virtual bool check_permissions(
        const ResourcePtr& resource,
        std::shared_ptr<class ResourceProcessor>& processor
    ) const;

    virtual void handle_no_permissions(
        const ResourcePtr& resource,
        std::shared_ptr<class ResourceProcessor>& processor
    ) = 0;

    virtual void handle_resource(
        ResourcePtr& resource, 
        std::shared_ptr<class ResourceProcessor> processor, 
        ResourceLoader::ResourceLoadResults results
    ) = 0;

private:
    bool success_;
    std::weak_ptr<class ResourceProcessor> processor_;
    ResourceLoadersFactory::ResourceLoaderPtr loader_;
};

} // namespace crawler