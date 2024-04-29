#pragma once

#include <string>
#include "automaton.hpp"

namespace html_analyzer {

class TitleAutomaton final : public HTMLAutomaton {
public:
    TitleAutomaton() = delete;
    TitleAutomaton(PageInfo &info);

protected:
    void update_impl(const GumboNode* node) override final;
};

} // namespace html-analyzer