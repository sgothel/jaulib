/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020-2024 Gothel Software e.K.
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

#ifndef JAU_DYN_ARRAY_HPP_
#define JAU_DYN_ARRAY_HPP_

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <numbers>
#include <string>

#include <jau/basic_algos.hpp>
#include <jau/basic_types.hpp>
#include <jau/callocator.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/debug.hpp>
#include <jau/ordered_atomic.hpp>
#include <jau/secmem.hpp>

namespace jau {

// #define JAU_DEBUG_DARRAY0 1
#if JAU_DEBUG_DARRAY0
    #define JAU_DARRAY_PRINTF0(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); }
#else
    #define JAU_DARRAY_PRINTF0(...)
#endif

// #define JAU_DEBUG_DARRAY 1
#if JAU_DEBUG_DARRAY
    #define JAU_DARRAY_PRINTF(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); }
#else
    #define JAU_DARRAY_PRINTF(...)
#endif

    /** \addtogroup DataStructs
     *
     *  @{
     */

    /**
     * Implementation of a dynamic linear array storage, aka vector, including relative positional access.
     *
     * Goals are to support a high-performance CoW dynamic array implementation, jau::cow_darray,<br>
     * exposing fine grained control over its underlying storage facility.<br>
     * Further, jau::darray provides high-performance and efficient storage properties on its own.
     *
     * This class shall be compliant with <i>C++ named requirements for Container</i>.
     *
     * API and design differences to std::vector
     * - jau::darray adds a parameterized <i>growth factor</i> aspect, see setGrowthFactor(). Defaults to golden ration jau::darray::DEFAULT_GROWTH_FACTOR.
     * - <i>capacity</i> control via constructor and operations, related to *growth factor*.
     * - Iterator jau::darray::const_iterator .. are harmonized with jau::cow_ro_iterator .. used in jau:cow_darray.
     * - ...
     * - Custom constructor and operations, supporting a more efficient jau::cow_darray implementation.
     * - Custom template typename Size_type, defaults to jau::nsize_t.
     * - ...
     * - <b>TODO</b>: std::initializer_list<T> methods, ctor is provided.
     *
     * Implementation differences to std::vector and some details
     * - Using zero overhead <i>value_type*</i> as iterator type.
     * - ...
     * - Storage is operated on three iterator: *begin* <= *end* <= *storage_end*.
     * - Constructs and destructs value_type via *placement new* within the pre-allocated array capacity. Latter is managed via allocator_type.
     *
     * ### Relative access
     * Additionally to the ransom access, relative positional access via get(), put(), putN()
     * and position(), limit(), remaining(), clearPosition() is supported, as well as slice() and duplicate().
     *
     * 0 <= *position* <= *limit* <= *size* <= *capacity*
     *
     * All mutable relative accessors validate range and throw jau::IndexOutOfBoundsError, similar to the at() method.
     *
     * @anchor darray_ntt_params
     * ### Non-Type Template Parameter (NTTP) controlling Value_type memory
     * @anchor darray_memmove
     * #### use_memmove
     * `use_memmove` can be overriden and defaults to `std::is_trivially_copyable_v<Value_type>`.
     *
     * The default value has been chosen with care, see C++ Standard section 6.9 Types [TriviallyCopyable](https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable).
     *
     * See [Trivial destructor](https://en.cppreference.com/w/cpp/language/destructor#Trivial_destructor)
     * being key requirement to [TriviallyCopyable](https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable).
     * > A trivial destructor is a destructor that performs no action.
     * > Objects with trivial destructors don't require a delete-expression and may be disposed of by simply deallocating their storage.
     * > All data types compatible with the C language (POD types) are trivially destructible.`
     *
     * However, since the destructor is not being called when using `memmove` on elements within this container,
     * the requirements are more relaxed and *TriviallyCopyable* not required nor guaranteed, see below.
     *
     * `memmove` will be used to move an object inside this container memory only,
     * i.e. where construction and destruction is controlled.
     * Not requiring *TriviallyCopyable* constraints `memmove` as follows:
     * - We can't `memmove` one or more objects into this container, even with an `rvalue` reference.
     *   The `rvalue`'s destructor will be called and potential acquired resources are lost.
     * - We can `memmove` an object around within this container, i.e. when growing or shrinking the array. (*Used*)
     * - We can `memmove` an object out of this container to the user. (*Unused*)
     *
     * Relaxed requirements for `use_memmove` are:
     * - Not using inner class pointer to inner class fields or methods (like launching a thread).
     * - TBD ???
     *
     * Since element pointer and iterator are always invalidated for container after storage mutation,
     * above constraints are not really anything novel and go along with normal std::vector.
     *
     * Users may include `typedef container_memmove_compliant` in their Value_type class
     * to enforce `use_memmove` as follows:
     * - `typedef std::true_type  container_memmove_compliant;`
     *
     * @anchor darray_secmem
     * #### use_secmem
     * `use_secmem` can be overriden and defaults to `false`.
     *
     * `use_secmem`, if enabled, ensures that the underlying memory will be zeroed out
     * after use and element erasure.
     *
     * Users may include `typedef enforce_secmem` in their Value_type class
     * to enforce `use_secmem` as follows:
     * - `typedef std::true_type  enforce_secmem;`
     *
     * @see cow_darray
     */
    template <typename Value_type, typename Size_type = jau::nsize_t, typename Alloc_type = jau::callocator<Value_type>,
              bool use_memmove = std::is_trivially_copyable_v<Value_type> || is_container_memmove_compliant_v<Value_type>,
              bool use_secmem  = is_enforcing_secmem_v<Value_type>
             >
    class darray
    {
        public:
            /** Default growth factor using the golden ratio 1.618. See setGrowthFactor(). */
            constexpr static const float DEFAULT_GROWTH_FACTOR = std::numbers::phi_v<float>; // 1.618f;

            constexpr static const bool uses_memmove = use_memmove;
            constexpr static const bool uses_secmem  = use_secmem;
            constexpr static const bool uses_realloc = use_memmove && std::is_base_of_v<jau::callocator<Value_type>, Alloc_type>;

            // typedefs' for C++ named requirements: Container

            typedef Value_type                                  value_type;
            typedef value_type*                                 pointer;
            typedef const value_type*                           const_pointer;
            typedef value_type&                                 reference;
            typedef const value_type&                           const_reference;
            typedef value_type*                                 iterator;
            typedef const value_type*                           const_iterator;
            typedef Size_type                                   size_type;
            typedef std::make_signed_t<size_type>               difference_type;
            // typedef std::reverse_iterator<iterator>          reverse_iterator;
            // typedef std::reverse_iterator<const_iterator>    const_reverse_iterator;
            typedef Alloc_type                                  allocator_type;

            typedef darray<value_type, size_type,
                           allocator_type,
                           use_memmove, use_secmem>             self_t;

            /** Used to determine whether this type is a darray or has a darray, see ::is_darray_type<T> */
            typedef bool                                        darray_tag;

        private:
            typedef std::remove_const_t<Value_type>             value_type_mutable;
            /** Required to create and move immutable elements, aka const */
            typedef value_type_mutable*                         pointer_mutable;

            static constexpr void* voidptr_cast(const_pointer p) { return reinterpret_cast<void*>( const_cast<pointer_mutable>( p ) ); }

            constexpr static size_type DIFF_MAX = std::numeric_limits<difference_type>::max();
            constexpr static size_type MIN_SIZE_AT_GROW = 10;

            allocator_type m_alloc_inst;
            float m_growth_factor;
            pointer m_begin;
            pointer m_end;
            pointer m_storage_end;
            pointer m_position, m_limit;

        public:
            /** Returns type signature of implementing class's stored value type. */
            const jau::type_info& valueSignature() const noexcept {
                return jau::static_ctti<Value_type>();
            }

            /** Returns type signature of implementing class. */
            const jau::type_info& classSignature() const noexcept {
                return jau::static_ctti<self_t>();
            }

            /** Returns growth factor, see setGrowthFactor() for semantics. */
            constexpr float growthFactor() const noexcept { return m_growth_factor; }

            /**
              * Sets the growth factor when size() == capacity() is reached for growing operations,
              * defaults to golden ratio DEFAULT_GROWTH_FACTOR.
              *
              * A growth factor of > 1 will grow storage by `max(required_elements, growth_factor*capacity)`,
              * to give room for further elements (efficiency).
              *
              * A growth factor of 1 would only grow storage by required elements.
              *
              * A growth factor of [0..1) disables growing the storage, i.e. pins storage, see pinned().
              *
              * A growth factor of < 0 denotes storage is pinned() and shared with a parent instance, see slice().
              * Use has to ensure that the parent storage owner outlives this instance.
              *
              * @see pinned()
              * @see shared()
              * @see slice()
              */
            constexpr void setGrowthFactor(float v) noexcept { m_growth_factor = v; }
            /** Returns true if growthFactor() < 1, otherwise false. See setGrowthFactor(). */
            constexpr bool pinned() const { return m_growth_factor < 1.0f; }
            /** Returns true if growthFactor() < 0, otherwise false. See setGrowthFactor(). */
            constexpr bool shared() const { return m_growth_factor < 0.0f; }

        private:
            /**
             * Allocates a new store using allocator_type.
             *
             * Throws jau::IllegalArgumentException if `size_ > std::numeric_limits<difference_type>::max()`, i.e. difference_type maximum.
             *
             * Throws jau::OutOfMemoryError if allocator_type::allocate() returns nullptr.
             *
             * @param alloc the allocator_type instance
             * @param size_ the element count, must be <= `std::numeric_limits<difference_type>::max()`
             * @return nullptr if given `0 == size_` or the newly allocated memory
             */
            [[nodiscard]] constexpr value_type * allocStore(const size_type size_) {
                if( 0 != size_ ) {
                    if( size_ > DIFF_MAX ) {
                        throw jau::IllegalArgumentError("alloc "+std::to_string(size_)+" > difference_type max "+
                                std::to_string(DIFF_MAX), E_FILE_LINE);
                    }
                    if( pinned() ) {
                        throw jau::IllegalStateError("alloc "+std::to_string(size_)+" elements * "+
                                std::to_string(sizeof(value_type))+" bytes/element = "+ // NOLINT(bugprone-sizeof-expression)
                                std::to_string(size_ * sizeof(value_type))+" bytes -> pinned: "+getInfo(), E_FILE_LINE); // NOLINT(bugprone-sizeof-expression)
                        return nullptr;
                    }
                    value_type * m = m_alloc_inst.allocate(size_);
                    if( nullptr == m && size_ > 0 ) {
                        // NOLINTBEGIN(bugprone-sizeof-expression)
                        throw jau::OutOfMemoryError("alloc "+std::to_string(size_)+" elements * "+
                                std::to_string(sizeof(value_type))+" bytes/element = "+
                                std::to_string(size_ * sizeof(value_type))+" bytes -> nullptr", E_FILE_LINE);
                        // NOLINTEND(bugprone-sizeof-expression)
                    }
                    return m;
                }
                return nullptr;
            }

