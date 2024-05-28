#include <indexer/indexer.hpp>

namespace se {

namespace indexer {

#define CONFIGURE(result)                                       \
    if (!result) {                                              \
        config_errors_.store(true, std::memory_order_release);  \
        return;                                                 \
    }                                                           \

Indexer::Indexer() :
    se::utils::Service{ false, false },
    config_errors_{ false }
{ }

void Indexer::run() {
    if (config_errors_.load(std::memory_order_acquire)) {
        LOG_ERROR_WITH_TAGS(se::utils::logging::main_category, "Indexer finishing with errors...");
        return;
    }
    auto options = se::utils::GlobalConfig<Config>::config.options();
    run_primary_indexing_stage(options);
    run_finally_indexing_stage(options);
    LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Indexing was proceed. Finishing...");
}

void Indexer::setup() {
    std::shared_ptr<BusHandlerType> bus_handler;
    CONFIGURE(try_configure_logging(bus_handler))
    LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Indexer started...");
    CONFIGURE(try_configure_db())
    CONFIGURE(try_configure_bus())
    bus_handler->set_target(bus_);
    LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Indexer was successfully configured!");
}

void Indexer::run_primary_indexing_stage(const IndexingOptions& opts) {
    tbb::flow::graph g;
    tbb::flow::queue_node<se::utils::CrawledResourceData> qdata{ g };
    ResourcesLoader puller{ opts, bus_ };
    tbb::flow::async_node<tbb::flow::continue_msg, se::utils::CrawledResourceData> pulling_node{ 
        g, 1, 
        [&puller](auto msg, auto& gateway) { puller.run(msg, gateway); }
    };
    tbb::flow::multifunction_node<se::utils::CrawledResourceData, std::tuple<tbb::flow::continue_msg>> primary_indexer {
        g, se::utils::GlobalConfig<Config>::config.thread_pool("indexer.options.primary_indexer"), PrimaryIndexer{ opts, db_ }
    };
    tbb::flow::multifunction_node<tbb::flow::continue_msg, std::tuple<tbb::flow::continue_msg>> breaker {
        g, 1, LoaderBreaker{ opts, puller, db_ }
    };

    tbb::flow::make_edge(pulling_node, qdata);
    tbb::flow::make_edge(qdata, primary_indexer);
    tbb::flow::make_edge(primary_indexer, breaker);

    pulling_node.try_put(tbb::flow::continue_msg{});

    g.wait_for_all();
}

void Indexer::run_finally_indexing_stage(const IndexingOptions& opts) {
    tbb::flow::graph g;
    tbb::flow::broadcast_node<tbb::flow::continue_msg> entry_point{ g };
    tbb::flow::function_node<tbb::flow::continue_msg> 
        secondary_indexer   { g, 1, SecondaryIndexer{ opts, db_ }           },
        adjacency_builder   { g, 1, ResourcesAdjacencyBuilder{ opts, db_ }  },
        resources_ranker    { g, 1, ResourcesRanker{ opts, db_ }            },
        synchronizer        { g, 1, IndexSynchronizer{ opts, db_ }          };
    tbb::flow::continue_node<tbb::flow::continue_msg> wait_node{ g, [](const auto&) {
        LOG_INFO_WITH_TAGS(
            se::utils::logging::main_category, 
            "Indexing processes were completed."
        );  
        return tbb::flow::continue_msg{};
    }};

    tbb::flow::make_edge(entry_point, secondary_indexer);
    tbb::flow::make_edge(entry_point, adjacency_builder);
    tbb::flow::make_edge(secondary_indexer, wait_node);
    tbb::flow::make_edge(adjacency_builder, resources_ranker);
    tbb::flow::make_edge(resources_ranker, wait_node);
    tbb::flow::make_edge(wait_node, synchronizer);

    entry_point.try_put(tbb::flow::continue_msg{});

    g.wait_for_all();
}

Indexer::~Indexer() {
    bus_->stop();
    bus_pool_.join_all();
}

bool Indexer::try_configure_db() {
    try {
        auto cfg = se::utils::GlobalConfig<Config>::config.db_config();
        db_ = std::make_shared<PostgresDataProvider>(cfg);
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

bool Indexer::try_configure_bus() {
    try {
        const auto& g_cfg = se::utils::GlobalConfig<Config>::config;
        auto cfg = g_cfg.bus_config("indexer.bus");
        bus_ = std::make_shared<se::utils::AMQPBusMixin<IExternalBus>>(cfg);
        bus_->run();
        for(auto i = 0; i < g_cfg.thread_pool("indexer.bus"); ++i) {
            bus_pool_.create_thread([this](){
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

bool Indexer::try_configure_logging(std::shared_ptr<BusHandlerType>& bus_out_handler) {
    try {
        const auto& g_cfg = se::utils::GlobalConfig<Config>::config;
        auto console_handler = quill::stdout_handler();
        console_handler->set_pattern(
            g_cfg.logging_message_pattern("console", "indexer.logging"), 
            g_cfg.logging_time_pattern("console", "indexer.logging"), 
            quill::Timezone::GmtTime
        );
        static_cast<quill::ConsoleHandler*>(console_handler.get())->enable_console_colours();

        auto bus_handler = quill::create_handler<BusHandlerType>("BUS_HANDLER");
        bus_handler->set_pattern(
            g_cfg.logging_message_pattern("bus", "indexer.logging"), 
            g_cfg.logging_time_pattern("bus", "indexer.logging"), 
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
            quill::loglevel_from_string(g_cfg.get<std::string>("indexer.logging.lvl"))
        );

        bus_out_handler = std::static_pointer_cast<BusHandlerType>(bus_handler); 
        bus_out_handler->set_component_name(g_cfg.get<std::string>("indexer.component_name"));

        LOG_INFO_WITH_TAGS(se::utils::logging::main_category, "Configured loggers.");  

        return true;

    } catch(const std::exception& ex) {
        std::cerr << "Exception occured in logger configuring: " << ex.what() << "\n";
        return false;
    }
}

#undef CONFIGURE

} // namespace logger

} // namespace se