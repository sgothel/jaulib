/*
 * big_int_ops.hpp (this file)
 * (c) 2024 Gothel Software e.K.
 *
 * Includes code from: Botan Lowest Level MPI Algorithms (mp_asmi.h)
 * (C) 1999-2010 Jack Lloyd
 *     2006 Luca Piccarreta
 *
 * Includes code from: Botan MPI Algorithms (mp_core.h)
 * (C) 1999-2010,2018 Jack Lloyd
 *     2006 Luca Piccarreta
 *     2016 Matthias Gierlings
 *
 * jaulib including this code is released under the MIT License (see COPYING)
 * Botan itself is released under the Simplified BSD License (see COPYING)
 */

#ifndef JAU_BIG_INT_OPS_HPP_
#define JAU_BIG_INT_OPS_HPP_

#include <cstdint>
#include <cassert>

#include <jau/cpp_lang_util.hpp>
#include <jau/cpuid.hpp>
#include <jau/ct_utils.hpp>
#include <jau/math/math_error.hpp>

namespace jau::mp {
    namespace impl {
        constexpr size_t best_word_byte_size() {
            if constexpr ( 64 == jau::cpu::get_arch_psize() && is_builtin_int128_available() ) {
                return 8;
            } else {
                return 4;
            }
        }
    }
    #undef JAU_FORCE_MP_WORD_32_BITS
    #if !defined( JAU_FORCE_MP_WORD_32_BITS )
        constexpr const size_t mp_word_bits = impl::best_word_byte_size() * CHAR_BIT;
        typedef typename jau::uint_bytes<impl::best_word_byte_size()>::type mp_word_t;
        typedef typename jau::uint_bytes<impl::best_word_byte_size()*2>::type mp_dword_t;
        constexpr const bool has_mp_dword = is_builtin_int128_available();
    #elif 1
        constexpr const size_t mp_word_bits = 32;
        typedef uint32_t mp_word_t;
        constexpr const bool has_mp_dword = true;
        typedef uint64_t mp_dword_t;
    #elif 0
        constexpr const size_t mp_word_bits = 64;
        typedef uint64_t mp_word_t;
        #if defined(__SIZEOF_INT128__)
            constexpr const bool has_mp_dword = true;
            typedef uint128_t mp_dword_t;
        #else
            constexpr const bool has_mp_dword = false;
        #endif
    #endif
    constexpr const mp_word_t mp_word_max = ~static_cast<mp_word_t>(0);
}

namespace jau::mp::ops {

    int bigint_cmp(const mp_word_t x[], size_t x_size, const mp_word_t y[], size_t y_size) noexcept;

    inline mp_word_t word_add(const mp_word_t x, const mp_word_t y, mp_word_t& carry) noexcept {
       mp_word_t z = x + y;
       mp_word_t c1 = (z < x);
       z += carry;
       carry = c1 | (z < carry);
       return z;
    }

    inline mp_word_t word8_add2(mp_word_t x[8], const mp_word_t y[8], mp_word_t carry) noexcept {
        x[0] = word_add(x[0], y[0], carry);
        x[1] = word_add(x[1], y[1], carry);
        x[2] = word_add(x[2], y[2], carry);
        x[3] = word_add(x[3], y[3], carry);
        x[4] = word_add(x[4], y[4], carry);
        x[5] = word_add(x[5], y[5], carry);
        x[6] = word_add(x[6], y[6], carry);
        x[7] = word_add(x[7], y[7], carry);
        return carry;
    }

    inline mp_word_t word8_add3(mp_word_t z[8], const mp_word_t x[8], const mp_word_t y[8], mp_word_t carry) noexcept
    {
        z[0] = word_add(x[0], y[0], carry);
        z[1] = word_add(x[1], y[1], carry);
        z[2] = word_add(x[2], y[2], carry);
        z[3] = word_add(x[3], y[3], carry);
        z[4] = word_add(x[4], y[4], carry);
        z[5] = word_add(x[5], y[5], carry);
        z[6] = word_add(x[6], y[6], carry);
        z[7] = word_add(x[7], y[7], carry);
        return carry;
    }

