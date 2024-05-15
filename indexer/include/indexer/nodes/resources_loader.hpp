#pragma once

#include <atomic>
#include <ratio>
#include <shared_mutex>
#include <condition_variable>
#include <tbb/flow_graph.h>
#include <indexer/bus/external_bus.hpp>
#include <indexer/models/resource.hpp>
#include <indexer/indexing_options.hpp>
#include <indexer/logging/logging.hpp>

namespace se {

namespace indexer {

class ResourcesLoader {
public:
    using node_t = tbb::flow::async_node<tbb::flow::continue_msg, se::utils::CrawledResourceData>;
    using gateway_t = node_t::gateway_type;
    
public:
    ResourcesLoader(const IndexingOptions& opts, std::shared_ptr<IExternalBus> bus);

    bool is_run() const noexcept;
    void run(tbb::flow::continue_msg, gateway_t& gateway);
    void stop();

private:
    template<typename Dur>
    Dur estimate_delay() {
        using multiplier = std::ratio_divide<decltype(LoaderOptions::speed)::period, typename Dur::period>;
        return Dur(
            static_cast<typename Dur::rep>(
                opts_.batch_size * (multiplier::num / multiplier::den) / opts_.speed.count() 
            )
        );
    }

    void receive_next();

private:
    std::atomic<bool> end_;
    std::mutex end_mutex_;
    std::condition_variable end_cv_;
    
private:
    std::atomic<bool> run_;
    std::atomic<size_t> current_size_;
    LoaderOptions opts_;
    std::chrono::milliseconds delay_;
    gateway_t* gateway_;
    std::shared_ptr<IExternalBus> bus_;
};

} // namespace indexer

} // namespace se