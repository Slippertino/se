#include <searcher/bus/external_bus.hpp>
#include <searcher/logging/logger.hpp>
#include <iostream>

namespace se {

namespace searcher {

ExternalBus::ExternalBus(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
) : 
    userver::components::RabbitMQ{ config, context },  
    config_{ extract_from_userver_config(config).bus_config() },
    client_{ GetClient() },
    logs_buffer_{ 
        config_.messages.at(logging_key_), 
        client_
    },
    queries_buffer_{
        config_.messages.at(query_key_), 
        client_            
    }
{
    bts_.AsyncDetach("init", [this, &logger = context.FindComponent<Logger>("logger")]() {
        bts_.AsyncDetach(logging_key_, [this]() { logs_buffer_.run(); });
        bts_.AsyncDetach(query_key_, [this]() { queries_buffer_.run(); });
        logger.notify_bus_configured(*this);
        LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Configured bus.");
    });
}

void ExternalBus::send_log(se::utils::LogData log) {
    logs_buffer_.push(std::move(log));
}

void ExternalBus::send_query(se::utils::QueryData query) {
    queries_buffer_.push(std::move(query));
}

userver::yaml_config::Schema ExternalBus::GetStaticConfigSchema() {
    return userver::yaml_config::MergeSchemas<userver::components::RabbitMQ>(R"(
            type: object
            description: bus
            additionalProperties: false
            properties:
                messages:
                    type: array
                    description: sending routes
                    items:
                        type: object
                        description: sending route
                        additionalProperties: false
                        properties:
                            name:
                                type: string
                                description: no description
                            enabled:
                                type: boolean
                                description: no description
                            exchange:
                                type: string
                                description: no description
                            routing_key:
                                type: string
                                description: no description
                            max_batch_size:
                                type: integer
                                description: no description
                                defaultDescription: 1
                            max_batch_volume:
                                type: integer
                                description: no description
                                defaultDescription: 10240
                            compression_type:
                                type: string
                                description: no description
                                defaultDescription:
        )"
    );
}

} // namespace searcher

} // namespace se