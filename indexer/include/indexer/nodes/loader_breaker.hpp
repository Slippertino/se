#pragma once

#include <tbb/flow_graph.h>
#include <indexer/models/resource.hpp>
#include <indexer/indexing_options.hpp>
#include <indexer/db/data_provider.hpp>
#include <indexer/nodes/resources_loader.hpp>

namespace se {

namespace indexer {

class LoaderBreaker {
public:
    using output_ports_type = tbb::flow::multifunction_node<
        tbb::flow::continue_msg, 
        std::tuple<tbb::flow::continue_msg>
    >::output_ports_type;

public:
    LoaderBreaker(const IndexingOptions& opts, ResourcesLoader& loader, std::shared_ptr<IDataProvider> db);
    void operator()(tbb::flow::continue_msg, output_ports_type& ports);
    
private:
    const size_t resources_count_limit_;
    size_t current_count_;
    ResourcesLoader& loader_;
};

} // namespace indexer

} // namespace se