#include <indexer/bus/external_bus.hpp>

namespace se {

namespace indexer {

IExternalBus::IExternalBus(const se::utils::BusConfig& config) :
    CrawledBuffer           { 0 },
    LogBuffer               { config.routes_options.at(logging_key_).max_batch_volume },
    config_{ config }
{ }

void IExternalBus::send_log(const se::utils::LogData& data) {
    send_to_batch<Log, se::utils::LogData>(
        data, 
        logging_key_
    );
}

IExternalBus::~IExternalBus() {
    while (!Log::get_thread_resource().is_empty())
        flush_batch<Log>(logging_key_);        
}

} // namespace indexer

} // namespace se