    inline mp_word_t word_sub(mp_word_t x, mp_word_t y, mp_word_t& carry) noexcept {
        mp_word_t t0 = x - y;
        mp_word_t c1 = (t0 > x);
        mp_word_t z = t0 - carry;
        carry = c1 | (z > t0);
        return z;
    }

    inline mp_word_t word8_sub2(mp_word_t x[8], const mp_word_t y[8], mp_word_t carry) noexcept {
        x[0] = word_sub(x[0], y[0], carry);
        x[1] = word_sub(x[1], y[1], carry);
        x[2] = word_sub(x[2], y[2], carry);
        x[3] = word_sub(x[3], y[3], carry);
        x[4] = word_sub(x[4], y[4], carry);
        x[5] = word_sub(x[5], y[5], carry);
        x[6] = word_sub(x[6], y[6], carry);
        x[7] = word_sub(x[7], y[7], carry);
        return carry;
    }

    inline mp_word_t word8_sub2_rev(mp_word_t x[8], const mp_word_t y[8], mp_word_t carry) noexcept {
        x[0] = word_sub(y[0], x[0], carry);
        x[1] = word_sub(y[1], x[1], carry);
        x[2] = word_sub(y[2], x[2], carry);
        x[3] = word_sub(y[3], x[3], carry);
        x[4] = word_sub(y[4], x[4], carry);
        x[5] = word_sub(y[5], x[5], carry);
        x[6] = word_sub(y[6], x[6], carry);
        x[7] = word_sub(y[7], x[7], carry);
        return carry;
    }

    inline mp_word_t word8_sub3(mp_word_t z[8], const mp_word_t x[8], const mp_word_t y[8], mp_word_t carry) noexcept {
        z[0] = word_sub(x[0], y[0], carry);
        z[1] = word_sub(x[1], y[1], carry);
        z[2] = word_sub(x[2], y[2], carry);
        z[3] = word_sub(x[3], y[3], carry);
        z[4] = word_sub(x[4], y[4], carry);
        z[5] = word_sub(x[5], y[5], carry);
        z[6] = word_sub(x[6], y[6], carry);
        z[7] = word_sub(x[7], y[7], carry);
        return carry;
    }

    /** 64x64->128 bit multiplication */
    inline void mul64x64_128(const uint64_t a, const uint64_t b, uint64_t& lo, uint64_t& hi) noexcept {
        if constexpr ( is_builtin_int128_available() ) {
            const uint128_t r = static_cast<uint128_t>(a) * b;
            hi = (r >> 64) & 0xFFFFFFFFFFFFFFFFUL;
            lo = (r      ) & 0xFFFFFFFFFFFFFFFFUL;
        } else {
            /*
             * Do a 64x64->128 multiply using four 32x32->64 multiplies plus
             * some adds and shifts. Last resort for CPUs like UltraSPARC (with
             * 64-bit registers/ALU, but no 64x64->128 multiply) or 32-bit CPUs.
             */
            constexpr const size_t HWORD_BITS = 32;
            constexpr const uint32_t HWORD_MASK = 0xFFFFFFFFU;

            const uint32_t a_hi = (a >> HWORD_BITS);
            const uint32_t a_lo = (a  & HWORD_MASK);
            const uint32_t b_hi = (b >> HWORD_BITS);
            const uint32_t b_lo = (b  & HWORD_MASK);

            uint64_t x0 = static_cast<uint64_t>(a_hi) * b_hi;
            uint64_t x1 = static_cast<uint64_t>(a_lo) * b_hi;
            uint64_t x2 = static_cast<uint64_t>(a_hi) * b_lo;
            uint64_t x3 = static_cast<uint64_t>(a_lo) * b_lo;

            // this cannot overflow as (2^32-1)^2 + 2^32-1 < 2^64-1
            x2 += x3 >> HWORD_BITS;

            // this one can overflow
            x2 += x1;

            // propagate the carry if any
            x0 += static_cast<uint64_t>(static_cast<bool>(x2 < x1)) << HWORD_BITS;

            hi = x0 + (x2 >> HWORD_BITS);
            lo  = ((x2 & HWORD_MASK) << HWORD_BITS) + (x3 & HWORD_MASK);
        }
    }

