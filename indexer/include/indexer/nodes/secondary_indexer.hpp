#pragma once

#include <string>
#include <vector>
#include <tbb/flow_graph.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <indexer/db/data_provider.hpp>
#include <indexer/indexing_options.hpp>
#include <indexer/logging/logging.hpp>

namespace se {

namespace indexer {

class SecondaryIndexer {
public:
    SecondaryIndexer(const IndexingOptions& opts, std::shared_ptr<IDataProvider> db);
    tbb::flow::continue_msg operator()(tbb::flow::continue_msg);

private:
    ChampionListsOptions options_;
    std::shared_ptr<IDataProvider> db_;
};

} // namespace indexer

} // namespace se