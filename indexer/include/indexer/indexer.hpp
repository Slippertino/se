#pragma once

#include <memory>
#include <boost/thread/thread.hpp>
#include <seutils/service.hpp>
#include <seutils/amqp/amqp_bus.hpp>
#include <seutils/logging/logging.hpp>
#include <seutils/logging/bus_handler.hpp>
#include <indexer/db/postgres_data_provider.hpp>
#include <indexer/bus/external_bus.hpp>
#include <indexer/nodes/nodes.hpp>
#include <indexer/indexing_options.hpp>
#include <indexer/config.hpp>

namespace se {

namespace indexer {

class Indexer final : public se::utils::Service {
private:
    using BusHandlerType = se::utils::logging::BusHandler<IExternalBus>;

public:
    Indexer();

    void setup();
    void run() override;

    ~Indexer();
    
private:
    void run_primary_indexing_stage(const IndexingOptions& opts);
    void run_finally_indexing_stage(const IndexingOptions& opts);

private:
    bool try_configure_db();
    bool try_configure_bus();
    bool try_configure_logging(std::shared_ptr<BusHandlerType>& bus_out_handler);

private:
    std::atomic<bool> config_errors_;

private:
    std::shared_ptr<IDataProvider> db_;
    std::shared_ptr<se::utils::AMQPBusMixin<IExternalBus>> bus_;

private:
    boost::thread_group bus_pool_;
};

} // namespace indexer

} // namespace se