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

#ifndef JAU_SIMPLE_TIMER_HPP_
#define JAU_SIMPLE_TIMER_HPP_

#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <thread>

#include <jau/ordered_atomic.hpp>
#include <jau/function_def.hpp>
#include <jau/service_runner.hpp>

namespace jau {

    /**
     * A simple timer for timeout and interval applications,
     * using one dedicated service_runner thread per instance.
     *
     * Discussion: It is contemplated to add an implementation using a unique singleton service_runner
     * for multiple timer instances via event loops.
     */
    class simple_timer {
        public:
            typedef simple_timer& Timer0_ref;

            /**
             * User defined timer function using millisecond granularity.
             *
             * Function gets invoked for each timer event,
             * i.e. after reaching the duration in milliseconds set earlier.
             *
             * @return duration in milliseconds for the next timer event or zero to end the timer thread.
             */
            typedef FunctionDef<nsize_t, Timer0_ref> Timer_func_ms;

        private:
            service_runner timer_service;
            std::mutex mtx_timerfunc;
            Timer_func_ms timer_func_ms;
            jau::relaxed_atomic_nsize_t duration_ms;

            void timer_worker(service_runner& sr_ref) {
                do {
                    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
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
                } while ( 0 < duration_ms );
            }

        public:
            simple_timer(const std::string& name, const nsize_t service_shutdown_timeout_ms) noexcept
            : timer_service(name, service_shutdown_timeout_ms, jau::bindMemberFunc(this, &simple_timer::timer_worker)),
              timer_func_ms(), duration_ms(0)
            {}

            /**
             * No copy constructor nor move constructor.
             * @param o
             */
            simple_timer(const simple_timer& o) = delete;

            /**
             * Return the given name of this timer
             */
            const std::string& name() const noexcept { return timer_service.name(); }

            /**
             * Return the thread-id of this timer's worker thread, zero if not running.
             */
            pthread_t thread_id() const noexcept { return timer_service.thread_id(); }

            /**
             * Returns true if timer is running
             */
            bool is_running() const { return timer_service.is_running(); }

            /**
             * Returns true if timer shall stop.
             *
             * This flag can be used by the Timer_func_ms function to determine whether to skip lengthly tasks.
             */
            bool shall_stop() const noexcept { return timer_service.shall_stop(); }

            /**
             * Start the timer with given user Timer_func_ms function and initial duration in milliseconds.
             *
             * @param duration_ms_ initial timer duration until next timer event in milliseconds
             * @param tofunc user Timer_func_ms to be called on next timer event
             * @return true if timer has been started, otherwise false implies timer is already running.
             */
            bool start(nsize_t duration_ms_, Timer_func_ms tofunc) {
                if( timer_service.is_running() ) {
                    return false;
                }
                timer_func_ms = tofunc;
                duration_ms = duration_ms_;
                timer_service.start();
                return true;
            }

            /**
             * Start or update the timer with given user Timer_func_ms function and initial duration in milliseconds.
             *
             * This is faster than calling stop() and start(), however,
             * an already started timer user Timer_func_ms will proceed.
             *
             * @param duration_ms_ initial timer duration until next timer event in milliseconds
             * @param tofunc user Timer_func_ms to be called on next timer event
             */
            void start_or_update(nsize_t duration_ms_, Timer_func_ms tofunc) {
                if( timer_service.is_running() ) {
                    std::unique_lock<std::mutex> lockReader(mtx_timerfunc); // RAII-style acquire and relinquish via destructor
                    timer_func_ms = tofunc;
                    duration_ms = duration_ms_;
                } else {
                    timer_func_ms = tofunc;
                    duration_ms = duration_ms_;
                    timer_service.start();
                }
            }

            /**
             * Stop timer, see service_runner::stop()
             */
            void stop() {
                timer_service.stop();
            }
    };

} /* namespace jau */

#endif /* JAU_SIMPLE_TIMER_HPP_ */
