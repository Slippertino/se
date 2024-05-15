#pragma once

#include <functional>
#include <cds/gc/hp.h>
#include <cds/container/michael_kvlist_hp.h>
#include <cds/container/michael_map.h>
#include <cds/container/feldman_hashset_hp.h>
#include <crawler/config.hpp>
#include <crawler/core/resource.hpp>
#include <seutils/service.hpp>
#include <crawler/db/data_provider.hpp>
#include <crawler/loaders/resource_loader.hpp>
#include <crawler/logging/logging.hpp>
#include <crawler/core/resources_repository.hpp>

namespace se {

namespace crawler {

class ResourceDistributor final : public se::utils::Service {
public:
    ResourceDistributor() = delete;
    ResourceDistributor(
        std::shared_ptr<IDataProvider> data,
        std::shared_ptr<ResourcesRepository> repository
    );

public:
    size_t size() const noexcept;
    size_t current_crawling_group_size() const noexcept;

    void distribute(ResourcePtr resource);
    void distribute_nowait(std::vector<ResourcePtr> resources);
    void mark_as_handled(const ResourcePtr& resource);

    ~ResourceDistributor();

protected:
    void dispatch_service_data() override;

private:
    void distribute_some_from_header(const std::string& name);
    void remove_crawled_headers();
    void add_new_headers();
    void distribute_no_check(ResourcePtr resource);

private:
    const size_t group_size_;
    const size_t max_pages_batch_size_;
    std::atomic<size_t> async_count_{0};
    std::shared_ptr<IDataProvider> data_;
    std::weak_ptr<ResourcesRepository> repository_;
    ResourceLoader resolver_;
    
private:
    struct HeaderData {
        ResourceHeader header;
        std::atomic<bool> is_empty;

        HeaderData(const ResourceHeader& rh, bool is_empty) :
            header{ rh },
            is_empty{ is_empty }
        { }
        
        HeaderData(const HeaderData& hd) :
            header{ hd.header },
            is_empty{ hd.is_empty.load(std::memory_order_acquire) }
        { }
    };

    struct HeaderDataComparator {
        int operator ()(const std::string& lhs, const std::string& rhs) {
            return lhs.compare(rhs);
        }
    };

    struct HeadersContainerTraits : public cds::container::michael_list::traits {
        typedef HeaderDataComparator compare;
    };

    using CrawlingHeadersContainer = cds::container::MichaelHashMap<
        cds::gc::HP,
        cds::container::MichaelKVList<
            cds::gc::HP, 
            std::string, 
            HeaderData, 
            HeadersContainerTraits
        >
    >;

    CrawlingHeadersContainer headers_;

private:
    struct UsedResourcesContainerTraits : public cds::container::feldman_hashset::traits {
        typedef std::hash<std::string> hash_accessor;
    };

    using UsedResourcesContainer = cds::container::FeldmanHashSet<
        cds::gc::HP, 
        std::string,
        UsedResourcesContainerTraits
    >;
    
    UsedResourcesContainer used_resources_;
};

} // namespace crawler

} // namespace se