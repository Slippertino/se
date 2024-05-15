#pragma once

#include <memory>
#include <boost/thread/thread.hpp>
#include <cds/threading/model.h>
#include <cds/container/fcpriority_queue.h>
#include <seutils/service.hpp>
#include <seutils/amqp/amqp_bus.hpp>
#include <seutils/models/log_data.hpp>
#include <seutils/logging/logging.hpp>
#include <logger/db/logs_provider.hpp>
#include <logger/logs_receiver.hpp>
#include "config.hpp"

namespace std {
    template<>
    struct less<se::utils::LogData> {
        bool operator()(
            const se::utils::LogData& lhs, 
            const se::utils::LogData& rhs
        ) const {
            return lhs.timestamp > rhs.timestamp;
        }
    };
}

namespace se {

namespace logger {

class LogsCollector final : public se::utils::Service {
public:
    LogsCollector();

    void run() override;
    void setup();
    void stop() override;

    ~LogsCollector();

private:
    bool try_configure_db();
    bool try_configure_bus();
    bool try_configure_logging();

private:
    void dispatch_service_data() override;
    void handle_log(se::utils::LogData&& ld);
    static size_t get_log_length(const se::utils::LogData& ld);

private:
    std::atomic<bool> config_errors_;

private:
    const size_t max_allocated_memory_size_;
    const double reduce_ratio_;
    std::atomic<size_t> current_memory_size_{ 0 }; 
    cds::container::FCPriorityQueue<se::utils::LogData> logs_queue_;
    std::vector<se::utils::LogData> logs_buffer_;

private:
    std::unique_ptr<LogsProvider> db_;
    std::unique_ptr<se::utils::AMQPBusMixin<LogsReceiver>> bus_;

private:
    boost::thread_group bus_pool_;
};

} // namespace logger

} // namespace se