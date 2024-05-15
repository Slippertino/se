#pragma once

#include <tqp/token.hpp>
#include <tqp/transformers/empty_transformer.hpp>
#include <list>

namespace tqp {

template<typename LexerType, typename Transformer = EmptyTransformer>
struct TQParser {
    TokenSequence operator()(const std::string& query, Language language) {
        auto tokens = LexerType{}(query, language);
        Transformer{}(tokens, language);
        return tokens;
    }
};

} // namespace tqp