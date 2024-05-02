#include <logger/logs_collector.hpp>

namespace se {

namespace logger {

#define CONFIGURE(result)                                       \
    if (!result) {                                              \
        config_errors_.store(true, std::memory_order_release);  \
        return;                                                 \
    }                                                           \

LogsCollector::LogsCollector() :
    se::utils::Service{ true, true, std::chrono::milliseconds(Config::get<size_t>("logger.options.periodicity_ms")), 1 },
    config_errors_{ false },
    max_allocated_memory_size_{ Config::get<size_t>("logger.options.memory_limit") },
    reduce_ratio_{ Config::get<double>("logger.options.reduce_ratio") } { 
    logs_buffer_.reserve(max_allocated_memory_size_ / sizeof(se::utils::LogData));
}

void LogsCollector::run() {
    if (config_errors_.load(std::memory_order_acquire)) {
        LOG_ERROR_WITH_TAGS(se::utils::logging::main_category, "Logger finishing with errors...");
        return;
    }
    bus_->start_receiving_logs([this](auto&& log) {
        handle_log(std::forward<se::utils::LogData>(log));
    });
    cds::threading::Manager::attachThread();
    se::utils::Service::run();
}

void LogsCollector::setup() {
    CONFIGURE(try_configure_logging())
    LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Logger started...");
    CONFIGURE(try_configure_db())
    CONFIGURE(try_configure_bus())
    LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Logger was successfully configured!");
}

void LogsCollector::stop() {
    se::utils::Service::stop();
    bus_->stop_receiving_logs();
}

bool LogsCollector::try_configure_db() {
    try {
        auto cfg = Config::db_config();
        db_ = std::make_unique<LogsProvider>(cfg);
        auto res = db_->enabled();
        if (res) {
            LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Configured database.");
        }
        else {
            LOG_CRITICAL_WITH_TAGS(
                se::utils::logging::main_category, 
                "Configuring database is failed. No connection."
            );    
        }
        return res;
    }
    catch(const std::exception& ex) {
        LOG_CRITICAL_WITH_TAGS(
            se::utils::logging::main_category, 
            "Configuring database completed with error: {}.", 
            ex.what()
        );
        return false;
    }
}

bool LogsCollector::try_configure_bus() {
    try {
        auto cfg = Config::bus_config("logger.bus");
        bus_ = std::make_unique<se::utils::AMQPBusMixin<LogsReceiver>>(cfg);
        for(auto i = 0; i < Config::thread_pool("logger.bus"); ++i) {
            bus_->run();
            bus_pool_.create_thread([this](){
                cds::threading::Manager::attachThread();
                bus_->attach();
            });
        }
        LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Configured bus.");
        return true;
    } catch(const std::exception& ex) {
        LOG_CRITICAL_WITH_TAGS(
            se::utils::logging::main_category, 
            "Configuring bus completed with error: {}.", 
            ex.what()
        );
        return false;            
    }
}

bool LogsCollector::try_configure_logging() {
    try {
        auto console_handler = quill::stdout_handler();
        console_handler->set_pattern(
            Config::get<std::string>("logger.logging.log_format"), 
            Config::get<std::string>("logger.logging.time_format"), 
            quill::Timezone::GmtTime
        );
        static_cast<quill::ConsoleHandler*>(console_handler.get())->enable_console_colours();
        quill::Config cfg;
        cfg.default_handlers.emplace_back(console_handler);
        cfg.backend_thread_sleep_duration = std::chrono::nanoseconds(static_cast<size_t>(1e4));
        cfg.backend_thread_empty_all_queues_before_exit = true;
        quill::configure(cfg);
        quill::start(); 
        LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Configured loggers.");  
        return true;

    } catch(const std::exception& ex) {
        std::cerr << "Exception occured in logger configuring: " << ex.what() << "\n";
        return false;
    }
}

void LogsCollector::dispatch_service_data() {
    if (current_memory_size_.load(std::memory_order_acquire) < max_allocated_memory_size_)
        return;
    size_t target_size = max_allocated_memory_size_ * reduce_ratio_;
    while(target_size < current_memory_size_.load(std::memory_order_acquire)) {
        se::utils::LogData cur;
        if (!logs_queue_.pop(cur))
            continue;
        current_memory_size_.fetch_sub(get_log_length(cur), std::memory_order_acq_rel);
        logs_buffer_.push_back(std::move(cur));
    }
    db_->load_logs_batch(logs_buffer_);
    LOG_INFO_WITH_TAGS(
        se::utils::logging::main_category, 
        "Buffer of {} logs was successfully recorded into database.",
        logs_buffer_.size()
    );
    logs_buffer_.clear();
}   

void LogsCollector::handle_log(se::utils::LogData&& ld) {
    auto len = get_log_length(ld);
    logs_queue_.push(std::move(ld));
    current_memory_size_.fetch_add(len, std::memory_order_acq_rel);
}

size_t LogsCollector::get_log_length(const se::utils::LogData& ld) {
    return  
        to_datetime<std::chrono::nanoseconds>(ld.timestamp).size() +
        ld.component.size() +
        ld.category.size() +
        ld.lvl.size() +
        ld.message.size();
}

#undef CONFIGURE

} // namespace logger

} // namespace se