/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022-2025 Gothel Software e.K.
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

#ifndef JAU_MEMORY_BUFFERS_HPP_
#define JAU_MEMORY_BUFFERS_HPP_

#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>

#include "jau/cpp_lang_util.hpp"
#include "jau/int_types.hpp"
#include "jau/float_types.hpp"
#include <jau/basic_types.hpp>
#include <jau/darray.hpp>
#include <jau/type_traits_queries.hpp>

#include "jau/math/vec2f.hpp"
#include "jau/math/vec3f.hpp"
#include "jau/math/vec4f.hpp"

namespace jau {
    /**
     * Abstract memory stream buffer container with absolute and relative read/write operations,
     * allowing the following procedures:
     * - using std::unique_ptr instances only
     * - supports slicing or cloning a subset using same underlying buffer and custom position/size
     * - using jau::DataBuffer<T> implementation for primitive or complex types
     *   - using a shared_ptr jau::darray<T> to support slicing
     *   - supports relative and absolute single and bulk get/put operations (read/write)
     * - supports polymorphic relative getPri<T> / putPri<T...> operations for primitives on jau::DataBuffer<T>
     *   - putPri template allows types <= storage type
     *   - getPri template allows types >= storage type
     *
     * Relative read/write operations follow:
     * `0 <= mark <= position <= limit <= capacity`
     *
     * @see jau::DataBuffer
     */
    class MemBuffer {
      public:
        typedef size_t size_type;
        typedef MemBuffer base_t;
        typedef std::unique_ptr<base_t> base_ref;

      private:
        template<typename BufferValue_type, typename T>
        static T get1Pri_impl(MemBuffer& b);

        template<typename BufferValue_type, typename... Targs,
            std::enable_if_t< jau::is_all_same_v<Targs...>, bool> = true>
        static void putPri_impl(MemBuffer& b, const Targs&...args);

      protected:
        size_type m_elemSize;
        std::optional<size_type> m_mark;
        size_type m_position, m_limit, m_capacity;
        size_type m_offset;

        struct Private{ explicit Private() = default; };

      public:
        /** Private ctor */
        MemBuffer(Private, size_type elemSize_, std::optional<size_type> mark_, size_type position_, size_type limit_, size_type capacity_, size_t offset_)
        : m_elemSize(elemSize_), m_mark(mark_), m_position(position_), m_limit(limit_), m_capacity(capacity_), m_offset(offset_) { }

        virtual ~MemBuffer() noexcept = default;

        /** Returns type signature of implementing class's stored value type. */
        virtual const jau::type_info& valueSignature() const noexcept = 0;

        /** Returns type signature of implementing class. */
        virtual const jau::type_info& classSignature() const noexcept = 0;

        /** Returns element's size in bytes */
        constexpr jau::nsize_t elementSize() const noexcept { return m_elemSize; }

        /** Buffer capacity of elements, with limit <= capacity. */
        constexpr size_type capacity() const noexcept { return m_capacity; }

        /** Next relative read/write element index, with 0 <= position <= limit.*/
        constexpr size_type position() const noexcept { return m_position; }
        /** Sets position and invalidates mark if > position. Throws exception if new position is out of bounds. */
        base_t& setPosition(size_type v) {
            if(v > m_limit) {
                throw jau::IndexOutOfBoundsError(std::to_string(v), std::to_string(m_limit), E_FILE_LINE);
            }
            if(m_mark > v) { m_mark = std::nullopt; }
            m_position = v;
            return *this;
        }

        /** Buffer read write limit, one element beyond maximum index with limit <= capacity.  */
        constexpr size_type limit() const noexcept { return m_limit; }
        /** Sets new limit and adjusts position and mark if new limit is below. Throws exception is new limit is > capacity. */
        base_t& setLimit(size_type v) {
            if(v > m_capacity) {
                throw jau::IndexOutOfBoundsError(std::to_string(v), std::to_string(m_capacity), E_FILE_LINE);
            }
            m_limit = v;
            if (m_position > v) { m_position = v; }
            if (m_mark > v) { m_mark = std::nullopt; }
            return *this;
        }

        constexpr std::optional<size_type> getMark() const noexcept { return m_mark; }
        /** Sets mark to position */
        constexpr base_t& mark() noexcept {
            m_mark = m_position;
            return *this;
        }