    /** Word Multiply/Add */
    inline mp_word_t word_madd2(mp_word_t a, mp_word_t b, mp_word_t& c) noexcept {
        if constexpr ( has_mp_dword ) {
            const mp_dword_t s = static_cast<mp_dword_t>(a) * b + c;
            c = static_cast<mp_word_t>(s >> mp_word_bits);
            return static_cast<mp_word_t>(s);
        } else if constexpr ( 64 == mp_word_bits ) {
            uint64_t hi, lo;

            mul64x64_128(a, b, lo, hi);

            lo += c;
            hi += (lo < c); // carry?

            c = hi;
            return lo;
        } else {
            assert( has_mp_dword || 64 == mp_word_bits );
            return 0;
        }
    }

    /** Word Multiply/Add */
    inline mp_word_t word_madd3(mp_word_t a, mp_word_t b, mp_word_t c, mp_word_t& d) noexcept {
        if constexpr ( has_mp_dword ) {
            const mp_dword_t s = static_cast<mp_dword_t>(a) * b + c + d;
            d = static_cast<mp_word_t>(s >> mp_word_bits);
            return static_cast<mp_word_t>(s);
        } else if constexpr ( 64 == mp_word_bits ) {
            uint64_t hi, lo;

            mul64x64_128(a, b, lo, hi);

            lo += c;
            hi += (lo < c); // carry?

            lo += d;
            hi += (lo < d); // carry?

            d = hi;
            return lo;
        } else {
            assert( has_mp_dword || 64 == mp_word_bits );
            return 0;
        }
    }

    /*
     * Eight Word Block Multiply/Add
     */
    inline mp_word_t word8_madd3(mp_word_t z[8], const mp_word_t x[8], mp_word_t y, mp_word_t carry) noexcept {
        z[0] = word_madd3(x[0], y, z[0], carry);
        z[1] = word_madd3(x[1], y, z[1], carry);
        z[2] = word_madd3(x[2], y, z[2], carry);
        z[3] = word_madd3(x[3], y, z[3], carry);
        z[4] = word_madd3(x[4], y, z[4], carry);
        z[5] = word_madd3(x[5], y, z[5], carry);
        z[6] = word_madd3(x[6], y, z[6], carry);
        z[7] = word_madd3(x[7], y, z[7], carry);
        return carry;
    }

    /*
     * Eight Word Block Linear Multiplication
     */
    inline mp_word_t word8_linmul2(mp_word_t x[8], mp_word_t y, mp_word_t carry) noexcept {
        x[0] = word_madd2(x[0], y, carry);
        x[1] = word_madd2(x[1], y, carry);
        x[2] = word_madd2(x[2], y, carry);
        x[3] = word_madd2(x[3], y, carry);
        x[4] = word_madd2(x[4], y, carry);
        x[5] = word_madd2(x[5], y, carry);
        x[6] = word_madd2(x[6], y, carry);
        x[7] = word_madd2(x[7], y, carry);
        return carry;
    }

    /*
     * Eight Word Block Linear Multiplication
     */
    inline mp_word_t word8_linmul3(mp_word_t z[8], const mp_word_t x[8], mp_word_t y, mp_word_t carry) noexcept {
        z[0] = word_madd2(x[0], y, carry);
        z[1] = word_madd2(x[1], y, carry);
        z[2] = word_madd2(x[2], y, carry);
        z[3] = word_madd2(x[3], y, carry);
        z[4] = word_madd2(x[4], y, carry);
        z[5] = word_madd2(x[5], y, carry);
        z[6] = word_madd2(x[6], y, carry);
        z[7] = word_madd2(x[7], y, carry);
        return carry;
    }

