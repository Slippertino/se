#pragma once

#include <string>
#include <quill/Quill.h>

namespace crawler {

namespace logging {

class CategoryTag : public quill::CustomTags {
public:
    constexpr CategoryTag(const char* name) :
        name_ { name }
    { }

    void format(std::string& out) const override {
        out.append(name_);
    }

private:
    const char* name_;
};

} // namespace logging 

} // namespace crawler