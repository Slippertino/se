#include <indexer/nodes/resources_loader.hpp>

namespace se {

namespace indexer {

ResourcesLoader::ResourcesLoader(const IndexingOptions& opts, std::shared_ptr<IExternalBus> bus) :
    end_{ false },
    run_{ false },
    current_size_ { 0 },
    opts_{ opts.loader_options },
    delay_(estimate_delay<decltype(delay_)>()),
    bus_ { bus }
{ }

bool ResourcesLoader::is_run() const noexcept {
    return run_.load(std::memory_order_acquire);
}

void ResourcesLoader::run(tbb::flow::continue_msg, gateway_t& gateway) {
    if (run_.load(std::memory_order_acquire))
        return;
    run_.store(true, std::memory_order_release);
    gateway_ = &gateway;
    end_.store(false, std::memory_order_release);
    gateway_->reserve_wait();
    receive_next();
    LOG_INFO_WITH_TAGS(
        logging::resources_loader_category, 
        "Loading resources from bus for indexing was successfully started."
    ); 
}

void ResourcesLoader::stop() {
    run_.store(false, std::memory_order_release);
    std::unique_lock<std::mutex> locker{ end_mutex_ };
    end_cv_.wait(locker, [this]() { return end_.load(std::memory_order_acquire); });
    gateway_->release_wait();      
    LOG_INFO_WITH_TAGS(
        logging::resources_loader_category, 
        "Pulling resources from bus for indexing was successfully finished."
    );      
}

void ResourcesLoader::receive_next() {
    bus_->receive_resources(
        [this](auto&& data) {
            current_size_.fetch_add(1, std::memory_order_acquire);
            gateway_->try_put(std::forward<se::utils::CrawledResourceData>(data));   
        },
        [this]() {
            if (!run_.load(std::memory_order_acquire))
                return;
            if (current_size_.load(std::memory_order_acquire) >= opts_.batch_size) {
                current_size_.store(0, std::memory_order_release);
                std::this_thread::sleep_for(delay_);
            }
            if (run_.load(std::memory_order_acquire)) {
                receive_next();
            } else {
                end_.store(true, std::memory_order_release);
                end_cv_.notify_one();
            }
        }
    );      
}

} // namespace indexer

} // namespace se