    /** Two operand addition with carry out */
    inline mp_word_t bigint_add2(mp_word_t x[], size_t x_size, const mp_word_t y[], size_t y_size) noexcept {
        mp_word_t carry = 0;

        assert(x_size >= y_size);

        const size_t blocks = y_size - (y_size % 8);

        for(size_t i = 0; i != blocks; i += 8) {
            carry = word8_add2(x + i, y + i, carry);
        }
        for(size_t i = blocks; i != y_size; ++i) {
            x[i] = word_add(x[i], y[i], carry);
        }
        for(size_t i = y_size; i != x_size; ++i) {
            x[i] = word_add(x[i], 0, carry);
        }
        return carry;
    }

    /** Three operand addition with carry out */
    inline mp_word_t bigint_add3_nc(mp_word_t z[], const mp_word_t x[], size_t x_size, const mp_word_t y[], size_t y_size) noexcept {
        if(x_size < y_size)
        { return bigint_add3_nc(z, y, y_size, x, x_size); }

        mp_word_t carry = 0;

        const size_t blocks = y_size - (y_size % 8);

        for(size_t i = 0; i != blocks; i += 8) {
            carry = word8_add3(z + i, x + i, y + i, carry);
        }
        for(size_t i = blocks; i != y_size; ++i) {
            z[i] = word_add(x[i], y[i], carry);
        }
        for(size_t i = y_size; i != x_size; ++i) {
            z[i] = word_add(x[i], 0, carry);
        }
        return carry;
    }
    /** Three operand addition */
    inline void bigint_add3(mp_word_t z[], const mp_word_t x[], size_t x_size, const mp_word_t y[], size_t y_size) noexcept {
        z[x_size > y_size ? x_size : y_size] +=
                bigint_add3_nc(z, x, x_size, y, y_size);
    }

    /** Two operand subtraction */
    inline mp_word_t bigint_sub2(mp_word_t x[], size_t x_size, const mp_word_t y[], size_t y_size) noexcept {
        mp_word_t borrow = 0;

        assert(x_size >= y_size);

        const size_t blocks = y_size - (y_size % 8);

        for(size_t i = 0; i != blocks; i += 8) {
            borrow = word8_sub2(x + i, y + i, borrow);
        }
        for(size_t i = blocks; i != y_size; ++i) {
            x[i] = word_sub(x[i], y[i], borrow);
        }
        for(size_t i = y_size; i != x_size; ++i) {
            x[i] = word_sub(x[i], 0, borrow);
        }
        return borrow;
    }

    /** Two operand subtraction, x = y - x; assumes y >= x */
    inline void bigint_sub2_rev(mp_word_t x[], const mp_word_t y[], size_t y_size) noexcept {
        mp_word_t borrow = 0;

        const size_t blocks = y_size - (y_size % 8);

        for(size_t i = 0; i != blocks; i += 8) {
            borrow = word8_sub2_rev(x + i, y + i, borrow);
        }
        for(size_t i = blocks; i != y_size; ++i) {
            x[i] = word_sub(y[i], x[i], borrow);
        }
        assert(borrow == 0); // y must be greater than x
    }

    /** Three operand subtraction */
    inline mp_word_t bigint_sub3(mp_word_t z[], const mp_word_t x[], size_t x_size, const mp_word_t y[], size_t y_size) noexcept {
        mp_word_t borrow = 0;

        assert(x_size >= y_size); // Expected sizes

        const size_t blocks = y_size - (y_size % 8);

        for(size_t i = 0; i != blocks; i += 8) {
            borrow = word8_sub3(z + i, x + i, y + i, borrow);
        }
        for(size_t i = blocks; i != y_size; ++i) {
            z[i] = word_sub(x[i], y[i], borrow);
        }
        for(size_t i = y_size; i != x_size; ++i) {
            z[i] = word_sub(x[i], 0, borrow);
        }
        return borrow;
    }

