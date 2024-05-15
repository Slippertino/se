#pragma once

#include <memory>
#include <vector>
#include <string>
#include <semaphore>
#include <condition_variable>
#include <cds/container/rwqueue.h>

namespace se {

namespace utils {

template<typename PoolType, typename ConnectionType>
class ConnectionHolder {
public:
    ConnectionHolder(PoolType& pool) :
        pool_{ pool },
        conn_{ pool.get_connection_internal() }
    { }

    ConnectionType& connection() {
        return *conn_;
    }

    ~ConnectionHolder() {
        pool_.release_connection_internal(std::move(conn_));
    }

private:
    PoolType& pool_;
    std::unique_ptr<ConnectionType> conn_;
};

template<typename Derived, typename ConnectionType>
class ConnectionsPool {
public:
    using ConnectionPtr = std::unique_ptr<ConnectionType>;
    using ConnectionHolderType = ConnectionHolder<Derived, ConnectionType>;

    friend class ConnectionHolder<Derived, ConnectionType>;

public:
    ConnectionsPool(size_t size) : 
        max_size_{ size },
        limiter_{ static_cast<std::ptrdiff_t>(size) }
    { }

    ConnectionHolderType get_connection() {
        return ConnectionHolderType{ *static_cast<Derived*>(this) };
    }

    void reset() {
        auto sz = pool_.size();
        pool_.clear();
        limiter_.release(sz);
    }

private:
    ConnectionPtr get_connection_internal() {
        limiter_.acquire();
        ConnectionPtr res;
        if (pool_.empty()) 
            res = static_cast<Derived*>(this)->create_connection();
        else 
            pool_.pop(res);
        return res;
    }

    void release_connection_internal(ConnectionPtr conn) {
        pool_.push(std::move(conn));
        limiter_.release();
    }

private:
    const size_t max_size_;
    std::counting_semaphore<> limiter_;
    cds::container::RWQueue<ConnectionPtr> pool_;
};

} // namespace utils

} // namespace se