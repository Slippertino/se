#pragma once

#include <memory>
#include <userver/utest/using_namespace_userver.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/merge_schemas.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <seutils/logging/logging.hpp>
#include <seutils/logging/bus_handler.hpp>
#include <searcher/config/config.hpp>
#include <searcher/config/config_extractor.hpp>
#include <searcher/logging/bus_logging_wrapper.hpp>

namespace se {

namespace searcher {

class Logger final : public userver::components::LoggableComponentBase {
public:
    static constexpr std::string_view kName = "logger";

    Logger(
        const components::ComponentConfig& config,
        const components::ComponentContext& context
    );

    void notify_bus_configured(ExternalBus& bus);

    static userver::yaml_config::Schema GetStaticConfigSchema();

private:
    void configure();

private:
    using BusHandlerType = se::utils::logging::BusHandler<BusLoggingWrapper>;

private:
    static inline const std::string& bus_handler_name_ = "BusHandler";

private:
    Config config_;
    std::shared_ptr<BusLoggingWrapper> bus_logging_;
};

} // namespace searcher

} // namespace se