#include <indexer/nodes/secondary_indexer.hpp>

namespace se {

namespace indexer {

SecondaryIndexer::SecondaryIndexer(const IndexingOptions& opts, std::shared_ptr<IDataProvider> db) :
    options_{ opts.champ_lists_options },
    db_{ db }
{ }

tbb::flow::continue_msg SecondaryIndexer::operator()(tbb::flow::continue_msg) {     
    LOG_INFO_WITH_TAGS(
        logging::secondary_indexer_category, 
        "Lexem's entries ranks estimation is starting..."
    );     
    db_->estimate_lexems_entries_ranks();
    LOG_INFO_WITH_TAGS(
        logging::secondary_indexer_category, 
        "Lexem's entries ranks estimation has finished."
    );  
    auto lexems_count = db_->get_lexems_count();
    LOG_INFO_WITH_TAGS(
        logging::secondary_indexer_category, 
        "Starting creation of champion lists for {} lexems...",
        lexems_count
    );  
    tbb::parallel_for(1ul, lexems_count + 1, [&](auto id) {
        db_->create_lexem_champion_list(id, options_.size, options_.threshold);
    });
    LOG_INFO_WITH_TAGS(
        logging::secondary_indexer_category, 
        "Creation of champion lists for {} lexems has finished.",
        lexems_count
    );  
    return tbb::flow::continue_msg{};
}

} // namespace indexer

} // namespace se