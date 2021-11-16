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

namespace jau {

    /**
     * Inspired by std::latch of C++20
     *
     * @see https://en.cppreference.com/w/cpp/thread/latch
     */
    class latch {
        private:
            std::mutex mtx_cd;
            std::mutex mtx_cv;
            std::condition_variable cv;
            jau::sc_atomic_size_t count;

        public:
            /** Returns the maximum value of the internal counter supported by the implementation. */
            static constexpr size_t max() noexcept { return std::numeric_limits<size_t>::max(); }

            constexpr latch(const size_t count_) noexcept
            : count(count_) {}

            latch(const latch& o) = delete;

            /**
             * Atomically decrements the internal counter by n
             * and notifies all blocked wait() threads if zero is reached.
             *
             * If n is greater than the value of the internal counter, the counter is set to zero.
             *
             * This operation strongly happens-before all calls that are unblocked on this latch.
             *
             * @param n the value by which the internal counter is decreased, defaults to 1
             */
            void count_down(const size_t n = 1) noexcept {
                bool notify;
                {
                    std::unique_lock<std::mutex> lock(mtx_cd); // Avoid data-race on concurrent count_down() calls
                    if( n < count ) {
                        count -= n;
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
             * Returns true only if the internal counter has reached zero.
             */
            bool try_wait() const noexcept {
                return 0 == count;
            }

            /**
             * Blocks the calling thread until the internal counter reaches 0. If it is zero already, returns immediately.
             */
            void wait() const noexcept {
                if( 0 < count ) {
                    std::unique_lock<std::mutex> lock(mtx_cv);
                    cv.wait(lock, [&count]{ return 0 == count; });
                }
            }

            /**
             * Atomically decrements the internal counter by n and (if necessary) blocks the calling thread until the counter reaches zero.
             *
             * Equivalent to count_down(n); wait();.
             *
             * @param n the value by which the internal counter is decreased, defaults to 1
             */
            void arrive_and_wait(const size_t n = 1) noexcept {
                count_down(n);
                wait();
            }
    };

} /* namespace jau */

#endif /* JAU_LATCH_HPP_ */
