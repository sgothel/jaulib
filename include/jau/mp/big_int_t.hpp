/**
 * big_int_t (this file)
 * (c) 2024 Gothel Software e.K.
 *
 * Includes code from: Botan BigInt (bigint.h, ..)
 * (C) 1999-2011,2012,2014,2019 Jack Lloyd
 *     2007 FlexSecure
 *
 * Includes code from: Botan Division Algorithms (divide.h)
 * (C) 1999-2007,2012,2018,2021 Jack Lloyd
 *
 * jaulib including this code is released under the MIT License (see COPYING)
 * Botan itself is released under the Simplified BSD License (see COPYING)
 */
#include <cstdint>
#include <cstdarg>
#include <limits>
#include <cassert>

#include <jau/mp/big_int_ops.hpp>
#include <jau/byte_util.hpp>
#include <jau/string_util.hpp>

namespace jau::mp {

    class big_int_t {
        public:
            static constexpr const nsize_t base = (nsize_t)std::numeric_limits<mp_word_t>::max() + (nsize_t)1;

            /**
             * Sign symbol definitions for positive and negative numbers
             */
            enum Sign { Negative = 0, Positive = 1 };


        public:
            big_int_t() noexcept = default;

            big_int_t(const big_int_t& o) noexcept = default;

            ~big_int_t() noexcept {
                clear();
            }

            /**
            * Create a 0-value big_int
            */
            static big_int_t zero() { return big_int_t(); }

            /**
            * Create a 1-value big_int
            */
            static big_int_t one() { return big_int_t::from_word(1); }

            /**
            * Create big_int from an unsigned 64 bit integer
            * @param n initial value of this big_int
            */
            static big_int_t from_u64(uint64_t n) {
                big_int_t bn;
                if( 64 == mp_word_bits ) {
                    bn.set_word_at(0, n);
                } else {
                    bn.set_word_at(1, static_cast<mp_word_t>(n >> 32));
                    bn.set_word_at(0, static_cast<mp_word_t>(n));
                }
                return bn;
            }

            /**
            * Create big_int from a mp_word_t (limb)
            * @param n initial value of this big_int
            */
            static big_int_t from_word(mp_word_t n) {
                big_int_t bn;
                bn.set_word_at(0, n);
                return bn;
            }

            /**
            * Create big_int from a signed 32 bit integer
            * @param n initial value of this big_int
            */
            static big_int_t from_s32(int32_t n) {
                if(n >= 0) {
                   return big_int_t::from_u64(static_cast<uint64_t>(n));
                } else {
                   return -big_int_t::from_u64(static_cast<uint64_t>(-n));
                }
            }

            /**
            * Create big_int of specified size, all zeros
            * @param n size of the internal register in words
            */
            static big_int_t with_capacity(nsize_t n) {
                big_int_t bn;
                bn.grow_to(n);
                return bn;
            }

            /**
            * Create a power of two
            * @param n the power of two to create
            * @return bigint representing 2^n
            */
            static big_int_t power_of_2(size_t n) {
                big_int_t b;
                b.set_bit(n);
                return b;
            }

            /**
            * Create big_int_t from an unsigned 64 bit integer
            * @param n initial value of this big_int_t
            */
            big_int_t(uint64_t n) {
                if( 64 == mp_word_bits ) {
                    m_data.set_word_at(0, n);
                } else {
                    m_data.set_word_at(1, static_cast<mp_word_t>(n >> 32));
                    m_data.set_word_at(0, static_cast<mp_word_t>(n));
                }
            }

            big_int_t(big_int_t&& other) {
                this->swap(other);
            }

            big_int_t& operator=(const big_int_t& r) noexcept = default;

            big_int_t& operator=(big_int_t&& other) noexcept {
                if(this != &other) {
                    this->swap(other);
                }
                return *this;
            }

            /**
            * Swap this value with another
            * @param other big_int to swap values with
            */
            void swap(big_int_t& other) noexcept {
               m_data.swap(other.m_data);
               std::swap(m_signedness, other.m_signedness);
            }

            void swap_reg(std::vector<mp_word_t>& reg) noexcept {
               m_data.swap(reg);
               // sign left unchanged
            }

            /** Unary negation operator, returns new negative instance of this. */
            big_int_t operator-() const noexcept { return big_int_t(*this).flip_sign(); }

            /**
             * @param n the offset to get a byte from
             * @result byte at offset n
             */
            uint8_t byte_at(nsize_t n) const noexcept {
                return get_byte_var(sizeof(mp_word_t) - (n % sizeof(mp_word_t)) - 1,
                                    word_at(n / sizeof(mp_word_t)));
            }

            /**
             * Return the mp_word_t at a specified position of the internal register
             * @param n position in the register
             * @return value at position n
             */
            mp_word_t word_at(nsize_t n) const noexcept {
                return m_data.get_word_at(n);
            }

            void set_word_at(nsize_t i, mp_word_t w) {
                m_data.set_word_at(i, w);
            }

            void set_words(const mp_word_t w[], nsize_t len) {
                m_data.set_words(w, len);
            }

