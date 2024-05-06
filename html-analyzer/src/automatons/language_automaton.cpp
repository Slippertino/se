#include <htmlanalyzer/automatons/language_automaton.hpp>

namespace html_analyzer {

LanguageAutomaton::LanguageAutomaton(PageInfo &info) : BaseLanguageAutomaton(info)
{ }

void LanguageAutomaton::update_impl(const GumboNode* node) {
    update_language(node);
    if (!info_.language.empty())
        exposed_ = true;
}

} // namespace html_analyzer