            template<class _Alloc_type>
            [[nodiscard]] constexpr value_type * reallocStore(const size_type new_capacity_,
                    std::enable_if_t< std::is_base_of_v<jau::callocator<value_type>, _Alloc_type>, bool > = true )
            {
                if( pinned() ) {
                    throw jau::IllegalStateError("realloc "+std::to_string(new_capacity_)+" elements * "+
                            std::to_string(sizeof(value_type))+" bytes/element = "+
                            std::to_string(new_capacity_ * sizeof(value_type))+" bytes -> pinned: "+getInfo(), E_FILE_LINE);
                    return nullptr;
                }
                if( new_capacity_ > DIFF_MAX ) {
                    throw jau::IllegalArgumentError("realloc "+std::to_string(new_capacity_)+" > difference_type max "+
                            std::to_string(DIFF_MAX), E_FILE_LINE);
                }
                value_type * m = m_alloc_inst.reallocate(m_begin, m_storage_end-m_begin, new_capacity_);
                if( nullptr == m && new_capacity_ > 0 ) {
                    free(const_cast<pointer_mutable>(m_begin)); // has not been touched by realloc
                    throw jau::OutOfMemoryError("realloc "+std::to_string(new_capacity_)+" elements * "+
                            std::to_string(sizeof(value_type))+" bytes/element = "+
                            std::to_string(new_capacity_ * sizeof(value_type))+" bytes -> nullptr", E_FILE_LINE);
                }
                return m;
            }
            template<class _Alloc_type>
            [[nodiscard]] constexpr value_type * reallocStore(const size_type new_capacity_,
                    std::enable_if_t< !std::is_base_of_v<jau::callocator<value_type>, _Alloc_type>, bool > = true )
            {
                (void)new_capacity_;
                throw jau::UnsupportedOperationException("realloc not supported on non allocator_type not based upon jau::callocator", E_FILE_LINE);
            }

            constexpr void freeStoreCheck() {
                if( shared() ) {
                    throw jau::IllegalStateError("freeStore -> shared: "+getInfo(), E_FILE_LINE);
                }
                if( m_begin && !shared() ) {
                    m_alloc_inst.deallocate(m_begin, m_storage_end-m_begin);
                }
            }
            constexpr void freeStore() noexcept {
                if( m_begin && !shared() ) {
                    m_alloc_inst.deallocate(m_begin, m_storage_end-m_begin);
                }
            }

            constexpr void clear_iterator() noexcept {
                m_begin       = nullptr;
                m_end         = nullptr;
                m_storage_end = nullptr;
                m_position    = nullptr;
                m_limit       = nullptr;
            }

            constexpr void set_iterator(pointer new_storage_, difference_type size_, difference_type capacity_) noexcept {
                const difference_type pos = std::min<difference_type>(size_, m_position - m_begin);
                m_begin       = new_storage_;
                m_end         = new_storage_+size_;
                m_storage_end = new_storage_+capacity_;
                m_position    = m_begin + pos;
                m_limit       = m_end;
            }

            constexpr void set_iterator_end(difference_type size_, difference_type capacity_) noexcept {
                m_end         = m_begin+size_;
                m_storage_end = m_begin+capacity_;
                m_limit       = m_end;
                if( m_position > m_limit) { m_position = m_limit; }
            }

            constexpr void dtor_one(iterator pos) {
                JAU_DARRAY_PRINTF0("dtor [%zd], count 1\n", (pos-m_begin));
                ( pos )->~value_type(); // placement new -> manual destruction!
                if constexpr ( uses_secmem ) {
                    zero_bytes_sec(voidptr_cast(pos), sizeof(value_type));
                }
            }

            constexpr size_type dtor_range(iterator first, const_iterator last) {
                size_type count=0;
                JAU_DARRAY_PRINTF0("dtor [%zd .. %zd], count %zd\n", (first-m_begin), (last-m_begin)-1, (last-first)-1);
                for(; first < last; ++first, ++count ) {
                    ( first )->~value_type(); // placement new -> manual destruction!
                }
                if constexpr ( uses_secmem ) {
                    zero_bytes_sec(voidptr_cast(last-count), count*sizeof(value_type));
                }
                return count;
            }

            constexpr void ctor_copy_range(pointer dest, iterator first, const_iterator last) {
                JAU_DARRAY_PRINTF0("ctor_copy_range [%zd .. %zd] -> ??, dist %zd\n", (first-m_begin), (last-m_begin)-1, (last-first)-1);
                /**
                 * TODO
                 *
                 * g++ (Debian 12.2.0-3) 12.2.0, Debian 12 Bookworm 2022-10-17
                 * g++ bug: False positive of '-Wnull-dereference'
                 * See <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=86172>
                 *
In copy constructor ‘std::__shared_count<_Lp>::__shared_count(const std::__shared_count<_Lp>&) [with __gnu_cxx::_Lock_policy _Lp = __gnu_cxx::_S_atomic]’,
    inlined from ‘std::__shared_ptr<_Tp, _Lp>::__shared_ptr(const std::__shared_ptr<_Tp, _Lp>&) [with _Tp = direct_bt::BTDevice; __gnu_cxx::_Lock_policy _Lp = __gnu_cxx::_S_atomic]’ at /usr/include/c++/12/bits/shared_ptr_base.h:1522:7,
    inlined from ‘std::shared_ptr<_Tp>::shared_ptr(const std::shared_ptr<_Tp>&) [with _Tp = direct_bt::BTDevice]’ at /usr/include/c++/12/bits/shared_ptr.h:204:7,
    inlined from ‘constexpr void jau::darray<Value_type, Size_type, Alloc_type, Size_type, use_memmove, use_secmem>::ctor_copy_range(pointer, iterator, const_iterator) [with Value_type = std::shared_ptr<direct_bt::BTDevice>; Alloc_type = jau::callocator<std::shared_ptr<direct_bt::BTDevice> >; Size_type = long unsigned int; bool use_memmove = false; bool use_secmem = false]’ at direct_bt/jaulib/include/jau/darray.hpp:300:21,
    ...
/usr/include/c++/12/bits/shared_ptr_base.h:1075:9: warning: potential null pointer dereference [-Wnull-dereference]
 1075 |       : _M_pi(__r._M_pi)
      |         ^~~~~~~~~~~~~~~~
                 */
                PRAGMA_DISABLE_WARNING_PUSH
                PRAGMA_DISABLE_WARNING_NULL_DEREFERENCE
                for(; first < last; ++dest, ++first) {
                    new (const_cast<pointer_mutable>(dest)) value_type( *first ); // placement new / TODO: See above
                }
                PRAGMA_DISABLE_WARNING_POP
            }
            constexpr pointer clone_range(iterator first, const_iterator last) {
                JAU_DARRAY_PRINTF0("clone_range [0 .. %zd], count %zd\n", (last-first)-1, (last-first)-1);
                pointer dest = allocStore(size_type(last-first));
                ctor_copy_range(dest, first, last);
                return dest;
            }
            constexpr pointer clone_range(const size_type dest_capacity, iterator first, const_iterator last) {
                JAU_DARRAY_PRINTF0("clone_range [0 .. %zd], count %zd -> %d\n", (last-m_begin)-1, (last-first)-1, (int)dest_capacity);
                pointer dest = allocStore(dest_capacity);
                ctor_copy_range(dest, first, last);
                return dest;
            }
            constexpr void ctor_copy_range_check(pointer dest, iterator first, const_iterator last) {
                JAU_DARRAY_PRINTF0("ctor_copy_range_check [%zd .. %zd] -> ??, dist %zd\n", (first-m_begin), (last-m_begin)-1, (last-first)-1);
                if( first > last ) {
                    throw jau::IllegalArgumentError("first "+toHexString(first)+" > last "+toHexString(last), E_FILE_LINE);
                }
                for(; first < last; ++dest, ++first) {
                    new (const_cast<pointer_mutable>(dest)) value_type( *first ); // placement new
                }
            }
            constexpr pointer clone_range_check(const size_type dest_capacity, iterator first, const_iterator last) {
                JAU_DARRAY_PRINTF0("clone_range_check [%zd .. %zd], count %zd -> %d\n", (first-m_begin), (last-m_begin)-1, (last-first)-1, (int)dest_capacity);
                if( dest_capacity < size_type(last-first) ) {
                    throw jau::IllegalArgumentError("capacity "+std::to_string(dest_capacity)+" < source range "+
                                                        std::to_string(difference_type(last-first)), E_FILE_LINE);
                }
                pointer dest = allocStore(dest_capacity);
                ctor_copy_range_check(dest, first, last);
                return dest;
            }

            constexpr void ctor_copy_value(pointer dest, size_type count, const value_type& val) {
                if( m_begin > dest || dest + count > m_end ) {
                    throw jau::IllegalArgumentError("dest "+jau::to_string( dest )+" + "+jau::to_string( count )+" not within ["+
                                                                 jau::to_string( m_begin )+".."+jau::to_string( m_end )+")", E_FILE_LINE);
                }
                if( 0 < count ) {
                    for(size_type i=0; i < count; ++i, ++dest) {
                        new (const_cast<pointer_mutable>(dest)) value_type( val ); // placement new // NOLINT(bugprone-multi-level-implicit-pointer-conversion): OK and intended
                    }
                }
            }
            template< class InputIt >
            constexpr static void ctor_copy_range_foreign(pointer dest, InputIt first, InputIt last) { // NOLINT(performance-unnecessary-value-param)
                if( first > last ) {
                    throw jau::IllegalArgumentError("first "+jau::to_string( first )+" > last "+
                                                                 jau::to_string( last ), E_FILE_LINE);
                }
                for(; first != last; ++dest, ++first) {
                    new (const_cast<pointer_mutable>(dest)) value_type( *first ); // placement new // NOLINT(bugprone-multi-level-implicit-pointer-conversion): OK and intended
                }
            }
            template< class InputIt >
            constexpr pointer clone_range_foreign(const size_t dest_capacity, InputIt first, InputIt last) { // NOLINT(performance-unnecessary-value-param)
                if( dest_capacity > std::numeric_limits<size_type>::max() ) {
                    throw jau::IllegalArgumentError("capacity "+std::to_string(dest_capacity)+" > size_type max "+
                                                    std::to_string(std::numeric_limits<size_type>::max()), E_FILE_LINE);
                }
                if( dest_capacity < size_type(last-first) ) {
                    throw jau::IllegalArgumentError("capacity "+std::to_string(dest_capacity)+" < source range "+
                                                        std::to_string(difference_type(last-first)), E_FILE_LINE);
                }
                pointer dest = allocStore(size_type(dest_capacity));
                ctor_copy_range_foreign(dest, first, last);
                return dest;
            }

