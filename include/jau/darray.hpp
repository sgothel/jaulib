/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
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

#include <cstring>
#include <string>
#include <cstdint>
#include <limits>
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <algorithm>

#include <jau/debug.hpp>
#include <jau/basic_types.hpp>
#include <jau/ordered_atomic.hpp>


namespace jau {

    /**
     * Implementation of a dynamic linear array storage, aka vector.<br>
     * Goals are to support a high-performance CoW dynamic array implementation, jau::cow_darray,<br>
     * exposing fine grained control over its underlying storage facility.<br>
     * Further, jau::darray provides high-performance and efficient storage properties on its own.
     * <p>
     * API and design differences to std::vector
     * <ul>
     * <li>jau::darray adds a parameterized <i>growth factor</i> aspect, default to golden ration jau::darray::DEFAULT_GROWTH_FACTOR</li>
     * <li><i>capacity</i> control via constructor and operations, related to <i>growth factor</i>.</li>
     * <li>Iterator jau::darray::const_iterator .. are harmonized with jau::cow_ro_iterator .. used in jau:cow_darray.</li>
     * <li>...</li>
     * <li>Custom constructor and operations, supporting a more efficient jau::cow_darray implementation.</li>
     * <li>Custom template typename Size_type, defaults to jau::nsize_t.</li>
     * <li>...</li>
     * <li><b>Removed</b>: size_type x value_type fill operations, e.g. assign, constructor, .... for clarity, since supporting <i>capacity</i>.</li>
     * <li>...</li>
     * <li><b>TODO</b>: emplace(..), list-initialization operations</li>
     * </ul>
     * </p>
     * <p>
     * Implementation differences to std::vector and some details
     * <ul>
     * <li>Using zero overhead <i>value_type*</i> as iterator type.</li>
     * <li>...</li>
     * <li>Storage is operated on three iterator: <i>begin</i>, <i>end</i> and <i>storage_end</i>.</li>
     * <li>Constructs and destructs value_type via <i>placement new</i> within the pre-allocated array capacity. Latter is managed via allocator_type.</li>
     * </ul>
     * </p>
     */
    template <typename Value_type, typename Alloc_type = std::allocator<Value_type>, typename Size_type = jau::nsize_t>
    class darray
    {
        public:
            /** Default growth factor using the golden ratio 1.618 */
            constexpr static const float DEFAULT_GROWTH_FACTOR = 1.618f;

            // std container conform typedefs'

            typedef Value_type                                  value_type;
            typedef value_type*                                 pointer;
            typedef const value_type*                           const_pointer;
            typedef value_type&                                 reference;
            typedef const value_type&                           const_reference;
            typedef value_type*                                 iterator;
            typedef const value_type*                           const_iterator;
            typedef Size_type                                   size_type;
            typedef typename std::make_signed<size_type>::type  difference_type;
            // typedef std::reverse_iterator<iterator>          reverse_iterator;
            // typedef std::reverse_iterator<const_iterator>    const_reverse_iterator;
            typedef Alloc_type                                  allocator_type;

        private:
            static constexpr size_type DIFF_MAX = std::numeric_limits<difference_type>::max();

            constexpr static void freeStore(allocator_type& alloc, value_type *ptr, const size_type size_) {
                if( nullptr != ptr ) {
                    alloc.deallocate(ptr, size_);
                }
            }