        /** Sets position to mark. Throws exception if mark is invalid. */
        base_t& reset() {
            if( !m_mark ) {
                throw jau::IllegalStateError("mark not set", E_FILE_LINE);
            }
            setPosition(m_mark.value());
            return *this;
        }

        /** Sets position to zero, limit to capacity, invalidates mark and leaves elements and capacity (storage) untouched. */
        constexpr base_t& clear() noexcept {
            m_position = 0;
            m_limit = m_capacity;
            m_mark = std::nullopt;
            return *this;
        }

        /** Sets limit to position, position to zero and invalidates mark. */
        constexpr base_t& flip() noexcept {
            m_limit = m_position;
            m_position = 0;
            m_mark = std::nullopt;
            return *this;
        }

        /** Sets position to zero and invalidates mark. */
        constexpr base_t& rewind() noexcept {
            m_position = 0;
            m_mark = std::nullopt;
            return *this;
        }

        /** Returns limit - position. */
        constexpr size_type remaining() const noexcept { return m_limit - m_position; }
        /** Returns whether position < limit, i.e. has remaining elements. */
        constexpr bool hasRemaining() const noexcept { return m_position < m_limit; }

        virtual base_ref slice() = 0;

        virtual base_ref slice(size_type idx, size_type length) = 0;

        virtual base_ref clone() = 0;

        /**
         * Resize to new limit.
         *
         * Sets new limit and adjusts position and mark if new limit is below. Grows storage if new limit is > capacity.
         *
         * Must not be done on sliced, cloned or their parent buffer.
         *
         * Like std::vector::resize(size_type, const value_type&)
         */
        virtual base_t& resize(size_type new_limit) = 0;

        /**
         * Like std::vector::shrink_to_fit(), but ensured `constexpr`.
         *
         * Must not be done on sliced, cloned or their parent buffer.
         *
         * If capacity() > limit(), reallocate storage to limit().
         */
        virtual base_t& shrink_to_fit() = 0;

        std::string toString() const noexcept {
            std::string r("MemBuffer[[");
            r.append(valueSignature().name())
             .append(", ").append(std::to_string(m_elemSize))
             .append(" bytes], off ").append(std::to_string(m_offset))
             .append(", pos ").append(std::to_string(m_position))
             .append(", lim ").append(std::to_string(m_limit))
             .append(", cap ").append(std::to_string(m_capacity))
             .append("]");
            return r;
        }

        template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool> = true>
        T getPri();

        template<typename... Targs,
            std::enable_if_t< jau::is_all_same_v<Targs...> &&
                              ( std::is_integral_v<jau::first_type<Targs...>> ||
                                std::is_floating_point_v<jau::first_type<Targs...>> ), bool> = true>
        MemBuffer& putPri(const Targs&...args);

        inline MemBuffer& put2f(const jau::math::Vec2f& v) {
            return putPri(v.x, v.y);
        }
        inline MemBuffer& put3f(const jau::math::Vec3f& v) {
            return putPri(v.x, v.y, v.z);
        }
        inline MemBuffer& put4f(const jau::math::Vec4f& v) {
            return putPri(v.x, v.y, v.z, v.w);
        }
    };
    inline std::ostream& operator<<(std::ostream& out, const MemBuffer& v) {
        return out << v.toString();
    }

    /**
     * Memory stream buffer container implementation for jau::MemBuffer with absolute and relative read/write operations.
     *
     * @see jau::MemBuffer
     */
    template <typename Value_type>
    class DataBuffer : public MemBuffer {
      public:
        typedef Value_type value_type;
        typedef jau::darray<value_type, size_type, jau::callocator<value_type>> storage_t;
        typedef std::shared_ptr<storage_t> storage_ref;
        typedef DataBuffer<value_type> self_t;
        typedef std::unique_ptr<self_t> self_ref;

        typedef storage_t::iterator iterator;
        typedef storage_t::const_iterator const_iterator;
        typedef storage_t::reference reference;
        typedef storage_t::const_reference const_reference;

      protected:
        storage_ref m_storage;

        struct Private{ explicit Private() = default; };

      public:
        /** Private ctor, slicing */
        DataBuffer(Private, storage_ref store_, std::optional<size_type> mark_, size_type position_, size_type limit_, size_type capacity_, size_t offset_)
        : base_t(typename base_t::Private(), sizeof(value_type), mark_, position_, limit_, capacity_, offset_), m_storage(std::move(store_)) {
            if( m_capacity + m_offset > m_storage->capacity() ) {
                throw jau::IndexOutOfBoundsError(std::to_string(m_capacity + m_offset), std::to_string(m_storage->capacity()), E_FILE_LINE);
            }
        }

