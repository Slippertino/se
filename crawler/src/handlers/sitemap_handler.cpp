#include <crawler/handlers/sitemap_handler.hpp>
#include <crawler/core/resource_processor.hpp>

namespace se {

namespace crawler {

SitemapHandler::SitemapHandler(private_token) : HttpResourceHandler({})
{ }

void SitemapHandler::handle_http_resource(HttpResults& results) {
    std::vector<ResourcePtr> out;
    if (std::regex_match(resource_->url().c_str(), raw_sitemap_pattern_)) {
        extract_resources(resource_, std::move(results.body), out);
    }
    else {
        auto& buffer = get_thread_resource();
        std::istringstream in{ std::move(results.body) };
        std::string error;
        if (se::utils::CompressionHelper::decompress_complex(in, buffer, error, [this, &out](auto name, auto content) {
            extract_resources(resource_, std::move(content), out);
        })) {
            LOG_ERROR_WITH_TAGS(
                logging::handler_category, 
                "Error occurs while decompressing SITEMAP complex file from {} with message: {}.", 
                resource_->url().c_str(),
                error
            );
            return;
        }            
    }
    if (auto proc = processor_.lock())
        proc->handle_new_resources(std::move(out));
    set_handling_status(HandlingStatus::handled_success);
    LOG_INFO_WITH_TAGS(
        logging::handler_category, 
        "Successfully handled SITEMAP file with URL: {}", 
        resource_->url().c_str()
    );
}

void SitemapHandler::extract_resources(
    ResourcePtr& resource,
    std::string content, 
    std::vector<ResourcePtr>& out
) {
    try {
        for(auto&& sm : parsers::SitemapParser(content)) {
            ResourcePtr cur;
            boost::url url{ sm.loc };
            switch (sm.type)
            {
            case parsers::SitemapType::INDEX:
                cur = Resource::create_from_url<Sitemap>(url, url.path() + url.query());
                break;
            case parsers::SitemapType::URLSET:
                cur = Resource::create_from_url<Page>(url, url.path() + url.query());
                break;
            default:
                break;
            }
            out.push_back(std::move(cur));
        }
    }
    catch(const boost::property_tree::xml_parser_error& xpe) {
        LOG_ERROR_WITH_TAGS(
            logging::handler_category, 
            "Format error occurs while parsing SITEMAP file from {} with message: {}.", 
            resource->url().c_str(),
            xpe.message()
        );
    }
    catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            logging::handler_category, 
            "Unexpected error occurs while parsing SITEMAP file from {} with message: {}.", 
            resource->url().c_str(),
            ex.what()
        );
    }
}

} // namespace crawler

} // namespace se