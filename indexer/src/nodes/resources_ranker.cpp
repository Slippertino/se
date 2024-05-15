#include <indexer/nodes/resources_ranker.hpp>

namespace se {

namespace indexer {

ResourcesRanker::ResourcesRanker(const IndexingOptions& opts, std::shared_ptr<IDataProvider> db) :
    options_{ opts.ranking_options },
    db_{ db }
{ }

tbb::flow::continue_msg ResourcesRanker::operator()(tbb::flow::continue_msg) {     
    auto resources_count = db_->get_resources_count();
    LOG_INFO_WITH_TAGS(
        logging::resources_ranker_category, 
        "Ranking estimation of {} resources is starting...",
        resources_count
    );  
    db_->prepare_for_ranking();
    LOG_INFO_WITH_TAGS(
        logging::resources_ranker_category, 
        "Ranking estimation preparations were successfully ended. Starting..."
    );  
    auto tp_prob = options_.teleport_probability / static_cast<double>(resources_count);
    for(auto i = 0; i < options_.rank_estimation_iterations_count; ++i) {
        LOG_INFO_WITH_TAGS(
            logging::resources_ranker_category, 
            "Ranking estimation moved to {} iteration.",
            i + 1
        );  
        tbb::parallel_for(1ul, resources_count + 1, [this, tp_prob](auto id) {
            db_->update_resource_rank(id, tp_prob);
        });
        db_->commit_resources_ranks();
    }
    LOG_INFO_WITH_TAGS(
        logging::resources_ranker_category, 
        "Ranking estimation of {} resources had finished.",
        resources_count
    );  
    return tbb::flow::continue_msg{};
}

} // namespace indexer

} // namespace se