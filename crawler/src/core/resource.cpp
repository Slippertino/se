#include <crawler/core/resource.hpp>
#include <crawler/handlers/robotstxt_handler.hpp>
#include <crawler/handlers/sitemap_handler.hpp>
#include <crawler/handlers/page_handler.hpp>

namespace crawler {

std::shared_ptr<class ResourceHandler> RobotsTxt::get_handler() const {
    return RobotsTxtHandler::create<RobotsTxtHandler>();
}

ResourcePtr RobotsTxt::clone() const {
    return ResourcePtr{ new RobotsTxt(header) };
}

std::shared_ptr<class ResourceHandler> Sitemap::get_handler() const {
    return SitemapHandler::create<SitemapHandler>();
}

ResourcePtr Sitemap::clone() const {
    return ResourcePtr{ new Sitemap(header, uri) };
}

std::shared_ptr<class ResourceHandler> Page::get_handler() const {
    return PageHandler::create<PageHandler>();
}

ResourcePtr Page::clone() const {
    return ResourcePtr{ new Page(header, uri, priority, checksum, fin) };
}

} // namespace crawler