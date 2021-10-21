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

#ifndef JAU_OCTETS_HPP_
#define JAU_OCTETS_HPP_

#include <cstring>
#include <string>
#include <memory>
#include <cstdint>
#include <algorithm>

#include <mutex>
#include <atomic>

#include <jau/basic_types.hpp>
#include <jau/uuid.hpp>
#include <jau/eui48.hpp>

#include <jau/debug.hpp>

// #define TRACE_MEM 1
#ifdef TRACE_MEM
    #define TRACE_PRINT(...) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); fflush(stderr); }
#else
    #define TRACE_PRINT(...)
#endif

namespace jau {

    /**
     * Transient read only and endian aware octet data, i.e. non persistent passthrough, owned by caller.
     *
     * Endian byte order is passed at construction.
     *
     * Constructor and assignment operations are `noexcept`. In case invalid arguments are passed, abort() is being called.
     * This is a design choice based on reusing already existing underlying resources.
     */
    class TROOctets
    {
        private:
            /** Used memory size <= capacity, maybe zero. */
            nsize_t _size;
            /** Memory pointer, might be nullptr. Actual capacity known by owner, e.g. POctets. */
            uint8_t * _data;
            /** byte-order flag */
            bool _little_endian;

        protected:
            /**
             * Validates the given data_ and size_.
             *
             * Aborts if data_ is nullptr and size_ > 0.
             *
             * @param data_ a memory pointer
             * @param size_ claimed memory size
             */
            static inline void checkPtr(uint8_t *data_, nsize_t size_) noexcept {
                if( nullptr == data_ && 0 < size_ ) {
                    ABORT("TROOctets: nullptr with size %s > 0", std::to_string(size_).c_str());
                    abort(); // never reached
                }
            }

            static inline bool is_little_endian(const endian v) noexcept {
                switch(v) {
                    case endian::little: return true;
                    case endian::big: return false;
                    default: {
                        ABORT("TROOctets: endian choice must be little or big, given: %s", jau::to_string(v).c_str());
                        abort(); // never reached
                    }
                }
            }

            constexpr bool little_endian() const noexcept { return _little_endian; }

            constexpr uint8_t * data() noexcept { return _data; }

            /**
             * Internally sets the _size and _data fields after validation.
             *
             * Aborts if data_ is nullptr and size_ > 0
             * or byte_order not endian::little nor endian::big, see abort().
             *
             * @param data_ a memory pointer
             * @param size_ used memory size
             * @param byte_order endian::little or endian::big byte order, one may pass endian::native.
             */
            inline void setData(uint8_t *data_, nsize_t size_, const endian byte_order) noexcept {
                TRACE_PRINT("POctets setData: %zu bytes @ %p -> %zu bytes @ %p",
                        _size, _data, size_, data_);
                checkPtr(data_, size_);
                _size = size_;
                _data = data_;
                _little_endian = is_little_endian(byte_order);
            }
            constexpr void setSize(nsize_t s) noexcept { _size = s; }

        public:
            /**
             * Transient passthrough read-only memory, w/o ownership ..
             *
             * Aborts if source is nullptr and len > 0
             * or byte_order not endian::little nor endian::big, see abort().
             *
             * @param source a non nullptr memory, otherwise throws exception. Actual capacity known by owner.
             * @param len readable size of the memory, may be zero
             * @param byte_order endian::little or endian::big byte order, one may pass endian::native.
             */
            TROOctets(const uint8_t *source, const nsize_t len, const endian byte_order) noexcept
            : _size( len ), _data( const_cast<uint8_t *>(source) ),
              _little_endian( is_little_endian(byte_order) )
            {
                checkPtr(_data, _size);
            }

            TROOctets(const TROOctets &o) noexcept = default;
            TROOctets(TROOctets &&o) noexcept = default;
            TROOctets& operator=(const TROOctets &o) noexcept = default;
            TROOctets& operator=(TROOctets &&o) noexcept = default;

            virtual ~TROOctets() noexcept {}

            inline void check_range(const nsize_t i, const nsize_t count, const char *file, int line) const {
                if( i+count > _size ) {
                    throw IndexOutOfBoundsException(i, count, _size, file, line);
                }
            }
            #define check_range(I,C) check_range((I), (C), E_FILE_LINE)

