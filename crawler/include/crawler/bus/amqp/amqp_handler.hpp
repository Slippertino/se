#pragma once

#include <string>
#include <iostream>
#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>
#include <crawler/logging/logging.hpp>

namespace crawler {

class AMQPHandler final : public AMQP::LibBoostAsioHandler {
public:
    AMQPHandler(boost::asio::io_context& context) : LibBoostAsioHandler(context)
    { }

    void onReady(AMQP::TcpConnection *connection) override {
        LOG_INFO_WITH_TAGS(
            logging::bus_category, 
            "Bus connection was successfully estabilished."
        );
    }

    void onError(AMQP::TcpConnection *connection, const char *message) override {
        LOG_ERROR_WITH_TAGS(
            logging::bus_category, 
            "Bus connection errors were occured: {}.",
            message
        );
    }
};

} // namespace crawler