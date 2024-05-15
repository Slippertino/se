#include <iostream>
#include <vector>
#include <fstream>
#include "htmlanalyzer/html_analyzer.hpp"
#include "htmlanalyzer/automatons/automatons.hpp"

std::string get_content(const std::string &file) {
    std::ifstream in{file};
    in.seekg(0, std::ios::end);
    auto sz = in.tellg();
    std::vector<char> buf(sz);
    in.seekg(0, std::ios::beg);
    in.read(buf.data(), sz);
    return std::string(buf.begin(), buf.end());
}

using MyAutomaton = html_analyzer::CombinedAutomaton<
    html_analyzer::EncodingAutomaton,
    html_analyzer::LanguageAutomaton,
    html_analyzer::DescriptionAutomaton,
    html_analyzer::TextAutomaton<decltype([](std::string){ return 1.0; })>,
    html_analyzer::KeywordsAutomaton,
    html_analyzer::LinkAutomaton<true>,
    html_analyzer::TitleAutomaton,
    html_analyzer::RobotHintsAutomaton
>;

int main(int argc, char** argv) {
    auto content = get_content(argv[1]);
    html_analyzer::HTMLAnalyzer obj{content};
    auto res = obj.analyze<MyAutomaton>();
    std::cout << obj.is_valid() << "\n";
    for(const auto& ex : res.excerpts) 
        std::cout << ex.pos << " " << ex.lang << " " << ex.rank << " " << ex.text << "\n";
    return 0;
}