#include <searcher/logging/logger.hpp>

namespace se {

namespace searcher {

Logger::Logger(
    const components::ComponentConfig& config,
    const components::ComponentContext& context
) : 
    userver::components::LoggableComponentBase(config, context),
    config_{ extract_from_userver_config(config) }
{
    configure();
    LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Configured loggers.");
}

void Logger::notify_bus_configured(ExternalBus& bus) {
    bus_logging_ = std::make_shared<BusLoggingWrapper>(bus);
    auto bus_handler = quill::get_handler(bus_handler_name_);
    auto bus_handler_explicit = std::static_pointer_cast<BusHandlerType>(bus_handler); 
    bus_handler_explicit->set_target(bus_logging_);
}

userver::yaml_config::Schema Logger::GetStaticConfigSchema() {
    return userver::yaml_config::MergeSchemas<userver::components::LoggableComponentBase>(R"(
            type: object
            description: logger
            additionalProperties: false
            properties:
                component_name:
                    type: string
                    description: name
                lvl:
                    type: string
                    description: logging level
                log_formats:
                    type: object
                    description: log formats
                    additionalProperties: false
                    properties:
                        console:
                            type: string
                            description: console log format
                        bus:
                            type: string
                            description: bus log format
                time_formats:
                    type: object
                    description: time formats
                    additionalProperties: false
                    properties:
                        console:
                            type: string
                            description: console log time format
                        bus:
                            type: string
                            description: bus log time format
        )"
    );
}

void Logger::configure() {
    auto console_handler = quill::stdout_handler();
    console_handler->set_pattern(
        config_.logging_message_pattern("console"), 
        config_.logging_time_pattern("console"), 
        quill::Timezone::GmtTime
    );
    static_cast<quill::ConsoleHandler*>(console_handler.get())->enable_console_colours();

    auto bus_handler = quill::create_handler<BusHandlerType>(bus_handler_name_);
    bus_handler->set_pattern(
        config_.logging_message_pattern("bus"), 
        config_.logging_time_pattern("bus"), 
        quill::Timezone::GmtTime
    );

    quill::Config cfg;
    cfg.default_handlers.emplace_back(console_handler);
    cfg.default_handlers.emplace_back(bus_handler);
    cfg.backend_thread_sleep_duration = std::chrono::nanoseconds(static_cast<size_t>(1e4));
    cfg.backend_thread_empty_all_queues_before_exit = true;
    quill::configure(cfg);
    quill::start(); 

    quill::Logger* logger = quill::get_logger();
    logger->set_log_level(
        quill::loglevel_from_string(config_.get<std::string>("lvl"))
    );

    auto bus_handler_explicit = std::static_pointer_cast<BusHandlerType>(bus_handler); 
    bus_handler_explicit->set_component_name(config_.get<std::string>("component_name"));
}

} // namespace searcher

} // namespace se