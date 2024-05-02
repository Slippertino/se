#include <cds/init.h>
#include <logger/config.hpp>
#include <logger/logs_collector.hpp>

const char* default_cfg_name = "logger.yaml";

int main(int argc, char** argv) {
    cds::Initialize();
    {
        se::logger::Config::load(argc > 1 ? std::string(argv[1]) : default_cfg_name);
        se::logger::LogsCollector collector;
        collector.setup();
        collector.run();
    }
    cds::Terminate();
}