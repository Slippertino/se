#include <seutils/amqp/amqp_handler.hpp>

namespace se {

namespace utils {

AMQPHandler::AMQPHandler(boost::asio::io_context& context) : LibBoostAsioHandler(context)
{ }

void AMQPHandler::onReady(AMQP::TcpConnection *connection) {
    LOG_INFO_WITH_TAGS(
        logging::bus_category, 
        "Bus connection was successfully estabilished."
    );
}

void AMQPHandler::onError(AMQP::TcpConnection *connection, const char *message) {
    LOG_ERROR_WITH_TAGS(
        logging::bus_category, 
        "Bus connection errors were occured: {}.",
        message
    );
}

} // namespace utils

} // namespace se