#include <htmlanalyzer/automatons/automaton.hpp>

namespace html_analyzer {

HTMLAutomaton::HTMLAutomaton(PageInfo &info) : info_{info}
{ }

bool HTMLAutomaton::is_exposed() const noexcept {
    return exposed_;
}

void HTMLAutomaton::update(const GumboNode* node) {
    if (exposed_) return;
    update_impl(node);
}

HTMLAutomaton::~HTMLAutomaton() {}

bool HTMLAutomaton::is_node_text(const GumboNode* node) {
    return node->type == GUMBO_NODE_TEXT;
}

bool HTMLAutomaton::is_node_element(const GumboNode* node) {
    return node->type == GUMBO_NODE_ELEMENT;
}

} // namespace html_analyzer