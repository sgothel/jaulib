/*
 * Author: Sven Gothel <sgothel@jausoft.com> and Svenson Han Gothel
 * Copyright (c) 2022-2024 Gothel Software e.K.
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
#ifndef JAU_LOCKS_HPP_
#define JAU_LOCKS_HPP_

#include <cstdint>
#include <ios>
#include <mutex>
#include <ratio>
#include <string>
#include <thread>

#include <jau/basic_types.hpp>
#include <jau/int_types.hpp>
#include <jau/ordered_atomic.hpp>
#include <jau/string_util.hpp>

namespace jau {

    /** \addtogroup Concurrency
     *
     *  @{
     */

    class RecursiveLock {
      public:
        typedef void (*callback_func)() noexcept;

      private:
        std::recursive_timed_mutex m_mtx_lock;
        relaxed_atomic_nsize_t     m_lock_count;
        std::thread::id            m_owner_id;

      public:
        RecursiveLock() noexcept
        : m_lock_count(0), m_owner_id() { }

        RecursiveLock(const RecursiveLock &)  = delete;
        void operator=(const RecursiveLock &) = delete;

        bool isOwner(std::thread::id id) const noexcept { return id == m_owner_id; }
        bool isOwner() const noexcept { return std::this_thread::get_id() == m_owner_id; }

        /**
         * Return the number of locks issued to this lock by the same thread.
         * <ul>
         *   <li>A hold count of 0 identifies this lock as unlocked.</li>
         *   <li>A hold count of 1 identifies this lock as locked.</li>
         *   <li>A hold count of > 1 identifies this lock as recursively lock.</li>
         * </ul>
         */
        nsize_t holdCount() noexcept { return m_lock_count; }

        /**
         * Acquire this lock indefinitely (no timeout)
         */
        void lock() {
            m_mtx_lock.lock();
            if( 1 == ++m_lock_count ) {
                m_owner_id = std::this_thread::get_id();
            }
        }

        /**
         * Try to acquire this lock within given timeout in seconds
         *
         * @param timeout maximum duration in fractions of seconds to wait, where fractions_i64::zero waits infinitely
         * @return true if successful, otherwise false in case timeout occurred or otherwise.
         */
        bool tryLock(const fraction_i64 &timeout) {
            // const fraction_timespec timeout_time = getMonotonicTime() + fraction_timespec(timeout);
            bool overflow = false;

            std::chrono::duration<int64_t, std::nano> d = timeout.to_duration(std::chrono::nanoseconds::zero(), &overflow);
            if( overflow ) {
                return false;
            }
            auto timeout_time = std::chrono::steady_clock::now() + d;

            if( m_mtx_lock.try_lock_until(timeout_time) ) {
                if( 1 == ++m_lock_count ) {
                    m_owner_id = std::this_thread::get_id();
                }
                return true;
            } else {
                return false;
            }
        }

        void validateLocked() const {
            std::thread::id id = std::this_thread::get_id();
            if( !isOwner(id) ) {
                throw RuntimeException(threadName(id) + ": Not locked: " + toString(), E_FILE_LINE);
            }
        }

        /**
         * Unlock ...
         *
         * @param taskBeforeUnlock optional callback_func to be execiting before final unlock.
         *
         * @see lock()
         * @see tryLock()
         */
        void unlock(callback_func taskBeforeUnlock=nullptr) {
            validateLocked();

            if( 0 < --m_lock_count ) {
                m_mtx_lock.unlock();
                return;
            }
            if( nullptr != taskBeforeUnlock ) {
                taskBeforeUnlock();
            }
            m_owner_id = std::thread::id();
            m_mtx_lock.unlock();
        }

        std::string toString() const {
            return "RL[count " + std::to_string(m_lock_count.load()) + ", owner " + threadName(m_owner_id) + "]";
        }
    };

    /**@}*/

}  // namespace jau

#endif /*  JAU_LOCKS_HPP_ */
