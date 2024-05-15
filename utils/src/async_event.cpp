#include <seutils/async_event.hpp>

namespace se {

namespace utils {

AsyncEvent::AsyncEvent(boost::asio::io_service& io_service): 
    strand_(io_service.get_executor()),
    deadline_timer_(io_service, boost::posix_time::ptime(boost::posix_time::pos_infin))
{}

void AsyncEvent::async_notify_one() {
    boost::asio::post(strand_, boost::bind(&AsyncEvent::async_notify_one_serialized, this));
}

void AsyncEvent::async_notify_all() {
    boost::asio::post(strand_, boost::bind(&AsyncEvent::async_notify_all_serialized, this));
}

void AsyncEvent::async_notify_one_serialized() {
    deadline_timer_.cancel_one();
}

void AsyncEvent::async_notify_all_serialized() {
    deadline_timer_.cancel();
}

} // namespace utils

} // namespace se