#include <tqp/tools/stemmer.hpp>

namespace tqp {

Stemmer::Stemmer(Language lang) :
    name_ { get_algorithm_name_by_lang(lang) },
    stemmer_ { std::unique_ptr<sb_stemmer, Stemmer::stemmer_deleter>(
        sb_stemmer_new(name_.c_str(), nullptr)
    )}
{ }

Stemmer::Stemmer(Stemmer&& st) : 
    stemmer_{ std::move(st.stemmer_) }, 
    name_{ std::move(st.name_) }
{ }

std::vector<Language> Stemmer::get_available_languages() {
    std::vector<Language> res;
    for(const auto& cur : algorithm_by_langs_)
        res.push_back(cur.first);
    return res;
}

const std::string& Stemmer::get_target_language_name() const noexcept {
    return name_;
}

std::string Stemmer::get_stem(const std::string& target) const {
    return std::string(
        reinterpret_cast<const char*>(
            sb_stemmer_stem(
                stemmer_.get(),
                reinterpret_cast<const sb_symbol*>(target.c_str()),
                target.size()
            )
        )
    );
} 

std::string Stemmer::get_algorithm_name_by_lang(Language lang) const {
    auto type = lang.type();
    return algorithm_by_langs_.contains(type)
        ? algorithm_by_langs_.at(type)
        : algorithm_by_langs_.at(LanguageType::LT_UNKNOWN);
}

void Stemmer::stemmer_deleter::operator()(sb_stemmer* stemmer) {
    sb_stemmer_delete(stemmer);    
}

} // namespace tqp