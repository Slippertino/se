#pragma once

#include <vector>
#include <string>
#include <seutils/static_shared_resource.hpp>

namespace se {

namespace crawler {

using dns_result_t = std::vector<std::string>;
class DNSCache : public se::utils::StaticSharedResource<DNSCache, dns_result_t> { };

} // namespace crawler

} // namespace se