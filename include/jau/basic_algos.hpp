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
#include <type_traits>

#include <jau/cow_iterator.hpp>

namespace jau {

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

    /****************************************************************************************
     ****************************************************************************************/

    /**
     * Like std::find() of 'algorithm'
     * <p>
     * Only exists here as performance analysis over O(n*n) complexity
     * exposes std::find() to be approximately 3x slower.<br>
     * See test/test_cow_darray_perf01.cpp
     * </p>
     * @tparam InputIt the iterator type
     * @tparam T the data type
     * @param first range start of elements to examine
     * @param last range end of elements to examine, exclusive
     * @param value reference value for comparison
     * @return Iterator to the first element satisfying the condition or last if no such element is found.
     */
    template<class InputIt, class T>
    constexpr InputIt find(InputIt first, InputIt last, const T& value)
    {
        for (; first != last; ++first) {
            if (*first == value) {
                return first;
            }
        }
        return last; // implicit move since C++11
    }

    /**
     * Like std::find_if() of 'algorithm'
     * <p>
     * Only exists here as performance analysis over O(n*n) complexity
     * exposes std::find_if() to be approximately 3x slower for 1000 x 1000.<br>
     * See test/test_cow_darray_perf01.cpp
     * </p>
     * @tparam InputIt the iterator type
     * @tparam UnaryPredicate
     * @param first range start of elements to examine
     * @param last range end of elements to examine, exclusive
     * @param p unary predicate which returns ​true for the desired element.
     * @return Iterator to the first element satisfying the condition or last if no such element is found.
     */
    template<class InputIt, class UnaryPredicate>
    constexpr InputIt find_if(InputIt first, InputIt last, UnaryPredicate p)
    {
        for (; first != last; ++first) {
            if (p(*first)) {
                return first;
            }
        }
        return last; // implicit move since C++11
    }

    /**
     * Like std::find_if_not() of 'algorithm'
     * <p>
     * Only exists here as performance analysis over O(n*n) complexity
     * exposes std::find_if_not() to be approximately 3x slower for 1000 x 1000.<br>
     * See test/test_cow_darray_perf01.cpp
     * </p>
     * @tparam InputIt the iterator type
     * @tparam UnaryPredicate
     * @param first range start of elements to examine
     * @param last range end of elements to examine, exclusive
     * @param q unary predicate which returns ​false for the desired element.
     * @return Iterator to the first element satisfying the condition or last if no such element is found.
     */
    template<class InputIt, class UnaryPredicate>
    constexpr InputIt find_if_not(InputIt first, InputIt last, UnaryPredicate q)
    {
        for (; first != last; ++first) {
            if (!q(*first)) {
                return first;
            }
        }
        return last; // implicit move since C++11
    }

    /**
     * Like std::for_each() of 'algorithm'
     * <p>
     * Only exists here as performance analysis over O(n*n) complexity
     * exposes std::for_each() to be 'a little' slower for 1000 x 1000.<br>
     * See test/test_cow_darray_perf01.cpp
     * </p>
     * @tparam InputIt the iterator type
     * @tparam UnaryFunction
     * @param first range start of elements to apply the function
     * @param last range end of elements to apply the function
     * @param f the function object, like <code>void fun(const Type &a)</code>
     * @return the function
     */
    template<class InputIt, class UnaryFunction>
    constexpr UnaryFunction for_each(InputIt first, InputIt last, UnaryFunction f)
    {
        for (; first != last; ++first) {
            f(*first);
        }
        return f; // implicit move since C++11
    }

    /****************************************************************************************
     ****************************************************************************************/

    /**
     * Like jau::for_each(), see above.
     * <p>
     * Additionally this template function removes
     * the <code>const</code> qualifier
     * of the <code>UnaryFunction</code> sole argument.<br>
     * The latter is retrieved by dereferencing the iterator,
     * which might expose the <code>const</code> qualifier if
     * the iterator is a <code>const_iterator</code>.
     * </p>
     * <p>
     * Implementation casts argument in the following fashion
     * <code>const_cast<value_type*>(&arg)</code>,
     * allowing to use <code>const_iterator</code> and subsequent
     * non-const features of the argument, see below.
     * </p>
     * <p>
     * Such situations may occur when preferring to use
     * the <code>const_iterator</code> over non-const.<br>
     * jau::cow_darray is such a scenario, where one might
     * not mutate the elements of the container itself
     * but needs to invoke non-const functions <i>in good faith</i>.<br>
     * Here we can avoid costly side-effects of copying the CoW storage for later replacement.<br>
     * See jau::cow_ro_iterator and jau::cow_rw_iterator
     * in conjunction with jau::cow_darray.
     * </p>
     * <p>
     * Requirements for the given IteratorIt type are to
     * have typename <code>InputIt::value_type</code> available.
     * </p>
     * @tparam InputIt the iterator type, which might be a 'const_iterator' for non const types.
     * @tparam UnaryFunction
     * @param first range start of elements to apply the function
     * @param last range end of elements to apply the function
     * @param f the function object, like <code>void fun(const Type &a)</code>
     * @return the function
     * @see jau::cow_darray
     * @see jau::cow_ro_iterator
     * @see jau::cow_rw_iterator
     */
    template<class InputIt, class UnaryFunction>
    constexpr UnaryFunction for_each_fidelity(InputIt first, InputIt last, UnaryFunction f)
    {
        typedef typename InputIt::value_type value_type;

        for (; first != last; ++first) {
            f( *const_cast<value_type*>( & (*first) ) );
        }
        return f; // implicit move since C++11
    }

