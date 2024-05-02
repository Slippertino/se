#include <htmlanalyzer/automatons/encoding_automaton.hpp>

namespace html_analyzer {

std::string EncodingAutomaton::default_encoding = "utf-8";

EncodingAutomaton::EncodingAutomaton(PageInfo &info) : HTMLAutomaton(info) { 
    info.encoding = default_encoding;
}

void EncodingAutomaton::update_impl(const GumboNode* node) {
    if (!is_node_element(node))
        return;
        
    if (node->v.element.tag != GUMBO_TAG_META)
        return;

    GumboAttribute* attr;
    if (attr = gumbo_get_attribute(&node->v.element.attributes, "charset")) {
        info_.encoding = std::string(attr->value);
        exposed_ = true;
    }
    else if (attr = gumbo_get_attribute(&node->v.element.attributes, "content")) {
        auto cont = std::string(attr->value);
        auto key = std::string{"charset="};
        auto it = std::remove(cont.begin(), cont.end(), ' ');
        cont.erase(it, cont.end());
        auto pos = cont.find(key);
        if (pos != std::string::npos) {
            info_.encoding = cont.substr(pos + key.size());
            exposed_ = true;
        }
    }
}

} // namespace html_analyzer