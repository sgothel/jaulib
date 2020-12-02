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
#include <cmath>

namespace jau {
    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /**
     * Natural 'size_t' alternative using 'unsigned int' as its natural sized type.
     * <p>
     * The leading 'n' stands for natural.
     * </p>
     * <p>
     * This is a compromise to indicate intend,
     * but to avoid handling a multiple sized 'size_t' footprint where not desired.
     * </p>
     */
    typedef unsigned int nsize_t;

    /**
     * Natural 'ssize_t' alternative using 'signed int' as its natural sized type.
     * <p>
     * The leading 'n' stands for natural.
     * </p>
     * <p>
     * This is a compromise to indicate intend,
     * but to avoid handling a multiple sized 'ssize_t' footprint where not desired.
     * </p>
     */
    typedef signed int snsize_t;

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /**
     * Returns the value of the sign function.
     * <pre>
     * -1 for x < 0
     *  0 for x = 0
     *  1 for x > 0
     * </pre>
     * Implementation is type safe.
     * @tparam T an integral number type
     * @param x the integral number
     * @return function result
     */
    template <typename T>
    constexpr snsize_t sign(T x) noexcept
    {
        return (T(0) < x) - (x < T(0));
    }

    /**
     * Returns the absolute value of an integral number
     * @tparam T an integral number type
     * @param x the integral number
     * @return function result
     */
    template <typename T>
    constexpr T abs(T x) noexcept
    {
        return sign(x) < 0 ? -x : x;
    }

    /**
     * Returns the number of decimal digits of the given integral value number using std::log10<T>().
     * <pre>
     * x < 0: 2 + floor( log10( -x ) ) )
     * x = 0: 1
     * x > 0: 1 + (int)( log10(  x ) ) )
     * </p>
     * @tparam T an integral integer type
     * @param x the integral integer
     * @return digit count
     */
    template<typename T>
    constexpr nsize_t digits10(T x) noexcept
    {
        const snsize_t x_sign = jau::sign<T>(x);
        if( x_sign == 0 ) {
            return 1;
        }
        if( x_sign < 0 ) {
            return 2 + static_cast<nsize_t>( std::floor<T>( std::log10<T>( -x ) ) );
        } else {
            return 1 + static_cast<nsize_t>(                std::log10<T>(  x )   );
        }
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

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
