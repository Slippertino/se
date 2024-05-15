#pragma once

#include <string>
#include <list>
#include <unicode/uchar.h>
#include <unicode/unistr.h>
#include <tqp/language/language.hpp>

namespace tqp {

enum class TokenType {
    NUMBER,
    TEXT
};

struct Token {
    TokenType type;
    int pos;
    size_t token_symbols_count;
    size_t source_symbols_count;
    Language lang;
    std::string token;
    std::string source;
};

using TokenSequence = std::list<Token>;

std::ostream& operator<<(std::ostream& os, const Token& token);

} // namespace tqp