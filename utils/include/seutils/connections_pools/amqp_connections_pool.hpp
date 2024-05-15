#pragma once

#include <boost/url.hpp>
#include "../amqp/amqp_handler.hpp"
#include "connections_pool.hpp"

namespace se {

namespace utils {

struct AMQPConnection final {
    std::unique_ptr<AMQP::TcpHandler> handler;
    AMQP::TcpConnection conn;
    AMQP::TcpChannel channel;

    AMQPConnection(const AMQP::Address& addr, std::unique_ptr<AMQP::TcpHandler> hndl);
    ~AMQPConnection();
};
 
class AMQPConnectionsPool final : public ConnectionsPool<AMQPConnectionsPool, AMQPConnection> {
public:
    AMQPConnectionsPool() = delete;
    AMQPConnectionsPool(size_t size, const boost::url& server, boost::asio::io_context& context);
    std::unique_ptr<AMQPConnection> create_connection() const;

private:
    boost::asio::io_context& context_;
    boost::url server_url_;
};

} // namespace utils

} // namespace se