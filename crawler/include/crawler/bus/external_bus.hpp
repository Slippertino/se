#pragma once

#include <crawler/config.hpp>
#include <crawler/caches/memory_buffer_tags.hpp>
#include <seutils/threaded_memory_buffer.hpp>
#include <seutils/dtos/batched_dto.hpp>
#include <seutils/models/resource_data.hpp>
#include <seutils/models/log_data.hpp>
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
    IExternalBus() :
        CrawledBuffer           { Config::from_bus_message<size_t>(crawled_resources_key_, "max_batch_volume")  },
        LogBuffer               { Config::from_bus_message<size_t>(logging_key_, "max_batch_volume")            },
        max_resource_batch_size_{ Config::from_bus_message<size_t>(crawled_resources_key_, "max_batch_size")    },
        max_log_batch_size_     { Config::from_bus_message<size_t>(logging_key_, "max_batch_size")              }
    { }

    template<std::same_as<CrawledResourceType> R>
    CrawledResourceType create_thread_resource() {
        return CrawledResourceType {
            Config::from_bus_message<std::string>(crawled_resources_key_, "compression_type"),
            { },
            CrawledBuffer::get_thread_resource()
        };
    }
    
    template<std::same_as<LogType> R>
    LogType create_thread_resource() {
        return LogType {
            Config::from_bus_message<std::string>(logging_key_, "compression_type"),
            { },
            LogBuffer::get_thread_resource()
        };
    }
    
    void send_resource(const se::utils::CrawledResourceData& data) {
        send_to_batch<CrawledResource, se::utils::CrawledResourceData>(
            data, 
            crawled_resources_key_, 
            max_resource_batch_size_
        );
    }

    void send_log(const se::utils::LogData& data) {
        send_to_batch<Log, se::utils::LogData>(
            data, 
            logging_key_, 
            max_log_batch_size_
        );
    }

    virtual ~IExternalBus() {
        if (!CrawledResource::get_thread_resource().data.empty())
            flush_batch<CrawledResource>(crawled_resources_key_);
        if (!Log::get_thread_resource().data.empty())
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
    void send_to_batch(const ResourceDataType& data, const std::string& key, size_t batch_size) {
        auto &batch = ResourceType::get_thread_resource();
        if (batch.data.size() == batch_size)
            flush_batch<ResourceType>(key);
        batch.data.push_back(data);    
    }

    template<typename ThreadResource>
    void flush_batch(const std::string& key) {
        try {
            auto& batch = ThreadResource::get_thread_resource();
            std::ostringstream buffer;
            boost::archive::text_oarchive oa{ buffer, boost::archive::no_header };
            oa << batch;
            send_data(key, std::move(buffer.str()), key != logging_key_);        
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
    size_t max_resource_batch_size_;
    size_t max_log_batch_size_;
};

} // namespace crawler

} // namespace se