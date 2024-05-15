#pragma once

#include <tqp/token.hpp>

namespace tqp {

template<typename... Transformers>
struct CombinedTransformer {
    void operator()(TokenSequence& seq, Language lang) { 
        if (sizeof...(Transformers))
            (Transformers{}(seq, lang), ...);
    }
};

} // namespace tqp