            constexpr void realloc_storage_move(const size_type new_capacity) {
                if constexpr ( !uses_memmove ) {
                    pointer new_storage = allocStore(new_capacity);
                    {
                        iterator dest = new_storage;
                        iterator first = m_begin;
                        for(; first < m_end; ++dest, ++first) {
                            new (const_cast<pointer_mutable>(dest)) value_type( std::move( *first ) ); // placement new
                            dtor_one(first); // manual destruction, even after std::move (object still exists)
                        }
                    }
                    freeStoreCheck();
                    set_iterator(new_storage, size(), new_capacity);
                } else if constexpr ( uses_realloc ) {
                    pointer new_storage = reallocStore<allocator_type>(new_capacity);
                    set_iterator(new_storage, size(), new_capacity);
                } else {
                    pointer new_storage = allocStore(new_capacity);
                    ::memcpy(voidptr_cast(new_storage),
                             m_begin, (uint8_t*)m_end-(uint8_t*)m_begin); // we can simply copy the memory over, also no overlap
                    freeStoreCheck();
                    set_iterator(new_storage, size(), new_capacity);
                }
            }
            constexpr void grow_storage_move(size_type add=1) {
                realloc_storage_move( get_grown_capacity(add) );
            }

            constexpr void move_elements(iterator dest, const_iterator first, const difference_type count) {
                // Debatable here: "Moved source array has been taken over, flush sources' pointer to avoid value_type dtor releasing taken resources!"
                // Debatable, b/c is this even possible for user to hold an instance the way, that a dtor gets called? Probably not.
                // Hence we leave it to 'uses_secmem' to zero_bytes_sec...
                if constexpr ( uses_memmove ) {
                    // handles overlap
                    // NOLINTNEXTLINE(bugprone-undefined-memory-manipulation)
                    ::memmove(voidptr_cast(dest),
                              first, sizeof(value_type)*count);
                    if constexpr ( uses_secmem ) {
                        if( dest < first ) {
                            // move elems left
                            JAU_DARRAY_PRINTF0("move_elements.mmm.left [%zd .. %zd] -> %zd, dist %zd\n", (first-m_begin), ((first + count)-m_begin)-1, (dest-m_begin), (first-dest));
                            zero_bytes_sec(voidptr_cast(dest+count), (first-dest)*sizeof(value_type));
                        } else {
                            // move elems right
                            JAU_DARRAY_PRINTF0("move_elements.mmm.right [%zd .. %zd] -> %zd, dist %zd, size %zu\n", (first-m_begin), ((first + count)-m_begin)-1, (dest-m_begin), (dest-first), (dest-first)*sizeof(value_type));
                            PRAGMA_DISABLE_WARNING_PUSH
                            PRAGMA_DISABLE_WARNING_STRINGOP_OVERFLOW
                            zero_bytes_sec(voidptr_cast(first), (dest-first)*sizeof(value_type)); // TODO: See above
                            PRAGMA_DISABLE_WARNING_POP
                        }
                    }
                } else {
                    if( dest < first ) {
                        // move elems left
                        const_iterator last = first + count;
                        JAU_DARRAY_PRINTF0("move_elements.def.left [%zd .. %zd] -> %zd, dist %zd\n", (first-m_begin), (last-m_begin)-1, (dest-m_begin), (first-dest));
                        for(; first < last; ++dest, ++first ) {
                            new (const_cast<pointer_mutable>(dest)) value_type( std::move( *first ) ); // placement new
                            dtor_one( const_cast<value_type*>( first ) ); // manual destruction, even after std::move (object still exists)
                        }
                    } else {
                        // move elems right
                        iterator last = const_cast<iterator>(first + count);
                        JAU_DARRAY_PRINTF0("move_elements.def.right [%zd .. %zd] -> %zd, dist %zd\n", (first-m_begin), (last-m_begin)-1, (dest-m_begin), (dest-first));
                        dest += count - 1;
                        for(--last; first <= last; --dest, --last ) {
                            new (const_cast<pointer_mutable>(dest)) value_type( std::move( *last ) ); // placement new
                            dtor_one( last ); // manual destruction, even after std::move (object still exists)
                        }
                    }
                }
            }


        protected:
            /** Slicing ctor. Marks the created buffer shared() and pinned() and the given parent pinned(). */
            constexpr explicit darray(darray& parent, pointer begin, pointer position, pointer limit, size_type size)
            : m_alloc_inst( parent.m_alloc_inst ), m_growth_factor( /* shared+pinned */ -1 ),
              m_begin( begin ), m_end( m_begin + size ), m_storage_end( m_begin + size ),
              m_position(position), m_limit(limit)
            {
                if( m_begin > parent.end() || m_end > parent.end() || m_position > m_end || m_limit > m_end ) {
                    throw jau::IllegalArgumentError("Slice: Parent "+parent.getInfo()+", this "+getInfo(), E_FILE_LINE);
                }
                parent.m_growth_factor = 0; // pinned
                JAU_DARRAY_PRINTF("ctor 3: %s\n", getInfo().c_str());
            }

        public:

            // ctor w/o elements

            /**
             * Default constructor, giving zero capacity and zero memory footprint.
             */
            constexpr darray() noexcept
            : m_alloc_inst(), m_growth_factor(DEFAULT_GROWTH_FACTOR),
              m_begin( nullptr ), m_end( nullptr ), m_storage_end( nullptr ),
              m_position(m_begin), m_limit(m_end)
            {
                JAU_DARRAY_PRINTF("ctor def: %s\n", getInfo().c_str());
            }

            /**
             * Creating an empty instance with initial capacity and other (default) properties.
             * @param capacity initial capacity of the new instance.
             * @param growth_factor given growth factor, defaults to DEFAULT_GROWTH_FACTOR. See setGrowthFactor().
             * @param alloc given allocator_type, defaults to allocator_type()
             */
            constexpr explicit darray(size_type capacity, const float growth_factor=DEFAULT_GROWTH_FACTOR, const allocator_type& alloc = allocator_type())
            : m_alloc_inst( alloc ), m_growth_factor( growth_factor ),
              m_begin( allocStore(capacity) ), m_end( m_begin ), m_storage_end( m_begin + capacity ),
              m_position(m_begin), m_limit(m_end)
            {
                JAU_DARRAY_PRINTF("ctor 1: %s\n", getInfo().c_str());
            }

            /**
             * Creating a `size`d instance with initial size elements with default `value`.
             * @param std::nullptr_t argument to distinguish constructor argument overload
             * @param size initial capacity and size of new instance
             * @param value initial value for the size elements, defaults to value_type()
             * @param growth_factor given growth factor, defaults to DEFAULT_GROWTH_FACTOR. See setGrowthFactor().
             * @param alloc given allocator_type, defaults to allocator_type()
             */
            constexpr explicit darray(std::nullptr_t /*dummy*/, size_type size, value_type value=value_type(), const float growth_factor=DEFAULT_GROWTH_FACTOR, const allocator_type& alloc = allocator_type())
            : m_alloc_inst( alloc ), m_growth_factor( growth_factor ),
              m_begin( allocStore(size) ), m_end( m_begin + size ), m_storage_end( m_begin + size ),
              m_position(m_begin), m_limit(m_end)
            {
                ctor_copy_value(m_begin, size, value);
                JAU_DARRAY_PRINTF("ctor 2: %s\n", getInfo().c_str());
            }

            // copy_ctor on darray elements

            /**
             * Creates a new instance, copying all elements from the given darray.<br>
             * Capacity and size will equal the given array, i.e. the result is a trimmed jau::darray.
             *
             * Throws jau::IllegalStateError if the source instance is sliced, i.e. sharing memory
             *
             * @param x the given darray, all elements will be copied into the new instance.
             */
            constexpr darray(const darray& x)
            : m_alloc_inst( x.m_alloc_inst ), m_growth_factor( x.m_growth_factor ),
              m_begin( clone_range(x.m_begin, x.m_end) ), m_end( m_begin + x.size() ), m_storage_end( m_begin + x.size() ),
              m_position(m_begin), m_limit(m_end)
            {
                JAU_DARRAY_PRINTF("ctor copy0: this %s\n", getInfo().c_str());
                JAU_DARRAY_PRINTF("ctor copy0:    x %s\n", x.getInfo().c_str());
            }

            /**
             * Creates a new instance, copying all elements from the given darray.<br>
             * Capacity and size will equal the given array, i.e. the result is a trimmed jau::darray.
             *
             * Throws jau::IllegalStateError if the source instance is sliced, i.e. sharing memory
             *
             * @param x the given darray, all elements will be copied into the new instance.
             * @param growth_factor custom growth factor
             * @param alloc custom allocator_type instance
             */
            constexpr explicit darray(const darray& x, const float growth_factor, const allocator_type& alloc)
            : m_alloc_inst( alloc ), m_growth_factor( growth_factor ),
              m_begin( clone_range(x.m_begin, x.m_end) ), m_end( m_begin + x.size() ), m_storage_end( m_begin + x.size() ),
              m_position(m_begin), m_limit(m_end)
            {
                JAU_DARRAY_PRINTF("ctor copy1: this %s\n", getInfo().c_str());
                JAU_DARRAY_PRINTF("ctor copy1:    x %s\n", x.getInfo().c_str());
            }

