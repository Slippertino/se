#pragma once

#include "filter_transformer.hpp"
#include <tqp/tools/stop_words_checker.hpp>

namespace tqp {

using StopWordsTransformer = FilterTransformer<decltype([](Token& token, Language lang) {
    return StopWordsChecker::is_stop_word(token.source, lang);
})>;

} // namespace tqp