#include <tqp/lexers/basic_lexer.hpp>

namespace tqp {
    
TokenSequence BasicLexer::operator()(const std::string& text, Language lang) {
    TokenSequence res;
    std::string word;
    size_t word_pos{ std::string::npos };

    auto flush = [&]() {
        if (word_pos == std::string::npos)
            return;
        res.emplace_back(word_pos, lang, word, word);
        word.clear();
        word_pos = std::string::npos;
    };

    for(auto i = 0; i < text.size(); ) {
        auto ch = text[i];
        if (white_symbols_set.contains(ch)) {
            for(; i < text.size() && white_symbols_set.contains(text[i]); ++i);
            flush();
            continue;
        }
        if (removable_symbols_set.contains(ch)) {
            ++i;
            continue;
        }
        if (word_pos == std::string::npos)
            word_pos = i;
        word += ch;
        ++i;
    }
    flush();
    return res;
}

} // namespace tqp