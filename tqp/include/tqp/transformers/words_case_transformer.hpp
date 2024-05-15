#pragma once

#include <tqp/token.hpp>

namespace tqp {

template<bool Down = true>
struct WordsCaseTransformer {
    void operator()(TokenSequence& seq, Language lang) { 
        for(auto& cur : seq) {
            icu::UnicodeString us(cur.source.c_str());
            if constexpr (Down)
                us.toLower();
            else
                us.toUpper();
            cur.source.clear();
            us.toUTF8String(cur.source);
        }
    }
};

} // namespace tqp