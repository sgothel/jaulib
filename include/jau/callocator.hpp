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

#ifndef C_ALLOCATOR_HPP
#define C_ALLOCATOR_HPP

#include <cinttypes>
#include <memory>

#include <jau/basic_types.hpp>

namespace jau {

/**
 * A simple allocator using POSIX C functions: <code>::malloc()</code>, <code>::free()</code> and <code>::realloc()</code>.<br>
 * It is the missing <code>::realloc()</code> in <code>std::allocator</code>, motivating this class.<br>
 * Since <code>realloc()</code> requires the passed pointer to originate from <code>malloc()</code> or <code>calloc</code>,
 * we have to use it for <code>allocate()</code> as well.
 *
 * Added method is <code>reallocate()</code> using the native <code>realloc()</code>.
 *
 * This class shall be compliant with <i>C++ named requirements for Allocator</i>.
 *
 * Not implementing deprecated (C++17) and removed (C++20)
 * methods: address(), max_size(), construct() and destroy().
 */
template <class T>
struct callocator
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
    template <class U> struct rebind {typedef callocator<U> other;};

  private:
    typedef std::remove_const_t<T> value_type_mutable;
    /** Required to create and move immutable elements, aka const */
    typedef value_type_mutable*    pointer_mutable;

  public:
    callocator() noexcept = default; // C++11

#if __cplusplus > 201703L
    constexpr callocator(const callocator&) noexcept // NOLINT(modernize-use-equals-default)
    {} // C++20
#else
    callocator(const callocator&) noexcept // NOLINT(modernize-use-equals-default)
    {}
#endif

#if __cplusplus > 201703L
    template <typename U>
    constexpr callocator(const callocator<U>&) noexcept // NOLINT(modernize-use-equals-default)
    { } // C++20
#else
    template <typename U>
    callocator(const callocator<U>&) noexcept // NOLINT(modernize-use-equals-default)
    { }
#endif

#if __cplusplus > 201703L
    constexpr ~callocator() = default; // C++20
#else
    ~callocator() = default;
#endif

#if __cplusplus <= 201703L
    value_type* allocate(std::size_t n, const void *) { // C++17 deprecated; C++20 removed
        return reinterpret_cast<value_type*>( ::malloc( n * sizeof(value_type) ) );
    }
#endif

#if __cplusplus > 201703L
    [[nodiscard]] constexpr value_type* allocate(std::size_t n) { // C++20
        return reinterpret_cast<value_type*>( ::malloc( n * sizeof(value_type) ) ); // NOLINT(bugprone-sizeof-expression)
    }
#else
    value_type* allocate(std::size_t n) { // C++17
        return reinterpret_cast<value_type*>( ::malloc( n * sizeof(value_type) ) ); // NOLINT(bugprone-sizeof-expression)
    }
#endif

    [[nodiscard]] constexpr value_type* reallocate(value_type* p, std::size_t, std::size_t new_size) {
        return reinterpret_cast<value_type*>(
                            ::realloc( reinterpret_cast<void*>(const_cast<pointer_mutable>(p)), new_size * sizeof(value_type) ) );
    }

#if __cplusplus > 201703L
    constexpr void deallocate(value_type* p, std::size_t) noexcept {
        ::free( reinterpret_cast<void*>( const_cast<pointer_mutable>( p ) ) );
    }
#else
    void deallocate(value_type* p, std::size_t) noexcept {
        ::free( reinterpret_cast<void*>( const_cast<pointer_mutable>( p ) ) );
    }
#endif

};


#if __cplusplus > 201703L
template <class T1, class T2>
    constexpr bool operator==(const callocator<T1>& lhs, const callocator<T2>& rhs) noexcept {
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
    bool operator==(const callocator<T1>& lhs, const callocator<T2>& rhs) noexcept {
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
    bool operator!=(const callocator<T1>& lhs, const callocator<T2>& rhs) noexcept {
        return !(lhs==rhs);
    }
#endif

} /* namespace jau */

#endif // TEST_ALLOCATOR_HPP