            /**
             * Allocates a new store using allocator_type.
             * <p>
             * Throws jau::IllegalArgumentException() if size_ > <code>std::numeric_limits<difference_type>::max()</code>, i.e. difference_type maximum.
             * </p>
             * <p>
             * Throws jau::OutOfMemoryError() if allocator_type::allocate() returns nullptr.
             * </p>
             * @param alloc the allocator_type instance
             * @param size_ the element count, must be <= <code>std::numeric_limits<difference_type>::max()</code>
             * @return nullptr if given <code>0 == size_</code> or the newly allocated memory
             */
            constexpr static value_type * allocStore(allocator_type& alloc, const size_type size_) {
                if( 0 != size_ ) {
                    if( size_ > DIFF_MAX ) {
                        throw jau::IllegalArgumentException("allocData "+std::to_string(size_)+" > difference_type max "+
                                std::to_string(DIFF_MAX), E_FILE_LINE);
                    }
                    value_type * m = alloc.allocate(size_);
                    if( nullptr == m ) {
                        throw jau::OutOfMemoryError("alloc "+std::to_string(size_)+" elements * "+
                                std::to_string(sizeof(value_type))+" bytes/element = "+
                                std::to_string(size_ * sizeof(value_type))+" bytes -> nullptr", E_FILE_LINE);
                    }
                    return m;
                }
                return nullptr;
            }

            float growth_factor_;
            allocator_type alloc_inst;
            pointer begin_;
            pointer end_;
            pointer storage_end_;

            constexpr void set_iterator(pointer new_storage_, difference_type size_, difference_type capacity_) noexcept {
                begin_       = new_storage_;
                end_         = new_storage_+size_;
                storage_end_ = new_storage_+capacity_;
            }

            constexpr void set_iterator(difference_type size_, difference_type capacity_) noexcept {
                end_         = begin_+size_;
                storage_end_ = begin_+capacity_;
            }

            constexpr size_type dtor_range(iterator first, const_iterator last) {
                size_type count=0;
                for(; first < last; ++first, ++count ) {
                    ( first )->~value_type(); // placement new -> manual destruction!
                }
                return count;
            }

            constexpr static void ctor_copy_range(pointer dest, iterator first, const_iterator last) {
                for(; first < last; ++dest, ++first) {
                    new (dest) value_type( *first ); // placement new
                }
            }
            constexpr static pointer clone_range(allocator_type& alloc, const_iterator first, const_iterator last) {
                pointer dest = allocStore(alloc, size_type(last-first));
                ctor_copy_range(dest, first, last);
                return dest;
            }
            constexpr static pointer clone_range(allocator_type& alloc, const size_type dest_capacity, iterator first, const_iterator last) {
                if( first > last ) {
                    throw jau::IllegalArgumentException("first "+aptrHexString(first)+" > last "+aptrHexString(last), E_FILE_LINE);
                }
                if( dest_capacity < size_type(last-first) ) {
                    throw jau::IllegalArgumentException("capacity "+std::to_string(dest_capacity)+" < source range "+
                                                        std::to_string(difference_type(last-first)), E_FILE_LINE);
                }
                pointer dest = allocStore(alloc, dest_capacity);
                ctor_copy_range(dest, first, last);
                return dest;
            }

            template< class InputIt >
            constexpr static void ctor_copy_range_foreign(pointer dest, InputIt first, InputIt last) {
                for(; first < last; ++dest, ++first) {
                    new (dest) value_type( *first ); // placement new
                }
            }
            template< class InputIt >
            constexpr static pointer clone_range_foreign(allocator_type& alloc, const size_type dest_capacity, InputIt first, InputIt last) {
                if( first > last ) {
                    throw jau::IllegalArgumentException("first "+std::to_string(first)+" > last "+
                                                        std::to_string(last), E_FILE_LINE);
                }
                if( dest_capacity < size_type(last-first) ) {
                    throw jau::IllegalArgumentException("capacity "+std::to_string(dest_capacity)+" < source range "+
                                                        std::to_string(difference_type(last-first)), E_FILE_LINE);
                }
                pointer dest = allocStore(alloc, dest_capacity);
                ctor_copy_range_foreign(dest, first, last);
                return dest;
            }

            constexpr static void ctor_move_range(pointer dest, iterator first, const_iterator last) {
                for(; first < last; ++dest, ++first) {
                    new (dest) value_type( std::move( *first ) ); // placement new
                }
            }
            constexpr void grow_storage_move(const size_type new_capacity) {
                pointer dest = allocStore(alloc_inst, new_capacity);
                ctor_move_range(dest, begin_, end_);

                freeStore(alloc_inst, begin_, capacity());

                set_iterator(dest, size(), new_capacity);
            }

