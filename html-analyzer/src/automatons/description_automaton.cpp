#include <htmlanalyzer/automatons/description_automaton.hpp>

namespace html_analyzer {

DescriptionAutomaton::DescriptionAutomaton(PageInfo &info) : HTMLAutomaton(info)
{ }

void DescriptionAutomaton::update_impl(const GumboNode* node) {
    if (!is_node_element(node) || node->v.element.tag != GUMBO_TAG_META)
        return;
    auto attr = gumbo_get_attribute(&node->v.element.attributes, "name");
    if (attr && !strcmp(attr->value, "description")) {
        attr = gumbo_get_attribute(&node->v.element.attributes, "content");
        info_.description = std::string(attr->value);
        exposed_ = true;
    }
}

} // namespace html_analyzer