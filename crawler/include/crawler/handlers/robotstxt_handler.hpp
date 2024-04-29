#pragma once

#include <crawler/parsers/robots/robots.hpp>
#include "http_resource_handler.hpp"

namespace crawler {

class RobotsTxtHandler final : public HttpResourceHandler {
public:
    RobotsTxtHandler(private_token);

protected:
    void handle_failed_load(
        ResourcePtr& resource, 
        std::shared_ptr<ResourceProcessor>& processor        
    ) override;
    
    void handle_http_resource(
        ResourcePtr& resource, 
        std::shared_ptr<ResourceProcessor>& processor,
        HttpResults& results
    ) override;
};

} // namespace crawler