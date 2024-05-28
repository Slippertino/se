#include <string>
#include <filesystem>
#include <indexer/indexer.hpp>

const std::filesystem::path default_cfg_path = std::filesystem::path(R"(indexer.yaml)");

int main(int argc, char** argv) {
    se::utils::GlobalConfig<se::indexer::Config>::load_global_config(
        argc > 1 ? std::filesystem::path(argv[1]) : default_cfg_path
    );
    se::indexer::Indexer indexer;
    indexer.setup();
    indexer.run();
	return 0;
} 