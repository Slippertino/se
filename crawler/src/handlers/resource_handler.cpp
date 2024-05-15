#include <crawler/handlers/resource_handler.hpp>
#include <crawler/core/resource_processor.hpp>

namespace se {

namespace crawler {

ResourceHandler::ResourceHandler(ResourceHandler::private_token) :
    status_{ HandlingStatus::handled_failed }
{ }

void ResourceHandler::handle(
    ResourcePtr resource, 
    std::shared_ptr<ResourceProcessor> processor
) {
    processor_ = processor;
    resource_ = std::move(resource);
    auto& context = processor->get_context();
    const auto& type = *resource_->header.type;
    loader_ = ResourceLoadersFactory::get_loader(
        *resource_->header.type, 
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
    if (resource_->is_indexing && 
        !check_permissions()) {
        handle_no_permissions();
        processor->handle_resource_received(resource_, false);
        return;
    }
    auto url = resource_->url();
    loader_->load(
        url, 
        [hndlr = shared_from_this()](auto results) {
            hndlr->handle_resource(std::move(results));
        }
    );
}

void ResourceHandler::set_handling_status(HandlingStatus status) {
    status_ = status;
}

bool ResourceHandler::check_permissions() const {
    return true;
} 

ResourceHandler::~ResourceHandler() {
    if (auto proc = processor_.lock())
        proc->on_handling_end(std::move(resource_), status_);
}

} // namespace crawler

} // namespace se