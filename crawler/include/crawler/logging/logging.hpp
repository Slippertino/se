#pragma once

#include <seutils/logging/logging.hpp>
#include <seutils/logging/category_tag.hpp>

namespace se {

namespace crawler {

namespace logging {

static inline const se::utils::logging::CategoryTag distributor_category   { "DISTRIBUTOR" };
static inline const se::utils::logging::CategoryTag queue_category         { "QUEUE"       };
static inline const se::utils::logging::CategoryTag processor_category     { "PROCESSOR"   };
static inline const se::utils::logging::CategoryTag handler_category       { "HANDLER"     };

} // namespace logging

} // namespace crawler

} // namespace se