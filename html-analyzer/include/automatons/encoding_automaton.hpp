#pragma once

#include <string>
#include <algorithm>
#include "automaton.hpp"

namespace html_analyzer {

class EncodingAutomaton final : public HTMLAutomaton {
public:
    EncodingAutomaton() = delete;
    EncodingAutomaton(PageInfo &info);

protected:
    void update_impl(const GumboNode* node) override final;
};

} // namespace html_analyzer