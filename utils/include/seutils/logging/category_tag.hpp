#pragma once

#include <string>
#include <quill/Quill.h>

namespace se {

namespace utils {

namespace logging {

class CategoryTag : public quill::CustomTags {
public:
    CategoryTag(const char* name);
    void format(std::string& out) const override;

private:
    const char* name_;
};

} // namespace logging

} // namespace utils

} // namespace se