#pragma once

#include <algorithm>
#include <tqp/token.hpp>

namespace tqp {

template<typename Pred>
struct UniqueTransformer {
    void operator()(TokenSequence& seq, Language lang) { 
        Pred pred;
        auto it = std::unique(seq.begin(), seq.end(), [&](auto& lhs, auto& rhs){
            return pred(lhs, rhs, lang);
        });
        seq.erase(it, seq.end());
    }
};

} // namespace tqp