            constexpr bool is_range_valid(const nsize_t i, const nsize_t count) const noexcept {
                return i+count <= _size;
            }

            /** Returns byte order of this octet store. */
            constexpr endian byte_order() const noexcept { return _little_endian ? endian::little : endian::big; }

            /** Returns the used memory size for read and write operations, may be zero. */
            constexpr nsize_t size() const noexcept { return _size; }

            uint8_t get_uint8(const nsize_t i) const {
                check_range(i, 1);
                return _data[i];
            }
            constexpr uint8_t get_uint8_nc(const nsize_t i) const noexcept {
                return _data[i];
            }

            int8_t get_int8(const nsize_t i) const {
                check_range(i, 1);
                return jau::get_int8(_data, i);
            }
            constexpr int8_t get_int8_nc(const nsize_t i) const noexcept {
                return jau::get_int8(_data, i);
            }

            uint16_t get_uint16(const nsize_t i) const {
                check_range(i, 2);
                return jau::get_uint16(_data, i, _little_endian);
            }
            constexpr uint16_t get_uint16_nc(const nsize_t i) const noexcept {
                return jau::get_uint16(_data, i, _little_endian);
            }

            uint32_t get_uint32(const nsize_t i) const {
                check_range(i, 4);
                return jau::get_uint32(_data, i, _little_endian);
            }
            constexpr uint32_t get_uint32_nc(const nsize_t i) const noexcept {
                return jau::get_uint32(_data, i, _little_endian);
            }

            EUI48 get_eui48(const nsize_t i) const {
                check_range(i, sizeof(EUI48));
                return EUI48(_data+i, byte_order());
            }
            inline EUI48 get_eui48_nc(const nsize_t i) const noexcept {
                return EUI48(_data+i, byte_order());
            }

            uint64_t get_uint64(const nsize_t i) const {
                check_range(i, 8);
                return jau::get_uint64(_data, i, _little_endian);
            }
            constexpr uint64_t get_uint64_nc(const nsize_t i) const noexcept {
                return jau::get_uint64(_data, i, _little_endian);
            }

            uint128_t get_uint128(const nsize_t i) const {
                check_range(i, 8);
                return jau::get_uint128(_data, i, _little_endian);
            }
            constexpr uint128_t get_uint128_nc(const nsize_t i) const noexcept {
                return jau::get_uint128(_data, i, _little_endian);
            }

            uint192_t get_uint192(const nsize_t i) const {
                check_range(i, 8);
                return jau::get_uint192(_data, i, _little_endian);
            }
            constexpr uint192_t get_uint192_nc(const nsize_t i) const noexcept {
                return jau::get_uint192(_data, i, _little_endian);
            }

            uint256_t get_uint256(const nsize_t i) const {
                check_range(i, 8);
                return jau::get_uint256(_data, i, _little_endian);
            }
            constexpr uint256_t get_uint256_nc(const nsize_t i) const noexcept {
                return jau::get_uint256(_data, i, _little_endian);
            }

            /** Assumes a null terminated string */
            std::string get_string(const nsize_t i) const {
                check_range(i, 1); // minimum size
                return std::string( (const char*)(_data+i) );
            }
            /** Assumes a null terminated string */
            constexpr_cxx20 std::string get_string_nc(const nsize_t i) const noexcept {
                return std::string( (const char*)(_data+i) );
            }

            /** Assumes a string with defined length, not necessarily null terminated */
            inline std::string get_string(const nsize_t i, const nsize_t length) const {
                check_range(i, length);
                return std::string( (const char*)(_data+i), length );
            }

            uuid16_t get_uuid16(const nsize_t i) const {
                return uuid16_t(get_uint16(i));
            }
            inline uuid16_t get_uuid16_nc(const nsize_t i) const noexcept {
                return uuid16_t(get_uint16_nc(i));
            }

            uuid128_t get_uuid128(const nsize_t i) const {
                check_range(i, uuid_t::number(uuid_t::TypeSize::UUID128_SZ));
                return jau::get_uuid128(_data, i, _little_endian);
            }
            inline uuid128_t get_uuid128_nc(const nsize_t i) const noexcept {
                return jau::get_uuid128(_data, i, _little_endian);
            }