            /**
             * Creates a new instance with custom initial storage capacity, copying all elements from the given darray.<br>
             * Size will equal the given array.
             *
             * Throws jau::IllegalArgumentException if `_capacity < x.size()`.
             *
             * @param x the given darray, all elements will be copied into the new instance.
             * @param _capacity custom initial storage capacity
             * @param growth_factor custom growth factor, see setGrowthFactor().
             * @param alloc custom allocator_type instance
             */
            constexpr explicit darray(const darray& x, const size_type _capacity, const float growth_factor, const allocator_type& alloc)
            : m_alloc_inst( alloc ), m_growth_factor( growth_factor ),
              m_begin( clone_range( _capacity, x.m_begin, x.m_end) ), m_end( m_begin + x.size() ), m_storage_end( m_begin + _capacity ),
              m_position(m_begin), m_limit(m_end)
            {
                JAU_DARRAY_PRINTF("ctor copy2: this %s\n", getInfo().c_str());
                JAU_DARRAY_PRINTF("ctor copy2:    x %s\n", x.getInfo().c_str());
            }

            /**
             * Like std::vector::operator=(&), assignment
             */
            constexpr darray& operator=(const darray& x) {
                JAU_DARRAY_PRINTF("assignment copy.0: this %s\n", getInfo().c_str());
                JAU_DARRAY_PRINTF("assignment copy.0:    x %s\n", x.getInfo().c_str());
                if( this != &x ) {
                    const size_type capacity_ = capacity();
                    const size_type x_size_ = x.size();
                    dtor_range(m_begin, m_end);
                    m_growth_factor = x.m_growth_factor;
                    if( x_size_ > capacity_ ) {
                        const difference_type pos = std::min<difference_type>(x_size_, m_position - m_begin);
                        freeStoreCheck();
                        m_begin =  clone_range(x_size_, x.m_begin, x.m_end);
                        m_position = m_begin + pos;
                        set_iterator_end(x_size_, x_size_);
                    } else {
                        ctor_copy_range(m_begin, x.m_begin, x.m_end);
                        set_iterator_end(x_size_, capacity_);
                    }
                }
                JAU_DARRAY_PRINTF("assignment copy.X: this %s\n", getInfo().c_str());
                JAU_DARRAY_PRINTF("assignment copy.X:    x %s\n", x.getInfo().c_str());
                return *this;
            }

            // move_ctor on darray elements

            constexpr darray(darray && x) noexcept
            : m_alloc_inst( std::move(x.m_alloc_inst) ), m_growth_factor( x.m_growth_factor ),
              m_begin( std::move(x.m_begin) ), m_end( std::move(x.m_end) ), m_storage_end( std::move(x.m_storage_end) ),
              m_position( std::move(x.m_position) ), m_limit( std::move(x.m_limit) )
            {
                JAU_DARRAY_PRINTF("ctor move0: this %s\n", getInfo().c_str());
                JAU_DARRAY_PRINTF("ctor move0:    x %s\n", x.getInfo().c_str());
                // Moved source array has been taken over, flush sources' pointer to avoid value_type dtor releasing taken resources!
                x.clear_iterator();
            }

            constexpr explicit darray(darray && x, const float growth_factor, const allocator_type& alloc) noexcept
            : m_alloc_inst( std::move(alloc) ), m_growth_factor( growth_factor ),
              m_begin( std::move(x.m_begin) ), m_end( std::move(x.m_end) ), m_storage_end( std::move(x.m_storage_end) ),
              m_position( std::move(x.m_position) ), m_limit( std::move(x.m_limit) )
            {
                JAU_DARRAY_PRINTF("ctor move1: this %s\n", getInfo().c_str());
                JAU_DARRAY_PRINTF("ctor move1:    x %s\n", x.getInfo().c_str());
                // Moved source array has been taken over, flush sources' pointer to avoid value_type dtor releasing taken resources!
                x.clear_iterator();
            }

            /**
             * Like std::vector::operator=(&&), move.
             */
            constexpr darray& operator=(darray&& x) noexcept {
                JAU_DARRAY_PRINTF("assignment move.0: this %s\n", getInfo().c_str());
                JAU_DARRAY_PRINTF("assignment move.0:    x %s\n", x.getInfo().c_str());
                if( this != &x ) {
                    clear(true);
                    m_alloc_inst = std::move(x.m_alloc_inst);
                    m_growth_factor = x.m_growth_factor;
                    m_begin = std::move(x.m_begin);
                    m_end = std::move(x.m_end);
                    m_storage_end = std::move(x.m_storage_end);
                    m_position = std::move(x.m_position);
                    m_limit = std::move(x.m_limit);

                    // Moved source array has been taken over, flush sources' pointer to avoid value_type dtor releasing taken resources!
                    x.clear_iterator();
                }
                JAU_DARRAY_PRINTF("assignment move.X: this %s\n", getInfo().c_str());
                JAU_DARRAY_PRINTF("assignment move.X:    x %s\n", x.getInfo().c_str());
                return *this;
            }

            // ctor on const_iterator and foreign template iterator

            /**
             * Creates a new instance with custom initial storage capacity,
             * copying all elements from the given const_iterator value_type range [first, last).<br>
             * Size will equal the range [first, last), i.e. `size_type(last-first)`.
             *
             * Throws jau::IllegalArgumentException if `_capacity < size_type(last - first)`.
             *
             * @param _capacity custom initial storage capacity
             * @param first const_iterator to first element of value_type range [first, last)
             * @param last const_iterator to last element of value_type range [first, last)
             * @param growth_factor given growth factor, defaults to DEFAULT_GROWTH_FACTOR. See setGrowthFactor().
             * @param alloc custom allocator_type instance
             */
            constexpr explicit darray(const size_type _capacity, const_iterator first, const_iterator last,
                                      const float growth_factor=DEFAULT_GROWTH_FACTOR, const allocator_type& alloc = allocator_type())
            : m_alloc_inst( alloc ), m_growth_factor( growth_factor ),
              m_begin( clone_range_check(_capacity, first, last) ), m_end(m_begin + size_type(last - first) ), m_storage_end( m_begin + _capacity ),
              m_position(m_begin), m_limit(m_end)
            {
                JAU_DARRAY_PRINTF("ctor iters0: %s\n", getInfo().c_str());
            }

            /**
             * Creates a new instance with custom initial storage capacity,
             * copying all elements from the given template input-iterator value_type range [first, last).<br>
             * Size will equal the range [first, last), i.e. `size_type(last-first)`.
             *
             * Throws jau::IllegalArgumentException if `_capacity < size_type(last - first)`.
             *
             * @tparam InputIt template input-iterator custom type
             * @param _capacity custom initial storage capacity
             * @param first template input-iterator to first element of value_type range [first, last)
             * @param last template input-iterator to last element of value_type range [first, last)
             * @param growth_factor given growth factor, defaults to DEFAULT_GROWTH_FACTOR. See setGrowthFactor().
             * @param alloc custom allocator_type instance
             */
            template< class InputIt >
            constexpr explicit darray(const size_type _capacity, InputIt first, InputIt last, // NOLINT(performance-unnecessary-value-param)
                                      const float growth_factor=DEFAULT_GROWTH_FACTOR, const allocator_type& alloc = allocator_type())
            : m_alloc_inst( alloc ), m_growth_factor( growth_factor ),
              m_begin( clone_range_foreign(_capacity, first, last) ), m_end(m_begin + size_type(last - first) ), m_storage_end( m_begin + _capacity ),
              m_position(m_begin), m_limit(m_end)
            {
                JAU_DARRAY_PRINTF("ctor iters1: %s\n", getInfo().c_str());
            }

            /**
             * Creates a new instance,
             * copying all elements from the given template input-iterator value_type range [first, last).<br>
             * Size will equal the range [first, last), i.e. `size_type(last-first)`.
             * @tparam InputIt template input-iterator custom type
             * @param first template input-iterator to first element of value_type range [first, last)
             * @param last template input-iterator to last element of value_type range [first, last)
             * @param alloc custom allocator_type instance
             */
            template< class InputIt >
            constexpr darray(InputIt first, InputIt last, const allocator_type& alloc = allocator_type()) // NOLINT(performance-unnecessary-value-param)
            : m_alloc_inst( alloc ), m_growth_factor( DEFAULT_GROWTH_FACTOR ),
              m_begin( clone_range_foreign(size_type(last - first), first, last) ), m_end(m_begin + size_type(last - first) ),
              m_storage_end( m_begin + size_type(last - first) ),
              m_position(m_begin), m_limit(m_end)
            {
                JAU_DARRAY_PRINTF("ctor iters2: %s\n", getInfo().c_str());
            }

            /**
             * Create a new instance from an initializer list.
             *
             * Using the `std::initializer_list` requires to *copy* the given value_type objects into this darray.
             *
             * To utilize more efficient move semantics, see push_back_list() and jau::make_darray().
             *
             * @param initlist initializer_list.
             * @param alloc allocator
             * @see push_back_list()
             * @see jau::make_darray()
             */
            constexpr darray(std::initializer_list<value_type> initlist, const allocator_type& alloc = allocator_type())
            : m_alloc_inst( alloc ), m_growth_factor( DEFAULT_GROWTH_FACTOR ),
              m_begin( clone_range_foreign(initlist.size(), initlist.begin(), initlist.end()) ),
              m_end(m_begin + initlist.size() ), m_storage_end( m_begin + initlist.size() ),
              m_position(m_begin), m_limit(m_end)
            {
                JAU_DARRAY_PRINTF("ctor initlist: %s\n", getInfo().c_str());
            }

            ~darray() noexcept {
                JAU_DARRAY_PRINTF("dtor: %s\n", getInfo().c_str());
                clear(true);
            }

            /**
             * Returns `std::numeric_limits<difference_type>::max()` as the maximum array size.
             * <p>
             * We rely on the signed `difference_type` for pointer arithmetic,
             * deducing ranges from iterator.
             * </p>
             */
            constexpr static size_type max_size() noexcept { return DIFF_MAX; }

            // iterator

            constexpr iterator begin() noexcept { return m_begin; }
            constexpr const_iterator begin() const noexcept { return m_begin; }
            constexpr const_iterator cbegin() const noexcept { return m_begin; }

            constexpr iterator end() noexcept { return m_end; }
            constexpr const_iterator end() const noexcept { return m_end; }
            constexpr const_iterator cend() const noexcept { return m_end; }

#if 0
            constexpr iterator storage_end() noexcept { return m_storage_end; }
            constexpr const_iterator storage_end() const noexcept { return m_storage_end; }
            constexpr const_iterator cstorage_end() const noexcept { return m_storage_end; }
#endif

