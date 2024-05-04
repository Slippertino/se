#pragma once

#include <tqp/token.hpp>

namespace tqp {

template<typename Pred>
struct FilterTransformer {
    void operator()(TokenSequence& seq, Language lang) { 
        Pred pred;
        auto it = std::remove_if(seq.begin(), seq.end(), [&](auto& token){
            return pred(token, lang);
        });
        seq.erase(it, seq.end());
    }
};

} // namespace tqp