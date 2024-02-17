#pragma once

#include <string>
#include "automaton.hpp"

namespace html_analyzer {

class LinkAutomaton final : public HTMLAutomaton {
public:
    LinkAutomaton() = delete;
    LinkAutomaton(PageInfo &info);

protected:
    void update_impl(const GumboNode* node) override final;
};

} // namespace html-analyzer