            //
            // relative positional access
            //

            /** Next relative read/write element index, with 0 <= position <= limit.*/
            constexpr size_type position() const noexcept { return size_type(m_position - m_begin); }
            /** Pointer to mutable next relative read/write element index, with 0 <= position <= limit.*/
            constexpr pointer position_ptr() noexcept { return m_position; }
            /** Pointer to immutable next relative read/write element index, with 0 <= position <= limit.*/
            constexpr const_pointer position_ptr() const noexcept { return m_position; }
            /** Sets position. Throws exception if new position is > limit. */
            self_t& setPosition(size_type v) {
                pointer p = m_begin + v;
                if(p > m_limit) {
                    throw jau::IndexOutOfBoundsError(std::to_string(v), std::to_string(limit()), E_FILE_LINE);
                }
                m_position = p;
                return *this;
            }

            /** Read/write limit, one element beyond maximum index with limit <= size/end. */
            constexpr size_type limit() const noexcept { return size_type(m_limit - m_begin); }
            /** Pointer to immutable read/write limit, one element beyond maximum element with limit <= size/end. */
            constexpr const_pointer limit_ptr() const noexcept { return m_limit; }
            /** Sets new limit and adjusts position if new limit is below. Throws exception if new limit is > size/end. */
            self_t& setLimit(size_type v) {
                pointer p = m_begin + v;
                if(p > m_end) {
                    throw jau::IndexOutOfBoundsError(std::to_string(v), std::to_string(size()), E_FILE_LINE);
                }
                m_limit = p;
                if (m_position > p) { m_position = p; }
                return *this;
            }

            /** Sets limit to position and position to zero. */
            constexpr self_t& flip() noexcept {
                m_limit = m_position;
                m_position = m_begin;
                return *this;
            }

            /** Sets position to zero. */
            constexpr self_t& rewind() noexcept {
                m_position = m_begin;
                return *this;
            }

            /**
             * Clears the relative position and limit w/o destructing elements nor mutating storage.
             *
             * Sets position to zero and limit to size.
             *
             * @see position()
             * @see limit()
             * @see put()
             * @see get()
             */
            constexpr self_t& clearPosition() noexcept {
                m_position = m_begin;
                m_limit = m_end;
                return *this;
            }

            /**
             * Relative get() for single value reference, increasing position() by one.
             *
             * Throws if position() >= limit().
             */
            constexpr_cxx20 const_reference get() {
                if( m_position < m_limit ) {
                    return *(m_position++);
                }
                throw jau::IndexOutOfBoundsError(position(), size(), E_FILE_LINE);
            }

            /**
             * Relative put() for single value, increasing position().
             *
             * Grows storage and/or moves limit if required and grow==True().
             *
             * Throws if position() == limit().
             *
             * @param v value to be written
             * @param grow set to Bool::True if allowing to grow, otherwise exception is thrown if position() == limit(). Defaults to Bool::False.
             * @see setGrowthFactor()
             */
            constexpr_cxx20 self_t& put(const_reference v, Bool grow=Bool::False) {
                if( *grow && m_position == m_limit ) {
                    if( m_limit == m_storage_end ) {
                        grow_storage_move();
                        resize(size()+1);
                    } else if( limit() == size() ) {
                        resize(size()+1);
                    } else {
                        m_limit++;
                    }
                }
                if( m_position < m_limit ) {
                    *(m_position++) = v;
                    return *this;
                }
                throw jau::IndexOutOfBoundsError(position(), size(), E_FILE_LINE);
            }

            /**
             * Relative put() for an initializer list of same type, increasing position().
             *
             * Grows storage and/or moves limit if required and grow==True().
             *
             * Throws if position() + count > limit().
             *
             * @param initlist values to be written
             * @param grow set to Bool::True if allowing to grow, otherwise exception is thrown if position() == limit(). Defaults to Bool::False.
             * @see setGrowthFactor()
             */
            constexpr_cxx20 self_t& put(std::initializer_list<value_type> initlist, Bool grow=Bool::False)
            {
                if( initlist.size() > std::numeric_limits<size_type>::max() ) {
                    throw jau::IllegalArgumentError("capacity "+std::to_string(initlist.size())+" > size_type max "+
                                                    std::to_string(std::numeric_limits<size_type>::max()), E_FILE_LINE);
                }
                const size_type count1 = size_type(initlist.size()); // number of elements to put
                if( *grow && m_position + count1 > m_limit ) {
                    const size_type count2 = position() + count1 - limit(); // number of newly required elements (space)
                    if( m_limit + count2 > m_storage_end ) {
                        grow_storage_move(limit() + count2 - capacity());
                        // resize(limit() + count2);
                        m_end += count2;
                        m_limit += count2;
                    } else if( m_limit + count2 > m_end ) {
                        // resize(limit() + count2);
                        m_end += count2;
                        m_limit += count2;
                    } else {
                        m_limit+=count2;
                    }
                }
                if( m_position + count1 - 1 < m_limit ) {
                    ctor_copy_range_foreign(m_position, initlist.begin(), initlist.end());
                    m_position+=count1;
                    return *this;
                }
                throw jau::IndexOutOfBoundsError(getInfo(), position(), count1, E_FILE_LINE);
            }

            /**
             * Relative put() for multiple value of an assignable type fitting into value_type, increasing position().
             *
             * Grows storage and/or moves limit if required and grow==True().
             *
             * Throws if position() + count > limit().
             *
             * @tparam Targs argument types, which must all be of the same type
             * @param grow set to Bool::True if allowing to grow, otherwise exception is thrown if position() == limit(). Defaults to Bool::False.
             * @param args values to be written
             * @see setGrowthFactor()
             */
            template<typename... Targs,
                std::enable_if_t< jau::is_all_same_v<Targs...> &&
                                  sizeof(Value_type) >= sizeof(jau::first_type<Targs...>) &&                  // NOLINT(bugprone-sizeof-expression)
                                  std::is_assignable_v<value_type&, jau::first_type<Targs...>>, bool> = true>
            constexpr_cxx20 self_t& putN(Bool grow, const Targs&...args) {
                const size_type count1 = sizeof...(args);
                if( *grow && m_position + count1 > m_limit ) {
                    const size_type count2 = position() + count1 - limit();
                    if( m_limit + count2 > m_storage_end ) {
                        grow_storage_move(limit() + count2 - capacity());
                        resize(limit() + count2);
                    } else if( m_limit + count2 > m_end ) {
                        resize(limit() + count2);
                    } else {
                        m_limit+=count2;
                    }
                }
                if( m_position + count1 - 1 < m_limit ) {
                    ( (*m_position++ = static_cast<Value_type>(args)), ... ); // NOLINT(bugprone-signed-char-misuse)
                    return *this;
                }
                throw jau::IndexOutOfBoundsError(getInfo(), position(), count1, E_FILE_LINE);
            }

            /**
             * Returns a sliced duplicate starting from this buffers' current position.
             *
             * This buffer is pinned() afterwards, to not allow storage mutation.
             *
             * Returned buffer is shared() and pinned(), i.e.  shares the same storage of this buffer.
             * Its position is zero and limit set to this buffers' remaining elements.
             *
             * @see pinned()
             * @see shared()
             * @see setGrowthFactor()
             */
            self_t slice() {
                const size_type new_size = size_type(m_limit - m_position);
                return darray(*this, m_position, m_position, m_position+new_size, new_size);
            }

            /**
             * Returns a sliced duplicate starting from the given `idx`.
             *
             * This buffer is pinned() afterwards, to not allow storage mutation.
             *
             * Returned buffer is shared() and pinned(), i.e. shares the same storage of this buffer.
             * Its position is zero and limit set to the given `length`.
             */
            self_t slice(size_type idx, size_type length) {
                if(m_position + idx + length > m_limit) {
                    throw jau::IndexOutOfBoundsError(std::to_string(position()), std::to_string(limit()), E_FILE_LINE);
                }
                return darray(*this, m_position+idx, m_position+idx, m_position+idx+length, length);
            }

            /**
             * Returns a duplicate with same position and limit.
             *
             * This buffer is pinned() afterwards, to not allow storage mutation.
             *
             * Returned buffer is shared() and pinned(), i.e.  shares the same storage of this buffer.
             * Its position and limit are same as with this buffer.
             */
            self_t duplicate() {
                m_growth_factor = 0; // pinned
                return darray(*this, m_begin, m_position, m_limit, size());
            }

            /** Returns limit - position. */
            constexpr size_type remaining() const noexcept { return size_type(m_limit - m_position); }
            /** Returns whether position < limit, i.e. has remaining elements. */
            constexpr bool hasRemaining() const noexcept { return m_position < m_limit; }

            //
            // read access
            //

            const allocator_type& get_allocator_ref() const noexcept {
                return m_alloc_inst;
            }

            allocator_type get_allocator() const noexcept {
                return allocator_type(m_alloc_inst);
            }

            /**
             * Return the current capacity.
             */
            constexpr size_type capacity() const noexcept { return size_type(m_storage_end - m_begin); }

            /**
             * Return the current capacity() multiplied by the growth factor, minimum is max(capacity()+add, 10).
             */
            constexpr size_type get_grown_capacity(size_type add=1) const noexcept {
                const size_type a_capacity = capacity();
                return std::max<size_type>( std::max<size_type>( MIN_SIZE_AT_GROW, a_capacity+add ),
                                            static_cast<size_type>( ::roundf(a_capacity * growthFactor()) ) );
            }

            /**
             * Like std::vector::empty().
             */
            constexpr bool empty() const noexcept { return m_begin == m_end; }

            /**
             * Returns true if capacity has been reached and the next push_back()
             * will grow the storage and invalidates all iterators and references.
             */
            constexpr bool capacity_reached() const noexcept { return m_end >= m_storage_end; }

            /**
             * Like std::vector::size().
             */
            constexpr size_type size() const noexcept { return size_type(m_end - m_begin); }

            // mixed mutable/immutable element access

            /**
             * Like std::vector::front(), mutable access.
             */
            constexpr reference front() { return *m_begin; }

            /**
             * Like std::vector::front(), immutable access.
             */
            constexpr const_reference front() const { return *m_begin; }

            /**
             * Like std::vector::back(), mutable access.
             */
            constexpr reference back() { return *(m_end-1); }

            /**
             * Like std::vector::back(), immutable access.
             */
            constexpr const_reference back() const { return *(m_end-1); }

