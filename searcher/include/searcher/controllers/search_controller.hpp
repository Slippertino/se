#pragma once

#include <iostream>
#include <userver/clients/dns/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/formats/json.hpp>
#include <userver/components/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/yaml_config/merge_schemas.hpp>
#include <userver/utils/daemon_run.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/concurrent/mpsc_queue.hpp>
#include <searcher/caches/search_queries_cache.hpp>
#include <searcher/tools/query_parser.hpp>
#include <searcher/tools/relevance_estimator.hpp>
#include <searcher/models/mappings.hpp>
#include <searcher/bus/external_bus.hpp>
#include <searcher/config/config_extractor.hpp>

namespace se {

namespace searcher {
 
class SearchController final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "websearch";

    using HttpHandlerBase::HttpHandlerBase;

    SearchController(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    );

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& req,
        userver::server::request::RequestContext&
    ) const override;

    static userver::yaml_config::Schema GetStaticConfigSchema();
 
private:
    struct Query {
        size_t offset;
        size_t limit;
        std::string text;
    };

private:
    std::string HandleSuccess(Query& query, const std::vector<ResourceInfo>& results) const;

    static Query GetQuery(const userver::server::http::HttpRequest&);

    static std::string BuildQueryScheme(const std::vector<LexemEntryType>& entries);
    static userver::formats::json::Value BuildSuccessResponse(
        const std::vector<ResourceInfo>& resources,
        size_t offset,
        size_t limit
    );
    static userver::formats::json::Value BuildFailResponse(const std::string& error);

private:
    SearchOptions opts_;
    mutable SearchQueriesCache::CacheWrapper cache_;
    ExternalBus& bus_;
    userver::storages::postgres::ClusterPtr db_;
};
 
} // namespace searcher

} // namespace se