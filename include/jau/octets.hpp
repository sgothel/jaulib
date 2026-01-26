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

#ifndef JAU_OCTETS_HPP_
#define JAU_OCTETS_HPP_

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <string>
#include "jau/cpp_lang_util.hpp"

#include <jau/basic_types.hpp>
#include <jau/debug.hpp>
#include <jau/io/eui48.hpp>
#include <jau/secmem.hpp>
#include <jau/uuid.hpp>

// #define JAU_TRACE_OCTETS 1
#ifdef JAU_TRACE_OCTETS
    #define JAU_TRACE_OCTETS_PRINT(...) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); fflush(stderr); }
#else
    #define JAU_TRACE_OCTETS_PRINT(...)
#endif

namespace jau {

    /** \addtogroup ByteUtils
     *
     *  @{
     */

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
            /** byte-order flag, little or big endian */
            lb_endian_t _byte_order;

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
                    jau_ABORT("TROOctets: nullptr with size %s > 0", std::to_string(size_).c_str());
                    abort(); // never reached
                }
            }

            constexpr uint8_t * data() noexcept { return _data; }

            /**
             * Internally sets the _size and _data fields after validation.
             *
             * Aborts if data_ is nullptr and size_ > 0.
             *
             * @param data_ a memory pointer
             * @param size_ used memory size
             * @param byte_order lb_endian::little or lb_endian::big byte order, one may pass lb_endian::native.
             */
            inline void setData(uint8_t *data_, nsize_t size_, const lb_endian_t byte_order) noexcept {
                JAU_TRACE_OCTETS_PRINT("POctets setData: %zu bytes @ %p -> %zu bytes @ %p",
                        _size, _data, size_, data_);
                checkPtr(data_, size_);
                _size = size_;
                _data = data_;
                _byte_order = byte_order;
            }
            constexpr void setSize(nsize_t s) noexcept { _size = s; }

        public:
            /**
             * Transient passthrough read-only memory, w/o ownership ..
             *
             * Aborts if source is nullptr and len > 0.
             *
             * @param source a non nullptr memory, otherwise throws exception. Actual capacity known by owner.
             * @param len readable size of the memory, may be zero
             * @param byte_order lb_endian::little or lb_endian::big byte order, one may pass lb_endian::native.
             */
            TROOctets(const uint8_t *source, const nsize_t len, const lb_endian_t byte_order_val ) noexcept
            : _size( len ), _data( const_cast<uint8_t *>(source) ),
              _byte_order( byte_order_val )
            {
                checkPtr(_data, _size);
            }

            /**
             * Default constructor with nullptr memory, zero size and lb_endian::native byte order.
             *
             * Conveniently exists to allow instantiation of variables
             * intended for later assignment.
             */
            TROOctets() noexcept
            : _size( 0 ), _data( nullptr ),
              _byte_order( lb_endian_t::native )
            { }

            TROOctets(const TROOctets &o) noexcept = default;
            TROOctets(TROOctets &&o) noexcept = default;
            TROOctets& operator=(const TROOctets &o) noexcept = default;
            TROOctets& operator=(TROOctets &&o) noexcept = default;

            virtual ~TROOctets() noexcept = default;

            inline void check_range(const nsize_t i, const nsize_t count, const char *file, int line) const {
                if( i+count > _size ) {
                    throw IndexOutOfBoundsError(i, count, _size, file, line);
                }
            }

            constexpr bool is_range_valid(const nsize_t i, const nsize_t count) const noexcept {
                return i+count <= _size;
            }

            /** Returns byte order of this octet store. */
            constexpr lb_endian_t byte_order() const noexcept { return _byte_order; }

            /** Returns the used memory size for read and write operations, may be zero. */
            constexpr nsize_t size() const noexcept { return _size; }

            uint8_t get_uint8(const nsize_t i) const {
                check_range(i, 1, E_FILE_LINE);
                return _data[i];
            }
            constexpr uint8_t get_uint8_nc(const nsize_t i) const noexcept {
                return _data[i];
            }

            int8_t get_int8(const nsize_t i) const {
                check_range(i, 1, E_FILE_LINE);
                return jau::get_int8(_data + i);
            }
            constexpr int8_t get_int8_nc(const nsize_t i) const noexcept {
                return jau::get_int8(_data + i);
            }

            uint16_t get_uint16(const nsize_t i) const {
                check_range(i, 2, E_FILE_LINE);
                return jau::get_uint16(_data + i, byte_order());
            }
            constexpr uint16_t get_uint16_nc(const nsize_t i) const noexcept {
                return jau::get_uint16(_data + i, byte_order());
            }

            uint32_t get_uint32(const nsize_t i) const {
                check_range(i, 4, E_FILE_LINE);
                return jau::get_uint32(_data + i, byte_order());
            }
            constexpr uint32_t get_uint32_nc(const nsize_t i) const noexcept {
                return jau::get_uint32(_data + i, byte_order());
            }

            jau::io::net::EUI48 get_eui48(const nsize_t i) const {
                check_range(i, sizeof(jau::io::net::EUI48), E_FILE_LINE);
                return jau::io::net::EUI48(_data+i, byte_order() );
            }
            inline jau::io::net::EUI48 get_eui48_nc(const nsize_t i) const noexcept {
                return jau::io::net::EUI48(_data+i, byte_order() );
            }

            uint64_t get_uint64(const nsize_t i) const {
                check_range(i, 8, E_FILE_LINE);
                return jau::get_uint64(_data + i, byte_order());
            }
            constexpr uint64_t get_uint64_nc(const nsize_t i) const noexcept {
                return jau::get_uint64(_data + i, byte_order());
            }

            uint128dp_t get_uint128(const nsize_t i) const {
                check_range(i, 8, E_FILE_LINE);
                return jau::get_uint128(_data + i, byte_order());
            }
            constexpr uint128dp_t get_uint128_nc(const nsize_t i) const noexcept {
                return jau::get_uint128(_data + i, byte_order());
            }

            uint192dp_t get_uint192(const nsize_t i) const {
                check_range(i, 8, E_FILE_LINE);
                return jau::get_uint192(_data + i, byte_order());
            }
            constexpr uint192dp_t get_uint192_nc(const nsize_t i) const noexcept {
                return jau::get_uint192(_data + i, byte_order());
            }

            uint256dp_t get_uint256(const nsize_t i) const {
                check_range(i, 8, E_FILE_LINE);
                return jau::get_uint256(_data + i, byte_order());
            }
            constexpr uint256dp_t get_uint256_nc(const nsize_t i) const noexcept {
                return jau::get_uint256(_data + i, byte_order());
            }

            /** Assumes a null terminated string */
            std::string get_string(const nsize_t i) const {
                check_range(i, 1, E_FILE_LINE); // minimum size
                return std::string( (const char*)(_data+i) );
            }
            /** Assumes a null terminated string */
            inline std::string get_string_nc(const nsize_t i) const noexcept {
                return std::string( (const char*)(_data+i) );
            }

            /** Assumes a string with defined length, not necessarily null terminated */
            std::string get_string(const nsize_t i, const nsize_t length) const {
                check_range(i, length, E_FILE_LINE);
                return std::string( (const char*)(_data+i), length );
            }

            uuid16_t get_uuid16(const nsize_t i) const {
                return uuid16_t(get_uint16(i));
            }
            inline uuid16_t get_uuid16_nc(const nsize_t i) const noexcept {
                return uuid16_t(get_uint16_nc(i));
            }

            uuid128_t get_uuid128(const nsize_t i) const {
                check_range(i, uuid_t::number(uuid_t::TypeSize::UUID128_SZ), E_FILE_LINE);
                return jau::get_uuid128(_data + i, byte_order());
            }
            inline uuid128_t get_uuid128_nc(const nsize_t i) const noexcept {
                return jau::get_uuid128(_data + i, byte_order());
            }

            std::unique_ptr<const uuid_t> get_uuid(const nsize_t i, const uuid_t::TypeSize tsize) const {
                check_range(i, uuid_t::number(tsize), E_FILE_LINE);
                return uuid_t::create(tsize, _data + i, byte_order());
            }

            constexpr uint8_t const * get_ptr() const noexcept { return _data; }
            uint8_t const * get_ptr(const nsize_t i) const {
                check_range(i, 1, E_FILE_LINE);
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
                std::string s;
                do_noexcept([&]() {
                    s.append("size ")
                     .append(std::to_string(_size))
                     .append(", [").append(to_string( byte_order() )).append(", ").append(to_string( byte_order() )).append("], ro: ");
                     });
                jau::appendHexString(s, _data, _size);
                return s;
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
             * Aborts if source is nullptr and len > 0.
             *
             * @param source transient data source
             * @param len length of transient data source
             * @param byte_order lb_endian::little or lb_endian::big byte order, one may pass lb_endian::native.
             */
            TOctets(uint8_t *source, const nsize_t len, const lb_endian_t byte_order) noexcept
            : TROOctets(source, len, byte_order) {}

            TOctets(const TOctets &o) noexcept = default;
            TOctets(TOctets &&o) noexcept = default;
            TOctets& operator=(const TOctets &o) noexcept = default;
            TOctets& operator=(TOctets &&o) noexcept = default;

            ~TOctets() noexcept override = default;

            void put_int8(const nsize_t i, const int8_t v) {
                check_range(i, 1, E_FILE_LINE);
                data()[i] = static_cast<uint8_t>(v);
            }
            constexpr void put_int8_nc(const nsize_t i, const int8_t v) noexcept {
                data()[i] = static_cast<uint8_t>(v);
            }

            void put_uint8(const nsize_t i, const uint8_t v) {
                check_range(i, 1, E_FILE_LINE);
                data()[i] = v;
            }
            constexpr void put_uint8_nc(const nsize_t i, const uint8_t v) noexcept {
                data()[i] = v;
            }

            void put_uint16(const nsize_t i, const uint16_t v) {
                check_range(i, 2, E_FILE_LINE);
                jau::put_uint16(data() + i, v, byte_order());
            }
            constexpr void put_uint16_nc(const nsize_t i, const uint16_t v) noexcept {
                jau::put_uint16(data() + i, v, byte_order());
            }

            void put_uint32(const nsize_t i, const uint32_t v) {
                check_range(i, 4, E_FILE_LINE);
                jau::put_uint32(data() + i, v, byte_order());
            }
            constexpr void put_uint32_nc(const nsize_t i, const uint32_t v) noexcept {
                jau::put_uint32(data() + i, v, byte_order());
            }

            void put_eui48(const nsize_t i, const jau::io::net::EUI48 & v) {
                check_range(i, sizeof(v.b), E_FILE_LINE);
                v.put(data() + i, byte_order() );
            }
            inline void put_eui48_nc(const nsize_t i, const jau::io::net::EUI48 & v) noexcept {
                v.put(data() + i, byte_order() );
            }

            void put_uint64(const nsize_t i, const uint64_t & v) {
                check_range(i, 8, E_FILE_LINE);
                jau::put_uint64(data() + i, v, byte_order());
            }
            constexpr void put_uint64_nc(const nsize_t i, const uint64_t & v) noexcept {
                jau::put_uint64(data() + i, v, byte_order());
            }

            void put_uint128(const nsize_t i, const uint128dp_t & v) {
                check_range(i, 8, E_FILE_LINE);
                jau::put_uint128(data() + i, v, byte_order());
            }
            constexpr void put_uint128_nc(const nsize_t i, const uint128dp_t & v) noexcept {
                jau::put_uint128(data() + i, v, byte_order());
            }

            void put_uint192(const nsize_t i, const uint192dp_t & v) {
                check_range(i, 8, E_FILE_LINE);
                jau::put_uint192(data() + i, v, byte_order());
            }
            constexpr void put_uint192_nc(const nsize_t i, const uint192dp_t & v) noexcept {
                jau::put_uint192(data() + i, v, byte_order());
            }

            void put_uint256(const nsize_t i, const uint256dp_t & v) {
                check_range(i, 8, E_FILE_LINE);
                jau::put_uint256(data() + i, v, byte_order());
            }
            constexpr void put_uint256_nc(const nsize_t i, const uint256dp_t & v) noexcept {
                jau::put_uint256(data() + i, v, byte_order());
            }

            void put_octets(const nsize_t i, const TROOctets & v) {
                check_range(i, v.size(), E_FILE_LINE);
                std::memcpy(data() + i, v.get_ptr(), v.size());
            }
            void put_octets_nc(const nsize_t i, const TROOctets & v) noexcept {
                std::memcpy(data() + i, v.get_ptr(), v.size());
            }
            void put_octets(const nsize_t i, const TROOctets & v, const nsize_t v_off, const nsize_t v_len) {
                const nsize_t size = std::min(v.size()-v_off, v_len);
                check_range(i, size, E_FILE_LINE);
                std::memcpy(data() + i, v.get_ptr() + v_off, size);
            }
            void put_octets_nc(const nsize_t i, const TROOctets & v, const nsize_t v_off, const nsize_t v_len) noexcept {
                const nsize_t size = std::min(v.size()-v_off, v_len);
                std::memcpy(data() + i, v.get_ptr() + v_off, size);
            }

            void put_bytes(const nsize_t i, const uint8_t *source, const nsize_t byte_count) {
                check_range(i, byte_count, E_FILE_LINE);
                std::memcpy(data() + i, source, byte_count);
            }
            void put_bytes_nc(const nsize_t i, const uint8_t *source, const nsize_t byte_count) noexcept {
                std::memcpy(data() + i, source, byte_count);
            }

            void memmove(const nsize_t i, const uint8_t *source, const nsize_t byte_count) {
                check_range(i, byte_count, E_FILE_LINE);
                std::memmove(data() + i, source, byte_count);
            }
            void memmove_nc(const nsize_t i, const uint8_t *source, const nsize_t byte_count) noexcept {
                std::memmove(data() + i, source, byte_count);
            }

            void memset(const nsize_t i, const uint8_t c, const nsize_t byte_count) {
                check_range(i, byte_count, E_FILE_LINE);
                std::memset(data() + i, c, byte_count);
            }
            void memset_nc(const nsize_t i, const uint8_t c, const nsize_t byte_count) noexcept {
                std::memset(data() + i, c, byte_count);
            }
            void bzero(const nsize_t i, const nsize_t byte_count) {
                check_range(i, byte_count, E_FILE_LINE);
                zero_bytes_sec(data() + i, byte_count);
            }
            void bzero_nc(const nsize_t i, const nsize_t byte_count) noexcept {
                zero_bytes_sec(data() + i, byte_count);
            }
            void bzero() noexcept {
                if( size() > 0 ) {
                    zero_bytes_sec(data(), size());
                }
            }

            void put_string(const nsize_t i, const std::string & v, const nsize_t max_len, const bool includeEOS) {
                const nsize_t size1 = v.size() + ( includeEOS ? 1 : 0 );
                const nsize_t size = std::min(size1, max_len);
                check_range(i, size, E_FILE_LINE);
                std::memcpy(data() + i, v.c_str(), size);
                if( size < size1 && includeEOS ) {
                    *(data() + i + size - 1) = 0; // ensure EOS
                }
            }
            void put_string_nc(const nsize_t i, const std::string & v, const nsize_t max_len, const bool includeEOS) noexcept {
                const nsize_t size1 = v.size() + ( includeEOS ? 1 : 0 );
                const nsize_t size = std::min(size1, max_len);
                std::memcpy(data() + i, v.c_str(), size);
                if( size < size1 && includeEOS ) {
                    *(data() + i + size - 1) = 0; // ensure EOS
                }
            }

            void put_uuid(const nsize_t i, const uuid_t & v) {
                check_range(i, v.getTypeSizeInt(), E_FILE_LINE);
                v.put(data() + i, byte_order());
            }
            void put_uuid_nc(const nsize_t i, const uuid_t & v) noexcept {
                v.put(data() + i, byte_order());
            }

            inline uint8_t * get_wptr() noexcept { return data(); }

            uint8_t * get_wptr(const nsize_t i) {
                check_range(i, 1, E_FILE_LINE);
                return data() + i;
            }
            inline uint8_t * get_wptr_nc(const nsize_t i) noexcept {
                return data() + i;
            }

            std::string toString() const noexcept {
                return string_noexcept([&](){ return "size "+std::to_string(size())+", rw: "+toHexString(get_ptr(), size()); });
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
                    throw IndexOutOfBoundsError(offset_, _size, buffer_.size(), E_FILE_LINE);
                }
            }

            /** Returns byte order of this octet store. */
            constexpr lb_endian_t byte_order() const noexcept { return _parent.byte_order(); }

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
                return string_noexcept([&](){ return "offset "+std::to_string(_offset)+", size "+std::to_string(_size)+": "+toHexString(_parent.get_ptr()+_offset, _size); } );
            }
    };

    /**
     * Persistent endian aware octet data, i.e. owned dynamic heap memory allocation.
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
                    JAU_TRACE_OCTETS_PRINT("POctets release: %p", ptr);
                    free(ptr);
                } // else: zero sized POctets w/ nullptr are supported
            }

            /**
             * Allocate a memory chunk.
             * @param size
             * @return
             * @throws OutOfMemoryError if running out of memory
             */
            [[nodiscard]] static uint8_t * allocData(const nsize_t size) {
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
            /** Returns the memory capacity, never zero, greater or equal size(). */
            constexpr nsize_t capacity() const noexcept { return _capacity; }

            /** Returns the remaining octets for put left, i.e. capacity() - size(). */
            constexpr nsize_t remaining() const noexcept { return _capacity - size(); }

            /**
             * Zero sized POctets instance.
             *
             * Will not throw an OutOfMemoryError exception due to no allocation.
             *
             * @param byte_order lb_endian::little or lb_endian::big byte order, one may pass lb_endian::native.
             */
            POctets(const lb_endian_t byte_order) noexcept
            : TOctets(nullptr, 0, byte_order), _capacity(0)
            {
                JAU_TRACE_OCTETS_PRINT("POctets ctor0: zero-sized");
            }

            /**
             * Takes ownership (malloc(size) and copy, free) ..
             *
             * Capacity and size will be of given source size.
             *
             * @param source_ source data to be copied into this new instance
             * @param size_ length of source data
             * @param byte_order lb_endian::little or lb_endian::big byte order, one may pass lb_endian::native.
             * @throws IllegalArgumentException if source_ is nullptr and size_ > 0
             * @throws OutOfMemoryError if allocation fails
             */
            POctets(const uint8_t *source_, const nsize_t size_, const lb_endian_t byte_order)
            : TOctets( allocData(size_), size_, byte_order),
              _capacity( size_ )
            {
                if( 0 < size_ ) {
                    if( nullptr == source_ ) {
                        throw IllegalArgumentError("source nullptr with size "+std::to_string(size_)+" > 0", E_FILE_LINE);
                    }
                    std::memcpy(data(), source_, size_);
                }
                JAU_TRACE_OCTETS_PRINT("POctets ctor1: %p", data());
            }

            /**
             * Takes ownership (malloc(size) and copy, free) ..
             *
             * Capacity and size will be of given source size.
             *
             * @param sourcelist source initializer list data to be copied into this new instance with implied size
             * @param byte_order lb_endian::little or lb_endian::big byte order, one may pass lb_endian::native.
             * @throws IllegalArgumentException if source_ is nullptr and size_ > 0
             * @throws OutOfMemoryError if allocation fails
             */
            POctets(std::initializer_list<uint8_t> sourcelist, const lb_endian_t byte_order)
            : TOctets( allocData(sourcelist.size()), sourcelist.size(), byte_order),
              _capacity( sourcelist.size() )
            {
                if( 0 < _capacity ) {
                    std::memcpy(data(), sourcelist.begin(), _capacity);
                }
                JAU_TRACE_OCTETS_PRINT("POctets ctor1: %p", data());
            }

            /**
             * New buffer (malloc(capacity), free)
             *
             * @param capacity_ new capacity
             * @param size_ new size with size <= capacity
             * @param byte_order lb_endian::little or lb_endian::big byte order, one may pass lb_endian::native.
             * @throws IllegalArgumentException if capacity_ < size_
             * @throws OutOfMemoryError if allocation fails
             */
            POctets(const nsize_t capacity_, const nsize_t size_, const lb_endian_t byte_order)
            : TOctets( allocData( capacity_ ), size_, byte_order ),
              _capacity( capacity_ )
            {
                if( capacity() < size() ) {
                    throw IllegalArgumentError("capacity "+std::to_string(capacity())+" < size "+std::to_string(size()), E_FILE_LINE);
                }
                JAU_TRACE_OCTETS_PRINT("POctets ctor2: %p", data());
            }

            /**
             * New buffer (malloc, free)
             *
             * @param size new size and capacity
             * @param byte_order lb_endian::little or lb_endian::big byte order, one may pass lb_endian::native.
             * @throws OutOfMemoryError if allocation fails
             */
            POctets(const nsize_t size, const lb_endian_t byte_order)
            : POctets(size, size, byte_order)
            {
                JAU_TRACE_OCTETS_PRINT("POctets ctor3: %p", data());
            }

            /**
             * Copy constructor
             *
             * Capacity of this new instance will be of source.size() only.
             *
             * @param source POctet source to be copied
             * @throws OutOfMemoryError if allocation fails
             */
            POctets(const POctets &source)
            : TOctets( allocData(source.size()), source.size(), source.byte_order() ),
              _capacity( source.size() )
            {
                std::memcpy(data(), source.get_ptr(), source.size());
                JAU_TRACE_OCTETS_PRINT("POctets ctor-cpy0: %p -> %p", source.get_ptr(), data());
            }

            /**
             * Copy constructor (explicit), allowing to set a higher capacity
             * than source.size() in contrast to the copy-constructor.
             *
             * @param source POctet source to be copied
             * @param capacity_ used to determine new capacity `std::max(capacity_, source.size())`
             * @throws OutOfMemoryError if allocation fails
             */
            explicit POctets(const POctets &source, const nsize_t capacity_)
            : TOctets( allocData(source.size()), source.size(), source.byte_order() ),
              _capacity( std::max(capacity_, source.size()) )
            {
                std::memcpy(data(), source.get_ptr(), source.size());
                JAU_TRACE_OCTETS_PRINT("POctets ctor-cpy-extra1: %p -> %p", source.get_ptr(), data());
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
                JAU_TRACE_OCTETS_PRINT("POctets ctor-move0: %p", data());
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
                JAU_TRACE_OCTETS_PRINT("POctets assign0: %p", data());
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
                JAU_TRACE_OCTETS_PRINT("POctets assign-move0: %p", data());
                return *this;
            }

            ~POctets() noexcept override {
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
                JAU_TRACE_OCTETS_PRINT("POctets ctor-cpy1: %p", data());
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
                JAU_TRACE_OCTETS_PRINT("POctets assign1: %p", data());
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
                JAU_TRACE_OCTETS_PRINT("POctets ctor-cpy2: %p", data());
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
                JAU_TRACE_OCTETS_PRINT("POctets assign2: %p", data());
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
                    throw IllegalArgumentError("newCapacity "+std::to_string(newCapacity)+" < newSize "+std::to_string(newSize), E_FILE_LINE);
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
                    throw IllegalArgumentError("capacity "+std::to_string(_capacity)+" < newSize "+std::to_string(newSize), E_FILE_LINE);
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
                    throw IllegalArgumentError("newCapacity "+std::to_string(newCapacity)+" < size "+std::to_string(size()), E_FILE_LINE);
                }
                if( newCapacity == _capacity ) {
                    return *this;
                }
                uint8_t* data2 = allocData(newCapacity);
                if( size() > 0 ) {
                    memcpy(data2, get_ptr(), size());
                }
                JAU_TRACE_OCTETS_PRINT("POctets recapacity: %p -> %p", data(), data2);
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

            std::string toString() const noexcept {
                return string_noexcept([&](){ return "size "+std::to_string(size())+", capacity "+std::to_string(capacity())+", "+toHexString(get_ptr(), size()); } );
            }
    };

    /**
     * Persistent endian aware octet data, i.e. owned automatic fixed size memory allocation.
     *
     * Endian byte order is passed at construction.
     *
     * Constructor and assignment operations are **not** completely `noexcept` and may throw exceptions.
     * This is a design choice based on dynamic resource allocation performed by this class.
     */
    template<jau::nsize_t FixedSize>
    class AOctets : public TOctets
    {
        public:
            /** Fixed maximum size */
            constexpr static const jau::nsize_t fixed_size = FixedSize;

        private:
            uint8_t smem[fixed_size];

        public:
            /**
             * Sized AOctets instance.
             *
             * @param byte_order lb_endian::little or lb_endian::big byte order, one may pass lb_endian::native.
             */
            AOctets(const lb_endian_t byte_order) noexcept
            : TOctets(smem, fixed_size, byte_order)
            {
                JAU_TRACE_OCTETS_PRINT("AOctets ctor0: sized");
            }

            /**
             * Takes ownership (malloc(size) and copy, free) ..
             *
             * Capacity and size will be of given source size.
             *
             * @param source_ source data to be copied into this new instance
             * @param size_ length of source data
             * @param byte_order lb_endian::little or lb_endian::big byte order, one may pass lb_endian::native.
             * @throws IllegalArgumentException if fixed_size < source_size_
             * @throws IllegalArgumentException if source_ is nullptr and size_ > 0
             */
            AOctets(const uint8_t *source_, const nsize_t source_size_, const lb_endian_t byte_order)
            : TOctets( smem, std::min(fixed_size, source_size_), byte_order)
            {
                if( source_size_ > fixed_size ) {
                    throw IllegalArgumentError("source size "+std::to_string(source_size_)+" > capacity "+std::to_string(fixed_size), E_FILE_LINE);
                } else if( 0 < source_size_ ) {
                    if( nullptr == source_ ) {
                        throw IllegalArgumentError("source nullptr with size "+std::to_string(source_size_)+" > 0", E_FILE_LINE);
                    }
                    std::memcpy(data(), source_, source_size_);
                }
                JAU_TRACE_OCTETS_PRINT("AOctets ctor1: %p", data());
            }

            /**
             * Takes ownership (malloc(size) and copy, free) ..
             *
             * Capacity and size will be of given source size.
             *
             * @param sourcelist source initializer list data to be copied into this new instance with implied size
             * @param byte_order lb_endian::little or lb_endian::big byte order, one may pass lb_endian::native.
             * @throws IllegalArgumentException if fixed_size < source size
             */
            AOctets(std::initializer_list<uint8_t> sourcelist, const lb_endian_t byte_order)
            : TOctets( smem, std::min(fixed_size, sourcelist.size()), byte_order)
            {
                if( sourcelist.size() > fixed_size ) {
                    throw IllegalArgumentError("source size "+std::to_string(sourcelist.size())+" > capacity "+std::to_string(fixed_size), E_FILE_LINE);
                } else if( 0 < sourcelist.size() ) {
                    std::memcpy(data(), sourcelist.begin(), sourcelist.size());
                }
                JAU_TRACE_OCTETS_PRINT("AOctets ctor1: %p", data());
            }

            /**
             * Copy constructor
             *
             * Capacity of this new instance will be of source.size() only.
             *
             * @param source POctet source to be copied
             * @throws IllegalArgumentException if fixed_size < source size
             */
            AOctets(const TROOctets &source)
            : TOctets( smem, std::min(fixed_size, source.size()), source.byte_order() )
            {
                if( source.size() > fixed_size ) {
                    throw IllegalArgumentError("source size "+std::to_string(source.size())+" > capacity "+std::to_string(fixed_size), E_FILE_LINE);
                } else if( 0 < source.size() ) {
                    std::memcpy(data(), source.get_ptr(), source.size());
                }
                JAU_TRACE_OCTETS_PRINT("AOctets ctor-cpy0: %p -> %p", source.get_ptr(), data());
            }

            /**
             * Assignment operator
             * @param _source POctet source to be copied
             * @return
             * @throws IllegalArgumentException if fixed_size < source size
             */
            AOctets& operator=(const TROOctets &_source) {
                if( this == &_source ) {
                    return *this;
                }
                if( _source.size() > fixed_size ) {
                    throw IllegalArgumentError("source size "+std::to_string(_source.size())+" > capacity "+std::to_string(fixed_size), E_FILE_LINE);
                } else if( 0 < _source.size() ) {
                    std::memcpy(smem, _source.get_ptr(), _source.size());
                }
                setData(smem, _source.size(), _source.byte_order());
                JAU_TRACE_OCTETS_PRINT("AOctets assign0: %p", data());
                return *this;
            }


            ~AOctets() noexcept override {
                setData(nullptr, 0, byte_order());
            }

            /**
             * Sets a new size for this instance.
             * @param newSize new size, must be <= current capacity()
             * @return
             * @throws IllegalArgumentException if fixed_size < newSize
             */
            AOctets & resize(const nsize_t newSize) {
                if( fixed_size < newSize ) {
                    throw IllegalArgumentError("capacity "+std::to_string(fixed_size)+" < newSize "+std::to_string(newSize), E_FILE_LINE);
                }
                setSize(newSize);
                return *this;
            }

            std::string toString() const noexcept {
                return string_noexcept([&](){ return "size "+std::to_string(size())+", fixed_size "+std::to_string(fixed_size)+", "+toHexString(get_ptr(), size()); });
            }
    };

    /**@}*/

} /* namespace jau */


#endif /* JAU_OCTETS_HPP_ */