        public:

            // ctor w/o elements

            /**
             * Default constructor, giving zero capacity and zero memory footprint.
             */
            constexpr darray() noexcept
            : growth_factor_(DEFAULT_GROWTH_FACTOR), alloc_inst(),
              begin_( nullptr ), end_( nullptr ), storage_end_( nullptr ) {}

            /**
             * Creating an empty instance with initial capacity and other (default) properties.
             * @param capacity initial capacity of the new instance.
             * @param growth_factor given growth factor
             * @param alloc given allocator_type
             */
            constexpr explicit darray(size_type capacity, const float growth_factor=DEFAULT_GROWTH_FACTOR, const allocator_type& alloc = allocator_type())
            : growth_factor_( growth_factor ), alloc_inst( alloc ), begin_( allocStore(alloc_inst, capacity) ),
              end_( begin_ ), storage_end_( begin_ + capacity ) {}

            // copy_ctor on darray elements

            /**
             * Creates a new instance, copying all elements from the given darray.<br>
             * Capacity and size will equal the given array, i.e. the result is a trimmed jau::darray.
             * @param x the given darray, all elements will be copied into the new instance.
             */
            constexpr darray(const darray& x)
            : growth_factor_( x.growth_factor_ ), alloc_inst( x.alloc_inst ), begin_( clone_range(alloc_inst, x.begin_, x.end_) ),
              end_( begin_ + x.size() ), storage_end_( begin_ + x.size() )
            { }

            /**
             * Creates a new instance, copying all elements from the given darray.<br>
             * Capacity and size will equal the given array, i.e. the result is a trimmed jau::darray.
             * @param x the given darray, all elements will be copied into the new instance.
             * @param growth_factor custom growth factor
             * @param alloc custom allocator_type instance
             */
            constexpr explicit darray(const darray& x, const float growth_factor, const allocator_type& alloc)
            : growth_factor_( growth_factor ), alloc_inst( alloc ), begin_( clone_range(alloc_inst, x.begin_, x.end_) ),
              end_( begin_ + x.size() ), storage_end_( begin_ + x.size() )
            { }

            /**
             * Creates a new instance with custom initial storage capacity, copying all elements from the given darray.<br>
             * Size will equal the given array.
             * <p>
             * Throws jau::IllegalArgumentException() if <code>_capacity < x.size()</code>.
             * </p>
             * @param x the given darray, all elements will be copied into the new instance.
             * @param _capacity custom initial storage capacity
             * @param growth_factor custom growth factor
             * @param alloc custom allocator_type instance
             */
            constexpr explicit darray(const darray& x, const size_type _capacity, const float growth_factor, const allocator_type& alloc)
            : growth_factor_( growth_factor ), alloc_inst( alloc ), begin_( clone_range(alloc_inst, _capacity, x.begin_, x.end_) ),
              end_( begin_ + x.size() ), storage_end_( begin_ + _capacity )
            { }

            // move_ctor on darray elements

            constexpr darray(darray && x) noexcept
            : growth_factor_(std::move(x.growth_factor_)), alloc_inst(std::move(x.alloc_inst)),
              begin_(std::move(x.begin_)), end_(std::move(x.end_)), storage_end_(std::move(x.storage_end_))
            {
                // complete swapping store_ref
                x.begin_ = nullptr;
                x.end_ = nullptr;
                x.storage_end_ = nullptr;
            }

            constexpr explicit darray(darray && x, const float growth_factor, const allocator_type& alloc) noexcept
            : growth_factor_(std::move(x.growth_factor_)), alloc_inst(alloc),
              begin_(std::move(x.begin_)), end_(std::move(x.end_)), storage_end_(std::move(x.storage_end_))
            {
                // complete swapping store_ref
                x.begin_ = nullptr;
                x.end_ = nullptr;
                x.storage_end_ = nullptr;
            }

            // ctor on const_iterator and foreign template iterator

