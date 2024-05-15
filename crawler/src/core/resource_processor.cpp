#include <crawler/core/resource_processor.hpp>

namespace se {

namespace crawler {

ResourceProcessor::ResourceProcessor(
    std::shared_ptr<ResourceDistributor> distributor,
    std::shared_ptr<ResourcesRepository> repository,
    std::shared_ptr<IDataProvider> db,
    std::shared_ptr<IExternalBus> bus,
    private_token
) : 
    Service(true, false),
    max_concurrent_handlers_count_{ Config::from_service<size_t>("processor", "max_size") },
    total_handled_{ 0 },
    total_succeed_handled_{ 0 },
    total_in_work_{ 0 },
    distributor_{ distributor }, 
    repository_{ repository }, 
    db_{ db },
    bus_{ bus }
{ }

std::shared_ptr<ResourceProcessor> ResourceProcessor::create(
    std::shared_ptr<ResourceDistributor> distributor,
    std::shared_ptr<ResourcesRepository> repository,
    std::shared_ptr<IDataProvider> db,
    std::shared_ptr<IExternalBus> bus
) {
    return std::make_shared<ResourceProcessor>(
        distributor,
        repository, 
        db,
        bus,
        private_token{}
    );
}

size_t ResourceProcessor::total_handled() const noexcept {
    return total_handled_.load(std::memory_order_acquire);
}

size_t ResourceProcessor::total_succeed_handled() const noexcept {
    return total_succeed_handled_.load(std::memory_order_acquire);
}

size_t ResourceProcessor::total_in_work() const noexcept {
    return total_in_work_.load(std::memory_order_acquire);
}

void ResourceProcessor::handle_new_resources(std::vector<ResourcePtr> resources) {
    if (auto distr = distributor_.lock()) {
        distr->distribute_nowait(std::move(resources));
    } else { 
        LOG_WARNING_WITH_TAGS(
            logging::processor_category, 
            "Distributor expired."
        );
    }
}

void ResourceProcessor::commit_resource(const IndexingResource& resource) {
    db_->upload_resource(resource);
}

void ResourceProcessor::send_to_index(const se::utils::CrawledResourceData& data) {
    bus_->send_resource(data);
}

void ResourceProcessor::on_handling_end(ResourcePtr resource, HandlingStatus status) {
    total_in_work_.fetch_sub(1, std::memory_order_acq_rel);
    switch(status) {
        case HandlingStatus::partly_handled:  
            context_.post([this, ptr = resource.release()](){
                auto resource = ResourcePtr{ ptr };
                if (auto repo = repository_.lock()) {
                    auto url = resource->url();
                    repo->push(
                        ResourceLoader::resolve_name_with_cache(url), 
                        std::move(resource)
                    );
                } else {
                    LOG_WARNING_WITH_TAGS(
                        logging::processor_category, 
                        "Queue expired."
                    );
                }
            });
            break;
        case HandlingStatus::handled_success:
            total_succeed_handled_.fetch_add(1, std::memory_order_acq_rel);
            [[fallthrough]];
        case HandlingStatus::handled_failed:
            total_handled_.fetch_add(1, std::memory_order_acq_rel);
            if (auto distr = distributor_.lock()) {
                distr->mark_as_handled(resource);
            } else { 
                LOG_WARNING_WITH_TAGS(
                    logging::processor_category, 
                    "Distributor expired."
                );
            }
            break;
        default:
            break;
    }
}

void ResourceProcessor::dispatch_service_data() {
    auto repo = repository_.lock();
    if (!repo) {
        LOG_WARNING_WITH_TAGS(
            logging::processor_category, 
            "Queue expired."
        );
        return;
    }
    size_t cur = total_in_work_.load(std::memory_order_acquire);
    do {
        if (cur >= max_concurrent_handlers_count_)
            return;
    } while(!total_in_work_.compare_exchange_weak(cur, cur + 1));
    ResourcePtr ptr;
    if (!repo->try_pop(ptr)) {
        total_in_work_.fetch_sub(1, std::memory_order_acq_rel);
        return;
    }
    context_.post([rptr = ptr.release(), proc = shared_from_this()]() mutable {
        auto handler = rptr->get_handler();
        handler->handle(ResourcePtr{ rptr }, proc); 
    });
}

} // namespace crawler

} // namespace se