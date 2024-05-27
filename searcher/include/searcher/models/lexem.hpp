#pragma once

#include <string>
#include <userver/storages/postgres/io/io_fwd.hpp>

namespace se {

namespace searcher {

struct LexemEntryType {
  std::string name;
  std::string lang;
  float wlf;    
};

struct RegisteredLexem {
  std::int32_t id;
  float rank; 
};

} // namespace searcher

} // namespace se