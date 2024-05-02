#pragma once

#include <string>
#include <iostream>
#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>
#include "../logging/logging.hpp"

namespace se {

namespace utils {

class AMQPHandler final : public AMQP::LibBoostAsioHandler {
public:
    AMQPHandler(boost::asio::io_context& context);
    void onReady(AMQP::TcpConnection *connection) override;
    void onError(AMQP::TcpConnection *connection, const char *message) override;
};

} // namespace utils

} // namespace se