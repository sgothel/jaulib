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

#ifndef JAU_ORDERED_ATOMIC_HPP_
#define JAU_ORDERED_ATOMIC_HPP_

#include <atomic>
#include <memory>

#include <jau/basic_types.hpp>

namespace jau {

#ifndef CXX_ALWAYS_INLINE
# define CXX_ALWAYS_INLINE inline __attribute__((__always_inline__))
#endif

/**
 * std::atomic<T> type with predefined fixed std::memory_order,
 * not allowing changing the memory model on usage and applying the set order to all operator.
 * <p>
 * See also:
 * <pre>
 * - Sequentially Consistent (SC) ordering or SC-DRF (data race free) <https://en.cppreference.com/w/cpp/atomic/memory_order#Sequentially-consistent_ordering>
 * - std::memory_order <https://en.cppreference.com/w/cpp/atomic/memory_order>
 * </pre>
 * </p>
 */
template <typename _Tp, std::memory_order _MO> struct ordered_atomic : private std::atomic<_Tp> {
  private:
    typedef std::atomic<_Tp> super;
    
  public:
    ordered_atomic() noexcept = default;
    ~ordered_atomic() noexcept = default;
    ordered_atomic(const ordered_atomic&) = delete;
    ordered_atomic& operator=(const ordered_atomic&) = delete;
    ordered_atomic& operator=(const ordered_atomic&) volatile = delete;

    constexpr ordered_atomic(_Tp __i) noexcept
    : super(__i)
    { }

    CXX_ALWAYS_INLINE
    operator _Tp() const noexcept
    { return super::load(_MO); }

    CXX_ALWAYS_INLINE
    operator _Tp() const volatile noexcept
    { return super::load(_MO); }

    CXX_ALWAYS_INLINE
    _Tp operator=(_Tp __i) noexcept
    { super::store(__i, _MO); return __i; }

    CXX_ALWAYS_INLINE
    _Tp operator=(_Tp __i) volatile noexcept
    { super::store(__i, _MO); return __i; }

    CXX_ALWAYS_INLINE
    _Tp operator++(int) noexcept // postfix ++
    { return super::fetch_add(1, _MO); }

    CXX_ALWAYS_INLINE
    _Tp operator++(int) volatile noexcept // postfix ++
    { return super::fetch_add(1, _MO); }

    CXX_ALWAYS_INLINE
    _Tp operator--(int) noexcept // postfix --
    { return super::fetch_sub(1, _MO); }

    CXX_ALWAYS_INLINE
    _Tp operator--(int) volatile noexcept // postfix --
    { return super::fetch_sub(1, _MO); }

#if 0 /* def _GLIBCXX_ATOMIC_BASE_H */

    // prefix ++, -- impossible w/o using GCC __atomic builtins and access to _M_i .. etc

    CXX_ALWAYS_INLINE
    _Tp operator++() noexcept // prefix ++
    { return __atomic_add_fetch(&_M_i, 1, int(_MO)); }

    CXX_ALWAYS_INLINE
    _Tp operator++() volatile noexcept // prefix ++
    { return __atomic_add_fetch(&_M_i, 1, int(_MO)); }

    CXX_ALWAYS_INLINE
    _Tp operator--() noexcept // prefix --
    { return __atomic_sub_fetch(&_M_i, 1, int(_MO)); }

    CXX_ALWAYS_INLINE
    _Tp operator--() volatile noexcept // prefix --
    { return __atomic_sub_fetch(&_M_i, 1, int(_MO)); }

#endif /* 0 _GLIBCXX_ATOMIC_BASE_H */

    CXX_ALWAYS_INLINE
    bool is_lock_free() const noexcept 
    { return super::is_lock_free(); }

    CXX_ALWAYS_INLINE
    bool is_lock_free() const volatile noexcept
    { return super::is_lock_free(); }

    static constexpr bool is_always_lock_free = super::is_always_lock_free;

    CXX_ALWAYS_INLINE
    void store(_Tp __i) noexcept
    { super::store(__i, _MO); }

    CXX_ALWAYS_INLINE
    void store(_Tp __i) volatile noexcept
    { super::store(__i, _MO); }

    CXX_ALWAYS_INLINE
    _Tp load() const noexcept
    { return super::load(_MO); }

    CXX_ALWAYS_INLINE
    _Tp load() const volatile noexcept
    { return super::load(_MO); }

    CXX_ALWAYS_INLINE
    _Tp exchange(_Tp __i) noexcept
    { return super::exchange(__i, _MO); }

    CXX_ALWAYS_INLINE
    _Tp exchange(_Tp __i) volatile noexcept
    { return super::exchange(__i, _MO); }

    CXX_ALWAYS_INLINE
    bool compare_exchange_weak(_Tp& __e, _Tp __i) noexcept
    { return super::compare_exchange_weak(__e, __i, _MO); }

    CXX_ALWAYS_INLINE
    bool compare_exchange_weak(_Tp& __e, _Tp __i) volatile noexcept
    { return super::compare_exchange_weak(__e, __i, _MO); }

    CXX_ALWAYS_INLINE
    bool compare_exchange_strong(_Tp& __e, _Tp __i) noexcept
    { return super::compare_exchange_strong(__e, __i, _MO); }

    CXX_ALWAYS_INLINE
    bool compare_exchange_strong(_Tp& __e, _Tp __i) volatile noexcept
    { return super::compare_exchange_strong(__e, __i, _MO); }

    CXX_ALWAYS_INLINE
    _Tp fetch_add(_Tp __i) noexcept
    { return super::fetch_add(__i, _MO); }

    CXX_ALWAYS_INLINE
    _Tp fetch_add(_Tp __i) volatile noexcept
    { return super::fetch_add(__i, _MO); }

    CXX_ALWAYS_INLINE
    _Tp fetch_sub(_Tp __i) noexcept
    { return super::fetch_sub(__i, _MO); }

    CXX_ALWAYS_INLINE
    _Tp fetch_sub(_Tp __i) volatile noexcept
    { return super::fetch_sub(__i, _MO); }

    CXX_ALWAYS_INLINE
    _Tp fetch_and(_Tp __i) noexcept
    { return super::fetch_and(__i, _MO); }

    CXX_ALWAYS_INLINE
    _Tp fetch_and(_Tp __i) volatile noexcept
    { return super::fetch_and(__i, _MO); }

    CXX_ALWAYS_INLINE
    _Tp fetch_or(_Tp __i) noexcept
    { return super::fetch_or(__i, _MO); }

    CXX_ALWAYS_INLINE
    _Tp fetch_or(_Tp __i) volatile noexcept
    { return super::fetch_or(__i, _MO); }

    CXX_ALWAYS_INLINE
    _Tp fetch_xor(_Tp __i) noexcept
    { return super::fetch_xor(__i, _MO); }

    CXX_ALWAYS_INLINE
    _Tp fetch_xor(_Tp __i) volatile noexcept
    { return super::fetch_xor(__i, _MO); }

  };

