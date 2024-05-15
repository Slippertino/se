#pragma once

#include <crawler/parsers/robots/robots.hpp>
#include "http_resource_handler.hpp"

namespace se {

namespace crawler {

class RobotsTxtHandler final : public HttpResourceHandler {
public:
    RobotsTxtHandler(private_token);

protected:
    void handle_failed_load() override;
    void handle_http_resource(HttpResults& results) override;
};

} // namespace crawler

} // namespace se