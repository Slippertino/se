#pragma once

#include <vector>
#include <string>
#include <crawler/utils/static_shared_resource.hpp>

namespace crawler {

using dns_result_t = std::vector<std::string>;
class DNSCache : public utils::StaticSharedResource<DNSCache, dns_result_t> { };

} // namespace crawler