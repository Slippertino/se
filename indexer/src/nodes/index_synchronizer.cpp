#include <indexer/nodes/index_synchronizer.hpp>

namespace se {

namespace indexer {

IndexSynchronizer::IndexSynchronizer(const IndexingOptions& opts, std::shared_ptr<IDataProvider> db) :
    db_{ db }
{ }

tbb::flow::continue_msg IndexSynchronizer::operator()(tbb::flow::continue_msg) { 
    LOG_INFO_WITH_TAGS(
        logging::synchronizer_category, 
        "Starting synchronization with production version..."
    );  
    db_->synchronize_with_production();   
    LOG_INFO_WITH_TAGS(
        logging::synchronizer_category, 
        "Synchronization with production version has finished."
    );  
    return tbb::flow::continue_msg{};
}

} // namespace indexer

} // namespace se