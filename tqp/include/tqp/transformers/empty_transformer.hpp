#pragma once

#include <tqp/token.hpp>

namespace tqp {

struct EmptyTransformer {
    void operator()(TokenSequence& seq, Language lang) { }
};

} // namespace tqp