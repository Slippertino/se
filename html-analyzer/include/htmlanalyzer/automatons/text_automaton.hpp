#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include "base_language_automaton.hpp"

namespace html_analyzer {

template<typename RankerType, typename RankAggregator = std::plus<double>>
class TextAutomaton final : public BaseLanguageAutomaton {
public:
    TextAutomaton() = delete;
    TextAutomaton(PageInfo &info) : BaseLanguageAutomaton(info)
    { }

    ~TextAutomaton() {
        auto lang = !info_.language.empty() 
            ? info_.language 
            : default_language_;
        for(auto& ex : info_.excerpts)
            if (ex.lang.empty())
                ex.lang = lang;
    }

protected:
    void update_impl(const GumboNode* node) override {
        if (info_.language.empty())
            update_language(node);
        if (is_node_text(node))
            update_text(node);
        else if (is_node_element(node))
            update_element(node);
    }

    void update_element(const GumboNode* node) {
        auto tag = node->v.element.tag;
        auto parent = node->parent;
        NodeState state;
        auto lang = get_from_lang(node);
        if (!lang.empty())
            state.lang = extract_lang(lang);
        state.rank = RankerType{}(tag);
        if (states_.contains(parent)) {
            const auto& pstate = states_.at(parent);
            if (state.lang.empty())
                state.lang = pstate.lang;
            state.rank = RankAggregator{}(state.rank, pstate.rank);
        }
        if (!state.empty())
            states_.insert({ node, std::move(state) });
    }

    void update_text(const GumboNode* node) {
        auto parent = node->parent;
        TextExcerpt text;
        text.text = node->v.text.text;
        text.pos = node->v.text.start_pos.offset;
        if (!states_.contains(parent)) {
            text.lang = info_.language;
            text.rank = 1.0;
        }
        else {
            const auto& state = states_.at(parent);
            text.lang = state.lang;
            text.rank = state.rank;
        }
        info_.excerpts.push_back(std::move(text));
    }

private:
    struct NodeState {
        double rank;
        std::string lang;

        bool empty() const noexcept {
            return !rank && lang.empty();
        }
    };

    std::unordered_map<const GumboNode*, NodeState> states_;
};

} // namespace html-analyzer