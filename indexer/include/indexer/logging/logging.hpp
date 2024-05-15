#pragma once

#include <seutils/logging/logging.hpp>
#include <seutils/logging/category_tag.hpp>

namespace se {

namespace indexer {

namespace logging {

static inline const se::utils::logging::CategoryTag resources_loader_category  { "RESOURCES_LOADER"     };
static inline const se::utils::logging::CategoryTag primary_indexer_category   { "PRIMARY_INDEXER"      };
static inline const se::utils::logging::CategoryTag secondary_indexer_category { "SECONDARY_INDEXER"    };
static inline const se::utils::logging::CategoryTag adjacency_builder_category { "ADJACENCY_BUILDER"    };
static inline const se::utils::logging::CategoryTag resources_ranker_category  { "RESOURCES_RANKER"     };
static inline const se::utils::logging::CategoryTag synchronizer_category      { "SYNCHRONIZER"         };

} // namespace logging

} // namespace indexer

} // namespace se