  /** SC atomic integral scalar boolean. Memory-Model (MM) guaranteed sequential consistency (SC) between acquire (read) and release (write) */
  typedef ordered_atomic<bool, std::memory_order::memory_order_seq_cst> sc_atomic_bool;

  /** Relaxed non-SC atomic integral scalar boolean. Memory-Model (MM) only guarantees the atomic value, _no_ sequential consistency (SC) between acquire (read) and release (write). */
  typedef ordered_atomic<bool, std::memory_order::memory_order_relaxed> relaxed_atomic_bool;

  /** SC atomic integral scalar uint8_t. Memory-Model (MM) guaranteed sequential consistency (SC) between acquire (read) and release (write) */
  typedef ordered_atomic<uint8_t, std::memory_order::memory_order_seq_cst> sc_atomic_uint8;

  /** Relaxed non-SC atomic integral scalar uint8_t. Memory-Model (MM) only guarantees the atomic value, _no_ sequential consistency (SC) between acquire (read) and release (write). */
  typedef ordered_atomic<uint8_t, std::memory_order::memory_order_relaxed> relaxed_atomic_uint8;

  /** SC atomic integral scalar uint16_t. Memory-Model (MM) guaranteed sequential consistency (SC) between acquire (read) and release (write) */
  typedef ordered_atomic<uint16_t, std::memory_order::memory_order_seq_cst> sc_atomic_uint16;

