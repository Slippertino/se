#include <crawler/core/resources_repository.hpp>

namespace se {

namespace crawler {

ResourcesRepository::ResourcesRepository() : 
    Service(false, false), 
    max_resources_count_  { Config::from_service<size_t>("queue", "max_size")            },
    group_fetch_delay_ms_{ Config::from_service<size_t>("queue", "group_fetch_delay_ms") },
    count_{ 0 },
    free_space_notifier_{ get_context() }
{ }

size_t ResourcesRepository::size() const {
    return count_.load(std::memory_order_acquire);
}

size_t ResourcesRepository::output_size() const {
    return output_queue_.size();
}

size_t ResourcesRepository::group_size(const std::string& group_name) {
    return groups_.get(group_name)->second.queue.size();
}

bool ResourcesRepository::is_full() const {
    return size() >= max_resources_count_;
}

void ResourcesRepository::push(const std::string& group_name, ResourcePtr rptr) {
    if (!groups_.contains(group_name)) {
        bool res{ false };
        while(!res) {
            res = groups_.insert_with(
                group_name,
                [this](auto& v) {
                    v.second.delayed.store(true, std::memory_order_release);
                    v.second.timer_ptr = std::make_unique<boost::asio::high_resolution_timer>(get_context());
                }
            );
            std::this_thread::yield();
        }
    }
    std::string name = rptr->url().c_str();
    push_impl(group_name, rptr.release());
    LOG_TRACE_L1_WITH_TAGS(
        logging::queue_category, 
        "Added to queue: {}.", 
        name
    );
}

bool ResourcesRepository::try_pop(ResourcePtr& ptr) {
    details::PackedResource pr;
    auto res = output_queue_.pop(pr);
    if (res) {
        ptr = ResourcePtr{ pr.resource };
        LOG_TRACE_L1_WITH_TAGS(
            logging::queue_category, 
            "Removed from queue: {}.", 
            ptr->url().c_str()
        );
        if (count_.load(std::memory_order_acquire) < max_resources_count_) 
            free_space_notifier_.async_notify_one();
    }
    return res;
}

void ResourcesRepository::reset() {
    while(!output_queue_.empty()) {
        details::PackedResource cur;
        if (output_queue_.pop(cur))
            delete cur.resource;
    }
    for(auto& dm : groups_) {
        auto& q = dm.second.queue;
        while(!q.empty()) {
            details::PackedResource cur;
            if (q.pop(cur))
                delete cur.resource;
        }
    }
    groups_.clear();
}

void ResourcesRepository::handle_expired_reload(
    const std::string& group_name,
    const boost::system::error_code&
) {
    auto ptr = groups_.get(group_name);
    if (ptr.empty()) return;

    auto& state = ptr->second;
    details::PackedResource res;
    if (!state.queue.pop(res)) 
        state.delayed.store(true, std::memory_order_release);
    else
        push_impl(group_name, res.resource, true);
}

void ResourcesRepository::push_impl(const std::string& group_name, Resource* rptr, bool output) {
    if (output) {
        push_to_output(rptr);
        return;
    }
    auto ptr_state = groups_.get(group_name);
    if (ptr_state.empty()) return;
    auto& state = ptr_state->second;
    if (state.delayed.load(std::memory_order_acquire)) {
        push_to_output(rptr);
        state.delayed.store(false, std::memory_order_release);
        return;
    }
    if (is_full()) {
        free_space_notifier_.async_wait([this, rptr, &state](auto) {
            push_with_overflow(rptr, state);
        });
    }
    else
        push_to_group(rptr, state);
}

void ResourcesRepository::push_with_overflow(Resource* rptr, ResourcesGroupState& state) {
    if (!is_full()) {
        push_to_group(rptr, state);
    }
    else {
        free_space_notifier_.async_wait([this, rptr, &state](auto) {
            push_with_overflow(rptr, state);
        });
    }
}

void ResourcesRepository::push_to_group(Resource* rptr, ResourcesGroupState& state) {
    count_.fetch_add(1, std::memory_order_acq_rel);
    state.queue.push(details::PackedResource(rptr));
}

void ResourcesRepository::push_to_output(Resource* rptr) {
    output_queue_.push(details::PackedResource(rptr));
}

} // namespace crawler

} // namespace se