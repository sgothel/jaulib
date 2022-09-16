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

#ifndef JAU_SERVICE_RUNNER_HPP_
#define JAU_SERVICE_RUNNER_HPP_

#include <cstring>
#include <string>
#include <cstdint>
#include <limits>
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <algorithm>

#include <jau/cpp_lang_util.hpp>
#include <jau/ordered_atomic.hpp>
#include <jau/fraction_type.hpp>
#include <jau/functional.hpp>

extern "C" {
    #include <unistd.h>
    #include <pthread.h>
}

namespace jau {

    /** \addtogroup Concurrency
     *
     *  @{
     */

    /**
     * Service runner, a reusable dedicated thread performing custom user services.
     */
    class service_runner {
        public:
            typedef service_runner& service_runner_ref;
            typedef function<void(service_runner_ref)> Callback;

            static const ::pid_t pid_self;

        private:
            std::string name_;

            /**
             * Maximum duration in fractions of seconds to wait for service to stop at stop() and join(), where fractions_i64::zero waits infinitely
             */
            fraction_i64 service_shutdown_timeout_;

            Callback service_work;
            Callback service_init_locked;
            Callback service_end_locked;

            std::mutex mtx_shall_stop_;
            std::condition_variable cv_shall_stop_;
            jau::sc_atomic_bool shall_stop_;
            jau::sc_atomic_bool running;
            ::pthread_t thread_id_;

            std::mutex mtx_lifecycle;
            std::condition_variable cv_init;

            void service_thread();

            static bool install_sighandler() noexcept;

        public:
            /**
             * Remove the sighandler
             */
            static bool remove_sighandler() noexcept;

            /**
             * Install the singleton SIGALRM sighandler instance.
             * - First call will install the sighandler
             * - Should be called at least once within the application using jau::service_runner
             */
            static bool singleton_sighandler() noexcept {
                /**
                 * Thread safe starting with C++11 6.7:
                 *
                 * If control enters the declaration concurrently while the variable is being initialized,
                 * the concurrent execution shall wait for completion of the initialization.
                 *
                 * (Magic Statics)
                 *
                 * Avoiding non-working double checked locking.
                 */
                static bool r = install_sighandler();
                return r;
            }

            /**
             * Service runner constructor.
             *
             * start() shall be issued to kick off this service.
             *
             * @param name service name
             * @param service_shutdown_timeout maximum duration in fractions of seconds to wait for service to stop at stop() and join(), where fractions_i64::zero waits infinitely
             * @param service_work service working function
             * @param service_init_locked optional service init function, lifecycle mutex is locked
             * @param service_end_locked optional service end function, lifecycle mutex is locked
             */
            service_runner(const std::string& name,
                           const fraction_i64& service_shutdown_timeout,
                           Callback service_work,
                           Callback service_init_locked = Callback(),
                           Callback service_end_locked = Callback()) noexcept;

            /**
             * Service runner destructor.
             *
             * Issues stop()
             */
            ~service_runner() noexcept;

            /**
             * Return the given name of this service
             */
            const std::string& name() const noexcept { return name_; }

            /**
             * Returns maximum duration in fractions of seconds to wait for service to stop at stop() and join(), where fractions_i64::zero waits infinitely
             * @see stop()
             * @see join()
             */
            fraction_i64 service_shutdown_timeout() const noexcept { return service_shutdown_timeout_; }

            /**
             * Return the thread-id of this service service thread, zero if not running.
             */
            pthread_t thread_id() const noexcept { return thread_id_; }

            /**
             * Returns true if service is running
             *
             * @see start()
             * @see stop()
             * @see join()
             */
            bool is_running() const noexcept { return running; }

            /**
             * Returns true if service shall stop.
             *
             * This flag can be used by the service_work Callback to determine whether to skip lengthly tasks,
             * or even to skip stopping this service (again).
             *
             * @see is_running()
             * @see set_shall_stop()
             * @see mtx_shall_stop()
             * @see cv_shall_stop()
             * @see start()
             * @see stop()
             * @see join()
             */
            bool shall_stop() const noexcept { return shall_stop_; }

            /**
             * Helper function to easy FunctionDef usage w/o creating a lambda alike capture
             * with same semantics as shall_stop().
             *
             * The const qualifier has also been lifted, since free function pointer declarations can't match it.
             *
             * @param dummy a dummy argument to help FunctionDef template parameter pack, not used
             * @see shall_stop()
             */
            bool shall_stop2(int dummy) /* const */ noexcept { (void)dummy; return shall_stop_; }

            /**
             * Marks the service thread to stop in due process by flagging `shall stop` to `true`.
             * @see is_running()
             * @see shall_stop()
             * @see mtx_shall_stop()
             * @see cv_shall_stop()
             * @see start()
             * @see stop()
             * @see join()
             */
            void set_shall_stop() noexcept;

            /** mtx_shall_stop() and cv_shall_stop() allows caller to be notified when shall_stop() changes, i.e. start(), set_shall_stop() or stop() is called. */
            std::mutex& mtx_shall_stop() noexcept { return mtx_shall_stop_; }

            /** mtx_shall_stop() and cv_shall_stop() allows caller to be notified when shall_stop() changes, i.e. start(), set_shall_stop() or stop() is called. */
            std::condition_variable& cv_shall_stop() noexcept { return  cv_shall_stop_; }

            /**
             * Starts this service, if not running already.
             *
             * Methods blocks the current thread until service is started.
             *
             * @see is_running()
             * @see shall_stop()
             * @see set_shall_stop()
             * @see mtx_shall_stop()
             * @see cv_shall_stop()
             * @see stop()
             * @see join()
             * @see service_shutdown_timeout()
             */
            void start() noexcept;

            /**
             * Stops this service, if running.
             *
             * If called from the service thread, method just issues set_shall_stop() without blocking,
             * otherwise methods blocks the current thread until service is stopped.
             *
             * Maximum blocked wait period is optionally limited by service_shutdown_timeout().
             *
             * Method attempts to stop the service thread
             * - by flagging `shall stop` via set_shall_stop()
             * - if not called from the service thread itself:
             *   - sending `SIGALRM` to the service thread
             *   - waiting until service thread has stopped or timeout occurred
             *
             * Implementation requires a `SIGALRM` handler to be install,
             * e.g. using singleton_sighandler().
             *
             * @returns true if thread has been stopped or false if timeout has been hit
             *
             * @see is_running()
             * @see shall_stop()
             * @see set_shall_stop()
             * @see mtx_shall_stop()
             * @see cv_shall_stop()
             * @see start()
             * @see join()
             * @see service_shutdown_timeout()
             * @see singleton_sighandler()
             */
            bool stop() noexcept;

            /**
             * Blocks the current thread until service is stopped
             * or returns immediately if not running or called from the service thread.
             *
             * Maximum blocked wait period is optionally limited by service_shutdown_timeout().
             *
             * @returns true if thread has been stopped or false if timeout has been hit
             * @see is_running()
             * @see shall_stop()
             * @see set_shall_stop()
             * @see mtx_shall_stop()
             * @see cv_shall_stop()
             * @see start()
             * @see stop()
             * @see service_shutdown_timeout()
             */
            bool join() noexcept;

            /**
             * Returns a string representation of this service
             */
            std::string toString() const noexcept;
    };

    /**@}*/
} // namespace jau



#endif /* JAU_SERVICE_RUNNER_HPP_ */
