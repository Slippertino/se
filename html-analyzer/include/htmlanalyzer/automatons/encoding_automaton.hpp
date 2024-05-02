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

private:
    static std::string default_encoding;
};

} // namespace html_analyzer