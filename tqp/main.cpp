#include <tqp/parser.hpp>
#include <tqp/transformers/transformers.hpp>
#include <tqp/lexers/basic_lexer.hpp>

using TextParser = tqp::TQParser<
    tqp::BasicLexer, 
    tqp::CombinedTransformer<
        tqp::WordsCaseTransformer<>,
        tqp::StopWordsTransformer,
        tqp::StemmingTransformer
    >
>;

int main(int argc, char** argv) {
    std::string query;
    while(std::getline(std::cin, query)) {
        for(auto&& token : TextParser{}(query, tqp::LanguageType::LT_ENGLISH)) {
            std::cout << token.token << "\n";
        }
    }
    return 0;
}