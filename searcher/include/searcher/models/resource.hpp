#pragma once

#include <string>
#include <userver/formats/json.hpp>
#include <userver/formats/serialize/common_containers.hpp>

namespace se {

namespace searcher {

struct ResourceInfo {
  std::string url;
  std::string title;
  float relevance;

  friend userver::formats::json::Value Serialize(
      const ResourceInfo& data,
      userver::formats::serialize::To<userver::formats::json::Value>
  ) {
      return userver::formats::json::MakeObject(
          "url", data.url,
          "title", data.title,
          "relevance", data.relevance
      );
  }
};

struct RankedResource {
  std::int64_t id;
  float rank;
};

struct ResourceRank {
  float dynamic_rank;
  float static_rank;
};

} // namespace searcher

} // namespace se