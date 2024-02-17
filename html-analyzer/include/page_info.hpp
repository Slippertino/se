#pragma once

#include <string>
#include <vector>
#include <unordered_set>

namespace html_analyzer {

struct PageInfo {
    std::string encoding;
    std::string title;
    std::unordered_set<std::string> linked_uris;
};

} // namespace html_analyzer