            /**
             * Tests if the sign of the integer is negative
             * @result true, iff the integer has a negative sign
             */
            bool is_negative() const noexcept { return sign() == Negative; }

            /**
             * Tests if the sign of the integer is positive
             * @result true, iff the integer has a positive sign
             */
            bool is_positive() const noexcept { return sign() == Positive; }

            /**
             * Return the sign of the integer
             * @result the sign of the integer
             */
            Sign sign() const noexcept { return (m_signedness); }

            /**
             * @result the opposite sign of the represented integer value
             */
            Sign reverse_sign() const noexcept {
                if(sign() == Positive) {
                    return Negative;
                }
                return Positive;
            }

            /**
             * Flip the sign of this big_int
             */
            big_int_t& flip_sign() noexcept {
                return set_sign(reverse_sign());
            }

            /**
             * Set sign of the integer
             * @param sign new Sign to set
             */
            big_int_t& set_sign(Sign sign) noexcept {
                if(sign == Negative && is_zero()) {
                    sign = Positive;
                }
                m_signedness = sign;
                return *this;
            }

            /** Returns absolute (positive) value of this instance */
            big_int_t abs() const noexcept {
                return big_int_t(*this).set_sign(Positive);
            }

            /**
             * Give size of internal register
             * @result size of internal register in words
             */
            nsize_t size() const noexcept { return m_data.size(); }

            /**
             * Return how many words we need to hold this value
             * @result significant words of the represented integer value
             */
            nsize_t sig_words() const noexcept { return m_data.sig_words(); }

            /** Returns byte length of the integer */
            nsize_t bytes() const { return jau::round_up(bits(), 8U) / 8U; }

            /** Returns bit length of the integer */
            nsize_t bits() const noexcept {
                const nsize_t words = sig_words();
                if(words == 0) {
                    return 0;
                }
                const nsize_t full_words = (words - 1) * mp_word_bits;
                const nsize_t top_bits = mp_word_bits - top_bits_free();
                return full_words + top_bits;
            }

            /**
            * Return a mutable pointer to the register
            * @result a pointer to the start of the internal register
            */
            mp_word_t* mutable_data() { return m_data.mutable_data(); }

            /**
            * Return a const pointer to the register
            * @result a pointer to the start of the internal register
            */
            const mp_word_t* data() const { return m_data.const_data(); }

            /**
            * Zeroize the big_int. The size of the underlying register is not
            * modified.
            */
            void clear() { m_data.set_to_zero(); m_signedness = Positive; }

            /**
             * Compares this instance against other considering both sign() value
             * Returns
             * - -1 if this < other
             * -  0 if this == other
             * -  1 if this > other
             */
            int compare(const big_int_t& b) const noexcept {
                return cmp(b, true);
            }

            /**
             * Test if the integer has an even value
             * @result true if the integer is even, false otherwise
             */
            bool is_even() const noexcept { return get_bit(0) == 0; }

            /**
             * Test if the integer has an odd value
             * @result true if the integer is odd, false otherwise
             */
            bool is_odd()  const noexcept { return get_bit(0) == 1; }

            /**
             * Test if the integer is not zero
             * @result true if the integer is non-zero, false otherwise
             */
            bool is_nonzero() const noexcept { return !is_zero(); }

            /**
             * Test if the integer is zero
             * @result true if the integer is zero, false otherwise
             */
            bool is_zero() const noexcept {
                return sig_words() == 0;
            }

            /**
             * Return bit value at specified position
             * @param n the bit offset to test
             * @result true, if the bit at position n is set, false otherwise
             */
            bool get_bit(nsize_t n) const noexcept {
                return (word_at(n / mp_word_bits) >> (n % mp_word_bits)) & 1;
            }

            /**
             * Set bit at specified position
             * @param n bit position to set
             */
            void set_bit(nsize_t n) noexcept {
                conditionally_set_bit(n, true);
            }

            /**
             * Conditionally set bit at specified position. Note if set_it is
             * false, nothing happens, and if the bit is already set, it
             * remains set.
             *
             * @param n bit position to set
             * @param set_it if the bit should be set
             */
            void conditionally_set_bit(nsize_t n, bool set_it) noexcept {
                const nsize_t which = n / mp_word_bits;
                const mp_word_t mask = static_cast<mp_word_t>(set_it) << (n % mp_word_bits);
                m_data.set_word_at(which, word_at(which) | mask);
            }

            /** ! operator, returns `true` iff this is zero, otherwise false. */
            bool operator !() const noexcept { return is_zero(); }

            bool operator==(const big_int_t& b) const noexcept { return is_equal(b); }
            bool operator!=(const big_int_t& b) const noexcept { return !is_equal(b); }
            bool operator<=(const big_int_t& b) const noexcept { return cmp(b) <= 0; }
            bool operator>=(const big_int_t& b) const noexcept { return cmp(b) >= 0; }
            bool operator<(const big_int_t& b)  const noexcept { return is_less_than(b); }
            bool operator>(const big_int_t& b)  const noexcept { return b.is_less_than(*this); }
#if 0
            std::strong_ordering operator<=>(const big_int_t& b) const noexcept {
                const int r = cmp(b);
                return 0 == r ? std::strong_ordering::equal : ( 0 > r ? std::strong_ordering::less : std::strong_ordering::greater);
            }
#endif

