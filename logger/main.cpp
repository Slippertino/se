#include <filesystem>
#include <cds/init.h>
#include <logger/config/config.hpp>
#include <logger/logs_collector.hpp>

const std::filesystem::path default_cfg_path = std::filesystem::path(R"(logger.yaml)");

int main(int argc, char** argv) {
    cds::Initialize();
    {
        se::utils::GlobalConfig<se::logger::Config>::load_global_config(
            argc > 1 ? std::filesystem::path(argv[1]) : default_cfg_path
        );
        se::logger::LogsCollector collector;
        collector.setup();
        collector.run();
    }
    cds::Terminate();
}