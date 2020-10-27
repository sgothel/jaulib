/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2020 ZAFENA AB
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

#ifndef JAU_BASIC_ALGOS_HPP_
#define JAU_BASIC_ALGOS_HPP_

#include <mutex>

namespace jau {

    /**
     * Custom for_each template, using indices instead of iterators,
     * allowing container to be modified within lambda 'callback'.
     * <p>
     * Method performs UnaryFunction on all elements [0..n-1],
     * where n is being retrieved once before the loop!
     * </p>
     */
    template<class InputArray, class UnaryFunction>
    constexpr UnaryFunction for_each_idx(InputArray &array, UnaryFunction f)
    {
        const size_t size = array.size();
        for (size_t i = 0; i < size; i++) {
            f(array[i]);
        }
        return f; // implicit move since C++11
    }

    /**
     * Custom for_each template, using indices instead of iterators,
     * allowing container to be modified within lambda 'callback'.
     * <p>
     * Method performs UnaryFunction on all elements [0..n-1],
     * where n is being retrieved once before the loop!
     * </p>
     * <p>
     * This method also utilizes a given mutex to ensure thread-safety,
     * by operating within an RAII-style std::lock_guard block.
     * </p>
     */
    template<class Mutex, class InputArray, class UnaryFunction>
    constexpr UnaryFunction for_each_idx_mtx(Mutex &mtx, InputArray &array, UnaryFunction f)
    {
        const std::lock_guard<Mutex> lock(mtx); // RAII-style acquire and relinquish via destructor

        const size_t size = array.size();
        for (size_t i = 0; i < size; i++) {
            f(array[i]);
        }
        return f; // implicit move since C++11
    }

    /**
     * Call on release allows the user to pass a function
     * to be called at destruction of this instance.
     * <p>
     * One goal was to provide a thread exit cleanup facility,
     * setting a 'is_running' flag to false when the thread exists
     * normally or abnormally.
     * <pre>
     *   jau::relaxed_atomic_bool is_running = true;
     *
     *   void some_thread_func() {
     *       thread_local jau::call_on_release lili([&]() {
     *           is_running = false;
     *       });
     *       ...
     *       do some work here, which might get cancelled
     *       ..
     *   }
     * </pre>
     * </p>
     * @tparam UnaryFunction user provided function to be called @ dtor
     */
    template <class UnaryFunction> class call_on_release {
      private:
        UnaryFunction f;

      public:
        call_on_release(UnaryFunction release_func) noexcept : f(release_func) {}
        ~call_on_release() noexcept { f(); }
        call_on_release(const call_on_release&) = delete;
        call_on_release& operator=(const call_on_release&) = delete;
        call_on_release& operator=(const call_on_release&) volatile = delete;
    };

} // namespace jau

#endif /* JAU_BASIC_ALGOS_HPP_ */
