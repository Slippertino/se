#pragma once

#include <indexer/config.hpp>
#include <indexer/caches/memory_buffer_tags.hpp>
#include <seutils/threaded_memory_buffer.hpp>
#include <seutils/dtos/batched_dto.hpp>
#include <seutils/models/resource_data.hpp>
#include <seutils/models/log_data.hpp>
#include <seutils/logging/logging.hpp>

namespace se {

namespace indexer {

class IExternalBus : 
    public se::utils::ThreadedResource<IExternalBus, se::utils::BatchedDTO<se::utils::CrawledResourceData>>,
    public se::utils::ThreadedMemoryBuffer<crawled_resource_compression_tag>,

    public se::utils::ThreadedResource<IExternalBus, se::utils::BatchedDTO<se::utils::LogData>>,
    public se::utils::ThreadedMemoryBuffer<logs_compression_tag> {

private:
    using CrawledResourceType = se::utils::BatchedDTO<se::utils::CrawledResourceData>;
    using CrawledResource = se::utils::ThreadedResource<IExternalBus, CrawledResourceType>;
    using CrawledBuffer = se::utils::ThreadedMemoryBuffer<crawled_resource_compression_tag>;

    using LogType = se::utils::BatchedDTO<se::utils::LogData>;
    using Log = se::utils::ThreadedResource<IExternalBus, LogType>;
    using LogBuffer = se::utils::ThreadedMemoryBuffer<logs_compression_tag>;

public:
    IExternalBus(const se::utils::BusConfig&);

    template<std::same_as<CrawledResourceType> R>
    CrawledResourceType create_thread_resource() {
        return CrawledResourceType(
            CrawledBuffer::get_thread_resource()
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

    void send_log(const se::utils::LogData& data);

    template<typename ParsingCallback, typename FinalCallback>
    void receive_resources(ParsingCallback&& on_parsed, FinalCallback&& on_final) {
        receive_data(index_key_,
            [   this, 
                on_parsed = std::forward<ParsingCallback>(on_parsed),
                on_final = std::forward<FinalCallback>(on_final)
            ] (const char* data, uint64_t len) {
                auto& batch = CrawledResource::get_thread_resource();
                batch.load_from_string(std::string{ data, len });
                auto size = batch.data().size();
                for(auto& res : batch.data())
                    on_parsed(std::move(res));
                batch.data().clear();
                on_final();
                LOG_INFO_WITH_TAGS(
                    se::utils::logging::bus_category,
                    "{} resources compressed by {} were successfully received from bus with key {}.",
                    size,
                    batch.compression_type(),
                    index_key_
                );
            },
            [](const char*) { }
        );
    }

    virtual ~IExternalBus();

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
    static inline const std::string index_key_ = "index";
    static inline const std::string logging_key_ = "logging";

private:
    const se::utils::BusConfig config_;
};

} // namespace indexer

} // namespace se