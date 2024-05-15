#pragma once

#include <tqp/token.hpp>

namespace tqp {

struct RemoveEmptiesTransformer {
    void operator()(TokenSequence& seq, Language lang) { 
        for(auto it = seq.begin(); it != seq.end();) {
            if (it->token.empty()) 
                seq.erase(it++);
            else 
                ++it;
        }
    }
};

} // namespace tqp