  /** Relaxed non-SC atomic integral scalar uint16_t. Memory-Model (MM) only guarantees the atomic value, _no_ sequential consistency (SC) between acquire (read) and release (write). */
  typedef ordered_atomic<uint16_t, std::memory_order::memory_order_relaxed> relaxed_atomic_uint16;

  /** SC atomic integral scalar integer. Memory-Model (MM) guaranteed sequential consistency (SC) between acquire (read) and release (write) */
  typedef ordered_atomic<int, std::memory_order::memory_order_seq_cst> sc_atomic_int;

  /** Relaxed non-SC atomic integral scalar integer. Memory-Model (MM) only guarantees the atomic value, _no_ sequential consistency (SC) between acquire (read) and release (write). */
  typedef ordered_atomic<int, std::memory_order::memory_order_relaxed> relaxed_atomic_int;

  /** SC atomic integral scalar jau::nsize_t. Memory-Model (MM) guaranteed sequential consistency (SC) between acquire (read) and release (write) */
  typedef ordered_atomic<jau::nsize_t, std::memory_order::memory_order_seq_cst> sc_atomic_nsize_t;

  /** Relaxed non-SC atomic integral scalar jau::nsize_t. Memory-Model (MM) only guarantees the atomic value, _no_ sequential consistency (SC) between acquire (read) and release (write). */
  typedef ordered_atomic<jau::nsize_t, std::memory_order::memory_order_relaxed> relaxed_atomic_nsize_t;

  /** SC atomic integral scalar size_t. Memory-Model (MM) guaranteed sequential consistency (SC) between acquire (read) and release (write) */
  typedef ordered_atomic<std::size_t, std::memory_order::memory_order_seq_cst> sc_atomic_size_t;

  /** Relaxed non-SC atomic integral scalar size_t. Memory-Model (MM) only guarantees the atomic value, _no_ sequential consistency (SC) between acquire (read) and release (write). */
  typedef ordered_atomic<std::size_t, std::memory_order::memory_order_relaxed> relaxed_atomic_size_t;

  /**
   * This class provides a RAII-style Sequentially Consistent (SC) data race free (DRF) critical block.
   * <p>
   * RAII-style SC-DRF acquire via constructor and SC-DRF release via destructor,
   * providing a DRF critical block.
   * </p>
   * <p>
   * This temporary object reuses a jau::sc_atomic_bool atomic synchronization element.
   * The type of the acting atomic is not relevant, only its atomic SC-DRF properties.
   * </p>
   *
   * See also:
   * <pre>
   * - Sequentially Consistent (SC) ordering or SC-DRF (data race free) <https://en.cppreference.com/w/cpp/atomic/memory_order#Sequentially-consistent_ordering>
   * - std::memory_order <https://en.cppreference.com/w/cpp/atomic/memory_order>
   * </pre>
   * @see jau::ringbuffer
   */
  class sc_atomic_critical {
      private:
          sc_atomic_bool & sync_ref;
          bool local_store;

      public:
        /** SC-DRF acquire via sc_atomic_bool::load() */
        sc_atomic_critical(sc_atomic_bool &sync) noexcept : sync_ref(sync), local_store(sync.load()) {}

        /** SC-DRF release via sc_atomic_bool::store() */
        ~sc_atomic_critical() noexcept { sync_ref.store(local_store); }

        sc_atomic_critical() noexcept = delete;
        sc_atomic_critical(const sc_atomic_critical&) = delete;
        sc_atomic_critical& operator=(const sc_atomic_critical&) = delete;
        sc_atomic_critical& operator=(const sc_atomic_critical&) volatile = delete;
  };

} /* namespace jau */

/** \example test_mm_sc_drf_00.cpp
 * Testing SC-DRF non-atomic global read and write within an atomic acquire/release critical block.
 * <p>
 * With test_mm_sc_drf_00.cpp, this work laid the groundwork for jau::sc_atomic_critical and jau::ringbuffer
 * </p>
 */

/** \example test_mm_sc_drf_01.cpp
 * Testing SC-DRF non-atomic global read and write within a locked mutex critical block.
 * <p>
 * With test_mm_sc_drf_00.cpp, this work laid the groundwork for jau::sc_atomic_critical and jau::ringbuffer
 * </p>
 */

#endif /* JAU_ORDERED_ATOMIC_HPP_ */
