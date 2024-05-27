#pragma once

#include <memory>
#include <userver/utest/using_namespace_userver.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/merge_schemas.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/rabbitmq.hpp>
#include <userver/engine/mutex.hpp>
#include <userver/engine/condition_variable.hpp>
#include <userver/concurrent/mpsc_queue.hpp>
#include <userver/concurrent/background_task_storage.hpp>
#include <seutils/amqp/amqp_config.hpp>
#include <seutils/dtos/transmission_options.hpp>
#include <seutils/dtos/batched_dto.hpp>
#include <seutils/models/query_data.hpp>
#include <seutils/models/log_data.hpp>
#include <seutils/logging/logging.hpp>
#include <searcher/config/config.hpp>
#include <searcher/config/config_extractor.hpp>

namespace se {

namespace searcher {

template<typename DataType>
class BusSendingBuffer {
private:
    using DataTypePtr = std::unique_ptr<DataType>;

public:
    BusSendingBuffer(
        const se::utils::AMQPBusMessageConfig& config,
        std::shared_ptr<userver::urabbitmq::Client> client
    ) : config_{ config },
        options_{ config_.transmission_options },
        buffer_(options_.max_batch_volume),
        queue_{ userver::concurrent::MpscQueue<DataTypePtr>::Create() },
        producer_{ queue_->GetMultiProducer() },
        consumer_{ queue_->GetConsumer() },
        client_{ client }
    { 
        queue_->SetSoftMaxSize(userver::concurrent::MpscQueue<DataTypePtr>::kUnbounded);
    }

    void push(DataType&& data) {
        if (!config_.enabled) return;
        [[maybe_unused]] auto res = producer_.Push(
            std::make_unique<DataType>(std::move(data)),
            userver::engine::Deadline{}            
        );
        waiter_.NotifyOne();
    }

    void run() {
        if (!config_.enabled) return;
        while(!userver::engine::current_task::IsCancelRequested()) {
            std::unique_lock<userver::engine::Mutex> locker{ wait_mutex_ };
            if (!waiter_.Wait(locker, [&]() { 
                return queue_->GetSizeApproximate() >= options_.max_batch_size;  
            })) {
                continue;
            }
            flush();
        }
    }

private:
    void flush() {
        auto size = queue_->GetSizeApproximate();
        DataTypePtr data;
        se::utils::BatchedDTO<DataType> obj{ buffer_, options_.max_batch_size, options_.compression_type };
        for(size_t i = 0; i < size && consumer_.PopNoblock(data); ++i)
            obj.try_add_data(*data.release());
        while(!obj.is_empty()) {
            auto msg = obj.serialize();
            client_->Publish(
                userver::urabbitmq::Exchange(config_.exchange), config_.routing_key, msg, 
                userver::urabbitmq::MessageType::kPersistent,
                userver::engine::Deadline{}
            );
            LOG_TRACE_L1_WITH_TAGS(
                se::utils::logging::bus_category,
                "Batch with exchange=\"{}\" and key=\"{}\" was sent to bus.",
                config_.exchange,
                config_.routing_key
            );
        }
    }

private:
    se::utils::AMQPBusMessageConfig config_;
    se::utils::TransmissionOptions& options_;
    se::utils::CompressionHelper::buffer_t buffer_;
    userver::engine::Mutex wait_mutex_;
    userver::engine::ConditionVariable waiter_;

    using QueueType = userver::concurrent::MpscQueue<DataTypePtr>;
    std::shared_ptr<QueueType> queue_;
    QueueType::MultiProducer producer_;
    QueueType::Consumer consumer_;

    std::shared_ptr<userver::urabbitmq::Client> client_;
};

class ExternalBus final : public userver::components::RabbitMQ {
public:
    static constexpr std::string_view kName = "bus";

    ExternalBus(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    );

    void send_log(se::utils::LogData log);
    void send_query(se::utils::QueryData query);

    static userver::yaml_config::Schema GetStaticConfigSchema();

private:
    static inline const std::string logging_key_ = "logging";
    static inline const std::string query_key_ = "queries";

private:
    se::utils::AMQPBusConfig config_;
    std::shared_ptr<userver::urabbitmq::Client> client_;
    BusSendingBuffer<se::utils::LogData> logs_buffer_;
    BusSendingBuffer<se::utils::QueryData> queries_buffer_;
    userver::concurrent::BackgroundTaskStorage bts_;
};

} // namespace searcher

} // namespace se