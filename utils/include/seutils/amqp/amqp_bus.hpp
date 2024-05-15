#pragma once

#include "amqp_config.hpp"
#include "../service.hpp"
#include "../connections_pools/amqp_connections_pool.hpp"

namespace se {

namespace utils {

template<typename Base>
class AMQPBusMixin final : public Base, public Service {
public:
    AMQPBusMixin() = delete;
    AMQPBusMixin(const AMQPBusConfig& config) : 
        Service(false, false),
        config_{ config },
        connections_{ config.pool_size, config.url, get_context() }
    { }

    void run() override {
        Service::run();
        for(const auto& qk : config_.queues)
            receiving_channels_.insert({ qk.first, connections_.create_connection() });
    }

    void stop() override {
        receiving_channels_.clear();
        connections_.reset();
        Service::stop();
    }

protected:
    void send_data(const std::string& key, std::string data, bool log_on_error) override {
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
        auto conn_holder = connections_.get_connection();
        auto& conn = conn_holder.connection();
        conn.channel.publish(msg.exchange, msg.routing_key, data.data(), data.size());        
    }

    void receive_data(        
        const std::string& key, 
        std::function<void(const char*, uint64_t)> on_success, 
        std::function<void(const char*)> on_fail
    ) {
        if (!config_.queues.contains(key)) {
            LOG_ERROR_WITH_TAGS(
                logging::bus_category, 
                "Unknown queue key {} was met in try to get single data.", 
                key
            );
            return;
        }
        const auto& queue = config_.queues.at(key);
        auto& conn = *receiving_channels_.at(key);        
        conn.channel.get(queue)
            .onReceived([on_success = std::move(on_success), queue, &conn](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered){
                LOG_TRACE_L1_WITH_TAGS(
                    logging::bus_category, 
                    "Received message from bus queue {}.", 
                    queue
                );
                on_success(message.body(), message.bodySize());
                conn.channel.ack(deliveryTag);
            })
            .onError([on_fail = std::move(on_fail), queue](const char* message){
                LOG_WARNING_WITH_TAGS(
                    logging::bus_category, 
                    "Fail to receive a message from bus queue {}: {}.", 
                    queue,
                    message
                );
                on_fail(message);                
            });
    }

    void start_receiving_data(
        const std::string& key, 
        std::function<void(const char*, uint64_t)> on_success, 
        std::function<void(const char*)> on_fail
    ) override {
        if (!config_.queues.contains(key)) {
            LOG_ERROR_WITH_TAGS(
                logging::bus_category, 
                "Unknown queue key {} was met in try to start receiving data.", 
                key
            );
            return;
        }
        const auto& queue = config_.queues.at(key);
        auto& conn = *receiving_channels_.at(key);
        conn.channel.consume(queue, key)
            .onReceived([on_success = std::move(on_success), queue, &conn](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered){
                LOG_TRACE_L1_WITH_TAGS(
                    logging::bus_category, 
                    "Received message from bus queue {}.", 
                    queue
                );
                on_success(message.body(), message.bodySize());
                conn.channel.ack(deliveryTag);
            })
            .onError([on_fail = std::move(on_fail), queue](const char* message){
                LOG_WARNING_WITH_TAGS(
                    logging::bus_category, 
                    "Fail to receive a message from bus queue {}: {}.", 
                    queue,
                    message
                );
                on_fail(message);                
            });
    }

    void resume_receiving_data(const std::string& key) override {
        if (!config_.queues.contains(key)) {
            LOG_ERROR_WITH_TAGS(
                logging::bus_category, 
                "Unknown queue key {} was met in try to resume receiving data.", 
                key
            );
            return;
        }
        auto& conn = *receiving_channels_.at(key);
        conn.channel.resume();
    }

    void pause_receiving_data(const std::string& key) override {
        if (!config_.queues.contains(key)) {
            LOG_ERROR_WITH_TAGS(
                logging::bus_category, 
                "Unknown queue key {} was met in try to pause receiving data.", 
                key
            );
            return;
        }
        auto& conn = *receiving_channels_.at(key);
        conn.channel.pause();
    }

    void stop_receiving_data(const std::string& key) override {
        if (!config_.queues.contains(key)) {
            LOG_ERROR_WITH_TAGS(
                logging::bus_category, 
                "Unknown queue key {} was met in try to stop receiving data.", 
                key
            );
            return;
        }
        auto& conn = *receiving_channels_.at(key);
        conn.channel.cancel(key);
    }

private:
    AMQPBusConfig config_;
    AMQPConnectionsPool connections_;
    std::unordered_map<std::string, std::unique_ptr<AMQPConnection>> receiving_channels_;
};

} // namespace utils

} // namespace se