#include <searcher/tools/query_parser.hpp>

namespace se {

namespace searcher {

std::vector<LexemEntryType> QueryParser::operator()(const std::string& query, const SearchOptions& opts) {
    using TextParser = tqp::TQParser<
        tqp::BasicLexer, 
        tqp::CombinedTransformer<
            tqp::WordsCaseTransformer<>,
            tqp::StopWordsTransformer,
            tqp::StemmingTransformer,
            tqp::UniqueTransformer<decltype([](auto& lhs, auto& rhs, auto) {
                return lhs.token == rhs.token;
            })>
        >
    >;
    
    std::vector<LexemEntryType> res;
    for(auto&& lang_entry : tqp::Language::find_top_nmost_freq_langs(query, opts.language_threshold, opts.max_query_languages_count)) {
        auto lang = lang_entry.lang;
        auto excerpt = query.substr(lang_entry.begin, lang_entry.end);
        if (lang.type() == tqp::LanguageType::LT_UNKNOWN)
            lang = tqp::Language(opts.default_language);
        auto lang_name = lang.name();
        for(auto&& token : TextParser{}(excerpt, lang))
            res.emplace_back(token.token, lang_name, lang_entry.probability);
    }
    return res;
}

} // namespace searcher

} // namespace se