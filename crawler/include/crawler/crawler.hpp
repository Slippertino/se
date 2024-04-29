#pragma once

#include <boost/thread/thread.hpp>
#include <cds/gc/hp.h>
#include <cds/threading/model.h>
#include <crawler/config.hpp>
#include <crawler/core/resources_repository.hpp>
#include <crawler/core/resource_distributor.hpp>
#include <crawler/core/resource_processor.hpp>
#include <crawler/db/postgres_data_provider.hpp>
#include <crawler/bus/amqp/amqp_bus.hpp>
#include <crawler/logging/logging.hpp>
#include <crawler/logging/bus_handler.hpp>

namespace crawler {

class Crawler : public utils::Service {
public:
    Crawler();
    void setup(const std::string& cfg);
    void stop() override;
    ~Crawler();

protected:
    void dispatch_service_data() override;

private:
    bool try_configure_db();
    bool try_configure_bus();
    bool try_configure_queue();
    bool try_configure_distributor();
    bool try_configure_processor();
    bool try_configure_logging(std::shared_ptr<logging::BusHandler>& bus_out_handler);

private:
    std::atomic<bool> config_errors_;
    std::shared_ptr<ResourceDistributor> distributor_;
    std::shared_ptr<ResourcesRepository> queue_;
    std::shared_ptr<ResourceProcessor> processor_;
    std::shared_ptr<PostgresDataProvider> db_;
    std::shared_ptr<AMQPBus> bus_;

private:
    boost::thread_group bus_group_;
    boost::thread_group distributor_group_;
    boost::thread_group queue_group_;
    boost::thread_group processor_group_;
}; 

} // namespace crawler