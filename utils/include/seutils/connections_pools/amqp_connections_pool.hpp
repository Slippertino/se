#pragma once

#include <boost/url.hpp>
#include "../amqp/amqp_handler.hpp"
#include "../threaded_resource.hpp"

namespace se {

namespace utils {

struct AMQPConnection final {
    AMQP::TcpConnection conn;
    AMQP::TcpChannel channel;
    std::unique_ptr<AMQP::TcpHandler> handler;

    AMQPConnection(const AMQP::Address& addr, std::unique_ptr<AMQP::TcpHandler> handler);
};
 
class AMQPConnectionsPool final : public ThreadedResource<AMQPConnectionsPool, AMQPConnection> {
public:
    AMQPConnectionsPool() = delete;
    AMQPConnectionsPool(const boost::url& server, boost::asio::io_context& context);

    template<std::same_as<AMQPConnection> R>
    AMQPConnection create_thread_resource() {
        return AMQPConnection{
            AMQP::Address{ server_url_.c_str() },
            std::unique_ptr<AMQP::TcpHandler>(new AMQPHandler(context_))
        };
    }

private:
    boost::asio::io_context& context_;
    boost::url server_url_;
};

} // namespace utils

} // namespace se