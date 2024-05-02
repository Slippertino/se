#pragma once

#include <concepts>

namespace se {

namespace utils {

template<class Derived, typename Resource>
class ThreadedResource {
public:
    Resource& get_thread_resource() {
        thread_local Resource resource = 
            static_cast<Derived*>(this)->template create_thread_resource<Resource>();
        return resource;
    }

protected:
    ThreadedResource() = default;
};

} // namespace utils

} // namespace se
