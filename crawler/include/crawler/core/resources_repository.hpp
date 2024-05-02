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
    struct DomainsContainerTraits : public cds::container::feldman_hashmap::traits {
        typedef std::hash<std::string> hash;
    };

    struct DomainState {
        std::atomic<bool> delayed;
        cds::container::FCPriorityQueue<details::PackedResource> queue;
        std::unique_ptr<boost::asio::high_resolution_timer> timer_ptr;

        DomainState() : delayed{ true }
        { }

        DomainState(DomainState&& ds) :
            delayed{ ds.delayed.load(std::memory_order_acquire) },
            timer_ptr{ std::move(ds.timer_ptr) }
        { }
    };

    using DomainsContainer = cds::container::FeldmanHashMap<cds::gc::HP, std::string, DomainState, DomainsContainerTraits>;

public:
    ResourcesRepository();

    size_t size() const;
    size_t output_size() const;
    size_t domain_size(const std::string& domain);

    bool is_full() const;
    bool try_push(ResourcePtr rptr);
    bool try_pop(ResourcePtr& ptr);

    void reload_domain(const std::string& domain, bool forced = false);
    void reset();

private:
    void handle_expired_reload(
        const std::string& domain,
        const boost::system::error_code& ec
    );
    void push_impl(Resource* rptr, bool output = false);
    void push_to_output(Resource* rptr);

private:
    const size_t max_resources_count_;
    const std::chrono::milliseconds domain_fetch_delay_ms_;
    std::atomic<size_t> count_;
    std::mutex full_mutex_;
    std::condition_variable full_cv_;
    cds::container::FCPriorityQueue<details::PackedResource> output_queue_;
    DomainsContainer domains_;
};

} // namespace crawler

} // namespace se