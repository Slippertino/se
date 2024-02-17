#include <iostream>
#include <vector>
#include <fstream>
#include "html_analyzer.hpp"
#include "automatons/automatons.hpp"

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
    html_analyzer::LinkAutomaton,
    html_analyzer::TitleAutomaton
>;

int main() {
    auto content = get_content("example.html");
    html_analyzer::HTMLAnalyzer obj{content};
    std::cout << obj.crop_content({"meta", "a", "div"}) << "\n";

    return 0;
}