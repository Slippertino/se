#pragma once

#include <string>
#include <functional>
#include <experimental/random>
#include <boost/asio.hpp>
#include <boost/url.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>
#include <crawler/caches/dns_cache.hpp>
#include "resource_loader.hpp"

namespace se {

namespace crawler {

class ResourceLoader {
public:
    struct ResourceLoadResults {
        bool success;
        std::string metadata;
        std::string content;
        std::string error_message; 
    };

    using content_handler_t = std::function<void(ResourceLoadResults)>;

public:
    ResourceLoader(boost::asio::io_context& context) : 
        context_{ context },
        resolver_{ context }
    { }

    void load(boost::url url, content_handler_t handler, bool with_resolving = true) {
        context_.post([url = std::move(url), this, handler = std::move(handler), with_resolving](){
            const std::string host{ url.encoded_host() };
            if (with_resolving && !DNSCache::instance().contains(host))
                post_loading_with_resolve(std::move(url), std::move(handler));
            else 
                post_loading(url, handler);
        });
    }

    virtual ~ResourceLoader() { }

protected:
    virtual void load_resolved(boost::url url, const content_handler_t& handler) = 0;

private:
    void post_loading_with_resolve(boost::url url, content_handler_t handler) {
        const boost::asio::ip::tcp::resolver::query q(std::string(url.encoded_host()), "");
        resolver_.async_resolve(q, [url = std::move(url), handler = std::move(handler), this](auto ec, const auto& results) {
            if (ec) return;
            DNSCache::instance().upload(std::string(url.encoded_host()), [&results]() {
                dns_result_t data;
                data.reserve(results.size());
                for(auto &cur : results) {
                    auto ep = cur.endpoint();
                    if (ep.address().is_v4())
                        data.push_back(ep.address().to_string());
                }
                return data;
            }); 
            post_loading(std::move(url), std::move(handler));
        });
    }

    inline void post_loading(boost::url url, content_handler_t handler) {
        context_.post(
            std::bind(&ResourceLoader::load_resolved, this, std::move(url), std::move(handler))
        );
    }

    static boost::asio::ssl::context init_ssl_context() {
        boost::asio::ssl::context ctx{boost::asio::ssl::context::tls_client};
        ctx.set_verify_mode(
            boost::asio::ssl::context::verify_none
        );
        ctx.set_default_verify_paths();
        boost::certify::enable_native_https_server_verification(ctx);
        return ctx;
    }

protected:
    static inline boost::asio::ssl::context ssl_context_{ init_ssl_context() };
    boost::asio::io_context& context_;

private:
    boost::asio::ip::tcp::resolver resolver_;
};

} // namespace icrawler

} // namespace se