        /** Private ctor, create */
        DataBuffer(Private, std::optional<size_type> mark_, size_type position_, size_type limit_, size_type capacity_)
        : base_t(typename base_t::Private(), sizeof(value_type), mark_, position_, limit_, capacity_, 0), m_storage(std::make_shared<storage_t>(capacity_)) { m_storage->resize(capacity_); }

        const jau::type_info& valueSignature() const noexcept override {
            return jau::static_ctti<Value_type>();
        }
        const jau::type_info& classSignature() const noexcept override {
            return jau::static_ctti<self_t>();
        }

        /** Creates a new instance with given properties. */
        static self_ref create(std::optional<size_type> mark_, size_type position_, size_type limit_, size_type capacity_) {
            return std::make_unique<self_t>(Private(), mark_, position_, limit_, capacity_);
        }

        /** Creates a new instance with given size used for capacity and limit. */
        static self_ref create(size_type size_) {
            return std::make_unique<self_t>(Private(), std::nullopt, 0, size_, size_);
        }

        base_ref slice() override {
            const size_type lim = (m_position <= m_limit ? m_limit - m_position : 0);
            return std::make_unique<self_t>(Private(), m_storage, std::nullopt /* mark */, 0 /* pos */, lim /* limit */, lim /* capacity */, m_position /* offset */);
        }

        base_ref slice(size_type idx, size_type length) override {
            if(m_position + idx + length > m_limit) {
                throw jau::IndexOutOfBoundsError(std::to_string(m_position), std::to_string(m_limit), E_FILE_LINE);
            }
            return std::make_unique<self_t>(Private(), m_storage, std::nullopt /* mark */, 0 /* pos */, length /* limit */, length /* capacity */, idx /* offset */);
        }

        base_ref clone() override {
            return std::make_unique<self_t>(Private(), m_storage, m_mark /* mark */, m_position /* pos */, m_limit /* limit */, m_capacity /* capacity */, 0 /* offset */);
        }

        /**
         * Resize to new limit.
         *
         * Sets new limit and adjusts position and mark if new limit is below. Grows storage if new limit is > capacity.
         *
         * Must not be done on sliced, cloned or their parent buffer.
         *
         * Like std::vector::resize(size_type, const value_type&)
         */
        constexpr base_t& resize(size_type new_limit, const value_type& val) {
            m_storage->resize(m_offset + new_limit, val);
            if( new_limit > m_capacity ) {
                m_capacity = new_limit;
            }
            base_t::setLimit(new_limit);
            return *this;
        }

        base_t& resize(size_type new_size) override { return resize(new_size, value_type()); }

        base_t& shrink_to_fit() override {
            if( m_capacity > m_limit ) {
                m_storage->resize(m_offset + m_limit);
                m_storage->shrink_to_fit();
            }
            return *this;
        }

        constexpr iterator begin() noexcept { return m_storage->begin() + m_offset; }
        constexpr const_iterator begin() const noexcept { return m_storage->begin() + m_offset; }
        constexpr const_iterator cbegin() const noexcept { return m_storage->cbegin() + m_offset; }
        constexpr iterator end() noexcept { return begin() + m_limit; }
        constexpr const_iterator end() const noexcept { return begin() + m_limit; }
        constexpr const_iterator cend() const noexcept { return cbegin() + m_limit; }

        /**
         * Like std::vector::data(), const immutable pointer
         */
        constexpr const value_type* data() const noexcept { return m_storage->data() + m_offset; }

        /**
         * Like std::vector::data(), mutable pointer
         */
        constexpr value_type* data() noexcept { return m_storage->data() + m_offset; }

        /**
         * Like std::vector::operator[](size_type), immutable reference.
         */
        constexpr_cxx20 const_reference operator[](size_type i) const noexcept {
            return *(begin()+i);
        }

        /**
         * Like std::vector::operator[](size_type), mutable reference.
         */
        constexpr_cxx20 reference operator[](size_type i) noexcept {
            return *(begin()+i);
        }

        /**
         * Like std::vector::at(size_type), immutable reference.
         */
        constexpr_cxx20 const_reference at(size_type i) const {
            if( i + 1 > m_limit ) {
                throw jau::IndexOutOfBoundsError(i, m_limit, E_FILE_LINE);
            }
            return *(begin()+i);
        }

