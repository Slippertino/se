#pragma once

#include <string>
#include <memory>
#include <quill/Quill.h>
#include "../models/log_data.hpp"

namespace se {

namespace utils {

namespace logging {

template<class Target>
class BusHandler : public quill::Handler {
public:
    BusHandler() = default;
    
    ~BusHandler() override = default;

    void set_component_name(const std::string& name) noexcept {
        component_name_ = name;
    }

    void set_target(std::shared_ptr<Target> bus) noexcept {
        bus_ = bus;
    }

    void write(quill::fmt_buffer_t const& formatted_log_message, quill::TransitEvent const& log_event) override {
        if (bus_.expired())
            return;
        auto bus_ptr = bus_.lock();
        utils::LogData data;
        data.timestamp = log_event.timestamp;
        data.component = component_name_;
        log_event.macro_metadata->custom_tags()->format(data.category);
        data.lvl = log_event.macro_metadata->log_level_string();
        data.message = std::string{ formatted_log_message.data(), formatted_log_message.size() };
        bus_ptr->send_log(data);
    }

    void flush() noexcept override { }

private:
    std::string component_name_;
    std::weak_ptr<Target> bus_;
};

} // namespace logging

} // namespace utils

} // namespace se