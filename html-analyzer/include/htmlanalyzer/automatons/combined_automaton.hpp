#pragma once

#include <string>
#include <memory>
#include <array>
#include "automaton.hpp"

namespace html_analyzer {

template<class... Automatons>
class CombinedAutomaton final : public HTMLAutomaton {
public:
    CombinedAutomaton() = delete;
    CombinedAutomaton(PageInfo &info) : 
        HTMLAutomaton(info),
        automatons_{std::make_unique<Automatons>(info)...}
    { }

protected:
    void update_impl(const GumboNode* node) override {
        exposed_ = true;
        for(auto &am : automatons_) {
            am->update(node);
            exposed_ &= am->is_exposed();
        }
    }

private:
    std::array<std::unique_ptr<HTMLAutomaton>, sizeof...(Automatons)> automatons_;
};

} // namespace html_analyzer