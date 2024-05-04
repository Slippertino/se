#pragma once

#include <tqp/token.hpp>
#include <list>

namespace tqp {

template<typename LexerType, typename... Transformers>
struct TQParser {
    TokenSequence operator()(const std::string& query, Language language) {
        auto tokens = LexerType{}(query, language);
        if (sizeof...(Transformers))
            (Transformers{}(tokens, language), ...);
        return tokens;
    }
};

} // namespace tqp