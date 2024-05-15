#include <indexer/bus/external_bus.hpp>

namespace se {

namespace indexer {

IExternalBus::IExternalBus() :
    CrawledBuffer           { 0 },
    LogBuffer               { Config::from_bus_message<size_t>(logging_key_, "max_batch_volume")            },
    max_log_batch_size_     { Config::from_bus_message<size_t>(logging_key_, "max_batch_size")              }
{ }

void IExternalBus::send_log(const se::utils::LogData& data) {
    send_to_batch<Log, se::utils::LogData>(
        data, 
        logging_key_, 
        max_log_batch_size_
    );
}

IExternalBus::~IExternalBus() {
    if (!Log::get_thread_resource().data.empty())
        flush_batch<Log>(logging_key_);        
}

void IExternalBus::read_resource_batch(const char* message, uint64_t len) {
    try {
        auto& batch = CrawledResource::get_thread_resource();
        std::string msg(message, len);
        std::stringstream in{ msg };
        boost::archive::text_iarchive ia(in, boost::archive::no_header);
        ia >> batch;
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::bus_category, 
            "Error occured while reading batch from bus with key {}: {}.",
            index_key_,
            ex.what()
        );
    }
}

} // namespace indexer

} // namespace se