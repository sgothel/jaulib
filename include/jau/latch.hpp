/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
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

#ifndef JAU_LATCH_HPP_
#define JAU_LATCH_HPP_

#include <cstdint>
#include <mutex>
#include <condition_variable>

#include <jau/ordered_atomic.hpp>
#include <jau/fraction_type.hpp>
#include <jau/basic_types.hpp>

namespace jau {

    /** \addtogroup Concurrency
     *
     *  @{
     */

    /**
     * Inspired by std::latch of C++20
     *
     * Adds count_up() as an extension, allowing to dynamically add *events* to required to complete.
     *
     * @see https://en.cppreference.com/w/cpp/thread/latch
     */
    class latch {
        private:
            mutable std::mutex mtx_cd;
            mutable std::condition_variable cv;
            jau::sc_atomic_size_t count;

        public:
            /** Returns the maximum value of the internal counter supported by the implementation. */
            static constexpr size_t max() noexcept { return std::numeric_limits<size_t>::max(); }

            /**
             * Initialize instance with counter zero.
             *
             * Useful for member instances in combination with count_up() or with set() before count_down().
             *
             * Extension of std::latch.
             */
            latch() noexcept
            : count(0) {}

            /**
             * Initialize instance with given counter.
             *
             * Compatible with std::latch.
             *
             * @param count_
             */
            latch(const size_t count_) noexcept
            : count(count_) {}

            /**
             * No copy constructor nor move constructor.
             *
             * Compatible with std::latch.
             *
             * @param o
             */
            latch(const latch& o) = delete;

            /**
             * Return the current atomic internal counter.
             *
             * Extension of std::latch.
             */
            size_t value() const noexcept { return count; }

            /**
             * Atomically decrements the internal counter by n
             * and notifies all blocked wait() threads if zero is reached.
             *
             * If n is greater than the value of the internal counter, the counter is set to zero.
             *
             * This operation strongly happens-before all calls that are unblocked on this latch.
             *
             * Compatible with std::latch.
             *
             * @param n the value by which the internal counter is decreased, defaults to 1
             */
            void count_down(const size_t n = 1) noexcept {
                bool notify;
                {
                    std::unique_lock<std::mutex> lock(mtx_cd); // Avoid data-race on concurrent count_down() and wait*() calls
                    if( n < count ) {
                        count = count - n;
                        notify = false;
                    } else {
                        count = 0;
                        notify = true;
                    }
                }
                if( notify ) {
                    cv.notify_all();
                }
            }

            /**
             * Atomically increments the internal counter by n, i.e. adds value to count down.
             *
             * If internal counter + n is greater than the maximum value of the internal counter, the counter is set to its maximum.
             *
             * This operation strongly happens-before all calls that are unblocked on this latch.
             *
             * Extension of std::latch.
             *
             * @param n the value by which the internal counter is increased, defaults to 1
             */
            void count_up(const size_t n = 1) noexcept {
                std::unique_lock<std::mutex> lock(mtx_cd); // Avoid data-race on concurrent count_down() and wait*() calls
                if( n <= std::numeric_limits<std::size_t>::max() - count ) {
                    count = count + n;
                } else {
                    count = std::numeric_limits<std::size_t>::max();
                }
            }

            /**
             * Atomically set the internal counter by n.
             *
             * This operation strongly happens-before all calls that are unblocked on this latch.
             *
             * Extension of std::latch.
             *
             * @param n the value to be assigned to the internal counter
             */
            void set(const size_t n) noexcept {
                std::unique_lock<std::mutex> lock(mtx_cd); // Avoid data-race on concurrent count_down() and wait*() calls
                count = n;
            }

            /**
             * Returns true only if the internal counter has reached zero.
             *
             * Compatible with std::latch.
             */
            bool try_wait() const noexcept {
                return 0 == count;
            }

            /**
             * Blocks the calling thread until the internal counter reaches 0.
             *
             * If the internal counter is zero already, returns immediately.
             *
             * Compatible with std::latch.
             */
            void wait() const noexcept {
                if( 0 < count ) {
                    std::unique_lock<std::mutex> lock(mtx_cd);
                    while( 0 < count ) {
                        cv.wait(lock);
                    }
                }
            }

