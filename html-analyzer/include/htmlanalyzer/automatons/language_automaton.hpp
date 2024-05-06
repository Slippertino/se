#pragma once

#include "base_language_automaton.hpp"

namespace html_analyzer {

class LanguageAutomaton final : public BaseLanguageAutomaton {
public:
    LanguageAutomaton() = delete;
    LanguageAutomaton(PageInfo &info);

protected:
    void update_impl(const GumboNode* node) override;
};

} // namespace html-analyzer