            big_int_t& operator++() noexcept { return *this += 1; }

            big_int_t& operator--() noexcept { return *this -= 1; }

            big_int_t  operator++(int) noexcept { big_int_t x = (*this); ++(*this); return x; }

            big_int_t  operator--(int) noexcept { big_int_t x = (*this); --(*this); return x; }

            big_int_t& operator+=(const big_int_t& y ) noexcept {
                return add(y.data(), y.sig_words(), y.sign());
            }

            big_int_t& operator-=(const big_int_t& y ) noexcept {
                return add(y.data(), y.sig_words(), y.sign() == Positive ? Negative : Positive);
            }

            big_int_t operator+(const big_int_t& y ) noexcept {
                return add2(*this, y.data(), y.sig_words(), y.sign());
            }

            big_int_t operator-(const big_int_t& y ) noexcept {
                return add2(*this, y.data(), y.sig_words(), y.reverse_sign());
            }

            big_int_t& operator<<=(nsize_t shift) noexcept {
                const nsize_t shift_words = shift / mp_word_bits;
                const nsize_t shift_bits  = shift % mp_word_bits;
                const nsize_t size = sig_words();

                const nsize_t bits_free = top_bits_free();

                const nsize_t new_size = size + shift_words + (bits_free < shift_bits);

                m_data.grow_to(new_size);

                ops::bigint_shl1(m_data.mutable_data(), new_size, size, shift_words, shift_bits);

                return *this;
            }

            big_int_t& operator>>=(nsize_t shift) noexcept {
                const nsize_t shift_words = shift / mp_word_bits;
                const nsize_t shift_bits  = shift % mp_word_bits;

                ops::bigint_shr1(m_data.mutable_data(), m_data.size(), shift_words, shift_bits);

                if(is_negative() && is_zero()) {
                   set_sign(Positive);
                }
                return *this;
            }

            big_int_t operator<<(nsize_t shift) const {
               const nsize_t shift_words = shift / mp_word_bits;
               const nsize_t shift_bits  = shift % mp_word_bits;
               const nsize_t x_sw = sig_words();

               big_int_t y = big_int_t::with_capacity(x_sw + shift_words + (shift_bits ? 1 : 0));
               ops::bigint_shl2(y.mutable_data(), data(), x_sw, shift_words, shift_bits);
               y.set_sign(sign());
               return y;
            }

            big_int_t operator>>(nsize_t shift) const {
                const nsize_t shift_words = shift / mp_word_bits;
                const nsize_t shift_bits  = shift % mp_word_bits;
                const nsize_t x_sw = sig_words();

                if(shift_words >= x_sw) {
                   return big_int_t::zero();
                }

                big_int_t y = big_int_t::with_capacity(x_sw - shift_words);
                ops::bigint_shr2(y.mutable_data(), data(), x_sw, shift_words, shift_bits);

                if(is_negative() && y.is_zero()) {
                   y.set_sign(big_int_t::Positive);
                } else {
                   y.set_sign(sign());
                }
                return y;
            }


            big_int_t& operator*=(const big_int_t& y) noexcept {
                std::vector<mp_word_t> ws;
                return this->mul(y, ws);
            }

            big_int_t operator*(const big_int_t& y) noexcept
            {
                const nsize_t x_sw = sig_words();
                const nsize_t y_sw = y.sig_words();

                big_int_t z;
                z.resize(size() + y.size());

                if(x_sw == 1 && y_sw) {
                    ops::bigint_linmul3(z.mutable_data(), y.data(), y_sw, word_at(0));
                } else if(y_sw == 1 && x_sw) {
                    ops::bigint_linmul3(z.mutable_data(), data(), x_sw, y.word_at(0));
                } else if(x_sw && y_sw) {
                    ops::basecase_mul(z.mutable_data(), z.size(), data(), x_sw, y.data(), y_sw);
                }
                z.cond_flip_sign(x_sw > 0 && y_sw > 0 && sign() != y.sign());
                return z;
            }

            big_int_t& operator/=(const big_int_t& y) {
                if(y.sig_words() == 1 && jau::is_power_of_2(y.word_at(0))) {
                    (*this) >>= (y.bits() - 1);
                } else {
                    (*this) = (*this) / y;
                }
                return (*this);
            }

            big_int_t operator/(const big_int_t& y) {
                if(y.sig_words() == 1) {
                    return *this / y.word_at(0);
                }
                big_int_t q, r;
                vartime_divide(*this, y, q, r);
                return q;
            }
            /**
            * Modulo operator
            * @param y the modulus to reduce this by
            */
            big_int_t& operator%=(const big_int_t& mod) {
                return (*this = (*this) % mod);
            }