            std::unique_ptr<const uuid_t> get_uuid(const nsize_t i, const uuid_t::TypeSize tsize) const {
                check_range(i, uuid_t::number(tsize));
                return uuid_t::create(tsize, _data, i, _little_endian);
            }

            constexpr uint8_t const * get_ptr() const noexcept { return _data; }
            uint8_t const * get_ptr(const nsize_t i) const {
                check_range(i, 1);
                return _data + i;
            }
            constexpr uint8_t const * get_ptr_nc(const nsize_t i) const noexcept {
                return _data + i;
            }

            bool operator==(const TROOctets& rhs) const noexcept {
                return _size == rhs._size && 0 == std::memcmp(_data, rhs._data, _size);
            }
            bool operator!=(const TROOctets& rhs) const noexcept {
                return !(*this == rhs);
            }

            std::string toString() const noexcept {
                return "size "+std::to_string(_size)+", ro: "+bytesHexString(_data, 0, _size, true /* lsbFirst */);
            }
    };

    /**
     * Transient endian aware octet data, i.e. non persistent passthrough, owned by caller.
     *
     * Endian byte order is passed at construction.
     *
     * Constructor and assignment operations are `noexcept`. In case invalid arguments are passed, abort() is being called.
     * This is a design choice based on reusing already existing underlying resources.
     */
    class TOctets : public TROOctets
    {
        public:
            /**
             * Transient passthrough r/w memory, w/o ownership ..
             *
             * Aborts if source is nullptr and len > 0
             * or byte_order not endian::little nor endian::big, see abort().
             *
             * @param source transient data source
             * @param len length of transient data source
             * @param byte_order endian::little or endian::big byte order, one may pass endian::native.
             */
            TOctets(uint8_t *source, const nsize_t len, const endian byte_order) noexcept
            : TROOctets(source, len, byte_order) {}

            TOctets(const TOctets &o) noexcept = default;
            TOctets(TOctets &&o) noexcept = default;
            TOctets& operator=(const TOctets &o) noexcept = default;
            TOctets& operator=(TOctets &&o) noexcept = default;

            virtual ~TOctets() noexcept override {}

            void put_int8(const nsize_t i, const int8_t v) {
                check_range(i, 1);
                data()[i] = static_cast<uint8_t>(v);
            }
            constexpr void put_int8_nc(const nsize_t i, const int8_t v) noexcept {
                data()[i] = static_cast<uint8_t>(v);
            }

            void put_uint8(const nsize_t i, const uint8_t v) {
                check_range(i, 1);
                data()[i] = v;
            }
            constexpr void put_uint8_nc(const nsize_t i, const uint8_t v) noexcept {
                data()[i] = v;
            }

            void put_uint16(const nsize_t i, const uint16_t v) {
                check_range(i, 2);
                jau::put_uint16(data(), i, v, little_endian());
            }
            constexpr void put_uint16_nc(const nsize_t i, const uint16_t v) noexcept {
                jau::put_uint16(data(), i, v, little_endian());
            }

            void put_uint32(const nsize_t i, const uint32_t v) {
                check_range(i, 4);
                jau::put_uint32(data(), i, v, little_endian());
            }
            constexpr void put_uint32_nc(const nsize_t i, const uint32_t v) noexcept {
                jau::put_uint32(data(), i, v, little_endian());
            }

            void put_eui48(const nsize_t i, const EUI48 & v) {
                check_range(i, sizeof(v.b));
                v.put(data(), i, byte_order());
            }
            inline void put_eui48_nc(const nsize_t i, const EUI48 & v) noexcept {
                v.put(data(), i, byte_order());
            }

            void put_uint64(const nsize_t i, const uint64_t & v) {
                check_range(i, 8);
                jau::put_uint64(data(), i, v, little_endian());
            }
            constexpr void put_uint64_nc(const nsize_t i, const uint64_t & v) noexcept {
                jau::put_uint64(data(), i, v, little_endian());
            }

            void put_uint128(const nsize_t i, const uint128_t & v) {
                check_range(i, 8);
                jau::put_uint128(data(), i, v, little_endian());
            }
            constexpr void put_uint128_nc(const nsize_t i, const uint128_t & v) noexcept {
                jau::put_uint128(data(), i, v, little_endian());
            }

