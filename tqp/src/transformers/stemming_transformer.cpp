#include <tqp/transformers/stemming_transformer.hpp>

namespace tqp {

void StemmingTransformer::operator()(TokenSequence& seq, Language lang) {
    auto& map = get_thread_resource();
    if (!map.contains(lang))
        lang = LanguageType::LT_UNKNOWN;
    auto& stemmer = map.at(lang);
    std::for_each(seq.begin(), seq.end(), [&stemmer](auto& v) {
        v.token = stemmer.get_stem(v.source);
    });
}

} // namespace tqp