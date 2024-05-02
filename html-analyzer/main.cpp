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
    html_analyzer::LinkAutomaton<true>,
    html_analyzer::TitleAutomaton,
    html_analyzer::RobotHintsAutomaton
>;

int main(int argc, char** argv) {
    auto content = get_content(argv[1]);
    html_analyzer::HTMLAnalyzer obj{content};
    auto res = obj.analyze<MyAutomaton>();
    std::cout << std::boolalpha << res.can_index << "\n" << res.can_follow << "\n";
    std::cout << res.encoding << "\n";
    std::cout << res.linked_uris.size() << "\n";
    std::cout << res.title << "\n";
    std::cout << std::boolalpha << obj.is_valid() << "\n";

    return 0;
}