            void put_uint192(const nsize_t i, const uint192_t & v) {
                check_range(i, 8);
                jau::put_uint192(data(), i, v, little_endian());
            }
            constexpr void put_uint192_nc(const nsize_t i, const uint192_t & v) noexcept {
                jau::put_uint192(data(), i, v, little_endian());
            }

            void put_uint256(const nsize_t i, const uint256_t & v) {
                check_range(i, 8);
                jau::put_uint256(data(), i, v, little_endian());
            }
            constexpr void put_uint256_nc(const nsize_t i, const uint256_t & v) noexcept {
                jau::put_uint256(data(), i, v, little_endian());
            }

            void put_octets(const nsize_t i, const TROOctets & v) {
                check_range(i, v.size());
                memcpy(data() + i, v.get_ptr(), v.size());
            }
            void put_octets_nc(const nsize_t i, const TROOctets & v) noexcept {
                memcpy(data() + i, v.get_ptr(), v.size());
            }

            void put_bytes(const nsize_t i, const uint8_t *source, const nsize_t byte_count) {
                check_range(i, byte_count);
                memcpy(data() + i, source, byte_count);
            }
            void put_bytes_nc(const nsize_t i, const uint8_t *source, const nsize_t byte_count) noexcept {
                memcpy(data() + i, source, byte_count);
            }

            void put_string(const nsize_t i, const std::string & v, const nsize_t max_len, const bool includeEOS) {
                const nsize_t size1 = v.size() + ( includeEOS ? 1 : 0 );
                const nsize_t size = std::min(size1, max_len);
                check_range(i, size);
                memcpy(data() + i, v.c_str(), size);
                if( size < size1 && includeEOS ) {
                    *(data() + i + size - 1) = 0; // ensure EOS
                }
            }
            void put_string_nc(const nsize_t i, const std::string & v, const nsize_t max_len, const bool includeEOS) noexcept {
                const nsize_t size1 = v.size() + ( includeEOS ? 1 : 0 );
                const nsize_t size = std::min(size1, max_len);
                memcpy(data() + i, v.c_str(), size);
                if( size < size1 && includeEOS ) {
                    *(data() + i + size - 1) = 0; // ensure EOS
                }
            }

            void put_uuid(const nsize_t i, const uuid_t & v) {
                check_range(i, v.getTypeSizeInt());
                v.put(data(), i, little_endian());
            }
            void put_uuid_nc(const nsize_t i, const uuid_t & v) noexcept {
                v.put(data(), i, little_endian());
            }

            inline uint8_t * get_wptr() noexcept { return data(); }

            uint8_t * get_wptr(const nsize_t i) {
                check_range(i, 1);
                return data() + i;
            }
            inline uint8_t * get_wptr_nc(const nsize_t i) noexcept {
                return data() + i;
            }

            std::string toString() const noexcept {
                return "size "+std::to_string(size())+", rw: "+bytesHexString(get_ptr(), 0, size(), true /* lsbFirst */);
            }
    };

    /**
     * Transient endian aware octet data slice, i.e. a view of an TOctet.
     *
     * Endian byte order is defined by its parent TOctet.
     */
    class TOctetSlice
    {
        private:
            const TOctets & _parent;
            nsize_t const _offset;
            nsize_t const _size;

        public:
            /**
             * Creates a view of a given TOctet with the specified offset_ and size_.
             *
             * @param buffer_ the parent TOctet buffer
             * @param offset_ offset to the parent TOctet buffer
             * @param size_ size of this view, starting at offset_
             * @throws IndexOutOfBoundsException if offset_ + size_ > parent buffer_.size()
             */
            TOctetSlice(const TOctets &buffer_, const nsize_t offset_, const nsize_t size_)
            : _parent(buffer_), _offset(offset_), _size(size_)
            {
                if( offset_+_size > buffer_.size() ) {
                    throw IndexOutOfBoundsException(offset_, _size, buffer_.size(), E_FILE_LINE);
                }
            }

            /** Returns byte order of this octet store. */
            constexpr endian byte_order() const noexcept { return _parent.byte_order(); }

            constexpr nsize_t size() const noexcept { return _size; }
            constexpr nsize_t offset() const noexcept { return _offset; }
            constexpr const TOctets& parent() const noexcept { return _parent; }

