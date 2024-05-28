#include <filesystem>
#include <cds/init.h> 
#include <cds/gc/hp.h>
#include <crawler/crawler.hpp>

const std::filesystem::path default_cfg_path = std::filesystem::path(R"(crawler.yaml)");

int main(int argc, char** argv) {
    cds::Initialize();
    {
        cds::gc::HP hpGC;
        se::utils::GlobalConfig<se::crawler::Config>::load_global_config(
            argc > 1 ? std::filesystem::path(argv[1]) : default_cfg_path
        );
        se::crawler::Crawler crawler;
        crawler.setup();
        crawler.run();
    }
    cds::Terminate();
}