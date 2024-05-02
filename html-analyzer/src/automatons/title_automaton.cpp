#include <htmlanalyzer/automatons/title_automaton.hpp>

namespace html_analyzer {

TitleAutomaton::TitleAutomaton(PageInfo &info) : HTMLAutomaton(info)
{ }

void TitleAutomaton::update_impl(const GumboNode* node) {
    if (!is_node_text(node))
        return;

    auto parent = node->parent;
    if (parent && parent->v.element.tag == GUMBO_TAG_TITLE) {
        info_.title = std::string(node->v.text.text);
        exposed_ = true;
    }
}

} // namespace html_analyzer