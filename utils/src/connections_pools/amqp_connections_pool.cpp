#include <seutils/connections_pools/amqp_connections_pool.hpp>

namespace se {

namespace utils {

AMQPConnection::AMQPConnection(const AMQP::Address& addr, std::unique_ptr<AMQP::TcpHandler> handler) :
    conn{ handler.get(), addr },
    channel{ &conn },
    handler{ std::move(handler) }
{ }

AMQPConnectionsPool::AMQPConnectionsPool(const boost::url& server, boost::asio::io_context& context) :
    context_{ context },
    server_url_{ server }
{ }

} // namespace utils

} // namespace se