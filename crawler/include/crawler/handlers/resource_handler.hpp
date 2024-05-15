#pragma once

#include <functional>
#include <memory>
#include <crawler/handlers/handling_status.hpp>
#include <crawler/core/resource.hpp>
#include <crawler/loaders/loaders_factory.hpp>
#include <crawler/logging/logging.hpp>

namespace se {

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

    void set_handling_status(HandlingStatus status);
    void confirm_success_handling();

    void handle(
        ResourcePtr resource, 
        std::shared_ptr<class ResourceProcessor> processor
    );

    virtual ~ResourceHandler();

protected:
    virtual bool check_permissions() const;
    virtual void handle_no_permissions() = 0;
    virtual void handle_resource(
        ResourceLoader::ResourceLoadResults results
    ) = 0;

protected:
    std::weak_ptr<class ResourceProcessor> processor_;
    ResourcePtr resource_;

private:
    HandlingStatus status_;
    ResourceLoadersFactory::ResourceLoaderPtr loader_;
};

} // namespace crawler

} // namespace se