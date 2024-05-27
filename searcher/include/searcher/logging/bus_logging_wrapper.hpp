#pragma once

#include <searcher/bus/external_bus.hpp>

namespace se {

namespace searcher {

class BusLoggingWrapper final {
public:
    BusLoggingWrapper(ExternalBus& bus);
    void send_log(const se::utils::LogData& log);

private:
    ExternalBus& bus_;
};

} // namespace searcher

} // namespace se