            /**
             * Creates a new instance with custom initial storage capacity,
             * copying all elements from the given const_iterator value_type range [first, last).<br>
             * Size will equal the range [first, last), i.e. <code>size_type(last-first)</code>.
             * <p>
             * Throws jau::IllegalArgumentException() if <code>_capacity < size_type(last - first)</code>.
             * </p>
             * @param _capacity custom initial storage capacity
             * @param first const_iterator to first element of value_type range [first, last)
             * @param last const_iterator to last element of value_type range [first, last)
             * @param growth_factor custom growth factor
             * @param alloc custom allocator_type instance
             */
            constexpr darray(const size_type _capacity, const_iterator first, const_iterator last,
                             const float growth_factor=DEFAULT_GROWTH_FACTOR, const allocator_type& alloc = allocator_type())
            : growth_factor_( growth_factor ), alloc_inst( alloc ), begin_( clone_range(alloc_inst, _capacity, first, last) ),
              end_(begin_ + size_type(last - first) ), storage_end_( begin_ + _capacity )
            { }

            /**
             * Creates a new instance with custom initial storage capacity,
             * copying all elements from the given template input-iterator value_type range [first, last).<br>
             * Size will equal the range [first, last), i.e. <code>size_type(last-first)</code>.
             * <p>
             * Throws jau::IllegalArgumentException() if <code>_capacity < size_type(last - first)</code>.
             * </p>
             * @tparam InputIt template input-iterator custom type
             * @param _capacity custom initial storage capacity
             * @param first template input-iterator to first element of value_type range [first, last)
             * @param last template input-iterator to last element of value_type range [first, last)
             * @param growth_factor custom growth factor
             * @param alloc custom allocator_type instance
             */
            template< class InputIt >
            constexpr explicit darray(const size_type _capacity, InputIt first, InputIt last,
                                      const float growth_factor=DEFAULT_GROWTH_FACTOR, const allocator_type& alloc = allocator_type())
            : growth_factor_( growth_factor ), alloc_inst( alloc ), begin_( clone_range_foreign(alloc_inst, _capacity, first, last) ),
              end_(begin_ + size_type(last - first) ), storage_end_( begin_ + _capacity )
            { }

            ~darray() noexcept {
                clear();
            }

            /**
             * Returns <code>std::numeric_limits<difference_type>::max()</code> as the maximum array size.
             * <p>
             * We rely on the signed <code>difference_type</code> for pointer arithmetic,
             * deducing ranges from iterator.
             * </p>
             */
            constexpr size_type max_size() const noexcept { return DIFF_MAX; }

            // iterator

            constexpr iterator begin() noexcept { return begin_; }

            constexpr const_iterator begin() const noexcept { return begin_; }

            constexpr const_iterator cbegin() const noexcept { return begin_; }

            constexpr iterator end() noexcept { return end_; }

            constexpr const_iterator end() const noexcept { return end_; }

            constexpr const_iterator cend() const noexcept { return end_; }

#if 0
            constexpr iterator storage_end() noexcept { return storage_end_; }

            constexpr const_iterator storage_end() const noexcept { return storage_end_; }

            constexpr const_iterator cstorage_end() const noexcept { return storage_end_; }
#endif

            // read access

            const allocator_type& get_allocator_ref() const noexcept {
                return alloc_inst;
            }

            allocator_type get_allocator() const noexcept {
                return allocator_type(alloc_inst);
            }

            constexpr float growth_factor() const noexcept {
                return growth_factor_;
            }

            constexpr size_type capacity() const noexcept { return size_type(storage_end_ - begin_); }

            constexpr size_type grow_number(const size_type old_number ) const noexcept {
                const size_type res = static_cast<size_type>(old_number * growth_factor_ + 0.5f); // simple round up
                return std::max<size_type>(old_number+1, res);
            }
            constexpr size_type grow_capacity() const noexcept {
                const size_type old_number = capacity();
                const size_type res = static_cast<size_type>(old_number * growth_factor_ + 0.5f); // simple round up
                return std::max<size_type>(old_number+1, res);
            }

