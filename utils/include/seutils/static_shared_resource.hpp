#pragma once

#include <string>
#include <shared_mutex>
#include <unordered_map>
#include <concepts>

namespace se {

namespace utils {

template<class Derived, typename Resource>
class StaticSharedResource {
public:
    static Derived& instance() {
        static Derived inst;
        return inst;
    }

    bool contains(const std::string& key) const noexcept {
        std::shared_lock locker{ mutex_ };
        return source_.contains(key);
    }

    const Resource& operator[](const std::string& key) const {
        std::shared_lock locker{ mutex_ };
        if (!source_.contains(key))
            throw std::out_of_range("unknown key passed");
        return source_.at(key);
    }

    void upload_range(const std::vector<std::pair<std::string, Resource>>& range) {
        std::lock_guard locker{ mutex_ };
        for(const auto& v : range)
            source_.insert({ v.first, v.second });
    }

    void upload(const std::string& key, Resource res) {
        upload_range({{ key, std::move(res)}});
    }

    template<std::invocable Func>
    void upload(const std::string& key, Func&& func) {
        upload(key, std::forward<Func>(func)());
    }

    template<typename Func>
    void modify(const std::string& key, Func&& func) {
        std::lock_guard locker{ mutex_ }; 
        std::forward<Func>(func)(source_.at(key));   
    }

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, Resource> source_;
};

} // namespace utils

} // namespace se