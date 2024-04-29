#pragma once

#include <iostream>
#include <chrono>
#include <boost/asio.hpp>
#include "threaded_resource.hpp"

namespace crawler {

namespace utils {

class Service : public utils::ThreadedResource<Service, boost::asio::high_resolution_timer> {
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

    virtual void run() {
        std::call_once(*dummy_flag_, [&]() {
            run_.store(true, std::memory_order_release);
            dummy_worker_ = std::make_unique<worker_t>(context_.get_executor());
        });

        if (!enabled_internal_dispatch_ || 
            workers_count_.load(std::memory_order_acquire) == max_dispatch_workers_)
            return;
        
        workers_count_.fetch_add(1, std::memory_order_acq_rel);
        
        if (!enabled_dispatch_loop_) {
            auto& timer = get_thread_resource();
            timer.expires_from_now(dispatch_delay_);
            timer.async_wait(std::bind(&Service::dispatch_timed, this));
        }
        else {
            dispatch_loop();
        }
    }

    void attach() {
        context_.run();
    }

    virtual void stop() {
        run_.store(false, std::memory_order_release);
        dummy_worker_->reset();
        dummy_worker_.reset();
        dummy_flag_ = std::make_unique<std::once_flag>();
    }

    boost::asio::io_context& get_context() {
        return context_;
    }

    template<std::same_as<boost::asio::high_resolution_timer> R>
    boost::asio::high_resolution_timer create_thread_resource() {
        return boost::asio::high_resolution_timer { context_ };
    }

    virtual ~Service() { }

protected:
    virtual void dispatch_service_data() { }

private:
    void dispatch_loop() {
        while(run_.load(std::memory_order_relaxed)) {
            dispatch_service_data();
            std::this_thread::sleep_for(dispatch_delay_);
        }
    }

    void dispatch_timed() {
        dispatch_service_data();
        if (run_.load(std::memory_order_relaxed)) {
            auto& timer = get_thread_resource();
            if (timer.expiry() < std::chrono::high_resolution_clock::now()) {
                timer.expires_from_now(dispatch_delay_);
                timer.async_wait(std::bind(&Service::dispatch_timed, this));
            }
        }
    }

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

} // namespace crawler