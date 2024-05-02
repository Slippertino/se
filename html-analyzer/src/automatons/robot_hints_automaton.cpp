#include <htmlanalyzer/automatons/robot_hints_automaton.hpp>

namespace html_analyzer {

RobotHintsAutomaton::RobotHintsAutomaton(PageInfo &info) : 
    HTMLAutomaton(info),
    index_{ 1, false },
    follow_{ 1, false }
{ 
    info.can_index = true;
    info.can_follow = true;
}

void RobotHintsAutomaton::update_impl(const GumboNode* node) {
    if (!is_node_element(node) || node->v.element.tag != GUMBO_TAG_META)
        return;
    auto name_attr = gumbo_get_attribute(&node->v.element.attributes, "name");
    if (!name_attr)
        return;
    auto name = std::string(name_attr->value);
    bool matched = false;
    for(const auto& vn : keys_) {
        if (vn == name) {
            matched = true;
            break;
        }
    }
    if (!matched)
        return;
    auto content_attr = gumbo_get_attribute(&node->v.element.attributes, "content");
    if (!content_attr)
        return;
    auto content = std::string{ content_attr->value };
    std::vector<std::string> mods;
    boost::split(mods, content, boost::is_any_of("\t\n ,;"));

    for (const auto& mod : mods) {
        if (!modifiers_.contains(mod)) 
            continue;
        apply_modifier(modifiers_.at(mod));
    }
    update_hints();
}


void RobotHintsAutomaton::TargetHint::assign(Hint source) {
    if (!source)
        return;
    if (!modified) {
        value = source;
        modified = true;
    }
    else {
        if (source > value)
            value = source;
    }    
}

void RobotHintsAutomaton::update_hints() {
    info_.can_index = index_.value == 1;
    info_.can_follow = follow_.value == 1;
}

void RobotHintsAutomaton::apply_modifier(const Modifier& mod) {
    index_.assign(mod.index);
    follow_.assign(mod.follow);
}

const std::vector<std::string> RobotHintsAutomaton::keys_ = {
    "robots"
};

const std::unordered_map<std::string, RobotHintsAutomaton::Modifier> RobotHintsAutomaton::modifiers_ = {
    { "all",        Modifier{ 1,  1 }  },
    { "noindex",    Modifier{ -1, 0 }  },
    { "index",      Modifier{ 1,  0 }  },
    { "nofollow",   Modifier{ 0, -1 }  },
    { "follow",     Modifier{ 0,  1 }  },
};

} // namespace html-analyzer