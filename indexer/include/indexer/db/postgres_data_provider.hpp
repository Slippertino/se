#pragma once

#include <optional>
#include <vector>
#include "data_provider.hpp"
#include "db_config.hpp"
#include "conversations.hpp"
#include <seutils/connections_pools/postgres_connections_pool.hpp>
#include <indexer/logging/logging.hpp>

namespace se {

namespace indexer {

class PostgresDataProvider : public IDataProvider {
public:
    PostgresDataProvider(const DbConfig& config);

    bool enabled() override;

    std::pair<std::optional<resource_id_t>, bool> upload_resource(const Resource& resource) override;
    void upload_lexem_entries(resource_id_t id, const std::vector<LexemEntry>& entries) override;
    size_t get_resources_count() override;
    size_t get_lexems_count() override;
    Resource get_resource(resource_id_t id) override;
    size_t upload_outcoming_resources(resource_id_t id, const std::vector<std::string>& outcoming_urls) override;
    void set_resources_probabilities(resource_id_t id, double probability) override;
    void estimate_lexems_entries_ranks() override;
    void create_lexem_champion_list(size_t lexem_id, size_t size, double threshold) override;
    void prepare_for_ranking() override;
    void update_resource_rank(resource_id_t id, double tp_prob) override;
    void commit_resources_ranks() override;
    void synchronize_with_production() override;

private:
    void validate_connection(pqxx::connection& con);

private:
    DbConfig config_;
    se::utils::PostgresConnectionsPool connections_;
};

} // namespace indexer

} // namespace se