            /**
             * Like std::vector::data(), const immutable pointer
             */
            constexpr const_pointer data() const noexcept { return m_begin; }

            /**
             * Like std::vector::data(), mutable pointer
             */
            constexpr pointer data() noexcept { return m_begin; }

            /**
             * Like std::vector::operator[](size_type), immutable reference.
             */
            constexpr_cxx20 const_reference operator[](size_type i) const noexcept {
                return *(m_begin+i);
            }

            /**
             * Like std::vector::operator[](size_type), mutable reference.
             */
            constexpr_cxx20 reference operator[](size_type i) noexcept {
                return *(m_begin+i);
            }

            /**
             * Like std::vector::at(size_type), immutable reference.
             */
            constexpr_cxx20 const_reference at(size_type i) const {
                if( 0 <= i && i < size() ) {
                    return *(m_begin+i);
                }
                throw jau::IndexOutOfBoundsError(i, size(), E_FILE_LINE);
            }

            /**
             * Like std::vector::at(size_type), mutable reference.
             */
            constexpr_cxx20 reference at(size_type i) {
                if( 0 <= i && i < size() ) {
                    return *(m_begin+i);
                }
                throw jau::IndexOutOfBoundsError(i, size(), E_FILE_LINE);
            }

            // write access, mutable array operations

            /**
             * Like std::vector::reserve(), increases this instance's capacity to `new_capacity`.
             * <p>
             * Only creates a new storage and invalidates iterators if `new_capacity`
             * is greater than the current jau::darray::capacity().
             * </p>
             */
            constexpr self_t& reserve(size_type new_capacity) {
                if( new_capacity > capacity() ) {
                    realloc_storage_move(new_capacity);
                }
                return *this;
            }

            /**
             * Like std::vector::resize(size_type, const value_type&)
             */
            constexpr self_t& resize(size_type new_size, const value_type& val) {
                const size_type sz = size();
                if( new_size > sz ) {
                    if( new_size > capacity() ) {
                        realloc_storage_move(new_size);
                    }
                    const size_type new_elem_count = new_size - sz;
                    m_end += new_elem_count;
                    m_limit += new_elem_count;
                    ctor_copy_value(m_begin + sz, new_elem_count, val);
                } else if( new_size < sz ) {
                    const size_type del_elem_count = dtor_range(m_begin + new_size, m_end);
                    assert(sz - new_size == del_elem_count);
                    m_end -= del_elem_count;
                }
                return *this;
            }

            /**
             * Like std::vector::resize(size_type)
             */
            constexpr self_t& resize(size_type new_size) { return resize(new_size, value_type()); }

            /**
             * Like std::vector::shrink_to_fit(), but ensured `constexpr`.
             *
             * If capacity() > size(), reallocate storage to size().
             */
            constexpr self_t& shrink_to_fit() {
                const size_type size_ = size();
                if( capacity() > size_ ) {
                    realloc_storage_move(size_);
                }
                return *this;
            }

            /**
             * Like std::vector::assign()
             * @tparam InputIt foreign input-iterator to range of value_type [first, last)
             * @param first first foreign input-iterator to range of value_type [first, last)
             * @param last last foreign input-iterator to range of value_type [first, last)
             */
            template< class InputIt >
            constexpr void assign( InputIt first, InputIt last ) { // NOLINT(performance-unnecessary-value-param)
                const size_type size_ = size();
                const size_type capacity_ = capacity();
                const size_type x_size_ = size_type(last - first);
                dtor_range(m_begin, m_end);
                if( x_size_ > capacity_ ) {
                    const difference_type pos = std::min<difference_type>(x_size_, m_position - m_begin);
                    freeStoreCheck();
                    m_begin =  clone_range_foreign(x_size_, first, last);
                    m_position = m_begin + pos;
                    set_iterator_end(x_size_, x_size_);
                } else {
                    ctor_copy_range_foreign(m_begin, first, last);
                    set_iterator_end(x_size_, capacity_);
                }
            }
            /**
             * Like std::vector::assign(), but non-template overload using const_iterator.
             * @param first first const_iterator to range of value_type [first, last)
             * @param last last const_iterator to range of value_type [first, last)
             */
            constexpr void assign( const_iterator first, const_iterator last ) {
                const size_type size_ = size();
                const size_type capacity_ = capacity();
                const size_type x_size_ = size_type(last - first);
                dtor_range(m_begin, m_end);
                if( x_size_ > capacity_ ) {
                    const difference_type pos = std::min<difference_type>(x_size_, m_position - m_begin);
                    freeStoreCheck();
                    m_begin =  clone_range_check(x_size_, first, last);
                    m_position = m_begin + pos;
                    set_iterator_end(x_size_, x_size_);
                } else {
                    ctor_copy_range_check(m_begin, first, last);
                    set_iterator_end(x_size_, capacity_);
                }
            }

            /**
             * Like std::vector::clear(), calls destructor on all elements and leaving capacity unchanged.
             *
             * Sets position and limit to zero.
             *
             * Use clear(true) or subsequently shrink_to_fit() to release capacity (storage).
             *
             * @see clear(bool)
             * @see shrink_to_fit()
             */
            constexpr self_t& clear() noexcept {
                dtor_range(m_begin, m_end);
                m_end = m_begin;
                m_position = m_begin;
                m_limit = m_end;
                return *this;
            }

            /**
             * Like std::vector::clear(), calls destructor on all elements.
             *
             *
             * Sets position and limit to zero.
             *
             * If `releaseMem` is `true`, releases capacity (memory),
             * otherwise leaves capacity unchanged and sets position to zero, limit to capacity.
             *
             * @see clear()
             */
            constexpr self_t& clear(bool releaseMem) noexcept {
                clear();
                if( releaseMem ) {
                    if( !shared() ) {
                        freeStore();
                    }
                    m_begin = nullptr;
                    m_end = nullptr;
                    m_storage_end = nullptr;
                    m_position = nullptr;
                    m_limit = nullptr;
                }
                return *this;
            }

            /**
             * Like std::vector::swap().
             */
            constexpr void swap(darray& x) noexcept {
                JAU_DARRAY_PRINTF("swap.0: this %s\n", getInfo().c_str());
                JAU_DARRAY_PRINTF("swap.0:    x %s\n", x.getInfo().c_str());
                std::swap(m_alloc_inst, x.m_alloc_inst);
                std::swap(m_growth_factor, x.m_growth_factor);
                std::swap(m_begin, x.m_begin);
                std::swap(m_end, x.m_end);
                std::swap(m_storage_end, x.m_storage_end);
                std::swap(m_position, x.m_position);
                std::swap(m_limit, x.m_limit);
                JAU_DARRAY_PRINTF("swap.X: this %s\n", getInfo().c_str());
                JAU_DARRAY_PRINTF("swap.X:    x %s\n", x.getInfo().c_str());
            }

            /**
             * Like std::vector::pop_back().
             *
             * Updates end, limit and adjusts position if > new limit.
             */
            constexpr void pop_back() noexcept {
                if( m_begin != m_end ) {
                    dtor_one( --m_end );
                    m_limit = m_end;
                    if( m_position > m_limit ) { m_position = m_limit; }
                }
            }

            /**
             * Like std::vector::erase(), removes the elements at pos.
             *
             * Updates end, limit and adjusts position if > > cpos
             *
             * @return iterator following the last removed element.
             */
            constexpr iterator erase (const_iterator cpos) {
                iterator pos = const_cast<iterator>(cpos);
                if( m_begin <= pos && pos < m_end ) {
                    dtor_one( pos );
                    const difference_type right_count = m_end - ( pos + 1 ); // pos is exclusive
                    if( 0 < right_count ) {
                        move_elements(pos, pos+1, right_count); // move right elems one left
                    }
                    m_limit = --m_end;
                    if( m_position > m_begin && m_position > pos ) { --m_position; }
                }
                return m_begin <= pos && pos <= m_end ? pos : m_end;
            }

            /**
             * Like std::vector::erase(), removes the elements in the range [first, last).
             *
             * Updates end, limit and adjusts position if > cfirst.
             *
             * @return iterator following the last removed element.
             */
            constexpr iterator erase (const_iterator cfirst, const_iterator clast) {
                iterator first = const_cast<iterator>(cfirst);
                const size_type count = dtor_range(first, clast);
                if( count > 0 ) {
                    const difference_type right_count = m_end - clast;  // last is exclusive
                    if( 0 < right_count ) {
                        move_elements(first, clast, right_count); // move right elems count left
                    }
                    m_end -= count;
                    m_limit = m_end;
                    if( m_position > m_begin && m_position > first )
                    { m_position -= std::min(count, size_type(m_position - first)); }
                }
                return m_begin <= first && first <= m_end ? first : m_end;
            }

            /**
             * Similar to std::vector::erase() using an index, removes the elements at pos_idx.
             *
             * Updates end, limit and adjusts position if > new limit.
             *
             * @return iterator following the last removed element.
             */
            constexpr iterator erase (const size_type pos_idx) {
                return erase(m_begin + pos_idx);
            }

            /**
             * Similar to std::vector::erase() using indices, removes the elements in the range [first_idx, last_idx).
             *
             * Updates end, limit and adjusts position if > new limit.
             *
             * @return iterator following the last removed element.
             */
            constexpr iterator erase (const size_type first_idx, const size_type last_idx) {
                return erase(m_begin + first_idx, m_begin + last_idx);
            }

            /**
             * Like std::vector::insert(), copy
             *
             * Inserts the element before pos
             * and moves all elements from there to the right beforehand.
             *
             * size/end and limit will be increased by one.
             *
             * @param pos iterator before which the content will be inserted. pos may be the end() iterator
             * @param x element value to insert
             */
            constexpr iterator insert(const_iterator pos, const value_type& x) {
                if( m_begin <= pos && pos <= m_end ) {
                    if( m_end == m_storage_end ) {
                        const size_type pos_idx = pos - m_begin;
                        grow_storage_move();
                        pos = m_begin + pos_idx;
                    }
                    const difference_type right_count = m_end - pos; // include original element at 'pos_new'
                    if( 0 < right_count ) {
                        move_elements(const_cast<iterator>(pos+1), pos, right_count); // move elems one right
                    }
                    new (const_cast<pointer_mutable>(pos)) value_type( x ); // placement new
                    m_limit = ++m_end;

                    return m_begin <= pos && pos <= m_end ? const_cast<iterator>(pos) : m_end;
                } else {
                    throw jau::IndexOutOfBoundsError(std::to_string(difference_type(pos - m_begin)), std::to_string(size()), E_FILE_LINE);
                }
            }

