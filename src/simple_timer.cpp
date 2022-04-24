/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <jau/simple_timer.hpp>

using namespace jau;

void simple_timer::timer_work(service_runner& sr_ref) {
    if( !sr_ref.shall_stop() ) {
        // non-blocking sleep in regards to stop()
        std::unique_lock<std::mutex> lock(sr_ref.mtx_shall_stop()); // RAII-style acquire and relinquish via destructor
        jau::nsize_t sleep_left_ms = duration_ms;
        while( !sr_ref.shall_stop() && 0 < sleep_left_ms ) {
            const std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
            const std::cv_status s = sr_ref.cv_shall_stop().wait_until(lock, t0 + std::chrono::milliseconds(sleep_left_ms));
            const jau::nsize_t slept = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - t0 ).count();
            sleep_left_ms = sleep_left_ms >= slept ? sleep_left_ms - slept : 0;
            if( std::cv_status::timeout == s && !sr_ref.shall_stop() ) {
                // Made it through whole period w/o being stopped nor spurious wakeups
                // This branch is only for documentation purposes, as shall_stop is being tested
                sleep_left_ms = 0;
                duration_ms = 0;
            }
        }
    }
    Timer_func_ms tf;
    {
        std::unique_lock<std::mutex> lockReader(mtx_timerfunc); // RAII-style acquire and relinquish via destructor
        tf = timer_func_ms;
    }
    if( !tf.isNullType() && !sr_ref.shall_stop() ) {
        duration_ms = tf(*this);
    } else {
        duration_ms = 0;
    }
    if( 0 == duration_ms ) {
        sr_ref.set_shall_stop();
    }
}

simple_timer::simple_timer(const std::string& name, const nsize_t service_shutdown_timeout_ms) noexcept
: timer_service(name, service_shutdown_timeout_ms, jau::bindMemberFunc(this, &simple_timer::timer_work)),
  timer_func_ms(), duration_ms(0)
{}

bool simple_timer::start(nsize_t duration_ms_, Timer_func_ms tofunc) noexcept {
    if( is_running() ) {
        return false;
    }
    timer_func_ms = tofunc;
    duration_ms = duration_ms_;
    timer_service.start();
    return true;
}

void simple_timer::start_or_update(nsize_t duration_ms_, Timer_func_ms tofunc) noexcept {
    if( is_running() ) {
        std::unique_lock<std::mutex> lockReader(mtx_timerfunc); // RAII-style acquire and relinquish via destructor
        timer_func_ms = tofunc;
        duration_ms = duration_ms_;
    } else {
        timer_func_ms = tofunc;
        duration_ms = duration_ms_;
        timer_service.start();
    }
}