            uint8_t get_uint8(const nsize_t i) const {
                return _parent.get_uint8(_offset+i);
            }
            constexpr uint8_t get_uint8_nc(const nsize_t i) const noexcept {
                return _parent.get_uint8_nc(_offset+i);
            }

            uint16_t get_uint16(const nsize_t i) const {
                return _parent.get_uint16(_offset+i);
            }
            constexpr uint16_t get_uint16_nc(const nsize_t i) const noexcept {
                return _parent.get_uint16_nc(_offset+i);
            }

            uint8_t const * get_ptr(const nsize_t i) const {
                return _parent.get_ptr(_offset+i);
            }
            constexpr uint8_t const * get_ptr_nc(const nsize_t i) const noexcept {
                return _parent.get_ptr_nc(_offset+i);
            }

            std::string toString() const noexcept {
                return "offset "+std::to_string(_offset)+", size "+std::to_string(_size)+": "+bytesHexString(_parent.get_ptr(), _offset, _size, true /* lsbFirst */);
            }
    };

    /**
     * Persistent endian aware octet data, i.e. owned memory allocation.
     *
     * Endian byte order is passed at construction.
     *
     * Constructor and assignment operations are **not** completely `noexcept` and may throw exceptions.
     * This is a design choice based on dynamic resource allocation performed by this class.
     */
    class POctets : public TOctets
    {
        private:
            nsize_t _capacity;

            void freeData() {
                uint8_t * ptr = data();
                if( nullptr != ptr ) {
                    TRACE_PRINT("POctets release: %p", ptr);
                    free(ptr);
                } // else: zero sized POctets w/ nullptr are supported
            }

            /**
             * Allocate a memory chunk.
             * @param size
             * @return
             * @throws OutOfMemoryError if running out of memory
             */
            static uint8_t * allocData(const nsize_t size) {
                if( size <= 0 ) {
                    return nullptr;
                }
                uint8_t * m = static_cast<uint8_t*>( std::malloc(size) );
                if( nullptr == m ) {
                    throw OutOfMemoryError("allocData size "+std::to_string(size)+" -> nullptr", E_FILE_LINE);
                }
                return m;
            }

        public:
            /** Returns the memory capacity, never zero, greater or equal {@link #getSize()}. */
            constexpr nsize_t capacity() const noexcept { return _capacity; }

            /**
             * Zero sized POctets instance.
             *
             * Aborts if byte_order not endian::little nor endian::big, see abort().
             *
             * Will not throw an OutOfMemoryError exception due to no allocation.
             *
             * @param byte_order endian::little or endian::big byte order, one may pass endian::native.
             */
            POctets(const endian byte_order) noexcept
            : TOctets(nullptr, 0, byte_order), _capacity(0)
            {
                TRACE_PRINT("POctets ctor0: zero-sized");
            }

            /**
             * Takes ownership (malloc(size) and copy, free) ..
             *
             * Aborts if byte_order not endian::little nor endian::big, see abort().
             *
             * Capacity and size will be of given source size.
             *
             * @param source_ source data to be copied into this new instance
             * @param size_ length of source data
             * @param byte_order endian::little or endian::big byte order, one may pass endian::native.
             * @throws IllegalArgumentException if source_ is nullptr and size_ > 0
             * @throws OutOfMemoryError if allocation fails
             */
            POctets(const uint8_t *source_, const nsize_t size_, const endian byte_order)
            : TOctets( allocData(size_), size_, byte_order),
              _capacity( size_ )
            {
                if( 0 < size_ ) {
                    if( nullptr == source_ ) {
                        throw IllegalArgumentException("source nullptr with size "+std::to_string(size_)+" > 0", E_FILE_LINE);
                    }
                    std::memcpy(data(), source_, size_);
                }
                TRACE_PRINT("POctets ctor1: %p", data());
            }

            /**
             * New buffer (malloc(capacity), free)
             *
             * Aborts if byte_order not endian::little nor endian::big, see abort().
             *
             * @param capacity_ new capacity
             * @param size_ new size with size <= capacity
             * @param byte_order endian::little or endian::big byte order, one may pass endian::native.
             * @throws IllegalArgumentException if capacity_ < size_
             * @throws OutOfMemoryError if allocation fails
             */
            POctets(const nsize_t capacity_, const nsize_t size_, const endian byte_order)
            : TOctets( allocData( capacity_ ), size_, byte_order ),
              _capacity( capacity_ )
            {
                if( capacity() < size() ) {
                    throw IllegalArgumentException("capacity "+std::to_string(capacity())+" < size "+std::to_string(size()), E_FILE_LINE);
                }
                TRACE_PRINT("POctets ctor2: %p", data());
            }

