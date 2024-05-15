#include <crawler/crawler.hpp>

namespace se {

namespace crawler { 

#define CONFIGURE(result)                                       \
    if (!result) {                                              \
        config_errors_.store(true, std::memory_order_release);  \
        return;                                                 \
    }                                                           \

Crawler::Crawler() : 
    se::utils::Service(true, true, std::chrono::seconds(5), 1),
    config_errors_{ false }
{ }

void Crawler::setup(const std::string& cfg) {
    Config::load(cfg);
    std::shared_ptr<BusHandlerType> bus_logging_handler;
    CONFIGURE(try_configure_logging(bus_logging_handler))
    LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Application started...");
    CONFIGURE(try_configure_db())
    CONFIGURE(try_configure_bus())
    bus_logging_handler->set_target(bus_);
    CONFIGURE(try_configure_queue())
    CONFIGURE(try_configure_distributor())
    CONFIGURE(try_configure_processor())
    LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Application was successfully configured!");
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

void Crawler::stop() {
    se::utils::Service::stop();
    if (bus_)
        bus_->stop();
    if (distributor_)
        distributor_->stop();
    if (queue_)
        queue_->stop();
    if (processor_)
        processor_->stop();
}

Crawler::~Crawler() {
    if (!config_errors_.load(std::memory_order_acquire))
        LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Application finishing...");
    bus_group_.join_all();
    distributor_group_.join_all();
    queue_group_.join_all();
    processor_group_.join_all();
}

void Crawler::dispatch_service_data() {
    if (config_errors_.load(std::memory_order_relaxed)) {
        stop();
        return;
    }

    auto dist_group_size = distributor_->current_crawling_group_size();
    auto dist_size = distributor_->size();
    auto queue_size = queue_->size();
    auto output_size = queue_->output_size();
    auto processor_size = processor_->total_in_work();

    LOG_INFO_WITH_TAGS(
        se::utils::logging::main_category,
        "State: queue size = {}, output_size = {}, dist = {}, total_handled = {}, success_handled = {}, now in work = {}",
        queue_size,
        output_size,
        dist_size,
        processor_->total_handled(),
        processor_->total_succeed_handled(),
        processor_size
    );

    if (queue_->is_full()) {
        LOG_WARNING_WITH_TAGS(
            se::utils::logging::main_category,
            "Size limit of queue was reached: {}", 
            queue_size
        );
    }

    if (!dist_group_size && !queue_size && !processor_size) {
        LOG_WARNING_WITH_TAGS(
            se::utils::logging::main_category,
            "All resources were crawled. Finishing..."
        );
        stop();
    }
}

bool Crawler::try_configure_db() {
    try {
        auto cfg = Config::db_config();
        auto provider = std::make_shared<PostgresDataProvider>(cfg);
        auto res = provider->enabled();
        if (res) {
            db_ = provider;
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

bool Crawler::try_configure_bus() {
    try {
        auto cfg = Config::bus_config("crawler.bus");
        bus_ = std::make_shared<se::utils::AMQPBusMixin<IExternalBus>>(cfg);
        bus_->run();
        for(auto i = 0; i < Config::thread_pool("crawler.bus"); ++i) {
            bus_group_.create_thread([this](){
                cds::threading::Manager::attachThread();
                bus_->attach();
                cds::threading::Manager::detachThread();
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

bool Crawler::try_configure_queue() {
    try {
        queue_ = std::make_shared<ResourcesRepository>();
        for(auto i = 0; i < Config::thread_pool("crawler.services.queue"); ++i) {
            queue_group_.create_thread([this]() {
                cds::threading::Manager::attachThread();
                queue_->run();
                queue_->attach();
                cds::threading::Manager::detachThread();
            });
        }
        LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Configured queue.");
        return true;
    } catch(const std::exception& ex) {
        LOG_CRITICAL_WITH_TAGS(
            se::utils::logging::main_category, 
            "Configuring queue completed with error: {}.", 
            ex.what()
        );            
        return false;
    }
}

bool Crawler::try_configure_distributor() {
    try {
        distributor_ = std::make_shared<ResourceDistributor>(db_, queue_);
        for(auto i = 0; i < Config::thread_pool("crawler.services.distributor"); ++i) {
            distributor_group_.create_thread([this]() {
                cds::threading::Manager::attachThread();
                distributor_->run();
                distributor_->attach();
                cds::threading::Manager::detachThread();
            });
        }
        LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Configured distributor.");
        return true;
    } catch(const std::exception& ex) {
        LOG_CRITICAL_WITH_TAGS(
            se::utils::logging::main_category, 
            "Configuring distributor completed with error: {}.", 
            ex.what()
        );            
        return false;
    }        
}

bool Crawler::try_configure_processor() {
    try {
        processor_ = ResourceProcessor::create(distributor_, queue_, db_, bus_);
        for(auto i = 0; i < Config::thread_pool("crawler.services.processor"); ++i) {
            processor_group_.create_thread([this, i]() {
                cds::threading::Manager::attachThread();
                if (!i)
                    processor_->run();
                processor_->attach();
                cds::threading::Manager::detachThread();
            });
        }
        LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Configured processor.");
        return true;
    } catch(const std::exception& ex) {
        LOG_CRITICAL_WITH_TAGS(
            se::utils::logging::main_category, 
            "Configuring processor. completed with error: {}.", 
            ex.what()
        );            
        return false;
    }        
}

bool Crawler::try_configure_logging(std::shared_ptr<BusHandlerType>& bus_out_handler) {
    try {
        auto console_handler = quill::stdout_handler();
        console_handler->set_pattern(
            Config::logging_message_pattern("console"), 
            Config::logging_time_pattern("console"), 
            quill::Timezone::GmtTime
        );
        static_cast<quill::ConsoleHandler*>(console_handler.get())->enable_console_colours();

        auto bus_handler = quill::create_handler<BusHandlerType>("BUS_HANDLER");
        bus_handler->set_pattern(
            Config::logging_message_pattern("bus"), 
            Config::logging_time_pattern("bus"), 
            quill::Timezone::GmtTime
        );

        quill::Config cfg;
        cfg.default_handlers.emplace_back(console_handler);
        cfg.default_handlers.emplace_back(bus_handler);
        cfg.backend_thread_sleep_duration = std::chrono::nanoseconds(static_cast<size_t>(1e4));
        cfg.backend_thread_empty_all_queues_before_exit = true;
        quill::configure(cfg);
        quill::start(); 

        quill::Logger* logger = quill::get_logger();
        logger->set_log_level(
            quill::loglevel_from_string(Config::get<std::string>("crawler.logging.lvl"))
        );

        bus_out_handler = std::static_pointer_cast<BusHandlerType>(bus_handler); 
        bus_out_handler->set_component_name(Config::get<std::string>("crawler.component_name"));

        LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Configured loggers.");  

        return true;

    } catch(const std::exception& ex) {
        std::cerr << "Exception occured in logger configuring: " << ex.what() << "\n";
        return false;
    }
}

#undef CONFIGURE

} // namespace crawler

} // namespace se