#pragma once

#include <boost/thread/thread.hpp>
#include <cds/gc/hp.h>
#include <cds/threading/model.h>
#include <crawler/config.hpp>
#include <crawler/core/resources_repository.hpp>
#include <crawler/core/resource_distributor.hpp>
#include <crawler/core/resource_processor.hpp>
#include <crawler/db/postgres_data_provider.hpp>
#include <crawler/logging/logging.hpp>
#include <seutils/amqp/amqp_bus.hpp>
#include <seutils/logging/bus_handler.hpp>

namespace se {

namespace crawler {

class Crawler : public se::utils::Service {
public:
    Crawler();
    void setup(const std::string& cfg);
    void stop() override;
    ~Crawler();

protected:
    void dispatch_service_data() override;

private:
    using BusHandlerType = se::utils::logging::BusHandler<IExternalBus>;
    
private:
    bool try_configure_db();
    bool try_configure_bus();
    bool try_configure_queue();
    bool try_configure_distributor();
    bool try_configure_processor();
    bool try_configure_logging(std::shared_ptr<BusHandlerType>& bus_out_handler);

private:
    std::atomic<bool> config_errors_;
    std::shared_ptr<ResourceDistributor> distributor_;
    std::shared_ptr<ResourcesRepository> queue_;
    std::shared_ptr<ResourceProcessor> processor_;
    std::shared_ptr<PostgresDataProvider> db_;
    std::shared_ptr<se::utils::AMQPBusMixin<IExternalBus>> bus_;

private:
    boost::thread_group bus_group_;
    boost::thread_group distributor_group_;
    boost::thread_group queue_group_;
    boost::thread_group processor_group_;
}; 

} // namespace crawler

} // namespace se