            big_int_t operator%(const big_int_t& mod) {
                if(mod.is_zero()) {
                    throw jau::MathDivByZeroError("mod == 0", E_FILE_LINE);
                }
                if(mod.is_negative()) {
                    throw jau::MathDomainError("mod < 0", E_FILE_LINE);
                }
                if(is_positive() && mod.is_positive() && *this < mod) {
                    return *this;
                }
                if(mod.sig_words() == 1) {
                    return from_word(*this % mod.word_at(0));
                }
                big_int_t q, r;
                vartime_divide(*this, mod, q, r);
                return r;
            }

            /**
            * Square value of *this
            * @param ws a temp workspace
            */
            big_int_t& square(std::vector<mp_word_t>& ws); // TODO

            /**
            * Set *this to y - *this
            * @param y the big_int_t to subtract from as a sequence of words
            * @param y_words length of y in words
            * @param ws a temp workspace
            */
            big_int_t& rev_sub(const mp_word_t y[], size_t y_words, std::vector<mp_word_t>& ws); // TODO

            /**
            * Set *this to (*this + y) % mod
            * This function assumes *this is >= 0 && < mod
            * @param y the big_int_t to add - assumed y >= 0 and y < mod
            * @param mod the positive modulus
            * @param ws a temp workspace
            */
            big_int_t& mod_add(const big_int_t& y, const big_int_t& mod, std::vector<mp_word_t>& ws); // TODO

            /**
            * Set *this to (*this - y) % mod
            * This function assumes *this is >= 0 && < mod
            * @param y the big_int_t to subtract - assumed y >= 0 and y < mod
            * @param mod the positive modulus
            * @param ws a temp workspace
            */
            big_int_t& mod_sub(const big_int_t& y, const big_int_t& mod, std::vector<mp_word_t>& ws); // TODO

            /**
            * Set *this to (*this * y) % mod
            * This function assumes *this is >= 0 && < mod
            * y should be small, less than 16
            * @param y the small integer to multiply by
            * @param mod the positive modulus
            * @param ws a temp workspace
            */
            big_int_t& mod_mul(uint8_t y, const big_int_t& mod, std::vector<mp_word_t>& ws); // TODO

            /**
            * @param rng a random number generator
            * @param min the minimum value (must be non-negative)
            * @param max the maximum value (must be non-negative and > min)
            * @return random integer in [min,max)
            static big_int_t random_integer(RandomNumberGenerator& rng,
                                         const big_int_t& min,
                                         const big_int_t& max); // TODO
            */

            std::string to_dec_string(bool add_details=false) const noexcept {
                // Use the largest power of 10 that fits in a mp_word_t
                mp_word_t conversion_radix, radix_digits;
                if constexpr ( 64 == mp_word_bits ) {
                    conversion_radix = 10000000000000000000U;
                    radix_digits = 19;
                } else {
                    conversion_radix = 1000000000U;
                    radix_digits = 9;
                }

                // (over-)estimate of the number of digits needed; log2(10) ~ 3.3219
                const nsize_t digit_estimate = static_cast<nsize_t>(1 + (this->bits() / 3.32));

                // (over-)estimate of db such that conversion_radix^db > *this
                const nsize_t digit_blocks = (digit_estimate + radix_digits - 1) / radix_digits;

                big_int_t value = *this;
                value.set_sign(Positive);

                // Extract groups of digits into words
                std::vector<mp_word_t> digit_groups(digit_blocks);

                for(nsize_t i = 0; i != digit_blocks; ++i) {
                    mp_word_t remainder = 0;
                    ct_divide_word(value, conversion_radix, value, remainder);
                    digit_groups[i] = remainder;
                }
                assert(value.is_zero());

                // Extract digits from the groups
                std::vector<uint8_t> digits(digit_blocks * radix_digits);

                for(nsize_t i = 0; i != digit_blocks; ++i) {
                    mp_word_t remainder = digit_groups[i];
                    for(nsize_t j = 0; j != radix_digits; ++j)
                    {
                        // Compiler should convert div/mod by 10 into mul by magic constant
                        const mp_word_t digit = remainder % 10;
                        remainder /= 10;
                        digits[radix_digits*i + j] = static_cast<uint8_t>(digit);
                    }
                }

                // remove leading zeros
                while(!digits.empty() && digits.back() == 0) {
                    digits.pop_back();
                }

                assert(digit_estimate >= digits.size());

                // Reverse the digits to big-endian and format to text
                std::string s;
                s.reserve(1 + digits.size());

                if(is_negative()) {
                    s += "-";
                }

                // Reverse and convert to textual digits
                for(auto i = digits.rbegin(); i != digits.rend(); ++i) {
                    s.push_back(*i + '0'); // assumes ASCII
                }

                if(s.empty()) {
                    s += "0";
                }
                if( add_details ) {
                    append_detail(s);
                }
                return s;
            }

