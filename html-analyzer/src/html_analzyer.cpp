#include <htmlanalyzer/html_analyzer.hpp>

#include <iostream>

namespace html_analyzer {

const std::regex HTMLAnalyzer::html_pattern_ = std::regex(
    R"(\s*<!DOCTYPE html>.*)", 
    std::regex_constants::grep
);

HTMLAnalyzer::HTMLAnalyzer(const std::string &content) : 
    content_{ content }, 
    document_{ gumbo_parse(content.c_str()) }
{ } 

bool HTMLAnalyzer::is_valid() const {
    return std::regex_match(content_, html_pattern_);
}

[[nodiscard]] std::string HTMLAnalyzer::crop_content(
    const std::initializer_list<std::string> &forbidden_tags,
    bool with_comments,
    bool with_whitespaces
) {
    auto black_list = get_tags_types(forbidden_tags);
    using crop_t = std::pair<size_t, size_t>;
    std::vector<crop_t> crops;
    exec_bfs([&crops, &black_list, with_comments, with_whitespaces](const GumboNode* cur, bool &stop, bool &next) {
        if (!with_whitespaces && cur->type == GUMBO_NODE_WHITESPACE ||
            !with_comments && cur->type == GUMBO_NODE_COMMENT)
            return;
        crop_t crop;
        switch (cur->type)
        {
        case GUMBO_NODE_WHITESPACE:
            [[fallthrough]];
        case GUMBO_NODE_COMMENT:
        {
            auto& el = cur->v.text;
            crop.first = el.start_pos.offset;
            crop.second = el.original_text.length;
            break;
        }
        case GUMBO_NODE_ELEMENT:
        {
            auto &el = cur->v.element;
            if (!black_list.contains(el.tag) && !is_element_empty(cur))
                return;
            next = true;
            crop.first = el.start_pos.offset;
            crop.second = (el.original_end_tag.length) 
                ? el.end_pos.offset + el.original_end_tag.length - el.start_pos.offset
                : el.original_tag.length;
            break;
        } 
        default:
            return;
        }
        crops.push_back(crop);
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

bool HTMLAnalyzer::is_element_empty(const GumboNode* node) {
    auto& el = node->v.element;
    return el.start_pos.offset + el.original_tag.length + el.original_end_tag.length == el.end_pos.offset;
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