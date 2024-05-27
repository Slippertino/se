#include <searcher/logging/bus_logging_wrapper.hpp>

namespace se {

namespace searcher {

BusLoggingWrapper::BusLoggingWrapper(ExternalBus& bus) : bus_{ bus }
{ }

void BusLoggingWrapper::send_log(const se::utils::LogData& log) {
    bus_.send_log(log);
}

} // namespace searcher

} // namespace se