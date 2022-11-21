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

#ifndef C_ALLOCATOR_SEC_HPP
#define C_ALLOCATOR_SEC_HPP

#include <cinttypes>
#include <memory>
#include <cstring>

#include <jau/basic_types.hpp>

namespace jau {

/**
 * A simple secure allocator for integral types using POSIX C functions: <code>::malloc()</code> and <code>::free()</code>.
 *
 * callocator_sec is similar to callocator, but
 * - only works for integral types
 * - deallocate explicitly bzero's the memory before free for secure scrubbing.
 * - dropped realloc() for security reasons, since realloc() could free old memory block w/o scrubbing.
 *
 * This class shall be compliant with <i>C++ named requirements for Allocator</i>.
 *
 * Not implementing deprecated (C++17) and removed (C++20)
 * methods: address(), max_size(), construct() and destroy().
 */
template <typename T,
          std::enable_if_t< std::is_integral_v<T>, bool> = true>
struct callocator_sec
{
  public:
    // typedefs' for C++ named requirements: Allocator
    typedef T               value_type;
    typedef std::size_t     size_type;
    typedef std::ptrdiff_t  difference_type;
    typedef std::true_type  propagate_on_container_move_assignment;

    // C++17, deprecated in C++20
    typedef std::true_type  is_always_equal;

    // deprecated in C++17 and removed in C++20
    typedef T*              pointer;
    typedef const T*        const_pointer;
    typedef T&              reference;
    typedef const T&        const_reference;
    template <class U> struct rebind {typedef callocator_sec<U> other;};

  private:
    typedef std::remove_const_t<T> value_type_mutable;
    /** Required to create and move immutable elements, aka const */
    typedef value_type_mutable*    pointer_mutable;

  public:
    callocator_sec() noexcept = default; // C++11

#if __cplusplus > 201703L
    constexpr callocator_sec(const callocator_sec& other) noexcept
    {} // C++20
#else
    callocator_sec(const callocator_sec& other) noexcept
    { (void)other; }
#endif

#if __cplusplus > 201703L
    template <typename U>
    constexpr callocator_sec(const callocator_sec<U>& other) noexcept
    { (void)other; } // C++20
#else
    template <typename U>
    callocator_sec(const callocator_sec<U>& other) noexcept
    { (void)other; }
#endif

#if __cplusplus > 201703L
    constexpr ~callocator_sec() {} // C++20
#else
    ~callocator_sec() = default;
#endif

#if __cplusplus <= 201703L
    value_type* allocate(std::size_t n, const void * hint) { // C++17 deprecated; C++20 removed
        (void)hint;
        return reinterpret_cast<value_type*>( ::malloc( n * sizeof(value_type) ) );
    }
#endif

#if __cplusplus > 201703L
    [[nodiscard]] constexpr value_type* allocate(std::size_t n) { // C++20
        return reinterpret_cast<value_type*>( malloc( n * sizeof(value_type) ) );
    }
#else
    value_type* allocate(std::size_t n) { // C++17
        return reinterpret_cast<value_type*>( ::malloc( n * sizeof(value_type) ) );
    }
#endif

#if __cplusplus > 201703L
    constexpr void deallocate(value_type* p, std::size_t n ) {
        (void)n;
        ::explicit_bzero(p, n); // non-optomized away bzero
        free( reinterpret_cast<void*>( p ) );
    }
#else
    void deallocate(value_type* p, std::size_t n ) {
        (void)n;
        ::explicit_bzero(p, n); // non-optomized away bzero
        ::free( reinterpret_cast<void*>( const_cast<pointer_mutable>(p) ) );
    }
#endif

};


#if __cplusplus > 201703L
template <class T1, class T2>
    constexpr bool operator==(const callocator_sec<T1>& lhs, const callocator_sec<T2>& rhs) noexcept {
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
    bool operator==(const callocator_sec<T1>& lhs, const callocator_sec<T2>& rhs) noexcept {
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
    bool operator!=(const callocator_sec<T1>& lhs, const callocator_sec<T2>& rhs) noexcept {
        return !(lhs==rhs);
    }
#endif

} /* namespace jau */

#endif // C_ALLOCATOR_SEC_HPP

