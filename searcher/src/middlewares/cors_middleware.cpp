#include <searcher/middlewares/cors_middleware.hpp>

namespace se {

namespace searcher {

CORSMiddleware::CORSMiddleware(const userver::server::handlers::HttpHandlerBase&) 
{ }

void CORSMiddleware::HandleRequest(
    userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context
) const {
    Next(request, context);
    auto& resp = request.GetHttpResponse();
    resp.SetHeader(cors_allow_origin_, "*");
    resp.SetHeader(cors_allow_methods_, "GET");
    resp.SetHeader(cors_allow_headers_, "*");        
}
 
} // namespace searcher

} // namespace se