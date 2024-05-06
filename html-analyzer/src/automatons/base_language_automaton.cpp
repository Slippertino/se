#include <htmlanalyzer/automatons/base_language_automaton.hpp>

namespace html_analyzer {

const std::regex BaseLanguageAutomaton::language_pattern_ = std::regex(R"(^\s*(\w+)\s*(-\w+)*$)");
const std::string BaseLanguageAutomaton::default_language_ = "en";

BaseLanguageAutomaton::BaseLanguageAutomaton(PageInfo &info) : HTMLAutomaton(info)
{ }

BaseLanguageAutomaton::~BaseLanguageAutomaton() {
    if (info_.language.empty())
        info_.language = default_language_;
}

std::string BaseLanguageAutomaton::get_from_lang(const GumboNode* node) {
    std::string lang;
    auto attr = gumbo_get_attribute(&node->v.element.attributes, "lang");
    if (attr)
        lang = std::string(attr->value);
    return lang;
}

std::string BaseLanguageAutomaton::extract_lang(const std::string& lang) {
    std::string res;
    std::smatch sm;
    if (std::regex_match(lang, sm, language_pattern_)) {
        res = sm[1];
        tolower(res);
    }
    return res;
}

void BaseLanguageAutomaton::update_language(const GumboNode* node) {
    if (!is_node_element(node))
        return;
    auto tag = node->v.element.tag;
    if (tag != GUMBO_TAG_HTML && tag != GUMBO_TAG_META)
        return;
    std::string lang;
    if (tag == GUMBO_TAG_HTML)
        lang = get_from_lang(node);
    GumboAttribute* attr;
    if (tag == GUMBO_TAG_META && 
        (attr = gumbo_get_attribute(&node->v.element.attributes, "http-equiv"))
    ) {
        auto category = std::string(attr->value);
        tolower(category);
        if (category == "content-language") {
            attr = gumbo_get_attribute(&node->v.element.attributes, "content");
            lang = std::string(attr->value);
        }
    }
    if (!lang.empty() && info_.language.empty())
        info_.language = extract_lang(lang);
}

} // namespace html_analyzer