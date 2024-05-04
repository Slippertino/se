#pragma once

#include <memory>
#include <string>
#include <libstemmer.h>
#include <tqp/language/language.hpp>

namespace tqp {

class Stemmer final {
public:
    Stemmer() = delete;
    Stemmer(Language lang);
    Stemmer(Stemmer&& st);

    static std::vector<Language> get_available_languages();
    
    const std::string& get_target_language_name() const noexcept;
    std::string get_stem(const std::string& target) const;

private:
    std::string get_algorithm_name_by_lang(Language lang) const;

private:
    static inline const std::unordered_map<Language, std::string> algorithm_by_langs_ = {
        { LanguageType::LT_UNKNOWN,		"unknown"	 },
        { LanguageType::LT_ENGLISH,		"english"    },
        { LanguageType::LT_RUSSIAN,		"russian"    },
    };

private:
    struct stemmer_deleter {
        void operator()(sb_stemmer* stemmer);
    };

private:
    std::string name_;
    std::unique_ptr<sb_stemmer, stemmer_deleter> stemmer_;
}; 

} // namespace tqp
