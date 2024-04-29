#pragma once

#include <string>
#include <regex>
#include "automaton.hpp"

namespace html_analyzer {

template<bool EnablePrompts = false>
class LinkAutomaton final : public HTMLAutomaton {
public:
    LinkAutomaton() = delete;
    LinkAutomaton(PageInfo &info) : HTMLAutomaton(info)
    { }

protected:
    void update_impl(const GumboNode* node) override final {
        if (!is_node_element(node))
            return;
            
        GumboAttribute* attr;
        if (node->v.element.tag == GUMBO_TAG_A &&
            (attr = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
            GumboAttribute* prompts = gumbo_get_attribute(&node->v.element.attributes, "rel");
            if (prompts && EnablePrompts && std::regex_match(prompts->value, nofollow_pattern))
                return;
            info_.linked_uris.insert(std::string(attr->value));
        }
    }

private:
    static inline const std::regex nofollow_pattern = std::regex(R"(.*nofollow.*)");
};

} // namespace html-analyzer