    /****************************************************************************************
     ****************************************************************************************/

    /**
     * Custom for_each template, same as jau::for_each but using a mutex.
     * <p>
     * Method performs UnaryFunction on all elements [first..last).
     * </p>
     * <p>
     * This method also utilizes a given mutex to ensure thread-safety,
     * by operating within an RAII-style std::lock_guard block.
     * </p>
     */
    template<class Mutex, class InputIt, class UnaryFunction>
    constexpr UnaryFunction for_each_mtx(Mutex &mtx, InputIt first, InputIt last, UnaryFunction f)
    {
        const std::lock_guard<Mutex> lock(mtx); // RAII-style acquire and relinquish via destructor

        for (; first != last; ++first) {
            f(*first);
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
     */
    template<class InputArray, class UnaryFunction>
    constexpr UnaryFunction for_each_idx(InputArray &array, UnaryFunction f)
    {
        const size_t size = array.size();
        for (size_t i = 0; i < size; ++i) {
            f(array[i]);
        }
        return f; // implicit move since C++11
    }

    /**
     * Custom for_each template,
     * same as jau::for_each but using indices instead of iterators and a mutex.
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
        for (size_t i = 0; i < size; ++i) {
            f(array[i]);
        }
        return f; // implicit move since C++11
    }

    /****************************************************************************************
     ****************************************************************************************/

    template<class T>
    const typename T::value_type * find_const(T& data, typename T::value_type const & elem,
            std::enable_if_t< is_cow_type<T>::value, bool> = true ) noexcept
    {
        typename T::const_iterator begin = data.cbegin();
        typename T::const_iterator end = begin.end();
        auto it = jau::find( begin, end, elem);
        if( it != end ) {
            return &(*it);
        }
        return nullptr;
    }
    template<class T>
    const typename T::value_type * find_const(T& data, typename T::value_type const & elem,
            std::enable_if_t< !is_cow_type<T>::value, bool> = true ) noexcept
    {
        typename T::const_iterator end = data.cend();
        auto it = jau::find( data.cbegin(), end, elem);
        if( it != end ) {
            return &(*it);
        }
        return nullptr;
    }

    /****************************************************************************************
     ****************************************************************************************/

    template<class T, class UnaryFunction>
    constexpr UnaryFunction for_each_const(T& data, UnaryFunction f,
            std::enable_if_t< is_cow_type<T>::value, bool> = true ) noexcept
    {
        typename T::const_iterator first = data.cbegin();
        typename T::const_iterator last = first.end();
        for (; first != last; ++first) {
            f(*first);
        }
        return f; // implicit move since C++11
    }
    template<class T, class UnaryFunction>
    constexpr UnaryFunction for_each_const(T& data, UnaryFunction f,
            std::enable_if_t< !is_cow_type<T>::value, bool> = true ) noexcept
    {
        typename T::const_iterator first = data.cbegin();
        typename T::const_iterator last = data.cend();
        for (; first != last; ++first) {
            f(*first);
        }
        return f; // implicit move since C++11
    }

    /****************************************************************************************
     ****************************************************************************************/

    /**
     * See jau::for_each_fidelity()
     */
    template<class T, class UnaryFunction>
    constexpr UnaryFunction for_each_fidelity(T& data, UnaryFunction f,
            std::enable_if_t< is_cow_type<T>::value, bool> = true ) noexcept
    {
        typename T::const_iterator first = data.cbegin();
        typename T::const_iterator last = first.end();
        for (; first != last; ++first) {
            f( *const_cast<typename T::value_type*>( & (*first) ) );
        }
        return f; // implicit move since C++11
    }
    /**
     * See jau::for_each_fidelity()
     */
    template<class T, class UnaryFunction>
    constexpr UnaryFunction for_each_fidelity(T& data, UnaryFunction f,
            std::enable_if_t< !is_cow_type<T>::value, bool> = true ) noexcept
    {
        typename T::const_iterator first = data.cbegin();
        typename T::const_iterator last = data.cend();
        for (; first != last; ++first) {
            f( *const_cast<typename T::value_type*>( & (*first) ) );
        }
        return f; // implicit move since C++11
    }

} // namespace jau

#endif /* JAU_BASIC_ALGOS_HPP_ */
