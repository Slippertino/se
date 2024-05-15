#include <indexer/nodes/loader_breaker.hpp>

namespace se {

namespace indexer {

LoaderBreaker::LoaderBreaker(const IndexingOptions& opts, ResourcesLoader& loader, std::shared_ptr<IDataProvider> db) :
    resources_count_limit_{ opts.resources_capacity },
    current_count_{ db->get_resources_count() },
    loader_{ loader }
{ }

void LoaderBreaker::operator()(tbb::flow::continue_msg, output_ports_type& ports) {
    ++current_count_;
    if (current_count_ >= resources_count_limit_ && loader_.is_run())
        loader_.stop();
}

} // namespace indexer

} // namespace se