#pragma once

#include <string>
#include <list>
#include <tqp/language/language.hpp>

namespace tqp {

struct Token {
    int pos;
    Language lang;
    std::string token;
    std::string source;
};

using TokenSequence = std::list<Token>;

std::ostream& operator<<(std::ostream& os, const Token& token);

} // namespace tqp