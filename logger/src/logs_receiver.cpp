#include <logger/logs_receiver.hpp>

namespace se {

namespace logger {

LogsReceiver::LogsReceiver(const se::utils::BusConfig&) : LogBuffer{ 0 }
{ }

void LogsReceiver::stop_receiving_logs() {
    stop_receiving_data(logging_key_);
    LOG_INFO_WITH_TAGS(
        se::utils::logging::bus_category,
        "Receiving logs were successfully stopped from bus with key {}.",
        logging_key_
    );
}

} // namespace logger

} // namespace se