    /**
    * Set z to abs(x-y), ie if x >= y, then compute z = x - y
    * Otherwise compute z = y - x
    * No borrow is possible since the result is always >= 0
    *
    * Return the relative size of x vs y (-1, 0, 1)
    *
    * @param z output array of max(x_size,y_size) words
    * @param x input param
    * @param x_size length of x
    * @param y input param
    * @param y_size length of y
    */
    inline int32_t
    bigint_sub_abs(mp_word_t z[], const mp_word_t x[], size_t x_size, const mp_word_t y[], size_t y_size) noexcept {
        const int32_t relative_size = bigint_cmp(x, x_size, y, y_size);

        // Swap if relative_size == -1
        const bool need_swap = relative_size < 0;
        CT::conditional_swap_ptr(need_swap, x, y);
        CT::conditional_swap(need_swap, x_size, y_size);

        /*
         * We know at this point that x >= y so if y_size is larger than
         * x_size, we are guaranteed they are just leading zeros which can
         * be ignored
         */
        y_size = std::min(x_size, y_size);

        bigint_sub3(z, x, x_size, y, y_size);

        return relative_size;
    }

    /*
     * Linear Multiply - returns the carry
     */
    [[nodiscard]] inline mp_word_t bigint_linmul2(mp_word_t x[], size_t x_size, mp_word_t y) noexcept {
        const size_t blocks = x_size - (x_size % 8);

        mp_word_t carry = 0;

        for(size_t i = 0; i != blocks; i += 8) {
            carry = word8_linmul2(x + i, y, carry);
        }
        for(size_t i = blocks; i != x_size; ++i) {
            x[i] = word_madd2(x[i], y, carry);
        }
        return carry;
    }

    inline void bigint_linmul3(mp_word_t z[], const mp_word_t x[], size_t x_size, mp_word_t y) noexcept {
        const size_t blocks = x_size - (x_size % 8);

        mp_word_t carry = 0;

        for(size_t i = 0; i != blocks; i += 8) {
            carry = word8_linmul3(z + i, x + i, y, carry);
        }
        for(size_t i = blocks; i != x_size; ++i) {
            z[i] = word_madd2(x[i], y, carry);
        }
        z[x_size] = carry;
    }

    inline void bigint_shl1(mp_word_t x[], size_t x_size, size_t x_words, size_t word_shift, size_t bit_shift) noexcept {
        std::memmove(x + word_shift, x, sizeof(mp_word_t)*x_words);
        std::memset(x, 0, sizeof(mp_word_t)*word_shift);

        const auto carry_mask = CT::Mask<mp_word_t>::expand(bit_shift);
        const size_t carry_shift = carry_mask.if_set_return(mp_word_bits - bit_shift);

        mp_word_t carry = 0;
        for(size_t i = word_shift; i != x_size; ++i) {
            const mp_word_t w = x[i];
            x[i] = (w << bit_shift) | carry;
            carry = carry_mask.if_set_return(w >> carry_shift);
        }
    }

    inline void bigint_shr1(mp_word_t x[], size_t x_size, size_t word_shift, size_t bit_shift) noexcept {
        const size_t top = x_size >= word_shift ? (x_size - word_shift) : 0;

        if(top > 0) {
            std::memmove(x, x + word_shift, sizeof(mp_word_t)*top);
        }
        std::memset(x + top, 0, sizeof(mp_word_t)*jau::min(word_shift, x_size));

        const auto carry_mask = CT::Mask<mp_word_t>::expand(bit_shift);
        const size_t carry_shift = carry_mask.if_set_return(mp_word_bits - bit_shift);

        mp_word_t carry = 0;
        for(size_t i = 0; i != top; ++i) {
            const mp_word_t w = x[top - i - 1];
            x[top-i-1] = (w >> bit_shift) | carry;
            carry = carry_mask.if_set_return(w << carry_shift);
        }
    }

