#pragma once

#include <algorithm>
#include <tqp/token.hpp>

namespace tqp {

template<typename Func>
struct ForEachTransformer {
    void operator()(TokenSequence& seq, Language lang) { 
        std::for_each(seq.begin(), seq.end(), [lang](Token& token){
            Func{}(token, lang);
        });
    }
};

} // namespace tqp