            /**
             * Like std::vector::empty().
             */
            constexpr bool empty() const noexcept { return begin_ == end_; }

            /**
             * Returns true if capacity has been reached and the next push_back()
             * will grow the storage and invalidates all iterators and references.
             */
            constexpr bool capacity_reached() const noexcept { return end_ >= storage_end_; }

            /**
             * Like std::vector::size().
             */
            constexpr size_type size() const noexcept { return size_type(end_ - begin_); }

            // mixed mutable/immutable element access

            /**
             * Like std::vector::front(), mutable access.
             */
            constexpr reference front() { return *begin_; }

            /**
             * Like std::vector::front(), immutable access.
             */
            constexpr const_reference front() const { return *begin_; }

            /**
             * Like std::vector::back(), mutable access.
             */
            constexpr reference back() { return *(end_-1); }

            /**
             * Like std::vector::back(), immutable access.
             */
            constexpr const_reference back() const { return *(end_-1); }

            /**
             * Like std::vector::data(), const immutable pointer
             */
            constexpr const_pointer data() const noexcept { return begin_; }

            /**
             * Like std::vector::data(), mutable pointer
             */
            constexpr pointer data() noexcept { return begin_; }

            /**
             * Like std::vector::operator[](size_type), immutable reference.
             */
            const_reference operator[](size_type i) const noexcept {
                return *(begin_+i);
            }

            /**
             * Like std::vector::operator[](size_type), mutable reference.
             */
            reference operator[](size_type i) noexcept {
                return *(begin_+i);
            }

            /**
             * Like std::vector::at(size_type), immutable reference.
             */
            const_reference at(size_type i) const {
                if( 0 <= i && i < size() ) {
                    return *(begin_+i);
                }
                throw jau::IndexOutOfBoundsException(i, size(), E_FILE_LINE);
            }

            /**
             * Like std::vector::at(size_type), mutable reference.
             */
            reference at(size_type i) {
                if( 0 <= i && i < size() ) {
                    return *(begin_+i);
                }
                throw jau::IndexOutOfBoundsException(i, size(), E_FILE_LINE);
            }

            // write access, mutable array operations

            /**
             * Like std::vector::reserve(), increases this instance's capacity to <code>new_capacity</code>.
             * <p>
             * Only creates a new storage and invalidates iterators if <code>new_capacity</code>
             * is greater than the current jau::darray::capacity().
             * </p>
             */
            void reserve(size_type new_capacity) {
                const size_type capacity_ = capacity();
                if( new_capacity > capacity_ ) {
                    pointer new_storage = allocStore(alloc_inst, new_capacity);
                    ctor_move_range(new_storage, begin_, end_);

                    freeStore(alloc_inst, begin_, capacity_);

                    set_iterator(new_storage, size(), new_capacity);
                }
            }

