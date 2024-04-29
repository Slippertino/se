#include <crawler/core/resources_repository.hpp>

namespace crawler {

ResourcesRepository::ResourcesRepository() : 
    Service(false, false), 
    max_resources_count_  { Config::from_service<size_t>("queue", "max_size")              },
    domain_fetch_delay_ms_{ Config::from_service<size_t>("queue", "domain_fetch_delay_ms") }
{ }

size_t ResourcesRepository::size() const {
    return count_.load(std::memory_order_acquire);
}

size_t ResourcesRepository::output_size() const {
    return output_queue_.size();
}

size_t ResourcesRepository::domain_size(const std::string& domain) {
    return domains_.get(domain)->second.queue.size();
}

bool ResourcesRepository::is_full() const {
    return size() >= max_resources_count_;
}

bool ResourcesRepository::try_push(ResourcePtr rptr) {
    const auto& domain = *rptr->header.domain;
    if (!domains_.contains(domain)) {
        bool res = domains_.insert_with(
            domain,
            [this](auto& v) {
                v.second.delayed.store(true, std::memory_order_release);
                v.second.timer_ptr = std::make_unique<boost::asio::high_resolution_timer>(get_context());
            }
        );
        if (!res)
            return false;
    }
    std::string name = rptr->url().c_str();
    push_impl(rptr.release());
    LOG_TRACE_L1_WITH_TAGS(
        logging::queue_category, 
        "Added to queue: {}.", 
        name
    );
    count_.fetch_add(1, std::memory_order_acq_rel);
    return true;
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
        auto cur = count_.fetch_sub(1, std::memory_order_acq_rel);
        if (cur < max_resources_count_) 
            full_cv_.notify_one();
    }
    return res;
}

void ResourcesRepository::reload_domain(const std::string& domain, bool forced) {
    auto ptr = domains_.get(domain);
    if (ptr.empty())
        return;
    auto &state = ptr->second;
    if (forced) {
        handle_expired_reload(domain, {});
        return;
    }
    state.delayed.store(false, std::memory_order_release);
    state.timer_ptr->expires_from_now(domain_fetch_delay_ms_);
    state.timer_ptr->async_wait(std::bind(&ResourcesRepository::handle_expired_reload, this, domain, std::placeholders::_1));
}

void ResourcesRepository::reset() {
    std::lock_guard locker{ full_mutex_ };
    while(!output_queue_.empty()) {
        details::PackedResource cur;
        if (output_queue_.pop(cur))
            delete cur.resource;
    }
    for(auto& dm : domains_) {
        auto& q = dm.second.queue;
        while(!q.empty()) {
            details::PackedResource cur;
            if (q.pop(cur))
                delete cur.resource;
        }
    }
    domains_.clear();
}

void ResourcesRepository::handle_expired_reload(
    const std::string& domain,
    const boost::system::error_code& ec
) {
    if (ec) {
        LOG_ERROR_WITH_TAGS(
            logging::queue_category, 
            "Queue's domain {} error occured on reload: {}", 
            domain,
            ec.message()
        );
        return;
    }
    auto ptr = domains_.get(domain);
    if (ptr.empty()) return;

    auto& state = ptr->second;
    details::PackedResource res;
    if (!state.queue.pop(res)) 
        state.delayed.store(true, std::memory_order_release);
    else
        push_impl(res.resource, true);
}

void ResourcesRepository::push_impl(Resource* rptr, bool output) {
    if (output) {
        push_to_output(rptr);
        return;
    }
    auto ptr_state = domains_.get(*rptr->header.domain);
    if (ptr_state.empty()) return;
    auto& state = ptr_state->second;
    if (state.delayed.load(std::memory_order_acquire)) {
        push_to_output(rptr);
        state.delayed.store(false, std::memory_order_release);
        return;
    }
    if (is_full()) {
        LOG_WARNING_WITH_TAGS(
            logging::queue_category, 
            "Size limit of queue was reached: {}", 
            count_.load(std::memory_order_relaxed)
        );
        std::unique_lock locker{ full_mutex_ };
        full_cv_.wait(locker, [&]() { return !is_full(); });
    }
    state.queue.push(details::PackedResource(rptr));
}

void ResourcesRepository::push_to_output(Resource* rptr) {
    output_queue_.push(details::PackedResource(rptr));
}

} // namespace crawler