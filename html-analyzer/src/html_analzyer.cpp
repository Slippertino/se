#include "html_analyzer.hpp"
#include <iostream>

namespace html_analyzer {

HTMLAnalyzer::HTMLAnalyzer(const std::string &content) : 
    content_{content}, 
    document_{gumbo_parse(content.c_str())}
{ } 

[[nodiscard]] std::string HTMLAnalyzer::crop_content(const std::initializer_list<std::string> &forbidden_tags) {
    auto black_list = get_tags_types(forbidden_tags);

    std::vector<std::pair<size_t, size_t>> crops;
    exec_bfs([&crops, &black_list](const GumboNode* cur, bool &stop, bool &next) {
        if (cur->type != GUMBO_NODE_ELEMENT)
            return;
        auto &el = cur->v.element;
        if (!black_list.contains(el.tag))
            return;
        next = true;
        auto offset = el.start_pos.offset;
        auto size = (el.original_end_tag.length) 
            ? el.end_pos.offset + el.original_end_tag.length - el.start_pos.offset
            : el.original_tag.length;
        crops.push_back({ offset, size });
    });
    std::sort(crops.begin(), crops.end(), [](auto &p1, auto &p2) { return p1.first < p2.first; });

    auto res = content_;
    size_t offset{0};
    for(const auto &cr : crops) {
        res.replace(cr.first - offset, cr.second, "");
        offset += cr.second;
    }

    return res;
}

HTMLAnalyzer::~HTMLAnalyzer() {
    gumbo_destroy_output(&kGumboDefaultOptions, document_);
}

bool HTMLAnalyzer::is_node_element(const GumboNode* node) {
    return node->type == GUMBO_NODE_ELEMENT;
}

std::unordered_set<GumboTag> HTMLAnalyzer::get_tags_types(const std::initializer_list<std::string> &forbidden_tags) {
    std::unordered_set<GumboTag> black_list;
    for(auto &t : forbidden_tags)
        black_list.insert(gumbo_tag_enum(t.c_str()));
    return black_list;
}

void HTMLAnalyzer::exec_bfs(const std::function<void(const GumboNode*, bool&, bool&)> &callback) {
    std::queue<GumboNode*> q;
    q.push(document_->root);

    while(!q.empty()) {
        auto cur = q.front();
        q.pop();

        bool stop{false}, next{false};
        callback(cur, stop, next);

        if (stop) return;
        if (next) continue;

        if (is_node_element(cur)) {
            GumboVector* children = &cur->v.element.children;
            for (size_t i = 0; i < children->length; ++i)
                q.push(static_cast<GumboNode*>(children->data[i]));
        }           
    }
}

} // namespace html_analyzer