            /**
             * Like std::vector::operator=(&), assignment
             */
            constexpr darray& operator=(const darray& x) {
                const size_type size_ = size();
                const size_type capacity_ = capacity();
                const size_type x_size_ = x.size();
                dtor_range(begin_, end_);
                if( x_size_ > capacity_ ) {
                    freeStore(alloc_inst, begin_, capacity_);
                    begin_ =  clone_range(alloc_inst, x_size_, x.begin_, x.end_);
                    set_iterator(x_size_, x_size_);
                } else {
                    ctor_copy_range(begin_, x.begin_, x.end_);
                    set_iterator(x_size_, capacity_);
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
            constexpr void assign( InputIt first, InputIt last ) {
                const size_type size_ = size();
                const size_type capacity_ = capacity();
                const size_type x_size_ = size_type(last - first);
                dtor_range(begin_, end_);
                if( x_size_ > capacity_ ) {
                    freeStore(alloc_inst, begin_, capacity_);
                    begin_ =  clone_range_foreign(alloc_inst, x_size_, first, last);
                    set_iterator(x_size_, x_size_);
                } else {
                    ctor_copy_range_foreign(begin_, first, last);
                    set_iterator(x_size_, capacity_);
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
                dtor_range(begin_, end_);
                if( x_size_ > capacity_ ) {
                    freeStore(alloc_inst, begin_, capacity_);
                    begin_ =  clone_range(alloc_inst, x_size_, first, last);
                    set_iterator(x_size_, x_size_);
                } else {
                    ctor_copy_range(begin_, first, last);
                    set_iterator(x_size_, capacity_);
                }
            }

            /**
             * Like std::vector::operator=(&&), move.
             */
            constexpr darray& operator=(darray&& x) noexcept {
                clear();
                alloc_inst = std::move(x.alloc_inst);
                begin_ = std::move(x.begin_);
                end_ = std::move(x.end_);
                storage_end_ = std::move(x.storage_end_);

                // complete swapping store_ref
                x.begin_ = nullptr;
                x.end_ = nullptr;
                x.storage_end_ = nullptr;

                return *this;
            }

            /**
             * Like std::vector::clear(), but ending with zero capacity.
             */
            constexpr void clear() noexcept {
                dtor_range(begin_, end_);
                freeStore(alloc_inst, begin_, capacity());
                begin_ = nullptr;
                end_ = nullptr;
                storage_end_ = nullptr;
            }

            /**
             * Like std::vector::swap().
             */
            constexpr void swap(darray& x) noexcept {
                std::swap(growth_factor_, x.growth_factor_);
                std::swap(alloc_inst, x.alloc_inst);
                std::swap(begin_, x.begin_);
                std::swap(end_, x.end_);
                std::swap(storage_end_, x.storage_end_);
            }

            /**
             * Like std::vector::pop_back().
             */
            constexpr void pop_back() noexcept {
                if( begin_ != end_ ) {
                    ( --end_ )->~value_type(); // placement new -> manual destruction!
                }
            }

            /**
             * Like std::vector::push_back(), copy
             * @param x the value to be added at the tail.
             */
            constexpr void push_back(const value_type& x) {
                if( end_ == storage_end_ ) {
                    grow_storage_move(grow_capacity());
                }
                new (end_) value_type( x ); // placement new
                ++end_;
            }

            /**
             * Like std::vector::push_back(), move
             */
            constexpr void push_back(value_type&& x) {
                if( end_ == storage_end_ ) {
                    grow_storage_move(grow_capacity());
                }
                new (end_) value_type( std::move(x) ); // placement new
                ++end_;
            }

            /**
             * Like std::vector::push_back(), but appends the whole value_type range [first, last).
             * @tparam InputIt foreign input-iterator to range of value_type [first, last)
             * @param first first foreign input-iterator to range of value_type [first, last)
             * @param last last foreign input-iterator to range of value_type [first, last)
             */
            template< class InputIt >
            constexpr void push_back( InputIt first, InputIt last ) {
                const size_type count = size_type(last - first);

                if( end_ + count >= storage_end_ ) {
                    grow_storage_move(size() + count);
                }
                ctor_copy_range_foreign(end_, first, last);
                end_ += count;
            }

            /**
             * Like std::vector::push_back(), but appends the whole value_type range [first, last).
             * @param first first const_iterator to range of value_type [first, last)
             * @param last last const_iterator to range of value_type [first, last)
             */
            constexpr void push_back( const_iterator first, const_iterator last ) {
                const size_type count = size_type(last - first);

                if( end_ + count >= storage_end_ ) {
                    grow_storage_move(size() + count);
                }
                ctor_copy_range(end_, first, last);
                end_ += count;
            }

            /**
             * Like std::vector::erase().
             * <p>
             * Removes the element at the given position
             * and moves all subsequent elements one left.
             * </p>
             * <p>
             * size will be reduced by one.
             * </p>
             * @param i the position of the element to be removed
             */
            constexpr void erase(const size_type i) {
                const size_type size_ = size();
                if( 0 <= i && i < size_ ) {
                    ( begin_ + i )->~value_type(); // placement new -> manual destruction!

                    const difference_type right_count = size_ - 1 - i;
                    if( 0 < right_count ) {
                        memmove(begin_+i, begin_+i+1, sizeof(value_type)*right_count); // move right elems one left
                    }
                    --end_;
                } else {
                    throw jau::IndexOutOfBoundsException(i, size_, E_FILE_LINE);
                }
            }

            /**
             * Like std::vector::erase(), removes the elements at pos.
             * @return iterator following the last removed element.
             */
            constexpr iterator erase (const_iterator pos) {
                if( begin_ <= pos && pos < end_ ) {
                    ( pos )->~value_type(); // placement new -> manual destruction!
                    const difference_type right_count = ( end_ - pos ) - 1;
                    if( 0 < right_count ) {
                        memmove(pos, pos+1, sizeof(value_type)*right_count); // move right elems one left
                    }
                    --end_;
                }
                return begin_ <= pos && pos <= end_ ? pos : nullptr;
            }

            /**
             * Like std::vector::erase(), removes the elements in the range [first, last).
             * @return iterator following the last removed element.
             */
            constexpr iterator erase (const_iterator first, const_iterator last) {
                const size_type count = dtor_range(first, last);
                if( count > 0 ) {
                    const difference_type right_count = end_ - last;  // last is exclusive
                    if( 0 < right_count ) {
                        memmove(first, last, sizeof(value_type)*right_count); // move right elems one left
                    }
                    end_ -= count;
                }
                return begin_ <= last && last <= end_ ? last : nullptr;
            }

            /**
             * Like std::vector::insert(), copy
             * <p>
             * Inserts the element at the given position
             * and moves all elements from there to the right beforehand.
             * </p>
             * <p>
             * size will be increased by one.
             * </p>
             * @param i the position of the element to be removed
             */
            constexpr void insert(size_type i, const value_type& x) {
                const size_type size_ = size();
                if( 0 <= i && i <= size_ ) {
                    const size_type old_capacity_ = capacity();
                    if( size_ + 1 > old_capacity_ ) {
                        grow_storage_move(grow_number(old_capacity_));
                    }
                    const difference_type right_count = size_ - i;
                    if( 0 < right_count ) {
                        memmove(begin_+i+1, begin_+i, sizeof(value_type)*right_count); // move right elems one right
                    }
                    new (begin_+i) value_type( x ); // placement new
                    ++end_;
                } else {
                    throw jau::IndexOutOfBoundsException(i, size_, E_FILE_LINE);
                }
            }

            /**
             * Like std::vector::insert(), move
             * <p>
             * Inserts the element at the given position
             * and moves all elements from there to the right beforehand.
             * </p>
             * <p>
             * size will be increased by one.
             * </p>
             * @param i the position of the element to be removed
             */
            void insert(size_type i, value_type&& x) {
                const size_type size_ = size();
                if( 0 <= i && i <= size_ ) {
                    const size_type old_capacity_ = capacity();
                    if( size_ + 1 > old_capacity_ ) {
                        grow_storage_move(grow_number(old_capacity_));
                    }
                    const difference_type right_count = size_ - i;
                    if( 0 < right_count ) {
                        memmove(begin_+i+1, begin_+i, sizeof(value_type)*right_count); // move right elems one right
                    }
                    new (begin_+i) value_type( std::move(x) ); // placement new
                    ++end_;
                } else {
                    throw jau::IndexOutOfBoundsException(i, size_, E_FILE_LINE);
                }
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
             */
            bool push_back_unique(const value_type& x, equal_comparator comparator) {
                for(auto it = begin_; it != end_; ) {
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
             * Erase either the first matching element or all matching elements.
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
             */
            int erase_matching(const value_type& x, const bool all_matching, equal_comparator comparator) {
                int count = 0;
                for(auto it = end_-1; begin_ <= it; --it) {
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
    };
} /* namespace jau */

#endif /* JAU_DYN_ARRAY_HPP_ */