            /**
             * Atomically decrements the internal counter by n and (if necessary) blocks the calling thread until the counter reaches zero.
             *
             * Equivalent to `count_down(n); wait();`.
             *
             * Compatible with std::latch.
             *
             * @param n the value by which the internal counter is decreased, defaults to 1
             */
            void arrive_and_wait(const size_t n = 1) noexcept {
                count_down(n);
                wait();
            }

            /**
             * Blocks the calling thread until the internal counter reaches 0 or the given timeout duration has expired.
             *
             * If the internal counter is zero already, returns immediately.
             *
             * Implementation uses wait_for() w/ a monotonic clock and fraction_i64.
             *
             * Extension of std::latch.
             *
             * @param timeout_duration maximum duration in fractions of seconds to wait
             * @return true if internal counter has reached zero, otherwise a timeout has occurred.
             */
            bool wait_for(const fraction_i64& timeout_duration) const noexcept {
                if( 0 < count ) {
                    std::unique_lock<std::mutex> lock(mtx_cd);
                    const fraction_timespec timeout_time = getMonotonicTime() + timeout_duration;
                    while( 0 < count ) {
                        std::cv_status s = wait_until(cv, lock, timeout_time);
                        if( 0 == count ) {
                            return true;
                        }
                        if( std::cv_status::timeout == s ) {
                            return false;
                        }
                    }
                }
                return true;
            }

            /**
             * Atomically decrements the internal counter by n and (if necessary) blocks the calling thread until the counter reaches zero
             * or the given timeout duration has expired.
             *
             * Equivalent to `count_down(n); wait(timeout_duration);`.
             *
             * Implementation uses `std::chrono::steady_clock::now()` and fraction_i64.
             *
             * Extension of std::latch.
             *
             * @param timeout_duration maximum duration in fractions of seconds to wait
             * @param n the value by which the internal counter is decreased, defaults to 1
             * @return true if internal counter has reached zero, otherwise a timeout has occurred.
             */
            bool arrive_and_wait_for(const fraction_i64& timeout_duration, const size_t n = 1) noexcept {
                count_down(n);
                return wait_for(timeout_duration);
            }

            /**
             * Blocks the calling thread until the internal counter reaches 0 or the given timeout duration has expired.
             *
             * If the internal counter is zero already, returns immediately.
             *
             * Implementation uses `std::chrono::steady_clock::now()`.
             *
             * Extension of std::latch.
             *
             * @tparam Rep
             * @tparam Period
             * @param timeout_duration maximum duration to wait
             * @return true if internal counter has reached zero, otherwise a timeout has occurred.
             */
            template<typename Rep, typename Period>
            bool wait_for(const std::chrono::duration<Rep, Period>& timeout_duration) const noexcept {
                if( 0 < count ) {
                    std::unique_lock<std::mutex> lock(mtx_cd);
                    std::chrono::steady_clock::time_point timeout_time = std::chrono::steady_clock::now() + timeout_duration;
                    while( 0 < count ) {
                        std::cv_status s = cv.wait_until(lock, timeout_time );
                        if( 0 == count ) {
                            return true;
                        }
                        if( std::cv_status::timeout == s ) {
                            return false;
                        }
                    }
                }
                return true;
            }

            /**
             * Atomically decrements the internal counter by n and (if necessary) blocks the calling thread until the counter reaches zero
             * or the given timeout duration has expired.
             *
             * Equivalent to `count_down(n); wait(timeout_duration);`.
             *
             * Implementation uses `std::chrono::steady_clock::now()`.
             *
             * Extension of std::latch.
             *
             * @tparam Rep
             * @tparam Period
             * @param timeout_duration maximum duration to wait
             * @param n the value by which the internal counter is decreased, defaults to 1
             * @return true if internal counter has reached zero, otherwise a timeout has occurred.
             */
            template<typename Rep, typename Period>
            bool arrive_and_wait_for(const std::chrono::duration<Rep, Period>& timeout_duration, const size_t n = 1) noexcept {
                count_down(n);
                return wait_for(timeout_duration);
            }
    };

    /**@}*/

} /* namespace jau */

#endif /* JAU_LATCH_HPP_ */