            std::string to_hex_string(bool add_details=false) const noexcept {
                std::vector<uint8_t> bits;
                const uint8_t* data;
                nsize_t data_len;

                if( is_zero() ) {
                    bits.push_back(0);
                    data = bits.data();
                    data_len = bits.size();
                } else {
                    data = reinterpret_cast<const uint8_t*>(m_data.const_data());
                    data_len = bytes();
                }

                std::string s;
                if(is_negative()) {
                    s += "-";
                }
                s.append( jau::bytesHexString(data, 0, data_len,
                                              false /* lsbFirst */, true /* lowerCase */) );
                if( add_details ) {
                    append_detail(s);
                }
                return s;
            }

        private:
            class Data
            {
                public:
                    mp_word_t* mutable_data() noexcept {
                        invalidate_sig_words();
                        return m_reg.data();
                    }

                    const mp_word_t* const_data() const noexcept {
                        return m_reg.data();
                    }

                    std::vector<mp_word_t>& mutable_vector() noexcept {
                        invalidate_sig_words();
                        return m_reg;
                    }

                    const std::vector<mp_word_t>& const_vector() const noexcept {
                        return m_reg;
                    }

                    mp_word_t get_word_at(nsize_t n) const noexcept {
                        if(n < m_reg.size()) {
                            return m_reg[n];
                        }
                        return 0;
                    }

                    void set_word_at(nsize_t i, mp_word_t w) {
                        invalidate_sig_words();
                        if(i >= m_reg.size()) {
                            if(w == 0) {
                                return;
                            }
                            grow_to(i + 1);
                        }
                        m_reg[i] = w;
                    }

                    void set_words(const mp_word_t w[], nsize_t len) {
                        invalidate_sig_words();
                        m_reg.assign(w, w + len);
                    }

                    void set_to_zero() {
                        m_reg.resize(m_reg.capacity());
                        clear_mem(m_reg.data(), m_reg.size());
                        m_sig_words = 0;
                    }

                    void set_size(nsize_t s) {
                        invalidate_sig_words();
                        clear_mem(m_reg.data(), m_reg.size());
                        m_reg.resize(s + (8 - (s % 8)));
                    }

                    void mask_bits(nsize_t n) noexcept {
                        if(n == 0) { return set_to_zero(); }

                        const nsize_t top_word = n / mp_word_bits;

                        // if(top_word < sig_words()) ?
                                if(top_word < size())
                                {
                                    const mp_word_t mask = (static_cast<mp_word_t>(1) << (n % mp_word_bits)) - 1;
                                    const nsize_t len = size() - (top_word + 1);
                                    if(len > 0)
                                    {
                                        clear_mem(&m_reg[top_word+1], len);
                                    }
                                    m_reg[top_word] &= mask;
                                    invalidate_sig_words();
                                }
                    }

                    void grow_to(nsize_t n) const {
                        if(n > size()) {
                            if(n <= m_reg.capacity()) {
                                m_reg.resize(n);
                            } else {
                                m_reg.resize(n + (8 - (n % 8)));
                            }
                        }
                    }

                    nsize_t size() const noexcept { return m_reg.size(); }

                    void shrink_to_fit(nsize_t min_size = 0) {
                        const nsize_t words = std::max(min_size, sig_words());
                        m_reg.resize(words);
                    }

                    void resize(nsize_t s) {
                        m_reg.resize(s);
                    }

                    void swap(Data& other) noexcept {
                        m_reg.swap(other.m_reg);
                        std::swap(m_sig_words, other.m_sig_words);
                    }

                    void swap(std::vector<mp_word_t>& reg) noexcept {
                        m_reg.swap(reg);
                        invalidate_sig_words();
                    }

                    void invalidate_sig_words() const noexcept {
                        m_sig_words = sig_words_npos;
                    }

                    nsize_t sig_words() const noexcept {
                        if(m_sig_words == sig_words_npos) {
                            m_sig_words = calc_sig_words();
                        } else {
                            assert(m_sig_words == calc_sig_words());
                        }
                        return m_sig_words;
                    }
                private:
                    static const nsize_t sig_words_npos = static_cast<nsize_t>(-1);

                    nsize_t calc_sig_words() const noexcept {
                        const nsize_t sz = m_reg.size();
                        nsize_t sig = sz;

                        mp_word_t sub = 1;

                        for(nsize_t i = 0; i != sz; ++i)
                        {
                            const mp_word_t w = m_reg[sz - i - 1];
                            sub &= ct_is_zero(w);
                            sig -= sub;
                        }

                        /*
                         * This depends on the data so is poisoned, but unpoison it here as
                         * later conditionals are made on the size.
                         */
                        CT::unpoison(sig);

                        return sig;
                    }

                    mutable std::vector<mp_word_t> m_reg;
                    mutable nsize_t m_sig_words = sig_words_npos;
            };
            Data m_data;
            Sign m_signedness = Positive;

            /**
            * Byte extraction
            * @param byte_num which byte to extract, 0 == highest byte
            * @param input the value to extract from
            * @return byte byte_num of input
            */
            template<typename T> static inline constexpr uint8_t get_byte_var(nsize_t byte_num, T input) noexcept {
               return static_cast<uint8_t>( input >> (((~byte_num)&(sizeof(T)-1)) << 3) );
            }