            /**
             * New buffer (malloc, free)
             *
             * Aborts if byte_order not endian::little nor endian::big, see abort().
             *
             * @param size new size and capacity
             * @param byte_order endian::little or endian::big byte order, one may pass endian::native.
             * @throws OutOfMemoryError if allocation fails
             */
            POctets(const nsize_t size, const endian byte_order)
            : POctets(size, size, byte_order)
            {
                TRACE_PRINT("POctets ctor3: %p", data());
            }

            /**
             * Copy constructor
             * @param _source POctet source to be copied
             * @throws OutOfMemoryError if allocation fails
             */
            POctets(const POctets &_source)
            : TOctets( allocData(_source.size()), _source.size(), _source.byte_order() ),
              _capacity( _source.size() )
            {
                std::memcpy(data(), _source.get_ptr(), _source.size());
                TRACE_PRINT("POctets ctor-cpy0: %p", data());
            }

            /**
             * Move constructor
             * @param o POctet source to be taken over
             */
            POctets(POctets &&o) noexcept
            : TOctets( o.data(), o.size(), o.byte_order() ),
              _capacity( o.capacity() )
            {
                // moved origin data references
                // purge origin
                o.setData(nullptr, 0, o.byte_order());
                o._capacity = 0;
                TRACE_PRINT("POctets ctor-move0: %p", data());
            }

            /**
             * Assignment operator
             * @param _source POctet source to be copied
             * @return
             * @throws OutOfMemoryError if allocation fails
             */
            POctets& operator=(const POctets &_source) {
                if( this == &_source ) {
                    return *this;
                }
                freeData();
                setData(allocData(_source.size()), _source.size(), _source.byte_order());
                _capacity = _source.size();
                std::memcpy(data(), _source.get_ptr(), _source.size());
                TRACE_PRINT("POctets assign0: %p", data());
                return *this;
            }

            /**
             * Move assignment operator
             * @param o POctet source to be taken over
             * @return
             */
            POctets& operator=(POctets &&o) noexcept {
                // move origin data references
                setData(o.data(), o.size(), o.byte_order());
                _capacity = o._capacity;
                // purge origin
                o.setData(nullptr, 0, o.byte_order());
                o._capacity = 0;
                TRACE_PRINT("POctets assign-move0: %p", data());
                return *this;
            }

            virtual ~POctets() noexcept override {
                freeData();
                setData(nullptr, 0, byte_order());
                _capacity=0;
            }

            /**
             * Makes a persistent POctets by copying the data from TROOctets.
             * @param _source TROOctets to be copied
             * @throws OutOfMemoryError if allocation fails
             */
            POctets(const TROOctets & _source)
            : TOctets( allocData(_source.size()), _source.size(), _source.byte_order() ),
              _capacity( _source.size() )
            {
                std::memcpy(data(), _source.get_ptr(), _source.size());
                TRACE_PRINT("POctets ctor-cpy1: %p", data());
            }

            /**
             * Assignment operator for TROOctets
             * @param _source TROOctets to be copied
             * @return
             * @throws OutOfMemoryError if allocation fails
             */
            POctets& operator=(const TROOctets &_source) {
                if( static_cast<TROOctets *>(this) == &_source ) {
                    return *this;
                }
                freeData();
                setData(allocData(_source.size()), _source.size(), _source.byte_order());
                _capacity = _source.size();
                std::memcpy(data(), _source.get_ptr(), _source.size());
                TRACE_PRINT("POctets assign1: %p", data());
                return *this;
            }

            /**
             * Makes a persistent POctets by copying the data from TOctetSlice.
             * @param _source TROOctetSlice to be copied
             * @throws OutOfMemoryError if allocation fails
             */
            POctets(const TOctetSlice & _source)
            : TOctets( allocData(_source.size()), _source.size(), _source.byte_order() ),
              _capacity( _source.size() )
            {
                std::memcpy(data(), _source.parent().get_ptr() + _source.offset(), _source.size());
                TRACE_PRINT("POctets ctor-cpy2: %p", data());
            }

