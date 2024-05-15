#include <tqp/lexers/basic_lexer.hpp>

namespace tqp {
    
TokenSequence BasicLexer::operator()(const std::string& text, Language lang) {
    TokenSequence res;
    std::string word;
    size_t word_pos{ std::string::npos };

    auto flush = [&]() {
        if (word_pos == std::string::npos)
            return;
        auto uword = icu::UnicodeString(word.c_str());
        auto count = uword.countChar32();
        auto type = is_number(uword) ? TokenType::NUMBER : TokenType::TEXT; 
        res.emplace_back(type, word_pos, count, count, lang, word, word);
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

bool BasicLexer::is_number(const icu::UnicodeString& text) {
    bool flag{ true };
    for(auto i = 0; i < text.countChar32() && flag; ++i)
        flag &= u_hasBinaryProperty(text.char32At(i), UCHAR_HEX_DIGIT);
    return flag;  
}

} // namespace tqp