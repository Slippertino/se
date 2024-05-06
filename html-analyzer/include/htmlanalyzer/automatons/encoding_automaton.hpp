#pragma once

#include <string>
#include <regex>
#include "automaton.hpp"

namespace html_analyzer {

class EncodingAutomaton final : public HTMLAutomaton {
public:
    EncodingAutomaton() = delete;
    EncodingAutomaton(PageInfo &info);

    ~EncodingAutomaton();

protected:
    void update_impl(const GumboNode* node) override;

private:
    static const std::string default_encoding_;
    static const std::regex encoding_pattern_;
};

} // namespace html_analyzer