            /**
            * Zero out some bytes. Warning: use secure_scrub_memory instead if the
            * memory is about to be freed or otherwise the compiler thinks it can
            * elide the writes.
            *
            * @param ptr a pointer to memory to zero
            * @param bytes the number of bytes to zero in ptr
            */
            static inline constexpr void clear_bytes(void* ptr, nsize_t bytes) noexcept {
                if(bytes > 0) {
                    std::memset(ptr, 0, bytes);
                }
            }

            /**
            * Zero memory before use. This simply calls memset and should not be
            * used in cases where the compiler cannot see the call as a
            * side-effecting operation (for example, if calling clear_mem before
            * deallocating memory, the compiler would be allowed to omit the call
            * to memset entirely under the as-if rule.)
            *
            * @param ptr a pointer to an array of Ts to zero
            * @param n the number of Ts pointed to by ptr
            */
            template<typename T> static inline constexpr void clear_mem(T* ptr, nsize_t n) noexcept {
               clear_bytes(ptr, sizeof(T)*n);
            }

            /**
            * Increase internal register buffer to at least n words
            * @param n new size of register
            */
            void grow_to(nsize_t n) const { m_data.grow_to(n); }

            void resize(nsize_t s) { m_data.resize(s); }

            nsize_t top_bits_free() const noexcept {
                const nsize_t words = sig_words();

                const mp_word_t top_word = word_at(words - 1);
                const nsize_t bits_used = jau::high_bit(top_word);
                CT::unpoison(bits_used);
                return mp_word_bits - bits_used;
            }

            /**
             * Compares this instance against other, considering sign depending on check_signs
             * Returns
             * - -1 if this < other
             * -  0 if this == other
             * -  1 if this > other
             */
            int cmp(const big_int_t& other, bool check_signs = true) const noexcept {
                if(check_signs) {
                    if(other.is_positive() && this->is_negative()) {
                        return -1;
                    }
                    if(other.is_negative() && this->is_positive()) {
                        return 1;
                    }
                    if(other.is_negative() && this->is_negative()) {
                        return (-ops::bigint_cmp(this->data(), this->size(),
                                other.data(), other.size()));
                    }
                }
                return ops::bigint_cmp(this->data(), this->size(),
                        other.data(), other.size());
            }

            bool is_equal(const big_int_t& other) const noexcept {
                if(this->sign() != other.sign()) {
                   return false;
                }
                return ops::bigint_ct_is_eq(this->data(), this->sig_words(),
                                            other.data(), other.sig_words()).is_set();
            }

            bool is_less_than(const big_int_t& other) const noexcept {
                if(this->is_negative() && other.is_positive()) {
                    return true;
                }
                if(this->is_positive() && other.is_negative()) {
                    return false;
                }
                if(other.is_negative() && this->is_negative()) {
                    return ops::bigint_ct_is_lt(other.data(), other.sig_words(),
                                                this->data(), this->sig_words()).is_set();
                }
                return ops::bigint_ct_is_lt(this->data(), this->sig_words(),
                                            other.data(), other.sig_words()).is_set();
            }

            big_int_t& add(const mp_word_t y[], nsize_t y_words, Sign y_sign) {
                const nsize_t x_sw = sig_words();
                grow_to(std::max(x_sw, y_words) + 1);

                if(sign() == y_sign)
                {
                    ops::bigint_add2(mutable_data(), size() - 1, y, y_words);
                } else {
                    const int32_t relative_size = ops::bigint_cmp(data(), x_sw, y, y_words);

                    if(relative_size >= 0)
                    {
                        // *this >= y
                        ops::bigint_sub2(mutable_data(), x_sw, y, y_words);
                    } else {
                        // *this < y
                        ops::bigint_sub2_rev(mutable_data(), y, y_words);
                    }
                    //this->sign_fixup(relative_size, y_sign);
                    if(relative_size < 0) {
                        set_sign(y_sign);
                    } else if(relative_size == 0) {
                        set_sign(Positive);
                    }
                }
                return (*this);
            }

            big_int_t& operator+=(mp_word_t y) noexcept {
                return add(&y, 1, Sign::Positive);
            }
            big_int_t& operator-=(mp_word_t y) noexcept {
                return add(&y, 1, Sign::Negative);
            }

            static big_int_t add2(const big_int_t& x, const mp_word_t y[], nsize_t y_words, Sign y_sign) {
                const nsize_t x_sw = x.sig_words();

                big_int_t z = big_int_t::with_capacity(std::max(x_sw, y_words) + 1);

                if(x.sign() == y_sign) {
                    ops::bigint_add3(z.mutable_data(), x.data(), x_sw, y, y_words);
                    z.set_sign(x.sign());
                } else {
                    const int32_t relative_size = ops::bigint_sub_abs(z.mutable_data(), x.data(), x_sw, y, y_words);

                    //z.sign_fixup(relative_size, y_sign);
                    if(relative_size < 0) {
                        z.set_sign(y_sign);
                    } else if(relative_size == 0) {
                        z.set_sign(Positive);
                    } else {
                        z.set_sign(x.sign());
                    }
                }
                return z;
            }

