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

#ifndef COUNTING_CALLOCATOR_HPP
#define COUNTING_CALLOCATOR_HPP

#include <cinttypes>
#include <memory>

#include <jau/basic_types.hpp>
#include <jau/callocator.hpp>

namespace jau {

/**
 * Performance counter jau::callocator specialization.
 * <p>
 * This class shall be compliant with <i>C++ named requirements for Allocator</i>.
 * </p>
 * <p>
 * Not overriding deprecated (C++17) and removed (C++20)
 * methods: address(), max_size(), construct() and destroy().
 * </p>
 */
template <class T>
struct counting_callocator : public jau::callocator<T>
{
  public:
    template <class U> struct rebind {typedef counting_callocator<U> other;};

    // typedefs' for C++ named requirements: Allocator
    typedef T  value_type;

    // std::size_t id;
    bool old_stats;
    std::size_t memory_usage;
    std::size_t alloc_count;
    std::size_t dealloc_count;
    std::size_t realloc_count;
    ssize_t alloc_balance;

  private:
    // inline static relaxed_atomic_size_t next_id = 1;

    /**
     * vector<value_type>::get_allocator() returns a copy of the allocator instance,
     * where we desire to access the copied statistics.
     * <p>
     * However, vector<value_type>(const vector<value_type>&) also copies the allocator instance,
     * but here the copied statistics shall be flushed since the elements are
     * copied into the new vector<value_type> instance using the new allocator.<br>
     * Without flushing the stats, we would see a size + size allocator stats,
     * the former size from the copied allocator and the latter from the
     * copied elements into the new vector<value_type> instance.
     * </p>
     */
    constexpr void flush_stats() noexcept {
        if( old_stats ) {
            old_stats = false;
            memory_usage = 0;
            alloc_count = 0;
            dealloc_count = 0;
            realloc_count = 0;
            alloc_balance = 0;
        }
    }

  public:
    std::string toString(const nsize_t mem_width=0, const nsize_t count_width=0) {
        return "CAlloc["/*+std::to_string(id)+", "*/+uint64DecString(memory_usage, ',', mem_width)+" bytes, alloc[balance "+
                int64DecString(alloc_balance, ',', count_width)+" = "+
                uint64DecString(alloc_count, ',', count_width)+" - "+uint64DecString(dealloc_count, ',', count_width)+
                ", realloc = "+uint64DecString(realloc_count, ',', count_width)+"]]";
    }

    counting_callocator() noexcept
    : jau::callocator<value_type>(),
      // id(next_id++),
      old_stats(false),
      memory_usage(0), alloc_count(0), dealloc_count(0), realloc_count(0), alloc_balance(0)
      { } // C++11

#if __cplusplus > 201703L
    constexpr counting_callocator(const counting_callocator& other) noexcept
    : jau::callocator<value_type>(other),
      // id(next_id++),
      old_stats(true),
      memory_usage(other.memory_usage),
      alloc_count(other.alloc_count), dealloc_count(other.dealloc_count),
      realloc_count(other.realloc_count), alloc_balance(other.alloc_balance)
      {} // C++20
#else
    counting_callocator(const counting_callocator& other) noexcept
    : jau::callocator<value_type>(other),
      // id(next_id++),
      old_stats(true),
      memory_usage(other.memory_usage),
      alloc_count(other.alloc_count), dealloc_count(other.dealloc_count),
      realloc_count(other.realloc_count), alloc_balance(other.alloc_balance)
      { }
#endif

#if __cplusplus > 201703L
    template <typename U>
    constexpr counting_callocator(const counting_callocator<U>& other) noexcept
    : jau::callocator<value_type>(other),
      // id(next_id++),
      old_stats(true),
      memory_usage(other.memory_usage),
      alloc_count(other.alloc_count), dealloc_count(other.dealloc_count),
      realloc_count(other.realloc_count), alloc_balance(other.alloc_balance)
      {} // C++20
#else
    template <typename U>
    counting_callocator(const counting_callocator<U>& other) noexcept
    : jau::callocator<value_type>(other),
      // id(next_id++),
      old_stats(true),
      memory_usage(other.memory_usage),
      alloc_count(other.alloc_count), dealloc_count(other.dealloc_count),
      realloc_count(other.realloc_count), alloc_balance(other.alloc_balance)
      { }
#endif

#if __cplusplus > 201703L
    constexpr ~counting_callocator() {} // C++20
#else
    ~counting_callocator() {}
#endif

#if __cplusplus <= 201703L
    value_type* allocate(std::size_t n, const void * hint) { // C++17 deprecated; C++20 removed
        flush_stats();
        memory_usage += n * sizeof(value_type);
        ++alloc_count;
        ++alloc_balance;
        return jau::callocator<value_type>::allocate(n, hint);
    }
#endif

#if __cplusplus > 201703L
    [[nodiscard]] constexpr value_type* allocate(std::size_t n) { // C++20
        flush_stats();
        memory_usage += n * sizeof(value_type);
        ++alloc_count;
        ++alloc_balance;
        return jau::callocator<value_type>::allocate(n);
    }
#else
    value_type* allocate(std::size_t n) { // C++17
        flush_stats();
        memory_usage += n * sizeof(value_type);
        ++alloc_count;
        ++alloc_balance;
        return jau::callocator<value_type>::allocate(n);
    }
#endif

    [[nodiscard]] constexpr value_type* reallocate(value_type* p, std::size_t old_size, std::size_t new_size) {
        value_type * const res = reinterpret_cast<value_type*>(
                jau::callocator<value_type>::reallocate( p, old_size, new_size ) );
        if( nullptr != res ) {
            memory_usage -= old_size * sizeof(value_type);
            memory_usage += new_size * sizeof(value_type);
            ++realloc_count;
        } // else realloc does not free storage on failure (nullptr)
        return res;
    }

#if __cplusplus > 201703L
    constexpr void deallocate(value_type* p, std::size_t n ) {
        flush_stats();
        memory_usage -= n * sizeof(value_type);
        ++dealloc_count;
        --alloc_balance;
        jau::callocator<value_type>::deallocate(p, n);
    }
#else
    void deallocate(value_type* p, std::size_t n ) {
        flush_stats();
        memory_usage -= n * sizeof(value_type);
        ++dealloc_count;
        --alloc_balance;
        jau::callocator<value_type>::deallocate(p, n);
    }
#endif
};


#if __cplusplus > 201703L
template <class T1, class T2>
    constexpr bool operator==(const counting_callocator<T1>& lhs, const counting_callocator<T2>& rhs) noexcept {
#if 0
        if( &lhs == &rhs ) {
            return true;
        }
        return lhs.memory_usage == rhs.memory_usage;
#else
        (void)lhs;
        (void)rhs;
        return true;
#endif
    }
#else
    template <class T1, class T2>
    bool operator==(const counting_callocator<T1>& lhs, const counting_callocator<T2>& rhs) noexcept {
#if 0
        if( &lhs == &rhs ) {
            return true;
        }
        return lhs.memory_usage == rhs.memory_usage;
#else
        (void)lhs;
        (void)rhs;
        return true;
#endif
    }
    template <class T1, class T2>
    bool operator!=(const counting_callocator<T1>& lhs, const counting_callocator<T2>& rhs) noexcept {
        return !(lhs==rhs);
    }
#endif

} /* namespace jau */

#endif // COUNTING_CALLOCATOR_HPP

