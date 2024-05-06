#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <boost/algorithm/string.hpp> 
#include "automaton.hpp"

namespace html_analyzer {

class RobotHintsAutomaton final : public HTMLAutomaton {
public:
    RobotHintsAutomaton() = delete;
    RobotHintsAutomaton(PageInfo &info);
    
protected:
    void update_impl(const GumboNode* node) override;

private:
    using Hint = int;

    struct TargetHint {
        Hint value;
        bool modified;

        void assign(Hint source);
    };

    struct Modifier {
        Hint index;
        Hint follow;
    };

private:
    void update_hints();
    void apply_modifier(const Modifier& mod);

private:
    TargetHint index_;
    TargetHint follow_;

private:
    static const std::vector<std::string> keys_;
    static const std::unordered_map<std::string, Modifier> modifiers_ ;
};

} // namespace html-analyzer