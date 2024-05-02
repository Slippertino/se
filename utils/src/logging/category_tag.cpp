#include <seutils/logging/category_tag.hpp>

namespace se {

namespace utils {

namespace logging {

CategoryTag::CategoryTag(const char* name) :
    name_ { name }
{ }

void CategoryTag::format(std::string& out) const {
    out.append(name_);
}

} // namespace logging

} // namespace utils

} // namespace se