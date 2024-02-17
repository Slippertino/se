#include "automatons/encoding_automaton.hpp"

namespace html_analyzer {

EncodingAutomaton::EncodingAutomaton(PageInfo &info) : HTMLAutomaton(info)
{ }

void EncodingAutomaton::update_impl(const GumboNode* node) {
    if (!is_node_element(node))
        return;
        
    GumboAttribute* attr;
    if (node->v.element.tag == GUMBO_TAG_META) {
        if (attr = gumbo_get_attribute(&node->v.element.attributes, "charset")) {
            info_.encoding = std::string(attr->value);
            exposed_ = true;
        }
        else if (attr = gumbo_get_attribute(&node->v.element.attributes, "content")) {
            auto cont = std::string(attr->value);
            auto key = std::string{"charset="};
            std::remove(cont.begin(), cont.end(), ' ');
            if (cont.find(key) != std::string::npos) {
                info_.encoding = cont.substr(cont.find(key) + key.size());
                exposed_ = true;
            }
        }
    }
}

} // namespace html_analyzer