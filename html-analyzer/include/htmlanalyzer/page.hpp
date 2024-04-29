#pragma once

#include <string>
#include <vector>
#include <unordered_set>

namespace html_analyzer {

struct PageInfo {
    bool can_index;
    bool can_follow;
    std::string encoding;
    std::string title;
    std::unordered_set<std::string> linked_uris;
};

} // namespace html_analyzer