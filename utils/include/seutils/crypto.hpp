#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

namespace se {

namespace utils {

std::string sha256(const std::string& str);

} // namespace utils

} // namespace se