            big_int_t& mul(const big_int_t& y, std::vector<mp_word_t>& ws) noexcept {
                const nsize_t x_sw = sig_words();
                const nsize_t y_sw = y.sig_words();
                set_sign((sign() == y.sign()) ? Positive : Negative);

                if(x_sw == 0 || y_sw == 0)
                {
                    clear();
                    set_sign(Positive);
                }
                else if(x_sw == 1 && y_sw)
                {
                    grow_to(y_sw + 1);
                    ops::bigint_linmul3(mutable_data(), y.data(), y_sw, word_at(0));
                }
                else if(y_sw == 1 && x_sw)
                {
                    mp_word_t carry = ops::bigint_linmul2(mutable_data(), x_sw, y.word_at(0));
                    set_word_at(x_sw, carry);
                }
                else
                {
                    const nsize_t new_size = x_sw + y_sw + 1;
                    // ws.resize(new_size);
                    (void)ws;
                    std::vector<mp_word_t> z_reg(new_size);

                    ops::basecase_mul(z_reg.data(), z_reg.size(), data(), x_sw, y.data(), y_sw);

                    this->swap_reg(z_reg);
                }

                return (*this);
            }

            big_int_t operator*(mp_word_t y) {
                const nsize_t x_sw = sig_words();
                big_int_t z = big_int_t::with_capacity(x_sw + 1);

                if(x_sw && y) {
                    ops::bigint_linmul3(z.mutable_data(), data(), x_sw, y);
                    z.set_sign(sign());
                }
                return z;
            }

            void cond_flip_sign(bool predicate) noexcept {
                // This code is assuming Negative == 0, Positive == 1
                const auto mask = CT::Mask<uint8_t>::expand(predicate);
                const uint8_t current_sign = static_cast<uint8_t>(sign());
                const uint8_t new_sign = mask.select(current_sign ^ 1, current_sign);
                set_sign(static_cast<Sign>(new_sign));
            }

            /**
             * Return *this % mod
             *
             * Assumes that *this is (if anything) only slightly larger than
             * mod and performs repeated subtractions. It should not be used if
             * *this is much larger than mod, instead use modulo operator.
             */
            inline nsize_t reduce_below(const big_int_t& p, std::vector<mp_word_t>& ws) {
                if(p.is_negative() || this->is_negative()) {
                    std::string msg;
                    if( p.is_negative() ) {
                        msg.append("p < 0");
                    }
                    if( this->is_negative() ) {
                        if( msg.length() > 0 ) {
                            msg.append(" and ");
                        }
                        msg.append("*this < 0");
                    }
                    throw jau::MathDomainError(msg, E_FILE_LINE);
                }
                const nsize_t p_words = p.sig_words();

                if(size() < p_words + 1) {
                    grow_to(p_words + 1);
                }
                if(ws.size() < p_words + 1) {
                    ws.resize(p_words + 1);
                }
                clear_mem(ws.data(), ws.size());

                nsize_t reductions = 0;

                for(;;)
                {
                    mp_word_t borrow = ops::bigint_sub3(ws.data(), data(), p_words + 1, p.data(), p_words);
                    if(borrow) {
                        break;
                    }
                    ++reductions;
                    swap_reg(ws);
                }
                return reductions;
            }

            static void sign_fixup(const big_int_t& x, const big_int_t& y, big_int_t& q, big_int_t& r) {
                q.cond_flip_sign(x.sign() != y.sign());

                if(x.is_negative() && r.is_nonzero())
                {
                    q -= 1;
                    r = y.abs() - r;
                }
            }

            static bool division_check(mp_word_t q, mp_word_t y2, mp_word_t y1,
                                       mp_word_t x3, mp_word_t x2, mp_word_t x1) noexcept {
                /*
                       Compute (y3,y2,y1) = (y2,y1) * q
                       and return true if (y3,y2,y1) > (x3,x2,x1)
                 */

                mp_word_t y3 = 0;
                y1 = ops::word_madd2(q, y1, y3);
                y2 = ops::word_madd2(q, y2, y3);

                const mp_word_t x[3] = { x1, x2, x3 };
                const mp_word_t y[3] = { y1, y2, y3 };

                return ops::bigint_ct_is_lt(x, 3, y, 3).is_set();
            }

