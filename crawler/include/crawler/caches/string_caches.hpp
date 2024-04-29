#pragma once

#include <thread>
#include <memory>
#include <functional>
#include <crawler/types.hpp>
#include <crawler/utils/static_shared_resource.hpp>

namespace crawler {

class SchemeNamesCache final : 
    public utils::StaticSharedResource<SchemeNamesCache, shared_string> { };

class DomainNamesCache final :
    public utils::StaticSharedResource<DomainNamesCache, shared_string> { };

} // namespace crawler 