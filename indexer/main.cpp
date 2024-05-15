#include <string>
#include <indexer/indexer.hpp>

const std::string default_cfg = "indexer.yaml";

int main(int argc, char** argv) {
    auto cfg = argc > 1 ? argv[1] : default_cfg;
    se::indexer::Indexer indexer;
    indexer.setup(cfg);
    indexer.run();
	return 0;
} 