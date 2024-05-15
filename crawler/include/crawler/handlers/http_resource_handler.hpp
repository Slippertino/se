#pragma once

#include <unordered_map>
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

private:
    using headers_parser_t = boost::beast::http::response_parser<boost::beast::http::string_body>;
    using headers_t = typename headers_parser_t::value_type;
    using http_code_handler_t = std::function<void(HttpResourceHandler*, ResourceLoader::ResourceLoadResults&, headers_t&)>;

public:
    HttpResourceHandler(private_token);
    virtual ~HttpResourceHandler();

protected:
    bool check_permissions() const override final;
    void handle_no_permissions() override final;
    void handle_resource(ResourceLoader::ResourceLoadResults results) override final;

    virtual void handle_failed_load();;
    virtual void handle_http_resource(HttpResults& results) = 0;

private:
    void handle_ok(ResourceLoader::ResourceLoadResults& results, headers_t& headers);
    void handle_redirect(ResourceLoader::ResourceLoadResults& results, headers_t& headers);
    void handle_bad_request(ResourceLoader::ResourceLoadResults& results, headers_t& headers);
    void handle_requests_overflow(ResourceLoader::ResourceLoadResults& results, headers_t& headers);
    void handle_unknown_code(ResourceLoader::ResourceLoadResults& results, headers_t& headers);

private:
    static inline const std::unordered_map<boost::beast::http::status, http_code_handler_t> code_handlers_ = {
        { boost::beast::http::status::ok,                   &HttpResourceHandler::handle_ok                  },
        { boost::beast::http::status::moved_permanently,    &HttpResourceHandler::handle_redirect            },
        { boost::beast::http::status::found,                &HttpResourceHandler::handle_redirect            },
        { boost::beast::http::status::see_other,            &HttpResourceHandler::handle_redirect            },
        { boost::beast::http::status::temporary_redirect,   &HttpResourceHandler::handle_redirect            },
        { boost::beast::http::status::permanent_redirect,   &HttpResourceHandler::handle_redirect            },
        { boost::beast::http::status::bad_request,          &HttpResourceHandler::handle_bad_request         },
        { boost::beast::http::status::not_found,            &HttpResourceHandler::handle_bad_request         },
        { boost::beast::http::status::too_many_requests,    &HttpResourceHandler::handle_requests_overflow   },
    };
};

} // namespace crawler

} // namespace se