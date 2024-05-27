#pragma once

#include "lexem.hpp"
#include "resource.hpp"
#include <userver/storages/postgres/io/io_fwd.hpp>

namespace userver::storages::postgres::io {
    
template <>
struct CppToUserPg<se::searcher::LexemEntryType> {
  static constexpr DBTypeName postgres_name = "public.new_lexem_entry_type";
};

template <>
struct CppToUserPg<se::searcher::RegisteredLexem> {
  static constexpr DBTypeName postgres_name = "public.lexem_type";
};

template <>
struct CppToUserPg<se::searcher::ResourceInfo> {
  static constexpr DBTypeName postgres_name = "public.resource_info";
};

template <>
struct CppToUserPg<se::searcher::RankedResource> {
  static constexpr DBTypeName postgres_name = "public.ranked_resource";
};

template <>
struct CppToUserPg<se::searcher::ResourceRank> {
  static constexpr DBTypeName postgres_name = "public.resource_rank";
};

}  // namespace storages::postgres::io