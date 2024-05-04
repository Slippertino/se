#pragma once

#include <string>
#include <list>
#include <tqp/token.hpp>

namespace tqp {

class BasicLexer final {
public:
    TokenSequence operator()(const std::string& text, Language lang);

private:
    static inline const std::string white_symbols_string = " \t\r,";
    static inline const std::unordered_set<char> white_symbols_set = std::unordered_set<char>(
        white_symbols_string.begin(), 
        white_symbols_string.end()
    );

    static inline const std::string removable_symbols_string = "-'\\@:\".[]()!?;";
    static inline const std::unordered_set<char> removable_symbols_set = std::unordered_set<char>(
        removable_symbols_string.begin(), 
        removable_symbols_string.end()
    );
};

} // namespace tqp