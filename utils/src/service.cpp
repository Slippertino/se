#include <seutils/service.hpp>

namespace se {

namespace utils {

void Service::run() {
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

void Service::attach() {
    context_.run();
}

void Service::stop() {
    run_.store(false, std::memory_order_release);
    dummy_worker_->reset();
    dummy_worker_.reset();
    dummy_flag_ = std::make_unique<std::once_flag>();
}

boost::asio::io_context& Service::get_context() {
    return context_;
}

Service::~Service() { }

void Service::dispatch_service_data() { }

void Service::dispatch_loop() {
    while(run_.load(std::memory_order_relaxed)) {
        dispatch_service_data();
        std::this_thread::sleep_for(dispatch_delay_);
    }
}

void Service::dispatch_timed() {
    dispatch_service_data();
    if (run_.load(std::memory_order_relaxed)) {
        auto& timer = get_thread_resource();
        if (timer.expiry() < std::chrono::high_resolution_clock::now()) {
            timer.expires_from_now(dispatch_delay_);
            timer.async_wait(std::bind(&Service::dispatch_timed, this));
        }
    }
}

} // namespace utils

} // namespace se