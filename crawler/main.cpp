#include <iostream>
#include <fstream>
#include "src/parsers/sitemap/parser.hpp"
#include <robotstxt/robots.h>
#include <html_analyzer.hpp>

std::string get_content(const std::string &file) {
    std::ifstream in{file};
    in.seekg(0, std::ios::end);
    auto sz = in.tellg();
    std::vector<char> buf(sz);
    in.seekg(0, std::ios::beg);
    in.read(buf.data(), sz);
    return std::string(buf.begin(), buf.end());
}

int main() { 
    auto res = std::get<crawler::sitemap::SitemapUrlset>(crawler::sitemap::parse(get_content("example2")));
    for(auto &cur : res) {
        std::cout << cur.loc << " ";
        if (cur.lastModified.has_value())
            std::cout << cur.lastModified.value() << " ";
        if (cur.priority.has_value())
            std::cout << cur.priority.value() << " ";
        if (cur.changeFreq.has_value())
            std::cout << cur.changeFreq.value() << " ";   
        std::cout << "\n";
    }

    return 0;
}