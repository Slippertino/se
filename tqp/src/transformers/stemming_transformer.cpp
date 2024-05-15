#include <tqp/transformers/stemming_transformer.hpp>

namespace tqp {

void StemmingTransformer::operator()(TokenSequence& seq, Language lang) {
    auto& map = get_thread_resource();
    if (!map.contains(lang))
        lang = LanguageType::LT_UNKNOWN;
    auto& stemmer = map.at(lang);
    for(auto it = seq.begin(); it != seq.end(); ) {
        if (it->type != TokenType::TEXT) {
            ++it;
            continue;
        }
        it->token = stemmer.get_stem(it->source);
        it->token_symbols_count = icu::UnicodeString(it->token.c_str()).countChar32();
        if (it->token.empty()) 
            seq.erase(it++);
        else 
            ++it;
    }
}

} // namespace tqp