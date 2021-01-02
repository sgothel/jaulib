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
     * <li><b>Removed</b>: Size_type x Value_type fill operations, e.g. assign, constructor, .... for clarity, since supporting <i>capacity</i>.</li>
     * <li>...</li>
     * <li><b>TODO</b>: emplace(..), list-initialization operations</li>
     * </ul>
     * </p>
     * <p>
     * Implementation differences to std::vector and some details
     * <ul>
     * <li>Using zero overhead <i>Value_type*</i> as iterator type.</li>
     * <li>...</li>
     * <li>Storage is operated on three iterator: <i>begin</i>, <i>end</i> and <i>storage_end</i>.</li>
     * <li>Constructs and destructs Value_type via <i>placement new</i> within the pre-allocated array capacity. Latter is managed via Alloc_type.</li>
     * </ul>
     * </p>
     */
    template <typename Value_type, typename Alloc_type = std::allocator<Value_type>, typename Size_type = jau::nsize_t>
    class darray
    {
        public:
            /** Default growth factor using the golden ratio 1.618 */
            inline static const float DEFAULT_GROWTH_FACTOR = 1.618f;

            typedef Value_type*                     iterator;
            typedef const Value_type*               const_iterator;

        private:
            inline static void freeStore(Alloc_type& alloc, Value_type *ptr, const Size_type size) {
                if( nullptr != ptr ) {
                    alloc.deallocate(ptr, size);
                }
            }

            inline static Value_type * allocStore(Alloc_type& alloc, const Size_type size) {
                if( 0 < size ) {
                    Value_type * m = alloc.allocate(size);
                    if( nullptr == m ) {
                        throw jau::OutOfMemoryError("allocData "+std::to_string(size)+" elements * "+
                                std::to_string(sizeof(Value_type))+" bytes/element = "+
                                std::to_string(size * sizeof(Value_type))+" bytes -> nullptr", E_FILE_LINE);
                    }
                    return m;
                }
                return nullptr;
            }

            typedef Value_type* array_ref;

            float growth_factor_;
            Alloc_type alloc_inst;
            array_ref begin_;
            array_ref end_;
            array_ref storage_end_;

            constexpr void set_iterator(array_ref new_storage_, Size_type size_, Size_type capacity_) noexcept {
                begin_       = new_storage_;
                end_         = new_storage_+size_;
                storage_end_ = new_storage_+capacity_;
            }

            constexpr void set_iterator(Size_type size_, Size_type capacity_) noexcept {
                end_         = begin_+size_;
                storage_end_ = begin_+capacity_;
            }

            Size_type dtor_range(array_ref first, const array_ref last) {
                Size_type count=0;
                for(; first < last; ++first, ++count ) {
                    ( first )->~Value_type(); // placement new -> manual destruction!
                }
                return count;
            }

            inline static void ctor_copy_range(array_ref dest, array_ref first, const array_ref last) {
                for(; first < last; ++dest, ++first) {
                    new (dest) Value_type( *first ); // placement new
                }
            }
            static array_ref clone_range(Alloc_type& alloc, const Size_type dest_capacity, const array_ref first, const array_ref last) {
                if( dest_capacity < Size_type(last-first) ) {
                    throw jau::IllegalArgumentException("capacity "+std::to_string(dest_capacity)+" < source range "+
                                                        std::to_string(Size_type(last-first)), E_FILE_LINE);
                }
                array_ref dest = allocStore(alloc, dest_capacity);
                ctor_copy_range(dest, first, last);
                return dest;
            }
            static array_ref clone_range(Alloc_type& alloc, const array_ref first, const array_ref last) {
                array_ref dest = allocStore(alloc, Size_type(last-first));
                ctor_copy_range(dest, first, last);
                return dest;
            }

            template< class InputIt >
            inline static void ctor_copy_range_foreign(array_ref dest, InputIt first, InputIt last) {
                for(; first < last; ++dest, ++first) {
                    new (dest) Value_type( *first ); // placement new
                }
            }
            template< class InputIt >
            static array_ref clone_range_foreign(Alloc_type& alloc, const Size_type dest_capacity, InputIt first, InputIt last) {
                if( dest_capacity < Size_type(last-first) ) {
                    throw jau::IllegalArgumentException("capacity "+std::to_string(dest_capacity)+" < source range "+
                                                        std::to_string(Size_type(last-first)), E_FILE_LINE);
                }
                array_ref dest = allocStore(alloc, dest_capacity);
                ctor_copy_range_foreign(dest, first, last);
                return dest;
            }

            inline static void ctor_move_range(array_ref dest, array_ref first, const array_ref last) {
                for(; first < last; ++dest, ++first) {
                    new (dest) Value_type( std::move( *first ) ); // placement new
                }
            }
            void grow_storage_move(const Size_type new_capacity) {
                array_ref dest = allocStore(alloc_inst, new_capacity);
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
             * @param alloc given Alloc_type
             */
            constexpr explicit darray(Size_type capacity, const float growth_factor=DEFAULT_GROWTH_FACTOR, const Alloc_type& alloc = Alloc_type())
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
             * @param alloc custom Alloc_type instance
             */
            constexpr explicit darray(const darray& x, const float growth_factor, const Alloc_type& alloc)
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
             * @param alloc custom Alloc_type instance
             */
            constexpr explicit darray(const darray& x, const Size_type _capacity, const float growth_factor, const Alloc_type& alloc)
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

            constexpr explicit darray(darray && x, const float growth_factor, const Alloc_type& alloc) noexcept
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
             * copying all elements from the given const_iterator Value_type range [first, last).<br>
             * Size will equal the range [first, last), i.e. <code>Size_type(last-first)</code>.
             * <p>
             * Throws jau::IllegalArgumentException() if <code>_capacity < Size_type(last - first)</code>.
             * </p>
             * @param _capacity custom initial storage capacity
             * @param first const_iterator to first element of Value_type range [first, last)
             * @param last const_iterator to last element of Value_type range [first, last)
             * @param growth_factor custom growth factor
             * @param alloc custom Alloc_type instance
             */
            constexpr darray(const Size_type _capacity, const_iterator first, const_iterator last,
                             const float growth_factor=DEFAULT_GROWTH_FACTOR, const Alloc_type& alloc = Alloc_type())
            : growth_factor_( growth_factor ), alloc_inst( alloc ), begin_( clone_range(alloc_inst, _capacity, first, last) ),
              end_(begin_ + Size_type(last - first) ), storage_end_( begin_ + _capacity )
            { }

            /**
             * Creates a new instance with custom initial storage capacity,
             * copying all elements from the given template input-iterator Value_type range [first, last).<br>
             * Size will equal the range [first, last), i.e. <code>Size_type(last-first)</code>.
             * <p>
             * Throws jau::IllegalArgumentException() if <code>_capacity < Size_type(last - first)</code>.
             * </p>
             * @tparam InputIt template input-iterator custom type
             * @param _capacity custom initial storage capacity
             * @param first template input-iterator to first element of Value_type range [first, last)
             * @param last template input-iterator to last element of Value_type range [first, last)
             * @param growth_factor custom growth factor
             * @param alloc custom Alloc_type instance
             */
            template< class InputIt >
            constexpr explicit darray(const Size_type _capacity, InputIt first, InputIt last,
                                      const float growth_factor=DEFAULT_GROWTH_FACTOR, const Alloc_type& alloc = Alloc_type())
            : growth_factor_( growth_factor ), alloc_inst( alloc ), begin_( clone_range_foreign(alloc_inst, _capacity, first, last) ),
              end_(begin_ + Size_type(last - first) ), storage_end_( begin_ + _capacity )
            { }

            ~darray() noexcept {
                clear();
            }

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

            const Alloc_type& get_allocator_ref() const noexcept {
                return alloc_inst;
            }

            Alloc_type get_allocator() const noexcept {
                return Alloc_type(alloc_inst);
            }

            constexpr float growth_factor() const noexcept {
                return growth_factor_;
            }

            constexpr Size_type capacity() const noexcept { return Size_type(storage_end_ - begin_); }

            constexpr Size_type grow_number(const Size_type old_number ) const noexcept {
                const Size_type res = static_cast<Size_type>(old_number * growth_factor_ + 0.5f); // simple round up
                return std::max<Size_type>(old_number+1, res);
            }
            constexpr Size_type grow_capacity() const noexcept {
                const Size_type old_number = capacity();
                const Size_type res = static_cast<Size_type>(old_number * growth_factor_ + 0.5f); // simple round up
                return std::max<Size_type>(old_number+1, res);
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
            constexpr Size_type size() const noexcept { return Size_type(end_ - begin_); }

            // mixed mutable/immutable element access

            /**
             * Like std::vector::front(), mutable access.
             */
            constexpr Value_type & front() { return *begin_; }

            /**
             * Like std::vector::front(), immutable access.
             */
            constexpr const Value_type & front() const { return *begin_; }

            /**
             * Like std::vector::back(), mutable access.
             */
            constexpr Value_type & back() { return *(end_-1); }

            /**
             * Like std::vector::back(), immutable access.
             */
            constexpr const Value_type & back() const { return *(end_-1); }

            /**
             * Like std::vector::data(), const immutable pointer
             */
            constexpr const Value_type* data() const noexcept { return begin_; }

            /**
             * Like std::vector::data(), mutable pointer
             */
            constexpr Value_type* data() noexcept { return begin_; }

            /**
             * Like std::vector::operator[](Size_type), immutable reference.
             */
            const Value_type & operator[](Size_type i) const noexcept {
                return *(begin_+i);
            }

            /**
             * Like std::vector::operator[](Size_type), mutable reference.
             */
            Value_type & operator[](Size_type i) noexcept {
                return *(begin_+i);
            }

            /**
             * Like std::vector::at(Size_type), immutable reference.
             */
            const Value_type & at(Size_type i) const {
                if( 0 <= i && i < size() ) {
                    return *(begin_+i);
                }
                throw jau::IndexOutOfBoundsException(i, size(), E_FILE_LINE);
            }

            /**
             * Like std::vector::at(Size_type), mutable reference.
             */
            Value_type & at(Size_type i) {
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
            void reserve(Size_type new_capacity) {
                const Size_type capacity_ = capacity();
                if( new_capacity > capacity_ ) {
                    array_ref new_storage = allocStore(alloc_inst, new_capacity);
                    ctor_move_range(new_storage, begin_, end_);

                    freeStore(alloc_inst, begin_, capacity_);

                    set_iterator(new_storage, size(), new_capacity);
                }
            }

            /**
             * Like std::vector::operator=(&), assignment
             */
            darray& operator=(const darray& x) {
                const Size_type size_ = size();
                const Size_type capacity_ = capacity();
                const Size_type x_size_ = x.size();
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
             * @tparam InputIt foreign input-iterator to range of Value_type [first, last)
             * @param first first foreign input-iterator to range of Value_type [first, last)
             * @param last last foreign input-iterator to range of Value_type [first, last)
             */
            template< class InputIt >
            constexpr void assign( InputIt first, InputIt last ) {
                const Size_type size_ = size();
                const Size_type capacity_ = capacity();
                const Size_type x_size_ = Size_type(last - first);
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
             * @param first first const_iterator to range of Value_type [first, last)
             * @param last last const_iterator to range of Value_type [first, last)
             */
            constexpr void assign( const_iterator first, const_iterator last ) {
                const Size_type size_ = size();
                const Size_type capacity_ = capacity();
                const Size_type x_size_ = Size_type(last - first);
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
            darray& operator=(darray&& x) {
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
            void clear() noexcept {
                dtor_range(begin_, end_);
                freeStore(alloc_inst, begin_, capacity());
                begin_ = nullptr;
                end_ = nullptr;
                storage_end_ = nullptr;
            }

            /**
             * Like std::vector::swap().
             */
            void swap(darray& x) noexcept {
                std::swap(growth_factor_, x.growth_factor_);
                std::swap(alloc_inst, x.alloc_inst);
                std::swap(begin_, x.begin_);
                std::swap(end_, x.end_);
                std::swap(storage_end_, x.storage_end_);
            }

            /**
             * Like std::vector::pop_back().
             */
            void pop_back() noexcept {
                if( begin_ != end_ ) {
                    ( --end_ )->~Value_type(); // placement new -> manual destruction!
                }
            }

            /**
             * Like std::vector::push_back(), copy
             * @param x the value to be added at the tail.
             */
            void push_back(const Value_type& x) {
                if( end_ == storage_end_ ) {
                    grow_storage_move(grow_capacity());
                }
                new (end_) Value_type( x ); // placement new
                ++end_;
            }

            /**
             * Like std::vector::push_back(), move
             */
            void push_back(Value_type&& x) {
                if( end_ == storage_end_ ) {
                    grow_storage_move(grow_capacity());
                }
                new (end_) Value_type( std::move(x) ); // placement new
                ++end_;
            }

            /**
             * Like std::vector::push_back(), but appends the whole Value_type range [first, last).
             * @tparam InputIt foreign input-iterator to range of Value_type [first, last)
             * @param first first foreign input-iterator to range of Value_type [first, last)
             * @param last last foreign input-iterator to range of Value_type [first, last)
             */
            template< class InputIt >
            constexpr void push_back( InputIt first, InputIt last ) {
                const Size_type count = Size_type(last - first);

                if( end_ + count >= storage_end_ ) {
                    grow_storage_move(size() + count);
                }
                ctor_copy_range_foreign(end_, first, last);
                end_ += count;
            }

            /**
             * Like std::vector::push_back(), but appends the whole Value_type range [first, last).
             * @param first first const_iterator to range of Value_type [first, last)
             * @param last last const_iterator to range of Value_type [first, last)
             */
            constexpr void push_back( const_iterator first, const_iterator last ) {
                const Size_type count = Size_type(last - first);

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
            void erase(const Size_type i) {
                const Size_type size_ = size();
                if( 0 <= i && i < size_ ) {
                    ( begin_ + i )->~Value_type(); // placement new -> manual destruction!

                    const Size_type right_count = size_ - 1 - i;
                    if( 0 < right_count ) {
                        memmove(begin_+i, begin_+i+1, sizeof(Value_type)*right_count); // move right elems one left
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
                    ( pos )->~Value_type(); // placement new -> manual destruction!
                    const Size_type right_count = ( end_ - pos ) - 1;
                    if( 0 < right_count ) {
                        memmove(pos, pos+1, sizeof(Value_type)*right_count); // move right elems one left
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
                const Size_type count = dtor_range(first, last);
                if( count > 0 ) {
                    const Size_type right_count = ( end_ - last ) - 1;
                    if( 0 < right_count ) {
                        memmove(first, last, sizeof(Value_type)*right_count); // move right elems one left
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
            void insert(Size_type i, const Value_type& x) {
                const Size_type size_ = size();
                if( 0 <= i && i <= size_ ) {
                    const Size_type old_capacity_ = capacity();
                    if( size_ + 1 > old_capacity_ ) {
                        grow_storage_move(grow_number(old_capacity_));
                    }
                    const Size_type right_count = size_ - i;
                    if( 0 < right_count ) {
                        memmove(begin_+i+1, begin_+i, sizeof(Value_type)*right_count); // move right elems one right
                    }
                    new (begin_+i) Value_type( x ); // placement new
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
            void insert(Size_type i, Value_type&& x) {
                const Size_type size_ = size();
                if( 0 <= i && i <= size_ ) {
                    const Size_type old_capacity_ = capacity();
                    if( size_ + 1 > old_capacity_ ) {
                        grow_storage_move(grow_number(old_capacity_));
                    }
                    const Size_type right_count = size_ - i;
                    if( 0 < right_count ) {
                        memmove(begin_+i+1, begin_+i, sizeof(Value_type)*right_count); // move right elems one right
                    }
                    new (begin_+i) Value_type( std::move(x) ); // placement new
                    ++end_;
                } else {
                    throw jau::IndexOutOfBoundsException(i, size_, E_FILE_LINE);
                }
            }

            /**
             * Generic Value_type equal comparator to be user defined for e.g. jau::darray::push_back_unique().
             * @param a one element of the equality test.
             * @param b the other element of the equality test.
             * @return true if both are equal
             */
            typedef bool(*equal_comparator)(const Value_type& a, const Value_type& b);

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
            bool push_back_unique(const Value_type& x, equal_comparator comparator) {
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
            int erase_matching(const Value_type& x, const bool all_matching, equal_comparator comparator) {
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
