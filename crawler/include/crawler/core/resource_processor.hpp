#pragma once

#include <functional>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include <crawler/core/resource.hpp>
#include <seutils/service.hpp>
#include <crawler/core/resource_distributor.hpp>
#include <crawler/core/resources_repository.hpp>
#include <crawler/bus/external_bus.hpp>
#include <crawler/db/data_provider.hpp>
#include <crawler/handlers/handling_status.hpp>
#include <crawler/handlers/resource_handler.hpp>

namespace se {

namespace crawler {

class ResourceProcessor final : 
    public se::utils::Service, 
    public std::enable_shared_from_this<ResourceProcessor> {
        
private: 
    struct private_token { };

public:
    ResourceProcessor() = delete;
    ResourceProcessor(
        std::shared_ptr<ResourceDistributor> distributor,
        std::shared_ptr<ResourcesRepository> repository,
        std::shared_ptr<IDataProvider> db,
        std::shared_ptr<IExternalBus> bus,
        private_token
    );

    static std::shared_ptr<ResourceProcessor> create(
        std::shared_ptr<ResourceDistributor> distributor,
        std::shared_ptr<ResourcesRepository> repository,
        std::shared_ptr<IDataProvider> db,
        std::shared_ptr<IExternalBus> bus
    );

    size_t total_handled() const noexcept;
    size_t total_succeed_handled() const noexcept;
    size_t total_in_work() const noexcept;
 
    template<typename Dur = std::chrono::milliseconds>
    void handle_resource_received(const ResourcePtr& resource, bool success = true, Dur retry_delay = Dur{0}) {
        if (auto repo = repository_.lock()) {
            repo->reload_group(
                ResourceLoader::resolve_name_with_cache(resource->url()), 
                !success,
                retry_delay
            );
        }
        else {
            LOG_WARNING_WITH_TAGS(
                logging::processor_category, 
                "Queue expired."
            );
        }
    }

    void handle_new_resources(std::vector<ResourcePtr> resources);
    void commit_resource(const IndexingResource& resource);
    void send_to_index(const se::utils::CrawledResourceData& data);
    void on_handling_end(ResourcePtr resource, HandlingStatus status);

protected:
    void dispatch_service_data() override;

private:
    const size_t max_concurrent_handlers_count_;
    std::atomic<size_t> total_handled_;
    std::atomic<size_t> total_succeed_handled_;
    std::atomic<size_t> total_in_work_;
    std::weak_ptr<ResourceDistributor> distributor_;
    std::weak_ptr<ResourcesRepository> repository_;
    std::shared_ptr<IDataProvider> db_;
    std::shared_ptr<IExternalBus> bus_;
};

} // namespace crawler

} // namespace se