        /**
         * Like std::vector::at(size_type), mutable reference.
         */
        constexpr_cxx20 reference at(size_type i) {
            if( i + 1 > m_limit ) {
                throw jau::IndexOutOfBoundsError(i, m_limit, E_FILE_LINE);
            }
            return *(begin()+i);
        }

        /**
         * Relative get operation from current position and increments it.
         *
         * @return true if successful, i.e. current position < limit, otherwise false.
         */
        const value_type& get() {
            if(m_position + 1 > m_limit) {
                throw jau::IndexOutOfBoundsError(std::to_string(m_position), std::to_string(m_limit), E_FILE_LINE);
            }
            return (*m_storage)[m_offset + m_position++];
        }

        /**
         * Relative bulk get operation from current position and increments it about length.
         *
         * @return true if successful, i.e. current position + length - 1 < limit, otherwise false.
         */
        void get(value_type* dst, size_type length) {
            if(m_position + length > m_limit) {
                throw jau::IndexOutOfBoundsError(std::to_string(m_position), std::to_string(m_limit), E_FILE_LINE);
            }
            typename storage_t::pointer b = m_storage->data() + m_offset + m_position;
            typename storage_t::const_pointer e = b + length;
            if( storage_t::uses_memmove ) {
                ::memcpy(reinterpret_cast<void*>(dst),
                         reinterpret_cast<void*>(b),
                         (uint8_t*)e-(uint8_t*)b); // we can simply copy the memory over, also no overlap
            } else {
                while( b < e ) {
                    *dst++ = *b++;
                }
            }
            m_position += length;
        }

        /**
         * Relative put operation on current position and increments it.
         *
         * @return true if successful, i.e. current position < limit, otherwise false.
         */
        self_t& put(value_type o) {
            if(m_position + 1 > m_limit) {
                throw jau::IndexOutOfBoundsError(std::to_string(m_position), std::to_string(m_limit), E_FILE_LINE);
            }
            (*m_storage)[m_offset + m_position++] = o;
            return *this;
        }

        /**
         * Relative bulk put operation on current position and increments it about length.
         *
         * @return true if successful, i.e. current position + length - 1 < limit, otherwise false.
         */
        self_t& put(const value_type* src, size_type length) {
            if(m_position + length > m_limit) {
                throw jau::IndexOutOfBoundsError(std::to_string(m_position), std::to_string(m_limit), E_FILE_LINE);
            }
            typename storage_t::pointer b = m_storage->data() + m_offset + m_position;
            typename storage_t::pointer e = b + length;
            if( storage_t::uses_memmove ) {
                ::memcpy(reinterpret_cast<void*>(b),
                         reinterpret_cast<void*>(src),
                         (uint8_t*)e-(uint8_t*)b); // we can simply copy the memory over, also no overlap
            } else {
                while( b < e ) {
                    *b++ = *src++;
                }
            }
            m_position += length;
            return *this;
        }
    };

    //
    // Template Implementation
    //

    template<typename BufferValue_type, typename T>
    T MemBuffer::get1Pri_impl(MemBuffer& b) {
        typedef DataBuffer<BufferValue_type> dbuffer_t;
        if( b.classSignature() != jau::static_ctti<dbuffer_t>() ) {
            throw jau::IllegalArgumentError("Buffer `"+b.toString()+"` of class '"+b.classSignature().name()+
                                            "' can't be downcast to `"+jau::static_ctti<dbuffer_t>().name()+"`", E_FILE_LINE);
        }
        dbuffer_t& d = *reinterpret_cast<DataBuffer<BufferValue_type>*>(&b);
        return static_cast<T>( d.get() );
    }

    template<typename BufferValue_type, typename... Targs,
        std::enable_if_t< jau::is_all_same_v<Targs...>, bool>>
    void MemBuffer::putPri_impl(MemBuffer& b, const Targs&...args) {
        typedef DataBuffer<BufferValue_type> dbuffer_t;
        if( b.classSignature() != jau::static_ctti<dbuffer_t>() ) {
            throw jau::IllegalArgumentError("Buffer `"+b.toString()+"` of class '"+b.classSignature().name()+
                                            "' can't be downcast to `"+jau::static_ctti<dbuffer_t>().name()+"`", E_FILE_LINE);
        }
        if(b.m_position + sizeof...(args) > b.m_limit) {
            throw jau::IndexOutOfBoundsError(std::to_string(b.m_position), std::to_string(b.m_limit), E_FILE_LINE);
        }
        dbuffer_t& d = *reinterpret_cast<DataBuffer<BufferValue_type>*>(&b);
        typename dbuffer_t::iterator it = d.begin() + b.m_position;
        ( (*it++ = static_cast<BufferValue_type>(args)), ... ); // NOLINT(bugprone-signed-char-misuse)
        b.m_position += sizeof...(args);
    }

