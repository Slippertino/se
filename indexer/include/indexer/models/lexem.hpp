#pragma once

#include <string>
#include <boost/container_hash/hash_fwd.hpp>
#include <tqp/language/language.hpp>

namespace se {

namespace indexer {

struct Lexem {
    tqp::Language lang;
    std::string value;

    bool operator==(const Lexem& lex) const noexcept {
        return lang.type() == lex.lang.type() && value == lex.value;
    }
};

struct LexemEntry {
    Lexem lexem;
    
    /* Weighted lexem frequency */
    double wlf;
};

} // namespace indexer

} // namespace se

namespace std {

template<>
struct hash<se::indexer::Lexem> {
    size_t operator()(const se::indexer::Lexem& lex) const noexcept {
        size_t res{13};
        boost::hash_combine(res, lex.value);
        boost::hash_combine(res, lex.lang.number());
        return res;
    }
};

} // namespace std