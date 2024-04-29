#pragma once

#define QUILL_USE_BOUNDED_QUEUE
#define QUILL_ROOT_LOGGER_ONLY

#include <quill/Quill.h>
#include <quill/TransitEvent.h>
#include <quill/handlers/Handler.h>
#include <quill/Utility.h>
#include "tags.hpp"

namespace crawler {

namespace logging {

static constexpr CategoryTag main_category          { "MAIN"        };
static constexpr CategoryTag distributor_category   { "DISTRIBUTOR" };
static constexpr CategoryTag queue_category         { "QUEUE"       };
static constexpr CategoryTag db_category            { "DB"          };
static constexpr CategoryTag processor_category     { "PROCESSOR"   };
static constexpr CategoryTag handler_category       { "HANDLER"     };
static constexpr CategoryTag bus_category           { "BUS"         };

} // namespace logging

} // namespace crawler