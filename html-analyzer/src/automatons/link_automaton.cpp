#include "automatons/link_automaton.hpp"

namespace html_analyzer {

LinkAutomaton::LinkAutomaton(PageInfo &info) : HTMLAutomaton(info)
{ }

void LinkAutomaton::update_impl(const GumboNode* node) {
    if (!is_node_element(node))
        return;
        
    GumboAttribute* attr;
    if (node->v.element.tag == GUMBO_TAG_A &&
        (attr = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
        info_.linked_uris.insert(std::string(attr->value));
    }
}

} // namespace html_analyzer