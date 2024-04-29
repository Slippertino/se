#include <crawler/handlers/page_handler.hpp>
#include <crawler/core/resource_processor.hpp>

namespace crawler {

PageHandler::PageHandler(private_token) : HttpResourceHandler({})
{ }

void PageHandler::handle_http_resource(
    ResourcePtr& resource, 
    std::shared_ptr<ResourceProcessor>& processor,
    HttpResults& results
) {
    html_analyzer::HTMLAnalyzer analyzer{ results.body };
    if (!analyzer.is_valid()) {
        LOG_ERROR_WITH_TAGS(
            logging::handler_category, 
            "Errors occured while parsing HTML-document of {}.", 
            resource->url().c_str()
        );
        return;
    }
    auto indexing_ptr = static_cast<IndexingResource*>(resource.get());
    indexing_ptr->checksum = utils::sha256(results.body);
    auto page_info = analyzer.analyze<Automaton>();
    auto& enc = page_info.encoding;
    std::transform(
        enc.begin(), enc.end(), enc.begin(), 
        [](auto ch) { return std::tolower(ch); }
    );
    if (enc != "utf-8") {
        LOG_WARNING_WITH_TAGS(
            logging::handler_category, 
            "Unsupported encoding {} was met in resource {}.", 
            enc,
            resource->url().c_str()
        );
        return;
    }
    if (page_info.title.empty() && page_info.linked_uris.empty()) {
        LOG_WARNING_WITH_TAGS(
            logging::handler_category, 
            "Incomplete HTML-document was provided by resource {}.", 
            resource->url().c_str()
        );
        return;        
    }
    processor->commit_resource(*indexing_ptr);
    if (page_info.can_follow) {
        auto links = extract_links(resource, page_info);
        processor->handle_new_resources(std::move(links));
    }
    if (page_info.can_index) {
        processor->send_to_index(
            utils::CrawledResourceData {
                resource->url(),
                std::move(results.body)
            }
        );
    }
    confirm_success_handling();
    LOG_INFO_WITH_TAGS(
        logging::handler_category, 
        "Successfully handled PAGE file with URL: {}.", 
        resource->url().c_str()
    );
}

std::vector<ResourcePtr> PageHandler::extract_links(
    const ResourcePtr& resource, 
    html_analyzer::PageInfo& info
) {
    std::vector<ResourcePtr> links;
    for(const auto& lnk : info.linked_uris) {
        boost::url url;
        try {
            url = boost::url{ lnk };
        } catch(const boost::system::system_error& se) {
            continue;
        }
        if (!url.scheme().empty())
            goto flush;
        if (!url.host().empty()) {
            url.set_scheme(*resource->header.type);
            goto flush;
        }
        if (url.is_path_absolute()) {
            url.set_scheme(*resource->header.type);
            url.set_host(*resource->header.domain);
            goto flush;
        }
        url.set_scheme(*resource->header.type);
        url.set_host(*resource->header.domain);
        url.set_path(resource->uri + url.path());
    flush:
        url.normalize();
        links.push_back(
            Resource::create_from_url<Page>(url, url.path() + url.query())
        );
    }    
    return links;
}

} // namespace crawler