#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <queue>
#include <boost/regex.hpp>
#include <concepts>
#include <gumbo.h>
#include <error.h>
#include "page.hpp"

namespace html_analyzer {

class HTMLAnalyzer final {
public:
    HTMLAnalyzer() = delete;
    HTMLAnalyzer(const std::string &content);

    HTMLAnalyzer(const HTMLAnalyzer& rhs) = delete;
    HTMLAnalyzer& operator=(const HTMLAnalyzer& rhs) = delete;

    bool is_valid() const;
    
    [[nodiscard]] std::string crop_content(
        const std::vector<std::string> &forbidden_tags = {},
        bool with_comments = true,
        bool with_whitespaces = true
    );

    template<class TAutomaton, std::derived_from<PageInfo> InfoType = PageInfo>
    [[nodiscard]] InfoType analyze(const std::vector<std::string> &forbidden_tags = {}) {
        auto black_list = get_tags_types(forbidden_tags);
        InfoType res;
        TAutomaton am{res};
        exec_bfs([&black_list, &am](const GumboNode* cur, bool &stop, bool &next) {
            am.update(cur);
            if (cur->type == GUMBO_NODE_ELEMENT && black_list.contains(cur->v.element.tag)) 
                next = true;
            if (am.is_exposed()) 
                stop = true;
        });
        return res;
    }

    ~HTMLAnalyzer();

private:
    static bool is_node_element(const GumboNode* node);
    static bool is_element_empty(const GumboNode* node);
    static std::unordered_set<GumboTag> get_tags_types(const std::vector<std::string> &forbidden_tags);

    void exec_bfs(const std::function<void(const GumboNode*, bool&, bool&)> &callback);

private:
    static const boost::regex html_pattern_;

private:
    std::string content_;
    GumboOutput *document_;
};

} // namespace html_analyzer