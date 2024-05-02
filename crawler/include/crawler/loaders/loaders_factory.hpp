#pragma once

#include <memory>
#include <unordered_map>
#include "resource_loader.hpp"
#include "http_loader.hpp"

namespace se {

namespace crawler {

class ResourceLoadersFactory final {
public:
    using ResourceLoaderPtr = std::unique_ptr<ResourceLoader>;

private:
    using ResourceLoaderCreator = std::function<ResourceLoaderPtr(boost::asio::io_context&)>;

public:
    static ResourceLoaderPtr get_loader(const std::string& method, boost::asio::io_context& ioc) {
        if (!loaders_.contains(method))
            return nullptr;
        return loaders_.at(method)(ioc);
    }

private:
    static inline const std::unordered_map<std::string, ResourceLoaderCreator> loaders_ = {
        { "http",  [](auto& ioc) { return std::make_unique<HttpLoader>(ioc);        } },
        { "https", [](auto& ioc) { return std::make_unique<HttpLoader>(ioc, true);  } },
    };
};

} // namespace crawler

} // namespace se