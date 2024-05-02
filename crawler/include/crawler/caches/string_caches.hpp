#pragma once

#include <thread>
#include <memory>
#include <functional>
#include <crawler/types.hpp>
#include <seutils/static_shared_resource.hpp>

namespace se {

namespace crawler {

class SchemeNamesCache final : 
    public se::utils::StaticSharedResource<SchemeNamesCache, shared_string> { };

class DomainNamesCache final :
    public se::utils::StaticSharedResource<DomainNamesCache, shared_string> { };

} // namespace crawler 

} // namespace se