    inline void bigint_shl2(mp_word_t y[], const mp_word_t x[], size_t x_size, size_t word_shift, size_t bit_shift) noexcept {
        std::memmove(y + word_shift, x, sizeof(mp_word_t)*x_size);

        const auto carry_mask = CT::Mask<mp_word_t>::expand(bit_shift);
        const size_t carry_shift = carry_mask.if_set_return(mp_word_bits - bit_shift);

        mp_word_t carry = 0;
        for(size_t i = word_shift; i != x_size + word_shift + 1; ++i) {
            const mp_word_t w = y[i];
            y[i] = (w << bit_shift) | carry;
            carry = carry_mask.if_set_return(w >> carry_shift);
        }
    }
    inline void bigint_shr2(mp_word_t y[], const mp_word_t x[], size_t x_size, size_t word_shift, size_t bit_shift) noexcept {
        const size_t new_size = x_size < word_shift ? 0 : (x_size - word_shift);

        if(new_size > 0) {
            std::memmove(y, x + word_shift, sizeof(mp_word_t)*new_size);
        }
        const auto carry_mask = CT::Mask<mp_word_t>::expand(bit_shift);
        const size_t carry_shift = carry_mask.if_set_return(mp_word_bits - bit_shift);

        mp_word_t carry = 0;
        for(size_t i = new_size; i > 0; --i) {
            mp_word_t w = y[i-1];
            y[i-1] = (w >> bit_shift) | carry;
            carry = carry_mask.if_set_return(w << carry_shift);
        }
    }

    /*
     * O(n*n) multiplication
     */
    void basecase_mul(mp_word_t z[], size_t z_size,
                      const mp_word_t x[], size_t x_size,
                      const mp_word_t y[], size_t y_size) noexcept
    {
        assert(z_size >= x_size + y_size); // z_size too small

        const size_t x_size_8 = x_size - (x_size % 8);

        for(size_t i=0; i<z_size; ++i) { z[i]=0; }

        for(size_t i = 0; i != y_size; ++i) {
            const mp_word_t y_i = y[i];

            mp_word_t carry = 0;

            for(size_t j = 0; j != x_size_8; j += 8) {
                carry = word8_madd3(z + i + j, x + j, y_i, carry);
            }
            for(size_t j = x_size_8; j != x_size; ++j) {
                z[i+j] = word_madd3(x[j], y_i, z[i+j], carry);
            }
            z[x_size+i] = carry;
        }
    }

    /** Computes ((n1<<bits) + n0) / d */
    inline mp_word_t bigint_divop(mp_word_t n1, mp_word_t n0, mp_word_t d) {
        if(d == 0) {
            throw jau::math::MathDivByZeroError("d == 0", E_FILE_LINE);
        }
        if constexpr ( has_mp_dword ) {
            return static_cast<mp_word_t>( ( ( static_cast<mp_dword_t>(n1) << mp_word_bits ) | n0 ) / d );
        } else {
            mp_word_t high = n1 % d;
            mp_word_t quotient = 0;

            for(size_t i = 0; i != mp_word_bits; ++i) {
                const mp_word_t high_top_bit = high >> ( mp_word_bits - 1 );

                high <<= 1;
                high |= (n0 >> (mp_word_bits-1-i)) & 1;
                quotient <<= 1;

                if(high_top_bit || high >= d) {
                    high -= d;
                    quotient |= 1;
                }
            }
            return quotient;
        }
    }

    /** Compute ((n1<<bits) + n0) % d */
    inline mp_word_t bigint_modop(mp_word_t n1, mp_word_t n0, mp_word_t d) {
        if(d == 0) {
            throw jau::math::MathDivByZeroError("d == 0", E_FILE_LINE);
        }
        if constexpr ( has_mp_dword ) {
            return ( ( static_cast<mp_dword_t>(n1) << mp_word_bits) | n0 ) % d;
        } else {
            mp_word_t z = bigint_divop(n1, n0, d);
            mp_word_t dummy = 0;
            z = word_madd2(z, d, dummy);
            return (n0-z);
        }
    }

