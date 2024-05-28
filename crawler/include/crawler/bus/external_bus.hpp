#pragma once

#include <crawler/config.hpp>
#include <crawler/caches/memory_buffer_tags.hpp>
#include <seutils/threaded_memory_buffer.hpp>
#include <seutils/dtos/batched_dto.hpp>
#include <seutils/models/resource_data.hpp>
#include <seutils/models/log_data.hpp>
#include <seutils/bus/bus_config.hpp>
#include <seutils/logging/logging.hpp>

namespace se {

namespace crawler {

class IExternalBus : 
    public se::utils::ThreadedResource<IExternalBus, se::utils::BatchedDTO<se::utils::CrawledResourceData>>,
    public se::utils::ThreadedMemoryBuffer<crawled_resources_compression_tag>,

    public se::utils::ThreadedResource<IExternalBus, se::utils::BatchedDTO<se::utils::LogData>>,
    public se::utils::ThreadedMemoryBuffer<logs_compression_tag> {

private:
    using CrawledResourceType = se::utils::BatchedDTO<se::utils::CrawledResourceData>;
    using CrawledResource = se::utils::ThreadedResource<IExternalBus, CrawledResourceType>;
    using CrawledBuffer = se::utils::ThreadedMemoryBuffer<crawled_resources_compression_tag>;

    using LogType = se::utils::BatchedDTO<se::utils::LogData>;
    using Log = se::utils::ThreadedResource<IExternalBus, LogType>;
    using LogBuffer = se::utils::ThreadedMemoryBuffer<logs_compression_tag>;

public:
    IExternalBus(const se::utils::BusConfig& config) :
        CrawledBuffer           { config.routes_options.at(crawled_resources_key_).max_batch_volume },
        LogBuffer               { config.routes_options.at(logging_key_).max_batch_volume           },
        config_{ config }
    { }

    template<std::same_as<CrawledResourceType> R>
    CrawledResourceType create_thread_resource() {
        return CrawledResourceType(
            CrawledBuffer::get_thread_resource(),
            config_.routes_options.at(crawled_resources_key_).max_batch_size,
            config_.routes_options.at(crawled_resources_key_).compression_type  
        );
    }
    
    template<std::same_as<LogType> R>
    LogType create_thread_resource() {
        return LogType(
            LogBuffer::get_thread_resource(),
            config_.routes_options.at(logging_key_).max_batch_size,
            config_.routes_options.at(logging_key_).compression_type  
        );
    }
    
    void send_resource(const se::utils::CrawledResourceData& data) {
        send_to_batch<CrawledResource, se::utils::CrawledResourceData>(
            data, 
            crawled_resources_key_
        );
    }

    void send_log(const se::utils::LogData& data) {
        send_to_batch<Log, se::utils::LogData>(
            data, 
            logging_key_
        );
    }

    virtual ~IExternalBus() {
        while (!CrawledResource::get_thread_resource().is_empty())
            flush_batch<CrawledResource>(crawled_resources_key_);
        while (!Log::get_thread_resource().is_empty())
            flush_batch<Log>(logging_key_);
    }

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
    template<typename ResourceType, typename ResourceDataType>
    void send_to_batch(const ResourceDataType& data, const std::string& key) {
        auto &batch = ResourceType::get_thread_resource();
        batch.try_add_data(data);
        while(!batch.is_empty())
            flush_batch<ResourceType>(key);
    }

    template<typename ResourceType>
    void flush_batch(const std::string& key) {
        try {
            auto& batch = ResourceType::get_thread_resource();
            send_data(key, batch.serialize(), key != logging_key_);        
        } catch(const std::exception& ex) {
            LOG_ERROR_WITH_TAGS(
                se::utils::logging::bus_category, 
                "Error occured while flushing batch with key {}: {}.", 
                key,
                ex.what()
            );
        }
    }

private:
    static inline const std::string crawled_resources_key_ = "crawled";
    static inline const std::string logging_key_ = "logging";

private:
    const se::utils::BusConfig config_;
};

} // namespace crawler

} // namespace se