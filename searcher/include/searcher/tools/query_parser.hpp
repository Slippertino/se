#pragma once

#include <string>
#include <vector>
#include <searcher/config/search_options.hpp>
#include <searcher/models/lexem.hpp>
#include <tqp/parser.hpp>
#include <tqp/transformers/transformers.hpp>
#include <tqp/lexers/basic_lexer.hpp>

namespace se {

namespace searcher {

struct QueryParser {
    std::vector<LexemEntryType> operator()(const std::string& query, const SearchOptions& opts);
};

} // namespace searcher

} // namespace se