    inline CT::Mask<mp_word_t>
    bigint_ct_is_eq(const mp_word_t x[], size_t x_size, const mp_word_t y[], size_t y_size) noexcept {
        const size_t common_elems = std::min(x_size, y_size);
        mp_word_t diff = 0;

        for(size_t i = 0; i != common_elems; i++) {
            diff |= (x[i] ^ y[i]);
        }

        // If any bits were set in high part of x/y, then they are not equal
        if(x_size < y_size)
        {
            for(size_t i = x_size; i != y_size; i++) {
                diff |= y[i];
            }
        } else if(y_size < x_size) {
            for(size_t i = y_size; i != x_size; i++) {
                diff |= x[i];
            }
        }
        return CT::Mask<mp_word_t>::is_zero(diff);
    }

    /**
     * Compare x and y
     * Return ~0 if x[0:x_size] < y[0:y_size] or 0 otherwise
     * If lt_or_equal is true, returns ~0 also for x == y
     */
    inline CT::Mask<mp_word_t>
    bigint_ct_is_lt(const mp_word_t x[], size_t x_size, const mp_word_t y[], size_t y_size, bool lt_or_equal = false) noexcept
    {
        const size_t common_elems = jau::min(x_size, y_size);

        auto is_lt = CT::Mask<mp_word_t>::expand(lt_or_equal);

        for(size_t i = 0; i != common_elems; i++)
        {
            const auto eq = CT::Mask<mp_word_t>::is_equal(x[i], y[i]);
            const auto lt = CT::Mask<mp_word_t>::is_lt(x[i], y[i]);
            is_lt = eq.select_mask(is_lt, lt);
        }

        if(x_size < y_size) {
            mp_word_t mask = 0;
            for(size_t i = x_size; i != y_size; i++) {
                mask |= y[i];
            }
            // If any bits were set in high part of y, then is_lt should be forced true
            is_lt |= CT::Mask<mp_word_t>::expand(mask);
        } else if(y_size < x_size) {
            mp_word_t mask = 0;
            for(size_t i = y_size; i != x_size; i++) {
                mask |= x[i];
            }

            // If any bits were set in high part of x, then is_lt should be false
            is_lt &= CT::Mask<mp_word_t>::is_zero(mask);
        }

        return is_lt;
    }

    /**
     * Compare unsigned x and y mp_word_t
     *
     * Returns
     * - -1 if x < y
     * -  0 if x == y
     * -  1 if x > y
     */
    inline int bigint_cmp(const mp_word_t x[], size_t x_size, const mp_word_t y[], size_t y_size) noexcept
    {
        constexpr const mp_word_t LT = static_cast<mp_word_t>(-1);
        constexpr const mp_word_t EQ = 0;
        constexpr const mp_word_t GT = 1;
        const size_t common_elems = jau::min(x_size, y_size);

        mp_word_t result = EQ; // until found otherwise

        for(size_t i = 0; i != common_elems; i++) {
            const auto is_eq = CT::Mask<mp_word_t>::is_equal(x[i], y[i]);
            const auto is_lt = CT::Mask<mp_word_t>::is_lt(x[i], y[i]);
            result = is_eq.select(result, is_lt.select(LT, GT));
        }
        if(x_size < y_size) {
            mp_word_t mask = 0;
            for(size_t i = x_size; i != y_size; i++) {
                mask |= y[i];
            }
            // If any bits were set in high part of y, then x < y
            result = CT::Mask<mp_word_t>::is_zero(mask).select(result, LT);
        } else if(y_size < x_size) {
            mp_word_t mask = 0;
            for(size_t i = y_size; i != x_size; i++) {
                mask |= x[i];
            }
            // If any bits were set in high part of x, then x > y
            result = CT::Mask<mp_word_t>::is_zero(mask).select(result, GT);
        }
        CT::unpoison(result);
        return static_cast<int>(result);
    }

} // namespace jau::mp::ops

#endif /* JAU_BIG_INT_OPS_HPP_ */
