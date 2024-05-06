#pragma once

#include <string>
#include <cstring>
#include "automaton.hpp"

namespace html_analyzer {

class KeywordsAutomaton final : public HTMLAutomaton {
public:
    KeywordsAutomaton() = delete;
    KeywordsAutomaton(PageInfo &info);

protected:
    void update_impl(const GumboNode* node) override;
};

} // namespace html-analyzer