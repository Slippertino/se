#include <htmlanalyzer/automatons/keywords_automaton.hpp>

namespace html_analyzer {

KeywordsAutomaton::KeywordsAutomaton(PageInfo &info) : HTMLAutomaton(info)
{ }

void KeywordsAutomaton::update_impl(const GumboNode* node) {
    if (!is_node_element(node) || node->v.element.tag != GUMBO_TAG_META)
        return;
    auto attr = gumbo_get_attribute(&node->v.element.attributes, "name");
    if (attr && !strcmp(attr->value, "keywords")) {
        attr = gumbo_get_attribute(&node->v.element.attributes, "content");
        info_.keywords = std::string(attr->value);
        exposed_ = true;
    }
}

} // namespace html_analyzer