            /**
             * Similar to std::vector::insert() using an index, copy
             * @param pos_idx index before which the content will be inserted. index may be the end size() index
             * @param x element value to insert
             * @see insert()
             */
            constexpr iterator insert(const size_type pos_idx, const value_type& x) {
                return insert(m_begin + pos_idx, x);
            }

            /**
             * Like std::vector::insert(), move
             *
             * Inserts the element before the given position
             * and moves all elements from there to the right beforehand.
             *
             * size/end and limit will be increased by one.
             *
             * @param pos iterator before which the content will be inserted. pos may be the end() iterator
             * @param x element value to be moved into
             */
            constexpr iterator insert(const_iterator pos, value_type&& x) {
                if( m_begin <= pos && pos <= m_end ) {
                    const size_type pos_idx = pos - m_begin;
                    if( m_end == m_storage_end ) {
                        grow_storage_move();
                    }
                    iterator pos_new = m_begin + pos_idx;
                    const difference_type right_count = m_end - pos_new; // include original element at 'pos_new'
                    if( 0 < right_count ) {
                        move_elements(pos_new+1, pos_new, right_count); // move elems one right
                    }
                    new (const_cast<pointer_mutable>(pos_new)) value_type( std::move( x ) ); // placement new
                    m_limit = ++m_end;

                    return m_begin <= pos_new && pos_new <= m_end ? pos_new : m_end;
                } else {
                    throw jau::IndexOutOfBoundsError(std::to_string(difference_type(pos - m_begin)), std::to_string(size()), E_FILE_LINE);
                }
            }

            /**
             * Like std::vector::emplace(), construct a new element in place.
             *
             * Constructs the element before the given position using placement new
             * and moves all elements from there to the right beforehand.
             *
             * size/end and limit will be increased by one.
             *
             * @param pos iterator before which the content will be inserted. pos may be the end() iterator
             * @param args arguments to forward to the constructor of the element
             */
            template<typename... Args>
            constexpr iterator emplace(const_iterator pos, Args&&... args) {
                if( m_begin <= pos && pos <= m_end ) {
                    const size_type pos_idx = pos - m_begin;
                    if( m_end == m_storage_end ) {
                        grow_storage_move();
                    }
                    iterator pos_new = m_begin + pos_idx;
                    const difference_type right_count = m_end - pos_new; // include original element at 'pos_new'
                    if( 0 < right_count ) {
                        move_elements(pos_new+1, pos_new, right_count); // move elems one right
                    }
                    new (const_cast<pointer_mutable>(pos_new)) value_type( std::forward<Args>(args)... ); // placement new, construct in-place
                    m_limit = ++m_end;

                    return m_begin <= pos_new && pos_new <= m_end ? pos_new : m_end;
                } else {
                    throw jau::IndexOutOfBoundsError(std::to_string(difference_type(pos - m_begin)), std::to_string(size()), E_FILE_LINE);
                }
            }

            /**
             * Like std::vector::insert(), inserting the value_type range [first, last).
             *
             * size/end and limit will be increased by inserted elements.
             *
             * @tparam InputIt foreign input-iterator to range of value_type [first, last)
             * @param pos iterator before which the content will be inserted. pos may be the end() iterator
             * @param first first foreign input-iterator to range of value_type [first, last)
             * @param last last foreign input-iterator to range of value_type [first, last)
             * @return Iterator pointing to the first element inserted, or pos if first==last.
             */
            template< class InputIt >
            constexpr iterator insert( const_iterator pos, InputIt first, InputIt last ) { // NOLINT(performance-unnecessary-value-param)
                if( m_begin <= pos && pos <= m_end ) {
                    const size_type new_elem_count = size_type(last - first);
                    const size_type pos_idx = pos - m_begin;
                    if( m_end + new_elem_count > m_storage_end ) {
                        grow_storage_move(new_elem_count);
                    }
                    iterator pos_new = m_begin + pos_idx;
                    const difference_type right_count = m_end - pos_new; // include original element at 'pos_new'
                    if( 0 < right_count ) {
                        move_elements(pos_new + new_elem_count, pos_new, right_count); // move elems count right
                    }
                    ctor_copy_range_foreign(pos_new, first, last);
                    m_end += new_elem_count;
                    m_limit = m_end;

                    return m_begin <= pos_new && pos_new <= m_end ? pos_new : m_end;
                } else {
                    throw jau::IndexOutOfBoundsError(std::to_string(difference_type(pos - m_begin)), std::to_string(size()), E_FILE_LINE);
                }
            }

            /**
             * Like std::vector::push_back(), copy
             *
             * size/end and limit will be increased by one, position set to limit/end.
             *
             * @param x the value to be added at the tail.
             */
            constexpr void push_back(const value_type& x) {
                if( m_end == m_storage_end ) {
                    grow_storage_move();
                }
                new (const_cast<pointer_mutable>(m_end)) value_type( x ); // placement new
                m_limit = ++m_end;
                m_position = m_limit;
            }

            /**
             * Like std::vector::push_back(), move
             *
             * size/end and limit will be increased by one, position set to limit/end.
             *
             * @param x the value to be added at the tail.
             */
            constexpr void push_back(value_type&& x) {
                if( m_end == m_storage_end ) {
                    grow_storage_move();
                }
                new (const_cast<pointer_mutable>(m_end)) value_type( std::move(x) ); // placement new, just one element - no optimization
                m_limit = ++m_end;
                m_position = m_limit;
            }

            /**
             * Like std::push_back(), but for an initializer list to copy.
             *
             * size/end and limit will be increased by inserted elements, position set to limit/end.
             *
             * @param initlist values to be written
             */
            constexpr_cxx20 void push_back(std::initializer_list<value_type> initlist)
            {
                if( initlist.size() > std::numeric_limits<size_type>::max() ) {
                    throw jau::IllegalArgumentError("capacity "+std::to_string(initlist.size())+" > size_type max "+
                                                    std::to_string(std::numeric_limits<size_type>::max()), E_FILE_LINE);
                }
                const size_type count1 = size_type(initlist.size());
                if( m_end + count1 > m_storage_end ) {
                    const size_type epos = size_type(m_end - m_begin);
                    const size_type spos = size_type(m_storage_end - m_begin);
                    const size_type count2 = epos + count1 - spos;
                    grow_storage_move(limit() + count2 - capacity());
                }
                ctor_copy_range_foreign(m_end, initlist.begin(), initlist.end());
                m_end += count1;
                m_limit = m_end;
                m_position = m_limit;
            }

            /**
             * Like std::vector::push_front(), copy
             *
             * size/end and limit will be increased by one.
             *
             * @param x the value to be added at the front.
             */
            constexpr void push_front(const value_type& x) {
                insert(m_begin, x);
            }

            /**
             * Like std::vector::push_front(), move
             *
             * size/end and limit will be increased by one.
             *
             * @param x the value to be added at the front.
             */
            constexpr void push_front(value_type&& x) {
                insert(m_begin, std::move(x));
            }

            /**
             * Like std::vector::emplace_back(), construct a new element in place at the end().
             *
             * Constructs the element at the end() using placement new.
             *
             * size/end and limit will be increased by one, position set to limit/end.
             *
             * @param args arguments to forward to the constructor of the element
             */
            template<typename... Args>
            constexpr reference emplace_back(Args&&... args) {
                if( m_end == m_storage_end ) {
                    grow_storage_move();
                }
                new (const_cast<pointer_mutable>(m_end)) value_type( std::forward<Args>(args)... ); // placement new, construct in-place
                reference res = *m_end;
                m_limit = ++m_end;
                m_position=m_limit;
                return res;
            }

            /**
             * Like std::vector::push_back(), but appends the value_type range [first, last).
             *
             * size/end and limit will be increased by inserted elements, position set to limit/end.
             *
             * @tparam InputIt foreign input-iterator to range of value_type [first, last)
             * @param first first foreign input-iterator to range of value_type [first, last)
             * @param last last foreign input-iterator to range of value_type [first, last)
             */
            template< class InputIt >
            constexpr void push_back( InputIt first, InputIt last ) { // NOLINT(performance-unnecessary-value-param)
                const size_type count = size_type(last - first);

                if( m_end + count > m_storage_end ) {
                    grow_storage_move(count);
                }
                ctor_copy_range_foreign(m_end, first, last);
                m_end += count;
                m_limit = m_end;
                m_position=m_limit;
            }

            /**
             * Like push_back(), but for more multiple const r-value to copy.
             *
             * size/end and limit will be increased by inserted elements, position set to limit/end.
             *
             * @tparam Args
             * @param args r-value references to copy into this storage
             */
            template <typename... Args>
            constexpr void push_back_list(const Args&... args)
            {
                const size_type count = sizeof...(Args);

                JAU_DARRAY_PRINTF("push_back_list.copy.0: %zu elems: this %s\n", count, getInfo().c_str());

                if( m_end + count > m_storage_end ) {
                    grow_storage_move(count);
                }
                // C++17 fold expression on above C++11 template pack args
                ( new (const_cast<pointer_mutable>(m_end++)) value_type( args ), ... ); // @suppress("Syntax error")
                m_limit = m_end;
                m_position=m_limit;

                JAU_DARRAY_PRINTF("push_back_list.copy.X: %zu elems: this %s\n", count, getInfo().c_str());
            }

            /**
             * Like push_back(), but for more multiple r-value references to move.
             *
             * size/end and limit will be increased by inserted elements, position set to limit/end.
             *
             * @tparam Args
             * @param args r-value references to move into this storage
             * @see jau::make_darray()
             */
            template <typename... Args>
            constexpr void push_back_list(Args&&... args)
            {
                const size_type count = sizeof...(Args);

                JAU_DARRAY_PRINTF("push_back_list.move.0: %zu elems: this %s\n", count, getInfo().c_str());

                if( m_end + count > m_storage_end ) {
                    grow_storage_move(count);
                }
                // C++17 fold expression on above C++11 template pack args
                ( new (const_cast<pointer_mutable>(m_end++)) value_type( std::move(args) ), ... ); // @suppress("Syntax error")
                m_limit = m_end;
                m_position=m_limit;

                JAU_DARRAY_PRINTF("push_back_list.move.X: %zu elems: this %s\n", count, getInfo().c_str());
            }