            /*
             * Solve x = q * y + r
             *
             * See Handbook of Applied Cryptography section 14.2.5
             */
            static void vartime_divide(const big_int_t& x, const big_int_t& y_arg, big_int_t& q_out, big_int_t& r_out) {
                if( y_arg.is_zero() ) {
                    throw jau::MathDivByZeroError("y_arg == 0", E_FILE_LINE);
                }
                const nsize_t y_words = y_arg.sig_words();

                assert(y_words > 0);

                big_int_t y = y_arg;

                big_int_t r = x;
                big_int_t q = big_int_t::zero();
                std::vector<mp_word_t> ws;

                r.set_sign(big_int_t::Positive);
                y.set_sign(big_int_t::Positive);

                // Calculate shifts needed to normalize y with high bit set
                const nsize_t shifts = y.top_bits_free();

                y <<= shifts;
                r <<= shifts;

                // we know y has not changed size, since we only shifted up to set high bit
                const nsize_t t = y_words - 1;
                const nsize_t n = std::max(y_words, r.sig_words()) - 1; // r may have changed size however
                assert(n >= t);

                q.grow_to(n - t + 1);

                mp_word_t* q_words = q.mutable_data();

                big_int_t shifted_y = y << (mp_word_bits * (n-t));

                // Set q_{n-t} to number of times r > shifted_y
                q_words[n-t] = r.reduce_below(shifted_y, ws);

                const mp_word_t y_t0  = y.word_at(t);
                const mp_word_t y_t1  = y.word_at(t-1);
                assert((y_t0 >> (mp_word_bits-1)) == 1);

                for(nsize_t j = n; j != t; --j) {
                    const mp_word_t x_j0  = r.word_at(j);
                    const mp_word_t x_j1 = r.word_at(j-1);
                    const mp_word_t x_j2 = r.word_at(j-2);

                    mp_word_t qjt = ops::bigint_divop(x_j0, x_j1, y_t0);

                    qjt = CT::Mask<mp_word_t>::is_equal(x_j0, y_t0).select(mp_word_max, qjt);

                    // Per HAC 14.23, this operation is required at most twice
                    qjt -= division_check(qjt, y_t0, y_t1, x_j0, x_j1, x_j2);
                    qjt -= division_check(qjt, y_t0, y_t1, x_j0, x_j1, x_j2);
                    assert(division_check(qjt, y_t0, y_t1, x_j0, x_j1, x_j2) == false);

                    shifted_y >>= mp_word_bits;
                    // Now shifted_y == y << (BOTAN_MP_WORD_BITS * (j-t-1))

                    // TODO this sequence could be better
                    r -= shifted_y * qjt;
                    qjt -= r.is_negative();
                    r += shifted_y * static_cast<mp_word_t>(r.is_negative());

                    q_words[j-t-1] = qjt;
                }

                r >>= shifts;

                sign_fixup(x, y_arg, q, r);

                r_out = r;
                q_out = q;
            }

            big_int_t operator/(mp_word_t y) {
                if(y == 0) {
                    throw jau::MathDivByZeroError("y == 0", E_FILE_LINE);
                }
                big_int_t q;
                mp_word_t r;
                ct_divide_word(*this, y, q, r);
                return q;
            }

            static void ct_divide_word(const big_int_t& x, mp_word_t y, big_int_t& q_out, mp_word_t& r_out) {
                if(y == 0) {
                    throw jau::MathDivByZeroError("y == 0", E_FILE_LINE);
                }
                const nsize_t x_words = x.sig_words();
                const nsize_t x_bits = x.bits();

                big_int_t q = big_int_t::with_capacity(x_words);
                mp_word_t r = 0;

                for(nsize_t i = 0; i != x_bits; ++i)
                {
                    const nsize_t b = x_bits - 1 - i;
                    const bool x_b = x.get_bit(b);

                    const auto r_carry = CT::Mask<mp_word_t>::expand(r >> (mp_word_bits - 1));

                    r *= 2;
                    r += x_b;

                    const auto r_gte_y = CT::Mask<mp_word_t>::is_gte(r, y) | r_carry;
                    q.conditionally_set_bit(b, r_gte_y.is_set());
                    r = r_gte_y.select(r - y, r);
                }

                if(x.is_negative()) {
                    q.flip_sign();
                    if(r != 0) {
                        --q;
                        r = y - r;
                    }
                }

                r_out = r;
                q_out = q;
            }

            mp_word_t operator%(mp_word_t mod) {
               if(mod == 0) {
                   throw jau::MathDivByZeroError("mod == 0", E_FILE_LINE);
               }
               if(mod == 1) {
                  return 0;
               }
               mp_word_t remainder = 0;

               if( jau::is_power_of_2(mod) ) {
                  remainder = (word_at(0) & (mod - 1));
               } else {
                   const size_t sw = sig_words();
                   for(size_t i = sw; i > 0; --i) {
                       remainder = ops::bigint_modop(remainder, word_at(i-1), mod);
                   }
               }
               if(remainder && sign() == big_int_t::Negative) {
                  return mod - remainder;
               }
               return remainder;
            }

            void append_detail(std::string& s) const noexcept {
                s.append(", bits ").append(std::to_string(bits())).append(", ").append(std::to_string(sig_words())).append(" word(s): ");
                for(nsize_t i=0; i<sig_words(); ++i) {
                    const mp_word_t w = word_at(i);
                    s.append( jau::bytesHexString(&w, 0, mp_word_bits/CHAR_BIT, false /* lsbFirst */, true /* lowerCase */) )
                     .append(", ");
                }
            }
    };

    std::ostream& operator<<(std::ostream& out, const big_int_t& v) {
        return out << v.to_dec_string();
    }
}


