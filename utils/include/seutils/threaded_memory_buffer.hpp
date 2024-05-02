#pragma once

#include <vector>
#include "threaded_resource.hpp"

namespace se {

namespace utils {

template<typename Tag>
class ThreadedMemoryBuffer : public ThreadedResource<ThreadedMemoryBuffer<Tag>, std::vector<char>> {
public:
    using type = std::vector<char>;

public:
    ThreadedMemoryBuffer(size_t size) : size_{ size }
    { }

    size_t size() const noexcept {
        return size_;
    }

    template<typename R>
    auto create_thread_resource() {
        return type(size_);
    }

protected:
    ThreadedMemoryBuffer() = default;

private:
    size_t size_;
};

} // namespace utils

} // namespace se