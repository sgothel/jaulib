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

#include <jau/service_runner.hpp>

extern "C" {
    #include <unistd.h>
    #include <sys/socket.h>
    #include <poll.h>
    #include <signal.h>
}

#include <jau/debug.hpp>

#include <jau/basic_algos.hpp>

using namespace jau;

void service_runner::service_thread() {
    {
        const std::lock_guard<std::mutex> lock(mtx_lifecycle); // RAII-style acquire and relinquish via destructor
        service_init_locked(*this);
        running = true;
        DBG_PRINT("%s::worker Started", name_.c_str());
    }
    cv_init.notify_all(); // have mutex unlocked before notify_all to avoid pessimistic re-block of notified wait() thread.

    thread_local jau::call_on_release thread_cleanup([&]() {
        DBG_PRINT("%s::worker::ThreadCleanup: serviceRunning %d -> 0", name_.c_str(), running.load());
        running = false;
        cv_init.notify_all();
    });

    while( !shall_stop_ ) {
        service_work(*this);
    }
    {
        const std::lock_guard<std::mutex> lock(mtx_lifecycle); // RAII-style acquire and relinquish via destructor
        WORDY_PRINT("%s::worker: Ended", name_.c_str());
        service_end_locked(*this);
        thread_id_ = 0;
        running = false;
    }
    cv_init.notify_all(); // have mutex unlocked before notify_all to avoid pessimistic re-block of notified wait() thread.
}

const pid_t service_runner::pid_self = getpid();

static void sigaction_handler(int sig, siginfo_t *info, void *ucontext) noexcept {
    bool pidMatch = info->si_pid == service_runner::pid_self;
    WORDY_PRINT("service_runner.sigaction: sig %d, info[code %d, errno %d, signo %d, pid %d, uid %d, fd %d], pid-self %d (match %d)",
            sig, info->si_code, info->si_errno, info->si_signo,
            info->si_pid, info->si_uid, info->si_fd,
            service_runner::pid_self, pidMatch);
    (void)ucontext;

    if( !pidMatch || SIGALRM != sig ) {
        return;
    }
#if 0
    // We do not de-install the handler on single use,
    // as we act for multiple SIGALRM events within direct-bt
    remove_sighandler();
#endif
}

bool service_runner::install_sighandler() noexcept {
    struct sigaction sa_setup;
    bzero(&sa_setup, sizeof(sa_setup));
    sa_setup.sa_sigaction = sigaction_handler;
    sigemptyset(&(sa_setup.sa_mask));
    sa_setup.sa_flags = SA_SIGINFO;
    if( 0 != sigaction( SIGALRM, &sa_setup, NULL ) ) {
        ERR_PRINT("service_runner::install_sighandler: Setting sighandler");
        return false;
    }
    DBG_PRINT("service_runner::install_sighandler: OK");
    return true;
}

bool service_runner::remove_sighandler() noexcept {
    struct sigaction sa_setup;
    bzero(&sa_setup, sizeof(sa_setup));
    sa_setup.sa_handler = SIG_DFL;
    sigemptyset(&(sa_setup.sa_mask));
    sa_setup.sa_flags = 0;
    if( 0 != sigaction( SIGALRM, &sa_setup, NULL ) ) {
        ERR_PRINT("service_runner::remove_sighandler: Resetting sighandler");
        return false;
    }
    DBG_PRINT("service_runner::remove_sighandler: OK");
    return true;
}

service_runner::service_runner(const std::string& name__,
                               nsize_t service_shutdown_timeout_ms__,
                               Callback service_work_,
                               Callback service_init_locked_,
                               Callback service_end_locked_) noexcept
: name_(name__),
  service_shutdown_timeout_ms_(service_shutdown_timeout_ms__),
  service_work(service_work_),
  service_init_locked(service_init_locked_),
  service_end_locked(service_end_locked_),
  shall_stop_(true), running(false),
  thread_id_(0)
{
    DBG_PRINT("%s::ctor", name_.c_str());
}

service_runner::~service_runner() noexcept {
    DBG_PRINT("%s::dtor: Begin", name_.c_str());
    stop();
    DBG_PRINT("%s::dtor: End", name_.c_str());
}

void service_runner::set_shall_stop() noexcept {
    {
        const std::lock_guard<std::mutex> lock_stop(mtx_shall_stop_); // RAII-style acquire and relinquish via destructor
        shall_stop_ = true;
    }
    cv_shall_stop_.notify_all(); // have mutex unlocked before notify_all to avoid pessimistic re-block of notified wait() thread.
}

