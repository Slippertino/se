#pragma once

#include <string>
#include <memory>
#include <quill/Quill.h>
#include <crawler/bus/external_bus.hpp>
#include <crawler/utils/models/log_data.hpp>

namespace crawler {

namespace logging {

class BusHandler : public quill::Handler {
public:
    BusHandler() = default;
    
    ~BusHandler() override = default;

    void set_target(std::shared_ptr<IExternalBus> bus) {
        bus_ = bus;
    }

    void write(quill::fmt_buffer_t const& formatted_log_message, quill::TransitEvent const& log_event) override {
        if (bus_.expired())
            return;
        auto bus_ptr = bus_.lock();
        utils::LogData data;
        data.timestamp = log_event.timestamp;
        data.component = Config::name();
        log_event.macro_metadata->custom_tags()->format(data.category);
        data.lvl = log_event.macro_metadata->log_level_string();
        data.message = std::string{ formatted_log_message.data(), formatted_log_message.size() };
        bus_ptr->send_log(data);
    }

    void flush() noexcept override { }

private:
    std::weak_ptr<IExternalBus> bus_;
};

} // namespace logging 

} // namespace crawler