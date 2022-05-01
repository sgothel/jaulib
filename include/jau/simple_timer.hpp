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
#include <jau/fraction_type.hpp>
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
             * User defined timer function using custom granularity via fraction_i64.
             *
             * Function gets invoked for each timer event,
             * i.e. after reaching the duration set earlier.
             *
             * @return duration in fractions of seconds for the next timer event or zero to end the timer thread.
             */
            typedef FunctionDef<fraction_i64, Timer0_ref> Timer_func;

        private:
            service_runner timer_service;
            std::mutex mtx_timerfunc;
            Timer_func timer_func;
            // Note: Requires libatomic with libstdc++10
            sc_atomic_fraction_i64 duration;

            void timer_work(service_runner& sr_ref);

        public:
            /**
             * Constructs a new service
             * @param name thread name of this service
             * @param service_shutdown_timeout maximum duration in fractions of seconds to wait for service to stop at stop(), where fractions_i64::zero waits infinitely
             */
            simple_timer(const std::string& name, const fraction_i64& service_shutdown_timeout) noexcept;

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
            bool is_running() const noexcept { return timer_service.is_running(); }

            /**
             * Returns true if timer shall stop.
             *
             * This flag can be used by the Timer_func function to determine whether to skip lengthly tasks.
             */
            bool shall_stop() const noexcept { return timer_service.shall_stop(); }

            /**
             * Start the timer with given user Timer_func function and initial duration.
             *
             * @param duration_ initial timer duration in fractions of seconds until next timer event
             * @param tofunc user Timer_func to be called on next timer event
             * @return true if timer has been started, otherwise false implies timer is already running.
             */
            bool start(const fraction_i64& duration_, Timer_func tofunc) noexcept;

            /**
             * Start or update the timer with given user Timer_func function and initial duration.
             *
             * This is faster than calling stop() and start(), however,
             * an already started timer user Timer_func will proceed.
             *
             * @param duration_ initial timer duration in fractions of seconds until next timer event
             * @param tofunc user Timer_func to be called on next timer event
             */
            void start_or_update(const fraction_i64& duration_, Timer_func tofunc) noexcept;

            /**
             * Stop timer, see service_runner::stop()
             */
            void stop() noexcept { timer_service.stop(); }
    };

} /* namespace jau */

#endif /* JAU_SIMPLE_TIMER_HPP_ */
