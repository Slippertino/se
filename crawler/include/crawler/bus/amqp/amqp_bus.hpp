#pragma once

#include "amqp_config.hpp"
#include "../external_bus.hpp"
#include <crawler/utils/service.hpp>
#include <crawler/caches/connections_pools/amqp_connections_pool.hpp>

namespace crawler {

class AMQPBus final : public IExternalBus, public utils::Service {
public:
    AMQPBus() = delete;
    AMQPBus(const AMQPBusConfig& config) : 
        Service(false, false),
        config_{ config },
        connections_{ config.url, get_context() }
    { }

    bool enabled() {
        auto conn = connections_.create_thread_resource<AMQPConnection>();
        auto res = conn.channel.ready();
        return res;
    }

protected:
    void send_data(const std::string& key, const std::string& data, bool log_on_error = false) override {
        if (!config_.messages.contains(key)) {
            if (log_on_error) {
                LOG_ERROR_WITH_TAGS(
                    logging::bus_category, 
                    "Unknown target key {} was met.", 
                    key
                );
            }
            return;
        }
        auto& msg = config_.messages.at(key);
        if (!msg.enabled) {
            LOG_TRACE_L1_WITH_TAGS(
                logging::bus_category, 
                "Tried to write in disabled bus path {}.", 
                key
            );
            return;
        }
        auto& conn = connections_.get_thread_resource();
        conn.channel.publish(msg.exchange, msg.routing_key, data.c_str());        
    }

private:
    AMQPBusConfig config_;
    AMQPConnectionsPool connections_;
};

} // namespace crawler