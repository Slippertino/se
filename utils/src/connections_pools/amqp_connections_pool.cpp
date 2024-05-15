#include <seutils/connections_pools/amqp_connections_pool.hpp>

namespace se {

namespace utils {

AMQPConnection::AMQPConnection(const AMQP::Address& addr, std::unique_ptr<AMQP::TcpHandler> hndl) :
    handler{ std::move(hndl) },
    conn{ handler.get(), addr },
    channel{ &conn }
{ }

AMQPConnection::~AMQPConnection() {
    channel.close();
    conn.close();
}

std::unique_ptr<AMQPConnection> AMQPConnectionsPool::create_connection() const {
    return std::make_unique<AMQPConnection>(
        AMQP::Address{ server_url_.c_str() },
        std::unique_ptr<AMQP::TcpHandler>(new AMQPHandler(context_))
    );
}

AMQPConnectionsPool::AMQPConnectionsPool(size_t size, const boost::url& server, boost::asio::io_context& context) :
    ConnectionsPool(size),
    context_{ context },
    server_url_{ server }
{ }

} // namespace utils

} // namespace se