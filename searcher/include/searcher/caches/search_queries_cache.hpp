#pragma once

#include <string>
#include <optional>
#include <vector>
#include <userver/utest/using_namespace_userver.hpp>
#include <userver/cache/lru_cache_component_base.hpp>
#include <userver/components/component_list.hpp>
#include <searcher/models/resource.hpp>

namespace se {

namespace searcher {

class SearchQueriesCache final 
    : public cache::LruCacheComponent<std::string, std::vector<ResourceInfo>> {
public:
    using KeyType = std::string;
    using ValueType = std::vector<ResourceInfo>;

    static constexpr std::string_view kName = "search-queries-cache";

    SearchQueriesCache(
        const components::ComponentConfig& config,
        const components::ComponentContext& context
    ) : ::cache::LruCacheComponent<KeyType, ValueType>(config, context)
    { }

private:
    ValueType DoGetByKey(
        [[maybe_unused]] const KeyType& key
    ) override {
        return {};
    }
};

} // namespace searcher

} // namespace se