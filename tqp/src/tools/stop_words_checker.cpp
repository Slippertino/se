#include <fstream>
#include <tqp/tools/stop_words_checker.hpp>

namespace tqp {

bool StopWordsChecker::is_stop_word(const std::string& word, Language lang) {
    if (!container_.contains(lang))
        return false;
	auto& cont = container_.at(lang);
	return cont.contains(word);
}

StopWordsChecker::StopWordsContainer StopWordsChecker::load_data() {
    StopWordsChecker::StopWordsContainer res;
    for(const auto& lang_source : relative_source_paths_) {
        auto lang = lang_source.first;
        auto path = base_data_path_ / lang_source.second;
        std::ifstream in;
        in.open(path);
        if (!in.is_open())
            continue;
        std::unordered_set<std::string> set;
        std::string word;
        while(in >> word) 
            set.insert(word);
        in.close();
        res.insert({ lang, std::move(set) });
    }
    return res;
}

const std::filesystem::path StopWordsChecker::base_data_path_ = std::filesystem::path(R"(./data)");
const std::unordered_map<Language, std::filesystem::path> StopWordsChecker::relative_source_paths_ = {
	{ LanguageType::LT_ENGLISH,  std::filesystem::path(R"(stop_words/english)") },
	{ LanguageType::LT_RUSSIAN,  std::filesystem::path(R"(stop_words/russian)") },
};
const StopWordsChecker::StopWordsContainer StopWordsChecker::container_ = StopWordsChecker::load_data();

} // namespace tqp