            /**
             * Assignment operator for TOctetSlice
             * @param _source TOctetSlice to be copied
             * @return
             * @throws OutOfMemoryError if allocation fails
             */
            POctets& operator=(const TOctetSlice &_source) {
                freeData();
                setData(allocData(_source.size()), _source.size(), _source.byte_order());
                _capacity = _source.size();
                std::memcpy(data(), _source.get_ptr(0), _source.size());
                TRACE_PRINT("POctets assign2: %p", data());
                return *this;
            }

            /**
             * Resizes this instance, including its capacity
             * @param newCapacity new capacity, must be >= newSize
             * @param newSize new size, must be < newCapacity
             * @return
             * @throws OutOfMemoryError if allocation fails
             * @throws IllegalArgumentException if newCapacity < newSize
             */
            POctets & resize(const nsize_t newCapacity, const nsize_t newSize) {
                if( newCapacity < newSize ) {
                    throw IllegalArgumentException("newCapacity "+std::to_string(newCapacity)+" < newSize "+std::to_string(newSize), E_FILE_LINE);
                }
                if( newCapacity != _capacity ) {
                    if( newSize > size() ) {
                        recapacity(newCapacity);
                        setSize(newSize);
                    } else {
                        setSize(newSize);
                        recapacity(newCapacity);
                    }
                } else {
                    setSize(newSize);
                }
                return *this;
            }

            /**
             * Sets a new size for this instance.
             * @param newSize new size, must be <= current capacity()
             * @return
             * @throws IllegalArgumentException if newSize > current capacity()
             */
            POctets & resize(const nsize_t newSize) {
                if( _capacity < newSize ) {
                    throw IllegalArgumentException("capacity "+std::to_string(_capacity)+" < newSize "+std::to_string(newSize), E_FILE_LINE);
                }
                setSize(newSize);
                return *this;
            }

            /**
             * Changes the capacity.
             *
             * @param newCapacity new capacity, must be >= size()
             * @return
             * @throws OutOfMemoryError if allocation fails
             * @throws IllegalArgumentException if newCapacity < size()
             */
            POctets & recapacity(const nsize_t newCapacity) {
                if( newCapacity < size() ) {
                    throw IllegalArgumentException("newCapacity "+std::to_string(newCapacity)+" < size "+std::to_string(size()), E_FILE_LINE);
                }
                if( newCapacity == _capacity ) {
                    return *this;
                }
                uint8_t* data2 = allocData(newCapacity);
                if( size() > 0 ) {
                    memcpy(data2, get_ptr(), size());
                }
                TRACE_PRINT("POctets recapacity: %p -> %p", data(), data2);
                free(data());
                setData(data2, size(), byte_order());
                _capacity = newCapacity;
                return *this;
            }

            /**
             * Append and assign operator
             * @param b
             * @return
             * @throws OutOfMemoryError if allocation due to potential recapacity() fails
             */
            POctets & operator+=(const TROOctets &b) {
                if( 0 < b.size() ) {
                    const nsize_t newSize = size() + b.size();
                    if( _capacity < newSize ) {
                        recapacity( newSize );
                    }
                    memcpy(data()+size(), b.get_ptr(), b.size());
                    setSize(newSize);
                }
                return *this;
            }
            /**
             * Append and assign operator
             * @param b
             * @return
             * @throws OutOfMemoryError if allocation due to potential recapacity() fails
             */
            POctets & operator+=(const TOctetSlice &b) {
                if( 0 < b.size() ) {
                    const nsize_t newSize = size() + b.size();
                    if( _capacity < newSize ) {
                        recapacity( newSize );
                    }
                    memcpy(data()+size(), b.parent().get_ptr()+b.offset(), b.size());
                    setSize(newSize);
                }
                return *this;
            }

            std::string toString() const {
                return "size "+std::to_string(size())+", capacity "+std::to_string(capacity())+", "+bytesHexString(get_ptr(), 0, size(), true /* lsbFirst */);
            }
    };

} /* namespace jau */


#endif /* JAU_OCTETS_HPP_ */
