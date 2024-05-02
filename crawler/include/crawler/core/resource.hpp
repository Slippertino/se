#pragma once

#include <memory>
#include <iostream>
#include <optional>
#include <boost/url/scheme.hpp>
#include <boost/url.hpp>
#include <crawler/types.hpp>
#include <crawler/caches/string_caches.hpp>

namespace se {

namespace crawler {

using ResourcePtr = std::unique_ptr<struct Resource>;
using IndexingResourcePtr = std::unique_ptr<struct IndexingResource>;

struct ResourceHeader {
    shared_string type;
    shared_string domain;

    static ResourceHeader create_from_url(const boost::url& domain_url) {
        auto& schemes_cache = SchemeNamesCache::instance();
        auto& domains_cache = DomainNamesCache::instance();
        auto stype   = std::string(domain_url.scheme());
        auto sdomain = std::string(domain_url.host());
        if (!schemes_cache.contains(stype))
            schemes_cache.upload(stype, std::make_shared<std::string>(stype));
        if (!domains_cache.contains(sdomain))
            domains_cache.upload(sdomain, std::make_shared<std::string>(sdomain));
        return ResourceHeader{
            schemes_cache[stype],
            domains_cache[sdomain]
        };
    }

    boost::url url() const {
        boost::url res;
        res.set_scheme(*type);
        res.set_host_name(*domain);
        return res;
    }
};

struct Resource {
    ResourceHeader header;
    std::string uri;
    bool is_indexing;
    double priority;

    Resource(ResourceHeader header, const std::string &uri, bool is_indexing, double priority = 0.5) : 
        header{ header },
        uri { uri },
        is_indexing { is_indexing },
        priority { priority }
    { }

    template<typename Target, typename ReturnTarget = Target, typename... Args>
    static std::unique_ptr<ReturnTarget> create_from_url(const boost::url& domain_url, Args&&... args) {
        auto obj = new Target(
            ResourceHeader::create_from_url(domain_url), 
            std::forward<Args>(args)...
        );
        auto target_obj = static_cast<ReturnTarget*>(obj);
        return std::unique_ptr<ReturnTarget>(target_obj);
    }

    boost::url url() const {
        boost::url res;
        res.set_scheme(*header.type);
        res.set_host_name(*header.domain);
        res.set_path(uri);
        return res;
    }

    virtual ResourcePtr clone() const = 0;
    virtual std::shared_ptr<class ResourceHandler> get_handler() const = 0;
    virtual ~Resource() { }
};

struct RobotsTxt : Resource {
    RobotsTxt(ResourceHeader header) : Resource(header, "/robots.txt", false, 1.0)
    { }

    ResourcePtr clone() const override;
    std::shared_ptr<class ResourceHandler> get_handler() const override;
};

struct Sitemap : Resource {
    Sitemap(ResourceHeader header, const std::string& uri) : Resource(header, uri, false, 0.5)
    { }

    ResourcePtr clone() const override;
    std::shared_ptr<class ResourceHandler> get_handler() const override;
};

struct IndexingResource : public Resource {
    std::string checksum;
    bool fin;

    IndexingResource(ResourceHeader header, const std::string &uri, double priority, const std::string& checksum, bool fin) : 
        Resource{ header, uri, true, priority },
        checksum{ checksum },
        fin{ fin }
    { }

    virtual ResourcePtr clone() const = 0;
    virtual std::shared_ptr<class ResourceHandler> get_handler() const = 0;  
    virtual ~IndexingResource() { }
};

struct Page : IndexingResource {
    Page(
        ResourceHeader header,
        const std::string& uri = "/", 
        double priority = 0.5,
        const std::string& checksum = "",
        bool fin = false
    ) : IndexingResource(header, uri, priority, checksum, fin)
    { }

    ResourcePtr clone() const override;
    std::shared_ptr<class ResourceHandler> get_handler() const override;
};

} // namespace crawler

} // namespace se