#pragma once

#include <string>
#include <regex>
#include <algorithm>
#include "automaton.hpp"

namespace html_analyzer {

class BaseLanguageAutomaton : public HTMLAutomaton {
public:
    BaseLanguageAutomaton() = delete;
    BaseLanguageAutomaton(PageInfo &info);

    ~BaseLanguageAutomaton();

protected:
    void update_impl(const GumboNode* node) = 0;
    void update_language(const GumboNode* node);

    static std::string get_from_lang(const GumboNode* node);
    static std::string extract_lang(const std::string& lang);

protected:
    static const std::string default_language_;
    static const std::regex language_pattern_;
};

} // namespace html_analyzer