#include <logger/logs_receiver.hpp>

namespace se {

namespace logger {

LogsReceiver::LogsReceiver() : LogBuffer{ 0 }
{ }

void LogsReceiver::stop_receiving_logs() {
    stop_receiving_data(logging_key_);
    LOG_INFO_WITH_TAGS(
        se::utils::logging::bus_category,
        "Receiving logs were successfully stopped from bus with key {}.",
        logging_key_
    );
}

void LogsReceiver::read_batch(const char* message, uint64_t len) {
    try {
        auto& batch = Log::get_thread_resource();
        std::string msg(message, len);
        std::stringstream in{ msg };
        boost::archive::text_iarchive ia(in, boost::archive::no_header);
        ia >> batch;
    } catch(const std::exception& ex) {
        LOG_ERROR_WITH_TAGS(
            se::utils::logging::bus_category, 
            "Error occured while reading batch from bus with key {}: {}.",
            logging_key_,
            ex.what()
        );
    }
}

} // namespace logger

} // namespace se