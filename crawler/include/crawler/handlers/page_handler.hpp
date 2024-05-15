#pragma once

#include <boost/system/system_error.hpp>
#include <htmlanalyzer/html_analyzer.hpp>
#include <htmlanalyzer/automatons/automatons.hpp>
#include <seutils/crypto.hpp>
#include "http_resource_handler.hpp"

namespace se {

namespace crawler {

class PageHandler final : public HttpResourceHandler {
public:
    PageHandler(private_token);

protected:
    void handle_http_resource(HttpResults& results) override;

private:
    static std::vector<ResourcePtr> extract_links(const ResourcePtr& resource, html_analyzer::PageInfo& info);
    
private:
    using Automaton = html_analyzer::CombinedAutomaton<
        html_analyzer::EncodingAutomaton,
        html_analyzer::TitleAutomaton,
        html_analyzer::LinkAutomaton<true>,
        html_analyzer::RobotHintsAutomaton
    >;
};

} // namespace crawler

} // namespace se