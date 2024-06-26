#pragma once

#include <functional>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include <cds/gc/hp.h>
#include <cds/container/feldman_hashmap_hp.h>
#include <cds/container/fcpriority_queue.h>
#include <crawler/config.hpp>
#include <crawler/core/resource.hpp>
#include <seutils/service.hpp>
#include <seutils/async_event.hpp>
#include <crawler/logging/logging.hpp>

namespace se {

namespace crawler {

namespace details {

struct PackedResource {
    int64_t timestamp;
    Resource* resource;

    PackedResource() = default;

    PackedResource(Resource* resource) :
        timestamp{ boost::posix_time::microsec_clock::local_time().time_of_day().total_microseconds() },
        resource{ resource }
    { }
};

} // namespace details

} // namespace crawler

} // namespace se

namespace std {
    template<>
    struct less<se::crawler::details::PackedResource> {
        bool operator()(
            const se::crawler::details::PackedResource& lhs, 
            const se::crawler::details::PackedResource& rhs
        ) const {
            return 
                (lhs.resource->priority < rhs.resource->priority) || 
                (lhs.resource->priority == rhs.resource->priority && lhs.timestamp > rhs.timestamp);
        }
    };
}

namespace se {

namespace crawler {

class ResourcesRepository final : public se::utils::Service {

    using se::utils::Service::context_;
    
private:
    struct ResourceGroupsContainerTraits : public cds::container::feldman_hashmap::traits {
        typedef std::hash<std::string> hash;
    };

    struct ResourcesGroupState {
        std::atomic<bool> delayed;
        cds::container::FCPriorityQueue<details::PackedResource> queue;
        std::unique_ptr<boost::asio::high_resolution_timer> timer_ptr;

        ResourcesGroupState() : delayed{ true }
        { }

        ResourcesGroupState(ResourcesGroupState&& ds) :
            delayed{ ds.delayed.load(std::memory_order_acquire) },
            timer_ptr{ std::move(ds.timer_ptr) }
        { }
    };

    using GroupsContainer = cds::container::FeldmanHashMap<
        cds::gc::HP, 
        std::string, 
        ResourcesGroupState, 
        ResourceGroupsContainerTraits
    >;

public:
    ResourcesRepository();

    size_t size() const;
    size_t output_size() const;
    size_t group_size(const std::string& group_name);

    bool is_full() const;
    void push(const std::string& group_name, ResourcePtr rptr);
    bool try_pop(ResourcePtr& ptr);

    template<typename Dur = std::chrono::milliseconds>
    void reload_group(const std::string& group_name, bool forced = false, Dur desired_delay = Dur{0}) {
        auto ptr = groups_.get(group_name);
        if (ptr.empty())
            return;
        auto &state = ptr->second;
        if (forced) {
            handle_expired_reload(group_name, {});
            return;
        }
        state.delayed.store(false, std::memory_order_release);
        auto desired_delay_ms = std::chrono::duration_cast<std::decay_t<decltype(group_fetch_delay_ms_)>>(desired_delay);
        state.timer_ptr->expires_from_now(std::max(group_fetch_delay_ms_, desired_delay_ms));
        state.timer_ptr->async_wait(std::bind(&ResourcesRepository::handle_expired_reload, this, group_name, std::placeholders::_1));
    }

    void reset();

private:
    void handle_expired_reload(
        const std::string& group_name,
        const boost::system::error_code& ec
    );
    void push_impl(const std::string& group_name, Resource* rptr, bool output = false);
    void push_with_overflow(Resource* rptr, ResourcesGroupState& state);
    void push_to_group(Resource* rptr, ResourcesGroupState& state);
    void push_to_output(Resource* rptr);

private:
    const size_t max_resources_count_;
    const std::chrono::milliseconds group_fetch_delay_ms_;
    std::atomic<size_t> count_;
    se::utils::AsyncEvent free_space_notifier_;
    cds::container::FCPriorityQueue<details::PackedResource> output_queue_;
    GroupsContainer groups_;
};

} // namespace crawler

} // namespace se