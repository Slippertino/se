#pragma once

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

namespace se {

namespace utils {

class AsyncEvent {
public:
    AsyncEvent(boost::asio::io_service& io_service);

    template<class WaitHandler>
    void async_wait(WaitHandler&& handler) {
        deadline_timer_.async_wait(std::forward<WaitHandler>(handler));
    }

    void async_notify_one();
    void async_notify_all();

private:
    void async_notify_one_serialized();
    void async_notify_all_serialized();

private:
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::deadline_timer deadline_timer_;
};

} // namespace utils

} // namespace se