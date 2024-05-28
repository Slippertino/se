#pragma once

#include <string>
#include <sstream>
#include <functional>
#include <concepts>
#include <boost/archive/text_iarchive.hpp>
#include <seutils/dtos/batched_dto.hpp>
#include <seutils/models/log_data.hpp>
#include <seutils/logging/logging.hpp>
#include <seutils/threaded_resource.hpp>
#include <seutils/threaded_memory_buffer.hpp>
#include <seutils/bus/bus_config.hpp>
#include "memory_buffer_tags.hpp"

namespace se {

namespace logger {

class LogsReceiver :    
    public se::utils::ThreadedResource<LogsReceiver, se::utils::BatchedDTO<se::utils::LogData>>,
    public se::utils::ThreadedMemoryBuffer<logs_compression_tag> {

private:
    using LogType = se::utils::BatchedDTO<se::utils::LogData>;
    using Log = se::utils::ThreadedResource<LogsReceiver, LogType>;
    using LogBuffer = se::utils::ThreadedMemoryBuffer<logs_compression_tag>;

public:
    LogsReceiver(const se::utils::BusConfig&);

    template<std::same_as<LogType> R>
    LogType create_thread_resource() {
        return LogType {
            LogBuffer::get_thread_resource()  
        };
    }

    template<typename Callback>
    void start_receiving_logs(Callback&& on_parsed) {
        start_receiving_data(logging_key_,
            [this, on_parsed = std::forward<Callback>(on_parsed)](const char* data, uint64_t len) {
                auto& batch = Log::get_thread_resource();
                batch.load_from_string(std::string(data, len));
                auto size = batch.data().size();
                for(auto& log : batch.data())
                    on_parsed(std::move(log));
                batch.data().clear();
                LOG_INFO_WITH_TAGS(
                    se::utils::logging::bus_category,
                    "{} logs compressed by {} were successfully received from bus with key {}.",
                    size,
                    batch.compression_type(),
                    logging_key_
                );
            },
            [](const char*) { }
        );
        LOG_INFO_WITH_TAGS(
            se::utils::logging::bus_category,
            "Receiving logs were successfully started from bus with key {}.",
            logging_key_
        );
    }

    void stop_receiving_logs();

protected:
    virtual void send_data(const std::string& type, std::string data, bool log_on_error = true) = 0;
    virtual void receive_data(        
        const std::string& key, 
        std::function<void(const char*, uint64_t)> on_success, 
        std::function<void(const char*)> on_fail
    ) = 0;
    virtual void start_receiving_data(
        const std::string& queue, 
        std::function<void(const char*, uint64_t)> on_success, 
        std::function<void(const char*)> on_fail
    ) = 0;
    virtual void resume_receiving_data(const std::string& key) = 0;
    virtual void stop_receiving_data(const std::string& key) = 0;
    virtual void pause_receiving_data(const std::string& key) = 0;

private:
    static inline std::string logging_key_ = "logging";
};

} // namespace logger

} // namespace se