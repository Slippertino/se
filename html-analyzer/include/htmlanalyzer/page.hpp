#pragma once

#include <string>
#include <vector>
#include <unordered_set>

namespace html_analyzer {

struct TextExcerpt {
    size_t pos;
    double rank;
    std::string lang;
    std::string text;
};

struct PageInfo {
    bool can_index;
    bool can_follow;
    std::string encoding;
    std::string language;
    std::string title;
    std::string description;
    std::string keywords;
    std::unordered_set<std::string> linked_uris;
    std::vector<TextExcerpt> excerpts;
};

template<typename CustomType>
struct ExtraPageInfo : PageInfo {
    CustomType custom;
};


} // namespace html_analyzer