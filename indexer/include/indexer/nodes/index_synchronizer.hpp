#pragma once

#include <string>
#include <vector>
#include <tbb/flow_graph.h>
#include <indexer/db/data_provider.hpp>
#include <indexer/indexing_options.hpp>
#include <indexer/logging/logging.hpp>

namespace se {

namespace indexer {

class IndexSynchronizer {
public:
    IndexSynchronizer(const IndexingOptions& opts, std::shared_ptr<IDataProvider> db);

    tbb::flow::continue_msg operator()(tbb::flow::continue_msg);

private:
    std::shared_ptr<IDataProvider> db_;
};

} // namespace indexer

} // namespace se