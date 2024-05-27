#include <searcher/controllers/search_controller.hpp>
#include <searcher/logging/logger.hpp>
#include <iostream>

namespace se {

namespace searcher {
 
SearchController::SearchController(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
) : HttpHandlerBase(config, context),
    opts_{ extract_from_userver_config(config).options("options") },
    cache_(context.FindComponent<SearchQueriesCache>("search-queries-cache").GetCache()),
    bus_(context.FindComponent<ExternalBus>("bus")),
    db_(context.FindComponent<userver::components::Postgres>("index-database").GetCluster())
{ 
    [[maybe_unused]] const auto& logger = context.FindComponent<Logger>("logger");
    LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Configured search controller.");
}

std::string SearchController::HandleRequestThrow(
    const userver::server::http::HttpRequest& req,
    userver::server::request::RequestContext&
) const {
    try {
        auto query = GetQuery(req);
        bus_.send_query(se::utils::QueryData{ query.text });
        LOG_INFO_WITH_TAGS(
            se::utils::logging::main_category, 
            "New query: \"{}\" with offset={} & limit={} ",
            query.text,            
            query.offset,
            query.limit
        );
        auto lexems_entries = QueryParser{}(query.text, opts_);
        std::sort(lexems_entries.begin(), lexems_entries.end(), [](auto& lhs, auto& rhs) {
            return lhs.name < rhs.name;
        });
        if (lexems_entries.empty()) {
            LOG_WARNING_WITH_TAGS(
                se::utils::logging::main_category, 
                "Emtpy query was received."
            );
            return userver::formats::json::ToPrettyString(
                BuildFailResponse("empty query")
            );
        }
        auto query_scheme = BuildQueryScheme(lexems_entries);
        auto cached = cache_.GetOptional(query_scheme);
        if (cached.has_value()) {
            LOG_TRACE_L1_WITH_TAGS(
                se::utils::logging::main_category, 
                "Cache hit for \"{}\".", 
                query.text
            );
            return HandleSuccess(query, cached.value());
        }
        auto trx = db_->Begin(userver::storages::postgres::TransactionOptions(
            userver::storages::postgres::IsolationLevel::kReadUncommitted,
            userver::storages::postgres::TransactionOptions::Mode::kReadOnly
        ));
        auto lexems = trx
            .Execute("SELECT * FROM get_existing_lexems($1)", lexems_entries)
            .AsContainer<std::vector<RegisteredLexem>>(userver::storages::postgres::RowTag{});
        std::vector<decltype(RegisteredLexem::id)> lexems_id(lexems.size());
        std::transform(lexems.begin(), lexems.end(), lexems_id.begin(), [](auto& v) {
            return v.id;
        });
        auto resources_queue = userver::concurrent::MpscQueue<RankedResource>::Create();
        auto producer = resources_queue->GetMultiProducer();
        auto consumer = resources_queue->GetConsumer();
        resources_queue->SetSoftMaxSize(userver::concurrent::MpscQueue<RankedResource>::kUnbounded);
        for(auto [res_id] : trx
            .Execute("SELECT * FROM UNNEST(get_champ_docs($1))", lexems_id)
            .AsSetOf<std::tuple<decltype(RankedResource::id)>>(userver::storages::postgres::RowTag{})
        ) {
            auto res_rank = trx
                .Execute("SELECT * FROM estimate_resource_rank($1, $2)", res_id, lexems)
                .AsSingleRow<ResourceRank>(userver::storages::postgres::RowTag{});
            auto relevance = RelevanceEstimator{}(res_rank, opts_);
            if (relevance >= opts_.resource_rank_threshold)
                [[maybe_unused]] auto res = producer.Push(
                    RankedResource{ res_id, relevance }, 
                    userver::engine::Deadline{}
                );
        }
        std::vector<RankedResource> resources;
        resources.reserve(resources_queue->GetSizeApproximate());
        RankedResource cur;
        while(consumer.PopNoblock(cur))
            resources.push_back(std::move(cur));
        auto resources_info = trx
            .Execute("SELECT * FROM UNNEST(get_resources_info($1))", resources)
            .AsContainer<std::vector<ResourceInfo>>(userver::storages::postgres::RowTag{});
        trx.Commit();
        cache_.GetCache()->Put(query_scheme, resources_info);
        return HandleSuccess(query, resources_info);
    }
    catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(se::utils::logging::main_category, "Error in search controller: {}.", ex.what());
        return userver::formats::json::ToPrettyString(
            BuildFailResponse(ex.what())
        );
    }
}

userver::yaml_config::Schema SearchController::GetStaticConfigSchema() {
    return userver::yaml_config::MergeSchemas<userver::server::handlers::HttpHandlerBase>(R"(
            type: object
            description: searcher
            additionalProperties: false
            properties:
                options:
                    type: object
                    description: search options
                    additionalProperties: false
                    properties:
                        language_threshold:
                            type: number
                            description: no description
                        max_query_languages_count:
                            type: integer
                            description: no description
                        resource_rank_threshold:
                            type: number
                            description: no description
                        resource_static_rank_component_weight:
                            type: number
                            description: no description
                        resource_dynamic_rank_component_weight:
                            type: number
                            description: no description
                        default_language:
                            type: string
                            description: no description
        )"
    );
}

std::string SearchController::HandleSuccess(SearchController::Query& query, const std::vector<ResourceInfo>& results) const {
    auto& offset = query.offset;
    auto& limit = query.limit;
    auto sz = results.size();
    offset = std::min(offset, sz);
    limit = std::min(limit, sz);
    auto selection_size = std::min(sz - offset, limit);
    LOG_INFO_WITH_TAGS(
        se::utils::logging::main_category, 
        "Sending {} results from {} total for query \"{}\" in range [{}, {}].",
        selection_size,
        sz,
        query.text,
        offset,
        offset + selection_size
    );
    return userver::formats::json::ToPrettyString(
        BuildSuccessResponse(results, offset, selection_size)
    );
}

SearchController::Query SearchController::GetQuery(const userver::server::http::HttpRequest& req) {
    return Query{
        req.HasArg("offset") ? std::stoul(req.GetArg("offset")) : 0,
        req.HasArg("limit") ? std::stoul(req.GetArg("limit")) : 0,
        req.GetArg("query")
    };
}

std::string SearchController::BuildQueryScheme(const std::vector<LexemEntryType>& entries) {
    constexpr char lang_delim{':'}, lexem_delim{'|'};
    std::ostringstream res;
    for(const auto& cur : entries)
        res << cur.name << lang_delim << cur.lang << lexem_delim;
    return res.str();
}

userver::formats::json::Value SearchController::BuildSuccessResponse(
    const std::vector<ResourceInfo>& resources,
    size_t offset,
    size_t limit
) {
    userver::formats::json::ValueBuilder builder;
    builder["status"] = 0;
    builder["total_size"] = resources.size();
    builder["size"] = limit;
    for(auto i = offset; i < offset + limit; ++i)
        builder["results"].PushBack(resources[i]);
    return builder.ExtractValue();
}

userver::formats::json::Value SearchController::BuildFailResponse(const std::string& error) {
    userver::formats::json::ValueBuilder builder;
    builder["status"] = 1;
    builder["message"] = error;
    return builder.ExtractValue();
}

} // namespace searcher

} // namespace se