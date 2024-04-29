#pragma once

#include <boost/url.hpp>
#include <crawler/bus/amqp/amqp_handler.hpp>
#include <crawler/utils/threaded_resource.hpp>

namespace crawler {

struct AMQPConnection final {
    AMQP::TcpConnection conn;
    AMQP::TcpChannel channel;
    std::unique_ptr<AMQP::TcpHandler> handler;

    AMQPConnection(const AMQP::Address& addr, std::unique_ptr<AMQP::TcpHandler> handler) :
        conn{ handler.get(), addr },
        channel{ &conn },
        handler{ std::move(handler) }
    { }
};
 
class AMQPConnectionsPool final : public utils::ThreadedResource<AMQPConnectionsPool, AMQPConnection> {
public:
    AMQPConnectionsPool() = delete;
    AMQPConnectionsPool(const boost::url& server, boost::asio::io_context& context) :
        context_{ context },
        server_url_{ server }
    { }

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

} // namespace crawler