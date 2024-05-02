#pragma once

#include <chrono>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <pqxx/composite>
#include <pqxx/strconv>
#include <seutils/models/log_data.hpp>

namespace se {

namespace logger {

template<typename Dur>
inline std::string to_datetime(int64_t time) {
    return boost::posix_time::to_iso_extended_string(
        boost::posix_time::from_time_t(time_t{ 
            std::chrono::duration_cast<std::chrono::seconds>(Dur(time)).count() 
        })
    );
}

} // namespace logger

} // namespace se

namespace pqxx {

template<> std::string const type_name<se::utils::LogData>{"LogDataType"};

template<> struct nullness<se::utils::LogData> : no_null<se::utils::LogData> {};

template<> 
struct string_traits<se::utils::LogData> {
    static se::utils::LogData from_string(std::string_view text) {
        return se::utils::LogData{};
    }

    static zview to_buf(char *begin, char *end, se::utils::LogData const &value) {
        return generic_to_buf(begin, end, value);
    }

    static char *into_buf(char *begin, char *end, se::utils::LogData const &value) {
        auto datetime = se::logger::to_datetime<std::chrono::nanoseconds>(value.timestamp);
        return composite_into_buf(
            begin, end, 
            datetime, 
            value.component, 
            value.category, 
            value.lvl, 
            value.message
        );
    }
    
    static size_t size_buffer(se::utils::LogData const &value) noexcept {
        auto datetime = se::logger::to_datetime<std::chrono::nanoseconds>(value.timestamp);
        return composite_size_buffer( 
            datetime, 
            value.component, 
            value.category, 
            value.lvl, 
            value.message
        );
    }
};

} // namespace pqxx
