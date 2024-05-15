#pragma once

#include <vector>
#include <optional>
#include <indexer/models/resource.hpp>
#include <indexer/models/lexem.hpp>

namespace se {

namespace indexer {

class IDataProvider {
public:
    virtual bool enabled() = 0;
    virtual std::pair<std::optional<resource_id_t>, bool> upload_resource(const Resource& resource) = 0;
    virtual void upload_lexem_entries(resource_id_t id, const std::vector<LexemEntry>& entries) = 0;
    virtual size_t get_resources_count() = 0;
    virtual size_t get_lexems_count() = 0;
    virtual Resource get_resource(resource_id_t id) = 0;
    virtual size_t upload_outcoming_resources(resource_id_t id, const std::vector<std::string>& outcoming_urls) = 0;
    virtual void set_resources_probabilities(resource_id_t id, double probability) = 0;
    virtual void estimate_lexems_entries_ranks() = 0;
    virtual void create_lexem_champion_list(size_t lexem_id, size_t size, double threshold) = 0;
    virtual void prepare_for_ranking() = 0;
    virtual void update_resource_rank(resource_id_t id, double tp_prob) = 0;
    virtual void commit_resources_ranks() = 0;
    virtual void synchronize_with_production() = 0;

    virtual ~IDataProvider() { }
};

} // namespace indexer

} // namespace se