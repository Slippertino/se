#include <htmlanalyzer/automatons/encoding_automaton.hpp>

namespace html_analyzer {

const std::string EncodingAutomaton::default_encoding_ = "utf-8";
const std::regex EncodingAutomaton::encoding_pattern_ = std::regex(R"(.*\s*charset=([^;\s]+).*)");

EncodingAutomaton::EncodingAutomaton(PageInfo &info) : HTMLAutomaton(info) 
{ }

EncodingAutomaton::~EncodingAutomaton() {
    if (info_.encoding.empty())
        info_.encoding = default_encoding_;
    tolower(info_.encoding);
}

void EncodingAutomaton::update_impl(const GumboNode* node) {
    if (!is_node_element(node) || node->v.element.tag != GUMBO_TAG_META)
        return;
    GumboAttribute* attr;
    if (attr = gumbo_get_attribute(&node->v.element.attributes, "charset")) {
        info_.encoding = std::string(attr->value);
        exposed_ = true;
    }
    else if (attr = gumbo_get_attribute(&node->v.element.attributes, "http-equiv")) {
        auto category = std::string(attr->value);
        tolower(category);
        if (category != "content-type") 
            return;
        attr = gumbo_get_attribute(&node->v.element.attributes, "content");
        std::smatch sm;
        std::string value = attr->value;
        if (std::regex_match(value, sm, encoding_pattern_)) {
            info_.encoding = sm[1];
            exposed_ = true;
        }
    }
}

} // namespace html_analyzer