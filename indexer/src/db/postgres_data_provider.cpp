#include <indexer/db/postgres_data_provider.hpp>

namespace se {

namespace indexer {

PostgresDataProvider::PostgresDataProvider(const DbConfig& config) : 
    config_ { config },
    connections_{ config.pool_size, config.connection_url }
{ 
    connections_   
    .add_prepared_query(
        "upload_resource",                
        "SELECT * FROM upload_resource($1, $2, $3, $4, $5, $6, $7)"
    )
    .add_prepared_query(
        "upload_lexems_entries",
        "CALL upload_lexems_entries($1, $2)"
    )
    .add_prepared_query(
        "get_resources_count",
        "SELECT COUNT(*) FROM resources_sec"
    )
    .add_prepared_query(
        "get_lexems_count",
        "SELECT COUNT(*) FROM lexems_sec"
    )
    .add_prepared_query(
        "get_resource",
        "SELECT * FROM get_resource_sec($1)"
    )
    .add_prepared_query(
        "upload_outcoming_resources",
        "SELECT * FROM upload_outcoming_resources($1, $2)"
    )
    .add_prepared_query(
        "set_resources_probabilities",
        "CALL set_outcoming_resources_probability($1, $2)"
    )
    .add_prepared_query(
        "estimate_lexems_entries_ranks",
        "CALL estimate_word_ranks()"
    )
    .add_prepared_query(
        "create_lexem_champion_list",
        "CALL create_lexem_champion_list($1, $2, $3)"
    )
    .add_prepared_query(
        "prepare_for_ranking",
        "CALL prepare_for_ranking_estimation()"
    )
    .add_prepared_query(
        "update_resource_rank",
        "CALL update_resource_rank($1, $2)"
    )
    .add_prepared_query(
        "commit_resources_ranks",
        "CALL commit_resources_ranks()"
    )
    .add_prepared_query(
        "synchronize_with_production",
        "CALL synchronize_with_secondary()"
    );
}
                
bool PostgresDataProvider::enabled() {
    auto conn_holder = connections_.get_connection();
    auto& conn = conn_holder.connection();
    auto res = conn.is_open();
    return res;
}

std::pair<std::optional<resource_id_t>, bool> PostgresDataProvider::upload_resource(const Resource& resource) {
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        pqxx::work session{ con };
        auto method = std::string(resource.url.scheme());
        auto domain = std::string(resource.url.host());
        auto uri = std::string(resource.url.path().append(resource.url.query()));
        auto sz = resource.content.size();
        auto [res_id, upd] = session.exec_prepared1("upload_resource", 
            method,
            domain,
            uri,
            resource.title,
            resource.compression_type,
            pqxx::binary_cast(resource.content),
            sz
        ).as<resource_id_t, bool>();
        if (!upd)
            session.commit();
        return { res_id, upd };
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }
    return { std::nullopt, false };
}

void PostgresDataProvider::upload_lexem_entries(resource_id_t id, const std::vector<LexemEntry>& entries) {
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        pqxx::work session{ con };
        session.exec_prepared0("upload_lexems_entries", id, entries);
        session.commit();
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }
}

size_t PostgresDataProvider::get_resources_count() {
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        pqxx::read_transaction session{ con };
        auto [count] = session.exec_prepared1("get_resources_count").as<size_t>();
        return count;
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }
    return 0;
}

size_t PostgresDataProvider::get_lexems_count() {
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        pqxx::read_transaction session{ con };
        auto [count] = session.exec_prepared1("get_lexems_count").as<size_t>();
        return count;
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }
    return 0;
}

Resource PostgresDataProvider::get_resource(resource_id_t id) {
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        pqxx::read_transaction session{ con };
        auto result = session.exec_prepared1("get_resource", id);
        Resource res;
        res.url.set_scheme(result["method"].as<std::string>());
        res.url.set_host(result["domain"].as<std::string>());
        res.url.set_path(result["name"].as<std::string>());
        res.title = result["title"].as<std::string>();
        res.compression_type = result["compression_type"].as<std::string>();
        auto bytes = result["content"].as<pqxx::bytes>();
        res.content = std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        res.size = result["size"].as<size_t>();
        return res;
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }
    return Resource{};
}

size_t PostgresDataProvider::upload_outcoming_resources(resource_id_t id, const std::vector<std::string>& outcoming_urls) {
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        pqxx::work session{ con };
        auto [count] = session.exec_prepared1("upload_outcoming_resources", id, outcoming_urls).as<size_t>();
        if (count)
            session.commit();
        return count;
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }
    return 0;
}

void PostgresDataProvider::set_resources_probabilities(resource_id_t id, double probability) {
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        pqxx::work session{ con };
        session.exec_prepared0("set_resources_probabilities", id, probability);
        session.commit();
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }
}

void PostgresDataProvider::estimate_lexems_entries_ranks() {
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        pqxx::work session{ con };
        session.exec_prepared0("estimate_lexems_entries_ranks");
        session.commit();
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }
}

void PostgresDataProvider::create_lexem_champion_list(size_t lexem_id, size_t size, double threshold) {
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        pqxx::work session{ con };
        session.exec_prepared0("create_lexem_champion_list", lexem_id, size, threshold);
        session.commit();
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }
}

void PostgresDataProvider::prepare_for_ranking() {
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        pqxx::work session{ con };
        session.exec_prepared0("prepare_for_ranking");
        session.commit();
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }
}

void PostgresDataProvider::update_resource_rank(resource_id_t id, double tp_prob) {
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        pqxx::work session{ con };
        session.exec_prepared0("update_resource_rank", id, tp_prob);
        session.commit();
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }
}

void PostgresDataProvider::commit_resources_ranks() {
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        pqxx::work session{ con };
        session.exec_prepared0("commit_resources_ranks");
        session.commit();
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }
}

void PostgresDataProvider::synchronize_with_production() {
    try {
        auto conn_holder = connections_.get_connection();
        auto& con = conn_holder.connection();
        pqxx::transaction<
            pqxx::isolation_level::serializable, 
            pqxx::write_policy::read_write
        > session{ con };
        session.exec_prepared0("synchronize_with_production");
        session.commit();
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::db_category, 
            "Database error in {}: {}.",
            __PRETTY_FUNCTION__,
            ex.what()
        );
    }        
}

void PostgresDataProvider::validate_connection(pqxx::connection& con) {
    auto res = con.is_open();
    if (!res)
        throw std::runtime_error("Database connection is not estabilished.");
}

} // namespace indexer

} // namespace se