#pragma once

namespace se {

namespace utils {

template<typename Tag>
struct result {
    typedef typename Tag::type type;
    static inline type ptr;
};

template<typename Tag, Tag::type Pointer>
struct rob : result<Tag> {
    struct filler {
        filler() { result<Tag>::ptr = Pointer; }
    };
    static inline filler filler_obj;
};

} // namespace utils

} // namespace se