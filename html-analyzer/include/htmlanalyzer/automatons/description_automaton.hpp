#pragma once

#include <string>
#include <cstring>
#include "automaton.hpp"

namespace html_analyzer {

class DescriptionAutomaton final : public HTMLAutomaton {
public:
    DescriptionAutomaton() = delete;
    DescriptionAutomaton(PageInfo &info);

protected:
    void update_impl(const GumboNode* node) override;
};

} // namespace html-analyzer