    template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool>>
    T MemBuffer::getPri() {
        if( m_elemSize > sizeof(T) ) {
            throw jau::IllegalArgumentError("Buffer `"+toString()+"` incompatible with type `"+jau::static_ctti<T>().name()+"`", E_FILE_LINE);
        }
        const jau::type_info& t = valueSignature();
        if( t == jau::int_ctti::i8() ) {
            return get1Pri_impl<int8_t, T>(*this);
        } else if( t == jau::int_ctti::u8() ) {
            return get1Pri_impl<uint8_t, T>(*this);
        } else if( t == jau::int_ctti::i16() ) {
            return get1Pri_impl<int16_t, T>(*this);
        } else if( t == jau::int_ctti::u16() ) {
            return get1Pri_impl<uint16_t, T>(*this);
        } else if( t == jau::int_ctti::i32() ) {
            return get1Pri_impl<int32_t, T>(*this);
        } else if( t == jau::int_ctti::u32() ) {
            return get1Pri_impl<uint32_t, T>(*this);
        } else if( t == jau::int_ctti::i64() ) {
            return get1Pri_impl<int64_t, T>(*this);
        } else if( t == jau::int_ctti::u64() ) {
            return get1Pri_impl<uint64_t, T>(*this);
        } else if( t == jau::float_ctti::f32() ) {
            return get1Pri_impl<jau::float32_t, T>(*this);
        } else if( t == jau::float_ctti::f64() ) {
            return get1Pri_impl<jau::float64_t, T>(*this);
        } else {
            throw jau::IllegalArgumentError("Buffer `"+toString()+"`, not supporting storing type `"+jau::static_ctti<T>().name()+"`", E_FILE_LINE);
        }
    }

    template<typename... Targs,
        std::enable_if_t< jau::is_all_same_v<Targs...> &&
                          ( std::is_integral_v<jau::first_type<Targs...>> ||
                            std::is_floating_point_v<jau::first_type<Targs...>> ), bool>>
    MemBuffer& MemBuffer::putPri(const Targs&...args) {
        using T0 = jau::first_type<Targs...>;
        if( m_elemSize < sizeof(T0) ) {
            throw jau::IllegalArgumentError("Buffer `"+toString()+"` incompatible with type `"+jau::static_ctti<T0>().name()+"`", E_FILE_LINE);
        }
        const jau::type_info& t = valueSignature();
        if( t == jau::int_ctti::i8() ) {
            putPri_impl<int8_t>(*this, args...);
        } else if( t == jau::int_ctti::u8() ) {
            putPri_impl<uint8_t>(*this, args...);
        } else if( t == jau::int_ctti::i16() ) {
            putPri_impl<int16_t>(*this, args...);
        } else if( t == jau::int_ctti::u16() ) {
            putPri_impl<uint16_t>(*this, args...);
        } else if( t == jau::int_ctti::i32() ) {
            putPri_impl<int32_t>(*this, args...);
        } else if( t == jau::int_ctti::u32() ) {
            putPri_impl<uint32_t>(*this, args...);
        } else if( t == jau::int_ctti::i64() ) {
            putPri_impl<int64_t>(*this, args...);
        } else if( t == jau::int_ctti::u64() ) {
            putPri_impl<uint64_t>(*this, args...);
        } else if( t == jau::float_ctti::f32() ) {
            putPri_impl<jau::float32_t>(*this, args...);
        } else if( t == jau::float_ctti::f64() ) {
            putPri_impl<jau::float64_t>(*this, args...);
        } else {
            throw jau::IllegalArgumentError("Buffer `"+toString()+"`, not supporting storing type `"+jau::static_ctti<T0>().name()+"`", E_FILE_LINE);
        }
        return *this;
    }
}

#endif /* JAU_MEMORY_BUFFERS_HPP_ */
