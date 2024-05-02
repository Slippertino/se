#include <cds/init.h> 
#include <cds/gc/hp.h>
#include <crawler/crawler.hpp>

const char* default_cfg_name = "crawler.yaml";

int main(int argc, char** argv) {
    cds::Initialize();
    {
        cds::gc::HP hpGC;
        se::crawler::Crawler crawler;
        crawler.setup(argc > 1 ? argv[1] : default_cfg_name);
        crawler.run();
    }
    cds::Terminate();
}