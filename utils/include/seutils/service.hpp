#pragma once

#include <iostream>
#include <chrono>
#include <boost/asio.hpp>
#include "threaded_resource.hpp"

namespace se {

namespace utils {

class Service : public ThreadedResource<Service, boost::asio::high_resolution_timer> {
public:
    template<typename Dur = std::chrono::microseconds>
    Service( 
        bool enabled_dispatch, 
        bool enabled_dispatch_loop = false, 
        Dur&& dispatch_delay = Dur(0), 
        size_t max_dispatch_workers = std::numeric_limits<size_t>::max()
    ) : 
        enabled_internal_dispatch_{ enabled_dispatch },
        enabled_dispatch_loop_{ enabled_dispatch_loop },
        max_dispatch_workers_{ max_dispatch_workers },
        dispatch_delay_{ std::chrono::duration_cast<std::chrono::microseconds>(dispatch_delay) },
        dummy_flag_{ std::make_unique<std::once_flag>() },
        dummy_worker_{ nullptr }
    { }

    virtual void run();
    void attach();
    virtual void stop();

    boost::asio::io_context& get_context();

    template<std::same_as<boost::asio::high_resolution_timer> R>
    boost::asio::high_resolution_timer create_thread_resource() {
        return boost::asio::high_resolution_timer { context_ };
    }

    virtual ~Service();

protected:
    virtual void dispatch_service_data();

private:
    void dispatch_loop();
    void dispatch_timed();

protected:
    boost::asio::io_context context_;

private:
    using worker_t = decltype(boost::asio::make_work_guard(context_));

private:
    const bool enabled_internal_dispatch_;
    const bool enabled_dispatch_loop_;
    const size_t max_dispatch_workers_;
    const std::chrono::microseconds dispatch_delay_;
    std::atomic<bool> run_;
    std::atomic<size_t> workers_count_;
    std::unique_ptr<std::once_flag> dummy_flag_;
    std::unique_ptr<worker_t> dummy_worker_;
};

} // namespace utils

} // namespace se