#include <searcher/config/config.hpp>
#include <searcher/logging/logger.hpp>
#include <userver/storages/secdist/component.hpp>
#include <userver/storages/secdist/provider_component.hpp>
#include <searcher/controllers/search_controller.hpp>

int main(int argc, char* argv[]) {
    const auto component_list =
        userver::components::MinimalServerComponentList()
        .Append<se::searcher::SearchController>()
        .Append<se::searcher::SearchQueriesCache>()
        .Append<se::searcher::ExternalBus>()
        .Append<se::searcher::Logger>()
        .Append<userver::components::Postgres>("index-database")
        .Append<userver::components::TestsuiteSupport>()
        .Append<userver::components::Secdist>()
        .Append<userver::components::DefaultSecdistProvider>()
        .Append<userver::clients::dns::Component>();
    return userver::utils::DaemonMain(argc, argv, component_list);
}