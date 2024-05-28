#include <crawler/core/resource_distributor.hpp>

namespace se {

namespace crawler {

ResourceDistributor::ResourceDistributor(
    std::shared_ptr<IDataProvider> data,
    std::shared_ptr<ResourcesRepository> repository
) :
    Service(
        true, true, 
        std::chrono::milliseconds(
            se::utils::GlobalConfig<Config>::config.from_service<size_t>("distributor", "interval_ms")
        ), 
        1ull
    ), 
    group_size_          {  
        se::utils::GlobalConfig<Config>::config.from_service<size_t>("distributor", "domain_group_size") 
    },
    max_pages_batch_size_{  
        se::utils::GlobalConfig<Config>::config.from_service<size_t>("distributor", "max_pages_batch_size") 
    },
    data_{ data },
    repository_{ repository },
    resolver_ { get_context() },
    headers_{ group_size_, 1 }
{ }

size_t ResourceDistributor::size() const noexcept {
    return async_count_.load(std::memory_order_acquire);
}

size_t ResourceDistributor::current_crawling_group_size() const noexcept {
    return headers_.size();
}

void ResourceDistributor::distribute(ResourcePtr resource) {
    if (!resource->is_indexing) {
        distribute_no_check(std::move(resource));
        return;
    }
    auto upd = data_->get_resource(resource->header, resource->uri);
    if (upd && upd->fin)
        return;
    if (upd)
        resource.reset(upd.release());
    auto str = std::string(resource->url().c_str());
    auto hash = std::hash<std::string>{}(str);
    if (used_resources_.contains(hash))
        return;
    distribute_no_check(std::move(resource));
    used_resources_.insert(str);
}

void ResourceDistributor::distribute_nowait(std::vector<ResourcePtr> resources) {
    context_.post([this, ress = std::move(resources)]() mutable {
        async_count_.fetch_add(1, std::memory_order_release);
        for(auto& r : ress)
            distribute(std::move(r));
        async_count_.fetch_sub(1, std::memory_order_release);
    });
}

void ResourceDistributor::mark_as_handled(const ResourcePtr& resource) {
    auto str = std::string(resource->url().c_str());
    auto hash = std::hash<std::string>{}(str);
    used_resources_.erase(hash);
}

ResourceDistributor::~ResourceDistributor() {
    data_->finalize();
}

void ResourceDistributor::dispatch_service_data() {
    remove_crawled_headers();
    if (headers_.size() < group_size_)
        add_new_headers();
    for(const auto& hd : headers_) {
        context_.post(
            std::bind(&ResourceDistributor::distribute_some_from_header, this, hd.first)
        );
    }
}

void ResourceDistributor::distribute_some_from_header(const std::string& name) {
    if (!headers_.contains(name))
        return;
    auto hdr = headers_.get(name);
    auto resources = data_->get_unhandled_top_by_header(hdr->second.header, max_pages_batch_size_);
    if (resources.empty())
        hdr->second.is_empty.store(false, std::memory_order_release);
    for(auto& res : resources)
        distribute_no_check(ResourcePtr{ res.release() });
}

void ResourceDistributor::remove_crawled_headers() {
    std::vector<std::string> crawled;
    for(const auto& hd : headers_)
        if (hd.second.is_empty.load(std::memory_order_acquire))
            crawled.push_back(hd.first);
    for(auto& hd : crawled) {
        headers_.erase(hd);
        LOG_INFO_WITH_TAGS(
            logging::distributor_category, 
            "Address {} was successfully crawled.", 
            hd
        );
    }
}

void ResourceDistributor::add_new_headers() {
    auto new_headers = data_->get_unhandled_headers_top(group_size_);
    for(auto i = 0; i < new_headers.size() && headers_.size() < group_size_; ++i) {
        const auto& hd = new_headers[i];
        auto key = std::string{ new_headers[i].url().c_str() };
        if (!headers_.contains(key)) {
            distribute(std::make_unique<Page>(new_headers[i]));
            headers_.insert(key, HeaderData{ new_headers[i], false });
            LOG_INFO_WITH_TAGS(
                logging::distributor_category, 
                "Address {} was added in crawling queue.", 
                key
            );
        }
    }
}

void ResourceDistributor::distribute_no_check(ResourcePtr resource) {
    auto url = resource->url();
    resolver_.load(url, [this, ptr = resource.release()](auto) {
        async_count_.fetch_add(1, std::memory_order_release);
        auto resource = ResourcePtr{ ptr };
        auto repo = repository_.lock();
        if (!repo) {
            LOG_WARNING_WITH_TAGS(
                logging::distributor_category, 
                "Queue expired."
            );
            return;
        }    
        auto url = resource->url();
        auto ip = ResourceLoader::resolve_name_with_cache(url);
        if (ip.empty()) {
            LOG_ERROR_WITH_TAGS(
                logging::distributor_category, 
                "Not managed to resolve ip of resource {} for adding to queue", 
                url.c_str()
            );        
            return;        
        }
        repo->push(ip, std::move(resource));
        async_count_.fetch_sub(1, std::memory_order_release);
    });
}

} // namespace crawler

} // namespace se