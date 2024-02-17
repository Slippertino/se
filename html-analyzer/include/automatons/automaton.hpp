#pragma once

#include <vector>
#include <gumbo.h>
#include "../page_info.hpp"

namespace html_analyzer {

class HTMLAutomaton {
public:
    HTMLAutomaton() = delete;
    HTMLAutomaton(PageInfo &info);

    bool is_exposed() const noexcept;
    void update(const GumboNode* node);

    virtual ~HTMLAutomaton();

protected:
    virtual void update_impl(const GumboNode* node) = 0;

    static bool is_node_text(const GumboNode* node);
    static bool is_node_element(const GumboNode* node);

protected:
    PageInfo &info_;
    bool exposed_ = false;
};

} // namespace html_analyzer