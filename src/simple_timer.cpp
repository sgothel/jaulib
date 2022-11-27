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
        bool overflow = false;
        const fraction_timespec timeout_time = getMonotonicTime() + fraction_timespec(duration, &overflow);
        if( overflow ) {
            sr_ref.set_shall_stop(); // bail out
        }
        std::cv_status s { std::cv_status::no_timeout };
        while( !sr_ref.shall_stop() && std::cv_status::timeout != s ) {
            s = wait_until( sr_ref.cv_shall_stop(), lock, timeout_time );
            if( std::cv_status::timeout == s && !sr_ref.shall_stop() ) {
                duration = fractions_i64::zero;
            }
        }
    }
    Timer_func tf;
    {
        std::unique_lock<std::mutex> lockReader(mtx_timerfunc); // RAII-style acquire and relinquish via destructor
        tf = timer_func;
    }
    if( !tf.is_null() && !sr_ref.shall_stop() ) {
        duration = tf(*this);
    } else {
        duration = fractions_i64::zero;
    }
    if( fractions_i64::zero == duration.load() ) {
        sr_ref.set_shall_stop();
    }
}

simple_timer::simple_timer(const std::string& name, const fraction_i64& service_shutdown_timeout) noexcept
: timer_service(name, service_shutdown_timeout, jau::bind_member(this, &simple_timer::timer_work)),
  timer_func(), duration()
{}

bool simple_timer::start(const fraction_i64& duration_, Timer_func tofunc) noexcept {
    if( is_running() ) {
        return false;
    }
    timer_func = std::move(tofunc);
    duration = duration_;
    timer_service.start();
    return true;
}

void simple_timer::start_or_update(const fraction_i64& duration_, Timer_func tofunc) noexcept {
    if( is_running() ) {
        std::unique_lock<std::mutex> lockReader(mtx_timerfunc); // RAII-style acquire and relinquish via destructor
        timer_func = std::move(tofunc);
        duration = duration_;
    } else {
        timer_func = std::move(tofunc);
        duration = duration_;
        timer_service.start();
    }
}
