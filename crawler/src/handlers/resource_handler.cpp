#include <crawler/handlers/resource_handler.hpp>
#include <crawler/core/resource_processor.hpp>

namespace crawler {

ResourceHandler::ResourceHandler(ResourceHandler::private_token) :
    success_{ false }
{ }

void ResourceHandler::confirm_success_handling() {
    success_ = true;
}

void ResourceHandler::handle(
    ResourcePtr resource, 
    std::shared_ptr<ResourceProcessor> processor
) {
    processor_ = processor;
    auto& context = processor->get_context();
    const auto& type = *resource->header.type;
    loader_ = ResourceLoadersFactory::get_loader(
        *resource->header.type, 
        context
    );
    if (!loader_) {
        LOG_ERROR_WITH_TAGS(
            logging::handler_category, 
            "Unknown resource type: {}.", 
            type
        );
        return;
    }
    if (resource->is_indexing && 
        !check_permissions(resource, processor)) {
        handle_no_permissions(resource, processor);
        processor->handle_resource_received(resource, false);
        return;
    }
    auto url = resource->url();
    loader_->load(
        url, 
        [hndlr = shared_from_this(), resource = resource.release(), processor](auto results) {
            auto res = ResourcePtr{ resource };
            hndlr->handle_resource(res, processor, results);
        }
    );
}

bool ResourceHandler::check_permissions(
    const ResourcePtr& resource,
    std::shared_ptr<class ResourceProcessor>& processor
) const {
    return true;
} 

ResourceHandler::~ResourceHandler() {
    if (!processor_.expired()) {
        processor_.lock()->on_handling_end(success_);
    }
}

} // namespace crawler