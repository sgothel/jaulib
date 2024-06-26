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

#include <jau/debug.hpp>

#include <jau/uuid.hpp>


using namespace jau;

// BASE_UUID '00000000-0000-1000-8000-00805F9B34FB'
static uint8_t bt_base_uuid_be[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
                                     0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
uuid128_t jau::BT_BASE_UUID( bt_base_uuid_be, lb_endian_t::big );

std::string uuid_t::getTypeSizeString(const TypeSize v) noexcept {
    switch( static_cast<TypeSize>(v) ) {
        case TypeSize::UUID16_SZ: return "uuid16";
        case TypeSize::UUID32_SZ: return "uuid32";
        case TypeSize::UUID128_SZ: return "uuid128";
        default: return "uuid_t unsupported size "+std::to_string(number(v));
    }
}

uuid_t::TypeSize uuid_t::toTypeSize(const jau::nsize_t size) {
    switch( static_cast<TypeSize>(size) ) {
        case TypeSize::UUID16_SZ: return TypeSize::UUID16_SZ;
        case TypeSize::UUID32_SZ: return TypeSize::UUID32_SZ;
        case TypeSize::UUID128_SZ: return TypeSize::UUID128_SZ;
    }
    throw jau::IllegalArgumentError("Given size "+std::to_string(size)+", not matching uuid16_t, uuid32_t or uuid128_t", E_FILE_LINE);
}

std::unique_ptr<uuid_t> uuid_t::create(TypeSize t, uint8_t const * const buffer, lb_endian_t const le_or_be) {
    if( TypeSize::UUID16_SZ == t ) {
        return std::make_unique<uuid16_t>(buffer, le_or_be);
    } else if( TypeSize::UUID32_SZ == t ) {
        return std::make_unique<uuid32_t>(buffer, le_or_be);
    } else if( TypeSize::UUID128_SZ == t ) {
        return std::make_unique<uuid128_t>(buffer, le_or_be);
    }
    throw jau::IllegalArgumentError("Unknown Type "+std::to_string(static_cast<jau::nsize_t>(t)), E_FILE_LINE);
}
std::unique_ptr<uuid_t> uuid_t::create(const std::string& str) {
    const size_t len = str.length();
    switch( len ) {
        case 4: // 16
            return std::make_unique<uuid16_t>(str);
        case 8: // 32
            return std::make_unique<uuid32_t>(str);
        case 36: // 128
            return std::make_unique<uuid128_t>(str);
        default: {
            std::string msg("UUID string not of length 4, 8 or 36 but ");
            msg.append(std::to_string(str.length()));
            msg.append(": "+str);
            throw jau::IllegalArgumentError(msg, E_FILE_LINE);
        }
    }
}

std::unique_ptr<uuid_t> uuid_t::clone() const noexcept {
    switch(type) {
        case TypeSize::UUID16_SZ: return std::make_unique<uuid16_t>( *( static_cast<const uuid16_t*>(this) ) );
        case TypeSize::UUID32_SZ: return std::make_unique<uuid32_t>( *( static_cast<const uuid32_t*>(this) ) );
        case TypeSize::UUID128_SZ: return std::make_unique<uuid128_t>( *( static_cast<const uuid128_t*>(this) ) );
    }
    ABORT("Unknown Type %d", static_cast<jau::nsize_t>(type));
    abort(); // never reached
}

bool uuid_t::operator==(uuid_t const &o) const noexcept {
    if( this == &o ) {
        return true;
    }
    if( type != o.getTypeSize() ) {
        return false;
    }
    switch( static_cast<TypeSize>(type) ) {
        case TypeSize::UUID16_SZ: return static_cast<uuid16_t const *>(this)->value == static_cast<uuid16_t const *>(&o)->value;
        case TypeSize::UUID32_SZ: return static_cast<uuid32_t const *>(this)->value == static_cast<uuid32_t const *>(&o)->value;
        case TypeSize::UUID128_SZ: return static_cast<uuid128_t const *>(this)->value == static_cast<uuid128_t const *>(&o)->value;
    }
    return false; // dead code
}

bool uuid_t::equivalent(uuid_t const &o) const noexcept {
    if( this == &o ) {
        return true;
    }
    if( getTypeSize() == o.getTypeSize() ) {
        return *this == o;
    }
    return toUUID128() == o.toUUID128();
}

uuid128_t uuid_t::toUUID128(uuid128_t const & base_uuid, jau::nsize_t const uuid32_le_octet_index) const noexcept {
    switch(type) {
        case TypeSize::UUID16_SZ:  return uuid128_t( *( static_cast<const uuid16_t*>(this)  ), base_uuid, uuid32_le_octet_index);
        case TypeSize::UUID32_SZ:  return uuid128_t( *( static_cast<const uuid32_t*>(this)  ), base_uuid, uuid32_le_octet_index);
        case TypeSize::UUID128_SZ: return uuid128_t( *( static_cast<const uuid128_t*>(this) ) );
    }
    ABORT("Unknown Type %d", static_cast<jau::nsize_t>(type));
    abort(); // never reached
}

uuid128_t::uuid128_t(uuid16_t const & uuid16, uuid128_t const & base_uuid, jau::nsize_t const uuid16_le_octet_index) noexcept
: uuid_t(TypeSize::UUID128_SZ), value(merge_uint128(uuid16.value, base_uuid.value, uuid16_le_octet_index)) {}

uuid128_t::uuid128_t(uuid32_t const & uuid32, uuid128_t const & base_uuid, jau::nsize_t const uuid32_le_octet_index) noexcept
: uuid_t(TypeSize::UUID128_SZ), value(merge_uint128(uuid32.value, base_uuid.value, uuid32_le_octet_index)) {}

std::string uuid16_t::toString() const noexcept {
    const jau::nsize_t length = 4;
    std::string str;
    str.reserve(length+1); // including EOS for snprintf
    str.resize(length);

    const jau::nsize_t count = snprintf(&str[0], str.capacity(), "%.4x", value);
    if( length != count ) {
        ABORT("UUID16 string not of length %d but %d", length, count);
    }
    return str;
}

std::string uuid16_t::toUUID128String(uuid128_t const & base_uuid, jau::nsize_t const le_octet_index) const noexcept
{
    uuid128_t u128(*this, base_uuid, le_octet_index);
    return u128.toString();
}

std::string uuid32_t::toString() const noexcept {
    const jau::nsize_t length = 8;
    std::string str;
    str.reserve(length+1); // including EOS for snprintf
    str.resize(length);

    const jau::nsize_t count = snprintf(&str[0], str.capacity(), "%.8x", value);
    if( length != count ) {
        ABORT("UUID32 string not of length %d but %d", length, count);
    }
    return str;
}

std::string uuid32_t::toUUID128String(uuid128_t const & base_uuid, jau::nsize_t const le_octet_index) const noexcept
{
    uuid128_t u128(*this, base_uuid, le_octet_index);
    return u128.toString();
}

// one static_assert is sufficient for whole compilation unit
static_assert( is_defined_endian(endian_t::native) );
static_assert( is_little_or_big_endian() );

std::string uuid128_t::toString() const noexcept {
    //               87654321-0000-1000-8000-00805F9B34FB
    //                   0      1    2    3      4    5
    // LE: low-mem - FB349B5F0800-0080-0010-0000-12345678 - high-mem
    //                  5    4      3    2    1      0
    //
    // BE: low-mem - 87654321-0000-1000-8000-00805F9B34FB - high-mem
    //                   0      1    2    3      4    5
    //
    const jau::nsize_t length = 36;
    std::string str;
    str.reserve(length+1); // including EOS for snprintf
    str.resize(length);
    uint32_t part0, part4;
    uint16_t part1, part2, part3, part5;

    // snprintf uses host data type, in which values are stored,
    // hence no endian conversion
    if( is_big_endian() ) {
        part0 = jau::get_uint32(value.data+  0);
        part1 = jau::get_uint16(value.data+  4);
        part2 = jau::get_uint16(value.data+  6);
        part3 = jau::get_uint16(value.data+  8);
        part4 = jau::get_uint32(value.data+ 10);
        part5 = jau::get_uint16(value.data+ 14);
    } else {
        part5 = jau::get_uint16(value.data+  0);
        part4 = jau::get_uint32(value.data+  2);
        part3 = jau::get_uint16(value.data+  6);
        part2 = jau::get_uint16(value.data+  8);
        part1 = jau::get_uint16(value.data+ 10);
        part0 = jau::get_uint32(value.data+ 12);
    }
    const jau::nsize_t count = snprintf(&str[0], str.capacity(), "%.8x-%.4x-%.4x-%.4x-%.8x%.4x",
                                part0, part1, part2, part3, part4, part5);
    if( length != count ) {
        ABORT("UUID128 string not of length %d but %d", length, count);
    }
    return str;
}

uuid16_t::uuid16_t(const std::string& str)
: uuid_t(TypeSize::UUID16_SZ), value(0)
{
    uint16_t part0;

    if( 4 != str.length() ) {
        std::string msg("UUID16 string not of length 4 but ");
        msg.append(std::to_string(str.length()));
        msg.append(": "+str);
        throw jau::IllegalArgumentError(msg, E_FILE_LINE);
    }
    if ( sscanf(str.c_str(), "%04hx", &part0) != 1 ) {
        std::string msg("UUID16 string not in format '0000' but "+str);
        throw jau::IllegalArgumentError(msg, E_FILE_LINE);
    }
    // sscanf provided host data type, in which we store the values,
    // hence no endian conversion
    value = part0;
}

uuid32_t::uuid32_t(const std::string& str)
: uuid_t(TypeSize::UUID32_SZ)
{
    uint32_t part0;

    if( 8 != str.length() ) {
        std::string msg("UUID32 string not of length 8 but ");
        msg.append(std::to_string(str.length()));
        msg.append(": "+str);
        throw jau::IllegalArgumentError(msg, E_FILE_LINE);
    }
    // if ( sscanf(str.c_str(), "%08x-%04hx-%04hx-%04hx-%08x%04hx",
    if ( sscanf(str.c_str(), "%08x", &part0) != 1 ) {
        std::string msg("UUID32 string not in format '00000000' but "+str);
        throw jau::IllegalArgumentError(msg, E_FILE_LINE);
    }
    // sscanf provided host data type, in which we store the values,
    // hence no endian conversion
    value = part0;
}

uuid128_t::uuid128_t(const std::string& str)
: uuid_t(TypeSize::UUID128_SZ)
{
    uint32_t part0, part4;
    uint16_t part1, part2, part3, part5;

    if( 36 != str.length() ) {
        std::string msg("UUID128 string not of length 36 but ");
        msg.append(std::to_string(str.length()));
        msg.append(": "+str);
        throw jau::IllegalArgumentError(msg, E_FILE_LINE);
    }
    if ( sscanf(str.c_str(), "%08x-%04hx-%04hx-%04hx-%08x%04hx",
                     &part0, &part1, &part2, &part3, &part4, &part5) != 6 )
    {
        std::string msg("UUID128 string not in format '00000000-0000-1000-8000-00805F9B34FB' but "+str);
        throw jau::IllegalArgumentError(msg, E_FILE_LINE);
    }

    // sscanf provided host data type, in which we store the values,
    // hence no endian conversion
    if( is_big_endian() ) {
        jau::put_uint32(value.data +  0, part0);
        jau::put_uint16(value.data +  4, part1);
        jau::put_uint16(value.data +  6, part2);
        jau::put_uint16(value.data +  8, part3);
        jau::put_uint32(value.data + 10, part4);
        jau::put_uint16(value.data + 14, part5);
    } else {
        jau::put_uint16(value.data +  0, part5);
        jau::put_uint32(value.data +  2, part4);
        jau::put_uint16(value.data +  6, part3);
        jau::put_uint16(value.data +  8, part2);
        jau::put_uint16(value.data + 10, part1);
        jau::put_uint32(value.data + 12, part0);
    }
}

