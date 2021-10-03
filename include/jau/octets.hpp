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
            static inline void checkPtr(uint8_t *d, nsize_t s) {
                if( nullptr == d && 0 < s ) {
                    throw IllegalArgumentException("TROOctets: nullptr with size "+std::to_string(s)+" > 0", E_FILE_LINE);
                }
            }
            static inline bool is_little_endian(const endian v, const bool throw_exception) {
                switch(v) {
                    case endian::little: return true;
                    case endian::big: return false;
                    default: {
                        if( throw_exception ) {
                            throw IllegalArgumentException("TROOctets: endian choice must be little or big, given: "+jau::to_string(v), E_FILE_LINE);
                        } else {
                            ABORT("TROOctets: endian choice must be little or big, given: %s", jau::to_string(v).c_str());
                            abort(); // never reached
                        }
                    }
                }
            }

            constexpr bool little_endian() const noexcept { return _little_endian; }

            constexpr uint8_t * data() noexcept { return _data; }

            /**
             * @param d a non nullptr memory, otherwise throws exception
             * @param s used memory size, may be zero
             * @param byte_order endian::little or endian::big byte order
             */
            inline void setData(uint8_t *d, nsize_t s, const endian byte_order) {
                TRACE_PRINT("POctets setData: %d bytes @ %p -> %d bytes @ %p",
                        _size, _data, s, d);
                checkPtr(d, s);
                _size = s;
                _data = d;
                _little_endian = is_little_endian(byte_order, false);
            }
            constexpr void setSize(nsize_t s) noexcept { _size = s; }

        public:
            /**
             * Transient passthrough read-only memory, w/o ownership ..
             * @param source a non nullptr memory, otherwise throws exception. Actual capacity known by owner.
             * @param len readable size of the memory, may be zero
             * @param byte_order endian::little or endian::big byte order
             * @throws IllegalArgumentException if source is nullptr but size > 0
             *         or byte_order not endian::little nor endian::big
             */
            TROOctets(const uint8_t *source, const nsize_t len, const endian byte_order)
            : _size( len ), _data( const_cast<uint8_t *>(source) ),
              _little_endian( is_little_endian(byte_order, true) )
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
            constexpr nsize_t getSize() const noexcept { return _size; }

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
                return EUI48(_data+i);
            }
            inline EUI48 get_eui48_nc(const nsize_t i) const noexcept {
                return EUI48(_data+i);
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
     */
    class TOctets : public TROOctets
    {
        public:
            /**
             * Transient passthrough r/w memory, w/o ownership ..
             * @param source transient data source
             * @param len length of transient data source
             * @param byte_order endian::little or endian::big byte order
             * @throws IllegalArgumentException if source is nullptr but size > 0
             *         or byte_order not endian::little nor endian::big
             */
            TOctets(uint8_t *source, const nsize_t len, const endian byte_order)
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
                memcpy(data() + i, v.b, sizeof(v.b));
            }
            void put_eui48_nc(const nsize_t i, const EUI48 & v) noexcept {
                memcpy(data() + i, v.b, sizeof(v.b));
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
                check_range(i, v.getSize());
                memcpy(data() + i, v.get_ptr(), v.getSize());
            }
            void put_octets_nc(const nsize_t i, const TROOctets & v) noexcept {
                memcpy(data() + i, v.get_ptr(), v.getSize());
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
                return "size "+std::to_string(getSize())+", rw: "+bytesHexString(get_ptr(), 0, getSize(), true /* lsbFirst */);
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
            const TOctets & parent;
            nsize_t const offset;
            nsize_t const size;

        public:
            TOctetSlice(const TOctets &buffer_, const nsize_t offset_, const nsize_t size_)
            : parent(buffer_), offset(offset_), size(size_)
            {
                if( offset_+size > buffer_.getSize() ) {
                    throw IndexOutOfBoundsException(offset_, size, buffer_.getSize(), E_FILE_LINE);
                }
            }

            /** Returns byte order of this octet store. */
            constexpr endian byte_order() const noexcept { return parent.byte_order(); }

            constexpr nsize_t getSize() const noexcept { return size; }
            constexpr nsize_t getOffset() const noexcept { return offset; }
            const TOctets& getParent() const noexcept { return parent; }

            uint8_t get_uint8(const nsize_t i) const {
                return parent.get_uint8(offset+i);
            }
            constexpr uint8_t get_uint8_nc(const nsize_t i) const noexcept {
                return parent.get_uint8_nc(offset+i);
            }

            uint16_t get_uint16(const nsize_t i) const {
                return parent.get_uint16(offset+i);
            }
            constexpr uint16_t get_uint16_nc(const nsize_t i) const noexcept {
                return parent.get_uint16_nc(offset+i);
            }

            uint8_t const * get_ptr(const nsize_t i) const {
                return parent.get_ptr(offset+i);
            }
            constexpr uint8_t const * get_ptr_nc(const nsize_t i) const noexcept {
                return parent.get_ptr_nc(offset+i);
            }

            std::string toString() const noexcept {
                return "offset "+std::to_string(offset)+", size "+std::to_string(size)+": "+bytesHexString(parent.get_ptr(), offset, size, true /* lsbFirst */);
            }
    };

    /**
     * Persistent endian aware octet data, i.e. owned memory allocation.
     *
     * Endian byte order is passed at construction.
     */
    class POctets : public TOctets
    {
        private:
            nsize_t capacity;

            void freeData() {
                uint8_t * ptr = data();
                if( nullptr != ptr ) {
                    TRACE_PRINT("POctets release: %p", ptr);
                    free(ptr);
                } // else: zero sized POctets w/ nullptr are supported
            }

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
            constexpr nsize_t getCapacity() const noexcept { return capacity; }

            /**
             * Intentional zero sized POctets instance.
             * @param byte_order endian::little or endian::big byte order
             */
            POctets(const endian byte_order)
            : TOctets(nullptr, 0, byte_order), capacity(0)
            {
                TRACE_PRINT("POctets ctor0: zero-sized");
            }

            /**
             * Takes ownership (malloc(size) and copy, free) ..
             *
             * Capacity and size will be of given source size.
             * @param _source source data to be copied into this new instance
             * @param size_ length of source data
             * @param byte_order endian::little or endian::big byte order
             * @throws IllegalArgumentException if byte_order not endian::little nor endian::big
             */
            POctets(const uint8_t *_source, const nsize_t size_, const endian byte_order)
            : TOctets( allocData(size_), size_, byte_order),
              capacity( size_ )
            {
                if( 0 < size_ ) {
                    std::memcpy(data(), _source, size_);
                }
                TRACE_PRINT("POctets ctor1: %p", data());
            }

            /**
             * New buffer (malloc(capacity), free)
             * @param _capacity new capacity
             * @param size_ new size with size <= capacity
             * @param byte_order endian::little or endian::big byte order
             * @throws IllegalArgumentException if byte_order not endian::little nor endian::big
             */
            POctets(const nsize_t _capacity, const nsize_t size_, const endian byte_order)
            : TOctets( allocData(_capacity), size_, byte_order),
              capacity( _capacity )
            {
                if( capacity < getSize() ) {
                    throw IllegalArgumentException("capacity "+std::to_string(capacity)+" < size "+std::to_string(getSize()), E_FILE_LINE);
                }
                TRACE_PRINT("POctets ctor2: %p", data());
            }

            /**
             * New buffer (malloc, free)
             * @param size new size and capacity
             * @param byte_order endian::little or endian::big byte order
             * @throws IllegalArgumentException if byte_order not endian::little nor endian::big
             */
            POctets(const nsize_t size, const endian byte_order)
            : POctets(size, size, byte_order)
            {
                TRACE_PRINT("POctets ctor3: %p", data());
            }

            POctets(const POctets &_source)
            : TOctets( allocData(_source.getSize()), _source.getSize(), _source.byte_order() ),
              capacity( _source.getSize() )
            {
                std::memcpy(data(), _source.get_ptr(), _source.getSize());
                TRACE_PRINT("POctets ctor-cpy0: %p", data());
            }

            POctets(POctets &&o) noexcept
            : TOctets( o.data(), o.getSize(), o.byte_order() ),
              capacity( o.getCapacity() )
            {
                // moved origin data references
                // purge origin
                o.setData(nullptr, 0, o.byte_order());
                o.capacity = 0;
                TRACE_PRINT("POctets ctor-move0: %p", data());
            }

            POctets& operator=(const POctets &_source) {
                if( this == &_source ) {
                    return *this;
                }
                freeData();
                setData(allocData(_source.getSize()), _source.getSize(), _source.byte_order());
                capacity = _source.getSize();
                std::memcpy(data(), _source.get_ptr(), _source.getSize());
                TRACE_PRINT("POctets assign0: %p", data());
                return *this;
            }

            POctets& operator=(POctets &&o) noexcept {
                // move origin data references
                setData(o.data(), o.getSize(), o.byte_order());
                capacity = o.capacity;
                // purge origin
                o.setData(nullptr, 0, o.byte_order());
                o.capacity = 0;
                TRACE_PRINT("POctets assign-move0: %p", data());
                return *this;
            }

            virtual ~POctets() noexcept override {
                freeData();
                setData(nullptr, 0, byte_order());
                capacity=0;
            }

            /** Makes a persistent POctets by copying the data from TROOctets. */
            POctets(const TROOctets & _source)
            : TOctets( allocData(_source.getSize()), _source.getSize(), _source.byte_order() ),
              capacity( _source.getSize() )
            {
                std::memcpy(data(), _source.get_ptr(), _source.getSize());
                TRACE_PRINT("POctets ctor-cpy1: %p", data());
            }

            POctets& operator=(const TROOctets &_source) {
                if( static_cast<TROOctets *>(this) == &_source ) {
                    return *this;
                }
                freeData();
                setData(allocData(_source.getSize()), _source.getSize(), _source.byte_order());
                capacity = _source.getSize();
                std::memcpy(data(), _source.get_ptr(), _source.getSize());
                TRACE_PRINT("POctets assign1: %p", data());
                return *this;
            }

            /** Makes a persistent POctets by copying the data from TOctetSlice. */
            POctets(const TOctetSlice & _source)
            : TOctets( allocData(_source.getSize()), _source.getSize(), _source.byte_order() ),
              capacity( _source.getSize() )
            {
                std::memcpy(data(), _source.getParent().get_ptr() + _source.getOffset(), _source.getSize());
                TRACE_PRINT("POctets ctor-cpy2: %p", data());
            }

            POctets& operator=(const TOctetSlice &_source) {
                freeData();
                setData(allocData(_source.getSize()), _source.getSize(), _source.byte_order());
                capacity = _source.getSize();
                std::memcpy(data(), _source.get_ptr(0), _source.getSize());
                TRACE_PRINT("POctets assign2: %p", data());
                return *this;
            }

            POctets & resize(const nsize_t newSize, const nsize_t newCapacity) {
                if( newCapacity < newSize ) {
                    throw IllegalArgumentException("newCapacity "+std::to_string(newCapacity)+" < newSize "+std::to_string(newSize), E_FILE_LINE);
                }
                if( newCapacity != capacity ) {
                    if( newSize > getSize() ) {
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

            POctets & resize(const nsize_t newSize) {
                if( capacity < newSize ) {
                    throw IllegalArgumentException("capacity "+std::to_string(capacity)+" < newSize "+std::to_string(newSize), E_FILE_LINE);
                }
                setSize(newSize);
                return *this;
            }

            POctets & recapacity(const nsize_t newCapacity) {
                if( newCapacity < getSize() ) {
                    throw IllegalArgumentException("newCapacity "+std::to_string(newCapacity)+" < size "+std::to_string(getSize()), E_FILE_LINE);
                }
                if( newCapacity == capacity ) {
                    return *this;
                }
                uint8_t* data2 = allocData(newCapacity);
                if( getSize() > 0 ) {
                    memcpy(data2, get_ptr(), getSize());
                }
                TRACE_PRINT("POctets recapacity: %p -> %p", data(), data2);
                free(data());
                setData(data2, getSize(), byte_order());
                capacity = newCapacity;
                return *this;
            }

            POctets & operator+=(const TROOctets &b) {
                if( 0 < b.getSize() ) {
                    const nsize_t newSize = getSize() + b.getSize();
                    if( capacity < newSize ) {
                        recapacity( newSize );
                    }
                    memcpy(data()+getSize(), b.get_ptr(), b.getSize());
                    setSize(newSize);
                }
                return *this;
            }
            POctets & operator+=(const TOctetSlice &b) {
                if( 0 < b.getSize() ) {
                    const nsize_t newSize = getSize() + b.getSize();
                    if( capacity < newSize ) {
                        recapacity( newSize );
                    }
                    memcpy(data()+getSize(), b.getParent().get_ptr()+b.getOffset(), b.getSize());
                    setSize(newSize);
                }
                return *this;
            }

            std::string toString() const {
                return "size "+std::to_string(getSize())+", capacity "+std::to_string(getCapacity())+", "+bytesHexString(get_ptr(), 0, getSize(), true /* lsbFirst */);
            }
    };

} /* namespace jau */


#endif /* JAU_OCTETS_HPP_ */