            /**
             * Generic value_type equal comparator to be user defined for e.g. jau::darray::push_back_unique().
             * @param a one element of the equality test.
             * @param b the other element of the equality test.
             * @return true if both are equal
             */
            typedef bool(*equal_comparator)(const value_type& a, const value_type& b);

            /**
             * Like std::vector::push_back(), but only if the newly added element does not yet exist.
             * <p>
             * Examples
             * <pre>
             *     static jau::darray<Thing>::equal_comparator thingEqComparator =
             *                  [](const Thing &a, const Thing &b) -> bool { return a == b; };
             *     ...
             *     jau::darray<Thing> list;
             *
             *     bool added = list.push_back_unique(new_element, thingEqComparator);
             *     ...
             *     darray<std::shared_ptr<Thing>> listOfRefs;
             *     bool added = listOfRefs.push_back_unique(new_element,
             *                    [](const std::shared_ptr<Thing> &a, const std::shared_ptr<Thing> &b) -> bool { return *a == *b; });
             * </pre>
             * </p>
             * @param x the value to be added at the tail, if not existing yet.
             * @param comparator the equal comparator to return true if both given elements are equal
             * @return true if the element has been uniquely added, otherwise false
             * @see push_back()
             */
            constexpr bool push_back_unique(const value_type& x, equal_comparator comparator) {
                for(auto it = m_begin; it != m_end; ) {
                    if( comparator( *it, x ) ) {
                        return false; // already included
                    } else {
                        ++it;
                    }
                }
                push_back(x);
                return true;
            }

            /**
             * Erase either the first matching element or all matching elements using erase().
             * <p>
             * Examples
             * <pre>
             *     darray<Thing> list;
             *     int count = list.erase_matching(element, true,
             *                    [](const Thing &a, const Thing &b) -> bool { return a == b; });
             *     ...
             *     static jau::darray<Thing>::equal_comparator thingRefEqComparator =
             *                  [](const std::shared_ptr<Thing> &a, const std::shared_ptr<Thing> &b) -> bool { return *a == *b; };
             *     ...
             *     darray<std::shared_ptr<Thing>> listOfRefs;
             *     int count = listOfRefs.erase_matching(element, false, thingRefEqComparator);
             * </pre>
             * </p>
             * @param x the value to be added at the tail, if not existing yet.
             * @param all_matching if true, erase all matching elements, otherwise only the first matching element.
             * @param comparator the equal comparator to return true if both given elements are equal
             * @return number of erased elements
             * @see erase()
             */
            constexpr size_type erase_matching(const value_type& x, const bool all_matching, equal_comparator comparator) {
                size_type count = 0;
                for(auto it = m_end-1; m_begin <= it; --it) {
                    if( comparator( *it, x ) ) {
                        erase(it);
                        ++count;
                        if( !all_matching ) {
                            break;
                        }
                    }
                }
                return count;
            }

            /**
             * Erase either the first matching element or all matching elements.
             * <p>
             * Examples
             * <pre>
             *     darray<Thing> list;
             *     int count = list.erase_if(true,
             *                    [&element](const Thing &a) -> bool { return a == element; });
             * </pre>
             * </p>
             * @param all_matching if true, erase all matching elements, otherwise only the first matching element.
             * @param p the unary predicate test to return true if given elements shall be erased
             * @return number of erased elements
             */
             template<class UnaryPredicate>
             constexpr size_type erase_if(const bool all_matching, UnaryPredicate p) {
                size_type count = 0;
                for(auto it = m_end-1; m_begin <= it; --it) {
                    if( p( *it ) ) {
                        erase(it);
                        ++count;
                        if( !all_matching ) {
                            break;
                        }
                    }
                }
                return count;
            }

            std::string toString() const {
                std::string res("{ " + valueSignature().name() + ", " + std::to_string( size() ) + "/" + std::to_string( capacity() ) + ": ");
                int i=0;
                jau::for_each_const(*this, [&res, &i](const value_type & e) {
                    if( 1 < ++i ) { res.append(", "); }
                    res.append( jau::to_string(e) );
                } );
                res.append(" }");
                return res;
            }

            std::string getInfo() const noexcept {
                difference_type cap_ = (m_storage_end-m_begin);
                difference_type size_ = (m_end-m_begin);
                std::string res("darray[this "+jau::toHexString(this)+
                                ", " + valueSignature().name() +
                                ", size "+std::to_string(size_)+" / "+std::to_string(cap_)+
                                ", flags[");
                if( pinned() ) {
                    res.append("pinned");
                    if( shared() ) {
                        res.append(", shared");
                    }
                }
                res.append("], growth "+std::to_string(m_growth_factor)+
                           ", type[integral "+std::to_string(std::is_integral_v<Value_type>)+
                           ", trivialCpy "+std::to_string(std::is_trivially_copyable_v<Value_type>)+
                           "], uses[mmove "+std::to_string(uses_memmove)+
                           ", realloc "+std::to_string(uses_realloc)+
                           ", smem "+std::to_string(uses_secmem)+
                           "], begin "+jau::toHexString(m_begin)+
                           ", [pos "+std::to_string(m_position-m_begin)+
                           ", lim "+std::to_string(m_limit-m_begin)+
                           ", end "+std::to_string(m_end-m_begin)+" "+jau::toHexString(m_end)+
                           ", send "+std::to_string(m_storage_end-m_begin)+" "+jau::toHexString(m_storage_end)+
                           "]");
                return res;
            }
    };

    /**
     * Construct a darray<T> instance, initialized by move semantics from the variadic (template pack) argument list.
     *
     * std::initializer_list<T> enforces to copy the created instances into the container,
     * since its iterator references to `const` value_type.
     *
     * This alternative template passes the r-value argument references to darray::push_back_list(),
     * hence using `std::move` without copying semantics.
     *
     * All argument types must be of same type, i.e. std::is_same.
     * The deduced darray<T> instance also uses same type as its Value_type.
     *
     * @tparam First the first argument type, must be same
     * @tparam Next all other argument types, must be same
     * @tparam
     * @param arg1 the first r-value
     * @param argsN the other r-values
     * @return the new `darray`
     * @see darray::push_back_list()
     * @see make_darray()
     */
    template <typename First, typename... Next,
              // std::enable_if_t< ( std::is_same<First, Next>::value && ... ), bool> = true>
              std::enable_if_t< std::conjunction_v<std::is_same<First, Next>... >, bool> = true>
    constexpr darray< First > make_darray(First&& arg1, Next&&... argsN)
    {
        darray< First > d(1 + sizeof...(Next));
        // C++17 fold expression on above C++11 template pack arg1 and argsN
        // d.push_back_list( std::forward<First>(arg1), ( std::forward<Next>(argsN), ... ) ); // @suppress("Syntax error")
        d.push_back_list( arg1, argsN... ); // @suppress("Syntax error")
        return d;
    }

    /**
     * Complement constructor for darray<T> instance, move semantics initializer for one argument.
     * @tparam First
     * @tparam Next
     * @param arg1
     * @return
     * @see darray::push_back()
     * @see darray::push_back_list()
     * @see make_darray()
     */
    template <typename First, typename... Next>
    constexpr darray< First > make_darray(First&& arg1)
    {
        darray< First > d(1);
        d.push_back( std::forward<First>(arg1) );
        return d;
    }

    /****************************************************************************************
     ****************************************************************************************/

    template<typename Value_type, typename Size_type, typename Alloc_type>
    std::ostream & operator << (std::ostream &out, const darray<Value_type, Size_type, Alloc_type> &c) {
        out << c.toString();
        return out;
    }

    /****************************************************************************************
     ****************************************************************************************/

    template<typename Value_type, typename Size_type, typename Alloc_type>
    inline bool operator==(const darray<Value_type, Size_type, Alloc_type>& rhs, const darray<Value_type, Size_type, Alloc_type>& lhs) {
        if( &rhs == &lhs ) {
            return true;
        }
        return (rhs.size() == lhs.size() && std::equal(rhs.cbegin(), rhs.cend(), lhs.cbegin()));
    }
    template<typename Value_type, typename Size_type, typename Alloc_type>
    inline bool operator!=(const darray<Value_type, Size_type, Alloc_type>& rhs, const darray<Value_type, Size_type, Alloc_type>& lhs) {
        return !(rhs==lhs);
    }

    template<typename Value_type, typename Size_type, typename Alloc_type>
    inline bool operator<(const darray<Value_type, Size_type, Alloc_type>& rhs, const darray<Value_type, Size_type, Alloc_type>& lhs)
    { return std::lexicographical_compare(rhs.cbegin(), rhs.cend(), lhs.cbegin(), lhs.cend()); }

    template<typename Value_type, typename Size_type, typename Alloc_type>
    inline bool operator>(const darray<Value_type, Size_type, Alloc_type>& rhs, const darray<Value_type, Size_type, Alloc_type>& lhs)
    { return lhs < rhs; }

    template<typename Value_type, typename Size_type, typename Alloc_type>
    inline bool operator<=(const darray<Value_type, Size_type, Alloc_type>& rhs, const darray<Value_type, Size_type, Alloc_type>& lhs)
    { return !(lhs < rhs); }

    template<typename Value_type, typename Size_type, typename Alloc_type>
    inline bool operator>=(const darray<Value_type, Size_type, Alloc_type>& rhs, const darray<Value_type, Size_type, Alloc_type>& lhs)
    { return !(rhs < lhs); }

    template<typename Value_type, typename Size_type, typename Alloc_type>
    inline void swap(darray<Value_type, Size_type, Alloc_type>& rhs, darray<Value_type, Size_type, Alloc_type>& lhs) noexcept
    { rhs.swap(lhs); }

    /****************************************************************************************
     ****************************************************************************************/

    /**
     * `template< class T > is_darray_type<T>::value` compile-time Type Trait,
     * determining whether the given template class is a - or has a darray type, e.g. jau::cow_darray,
     * jau::darray.
     */
    template< class, class = void >
    struct is_darray_type : std::false_type { };

    /**
     * `template< class T > is_darray_type<T>::value` compile-time Type Trait,
     * determining whether the given template class is a - or has a darray type, e.g. jau::cow_darray,
     * jau::darray.
     */
    template< class T >
    struct is_darray_type<T, std::void_t<typename T::darray_tag>> : std::true_type { };

    /**@}*/

} /* namespace jau */

#endif /* JAU_DYN_ARRAY_HPP_ */