void service_runner::start() noexcept {
    DBG_PRINT("%s::start: Begin: %s", name_.c_str(), toString().c_str());
    /**
     * We utilize a global SIGALRM handler, since we only can install one handler.
     */
    std::unique_lock<std::mutex> lock(mtx_lifecycle); // RAII-style acquire and relinquish via destructor
    {
        const std::lock_guard<std::mutex> lock_stop(mtx_shall_stop_); // RAII-style acquire and relinquish via destructor
        shall_stop_ = false;
    }
    cv_shall_stop_.notify_all(); // have mutex unlocked before notify_all to avoid pessimistic re-block of notified wait() thread.

    if( running ) {
        DBG_PRINT("%s::start: End.0: %s", name_.c_str(), toString().c_str());
        return;
    }

    std::thread t(&service_runner::service_thread, this); // @suppress("Invalid arguments")
    thread_id_ = t.native_handle();
    // Avoid 'terminate called without an active exception'
    // as t may end due to I/O errors.
    t.detach();

    while( false == running ) {
        cv_init.wait(lock);
    }
    DBG_PRINT("%s::start: End.X: %s", name_.c_str(), toString().c_str());
}

bool service_runner::stop() noexcept {
    DBG_PRINT("%s::stop: Begin: %s", name_.c_str(), toString().c_str());

    std::unique_lock<std::mutex> lock(mtx_lifecycle); // RAII-style acquire and relinquish via destructor
    const pthread_t tid_service = thread_id_;
    const bool is_service = tid_service == pthread_self();
    DBG_PRINT("%s::stop: service[running %d, shall_stop %d, is_service %d, tid %p)",
              name_.c_str(), running.load(), shall_stop_.load(), is_service, (void*)tid_service);
    set_shall_stop();
    bool result;
    if( running ) {
        if( !is_service ) {
            if( 0 != tid_service ) {
                int kerr;
                if( 0 != ( kerr = pthread_kill(tid_service, SIGALRM) ) ) {
                    ERR_PRINT("%s::stop: pthread_kill %p FAILED: %d", name_.c_str(), (void*)tid_service, kerr);
                }
            }
            // Ensure the reader thread has ended, no runaway-thread using *this instance after destruction
            result = true;
            while( true == running && result ) {
                std::cv_status s { std::cv_status::no_timeout };
                if( 0 < service_shutdown_timeout_ms_ ) {
                    std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
                    s = cv_init.wait_until(lock, t0 + std::chrono::milliseconds(service_shutdown_timeout_ms_));
                } else {
                    cv_init.wait(lock);
                }
                if( std::cv_status::timeout == s && true == running ) {
                    ERR_PRINT("%s::stop: Timeout (force !running): %s", name_.c_str(), toString().c_str());
                    result = false; // bail out w/ false
                }
            }
        } else {
            // is_service
            result = false; // initiated, but not stopped yet
        }
    } else {
        result = true;
    }
    DBG_PRINT("%s::stop: End: Result %d, %s", name_.c_str(), result, toString().c_str());
    return result;
}

bool service_runner::join() noexcept {
    DBG_PRINT("%s::join: Begin: %s", name_.c_str(), toString().c_str());
    std::unique_lock<std::mutex> lockReader(mtx_lifecycle); // RAII-style acquire and relinquish via destructor

    const bool is_service = thread_id_ == pthread_self();
    DBG_PRINT("%s::join: is_service %d, %s", name_.c_str(), is_service, toString().c_str());
    bool result;
    if( running ) {
        if( !is_service ) {
            // Ensure the reader thread has ended, no runaway-thread using *this instance after destruction
            result = true;
            while( true == running && result ) {
                std::cv_status s { std::cv_status::no_timeout };
                if( 0 < service_shutdown_timeout_ms_ ) {
                    std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
                    s = cv_init.wait_until(lockReader, t0 + std::chrono::milliseconds(service_shutdown_timeout_ms_));
                } else {
                    cv_init.wait(lockReader);
                }
                if( std::cv_status::timeout == s && true == running ) {
                    ERR_PRINT("%s::join: Timeout (force !running): %s", name_.c_str(), toString().c_str());
                    result = false; // bail out w/ false
                }
            }
        } else {
            // is_service
            result = false;
        }
    } else {
        result = true;
    }
    DBG_PRINT("%s::join: End: Result %d, %s", name_.c_str(), result, toString().c_str());
    return result;
}

std::string service_runner::toString() const noexcept {
    return "ServiceRunner["+name_+", running "+std::to_string(is_running())+", shall_stop "+std::to_string(shall_stop_)+", thread_id "+to_hexstring((void*)thread_id_)+"]";
}
