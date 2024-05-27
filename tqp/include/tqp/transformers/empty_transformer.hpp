#pragma once

#include <tqp/token.hpp>

namespace tqp {

struct EmptyTransformer {
    void operator()(TokenSequence&, Language) { }
};

} // namespace tqp