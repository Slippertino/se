#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <tqp/language/language.hpp>

namespace tqp {

class StopWordsChecker final {
public:
    using StopWordsContainer = std::unordered_map<Language, std::unordered_set<std::string>>;

public:
    static bool is_stop_word(const std::string& word, Language lang);

    template<typename It>
    static void copy_stop_words(Language lang, It dest) {
        if (!container_.contains(lang))
            return;
        auto& cont = container_.at(lang);
        std::copy(cont.begin(), cont.end(), dest);     
    }

private:
    static StopWordsContainer load_data();

private:
    static const std::filesystem::path base_data_path_;
    static const std::unordered_map<Language, std::filesystem::path> relative_source_paths_;
    static const StopWordsContainer container_;
};

} // namespace tqp