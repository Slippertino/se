#pragma once

#include <userver/clients/dns/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/formats/json.hpp>
#include <userver/components/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/middlewares/http_middleware_base.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_response.hpp>

namespace se {

namespace searcher {

class CORSMiddleware final : public userver::server::middlewares::HttpMiddlewareBase {
public:
    static constexpr std::string_view kName{"cors-middleware"};

    explicit CORSMiddleware(const userver::server::handlers::HttpHandlerBase&);

private:
    void HandleRequest(
        userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context
    ) const override;

private:
    static constexpr userver::http::headers::PredefinedHeader 
        cors_allow_origin_{"Access-Control-Allow-Origin"},
        cors_allow_methods_{"Access-Control-Allow-Methods"},
        cors_allow_headers_{"Access-Control-Allow-Headers"};
};

 
} // namespace searcher

} // namespace se