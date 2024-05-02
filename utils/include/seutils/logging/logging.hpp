#pragma once

#define QUILL_USE_BOUNDED_QUEUE
#define QUILL_ROOT_LOGGER_ONLY

#include <quill/Quill.h>
#include <quill/TransitEvent.h>
#include <quill/handlers/Handler.h>
#include <quill/Utility.h>
#include "category_tag.hpp"

namespace se {

namespace utils {

namespace logging {

static inline const CategoryTag main_category   { "MAIN"        };
static inline const CategoryTag db_category     { "DB"          };
static inline const CategoryTag bus_category    { "BUS"         };

} // namespace logging

} // namespace utils

} // namespace se