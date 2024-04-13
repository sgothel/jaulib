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
#include <cassert>

#include <jau/mp/big_int_ops.hpp>
#include <jau/byte_util.hpp>
#include <jau/string_util.hpp>

namespace jau::mp {

    /** \addtogroup Integer
     *
     *  @{
     */

    /**
     * Arbitrary precision integer type
     *
     * @anchor bigint_storage_format
     * ### Local storage format
     * Internally the big integer is stored in an array of mp_word_t ordered little-endian alike,
     * with the least significant word at the array-bottom and most significant word at the array-top.
     *
     * The mp_word_t itself is stored in jau::endian::native!
     */
    class BigInt {
        public:
            /**
             * Sign symbol definitions for positive and negative numbers
             */
            enum sign_t { negative = 0, positive = 1 };

        public:
            BigInt() noexcept = default;

            BigInt(const BigInt& o) noexcept = default;

            ~BigInt() noexcept = default;

            /**
            * Create a 0-value big_int
            */
            static BigInt zero() { return BigInt(); }

            /**
            * Create a 1-value big_int
            */
            static BigInt one() { return BigInt::from_word(1); }

            /**
            * Create big_int from an unsigned 64 bit integer
            * @param n initial value of this big_int
            */
            static BigInt from_u64(uint64_t n) {
                BigInt bn;
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
            static BigInt from_word(mp_word_t n) {
                BigInt bn;
                bn.set_word_at(0, n);
                return bn;
            }

            /**
            * Create big_int from a signed 32 bit integer
            * @param n initial value of this big_int
            */
            static BigInt from_s32(int32_t n) {
                if(n >= 0) {
                   return BigInt::from_u64(static_cast<uint64_t>(n));
                } else {
                   return -BigInt::from_u64(static_cast<uint64_t>(-n));
                }
            }

            /**
            * Create big_int of specified size, all zeros
            * @param n size of the internal register in words
            */
            static BigInt with_capacity(size_t n) {
                BigInt bn;
                bn.grow_to(n);
                return bn;
            }

            /**
            * Create a power of two
            * @param n the power of two to create
            * @return big_int_t representing 2^n
            */
            static BigInt power_of_2(size_t n) {
                BigInt b;
                b.set_bit(n);
                return b;
            }

            /**
            * Create big_int_t from an unsigned 64 bit integer
            * @param n initial value of this big_int_t
            */
            BigInt(uint64_t n) {
                if( 64 == mp_word_bits ) {
                    m_data.set_word_at(0, n);
                } else {
                    m_data.set_word_at(1, static_cast<mp_word_t>(n >> 32));
                    m_data.set_word_at(0, static_cast<mp_word_t>(n));
                }
            }

            /**
             * Construct a big_int_t from a string encoded as hexadecimal or decimal.
             *
             * Both number bases may lead a `-`, denoting a negative number.
             *
             * Hexadecimal is detected by a leading `0x`.
             */
            BigInt(const std::string& str) {
                size_t markers = 0;
                bool is_negative = false;

                if(str.length() > 0 && str[0] == '-')
                {
                    markers += 1;
                    is_negative = true;
                }

                if(str.length() > markers + 2 && str[markers    ] == '0' &&
                   str[markers + 1] == 'x')
                {
                    markers += 2;
                    *this = hex_decode(cast_char_ptr_to_uint8(str.data()) + markers, str.length() - markers, lb_endian::big);
                } else {
                    *this = dec_decode(cast_char_ptr_to_uint8(str.data()) + markers, str.length() - markers);
                }

                if(is_negative) set_sign(negative);
                else            set_sign(positive);
            }

            /**
             * Create a big_int_t from an integer in a byte array with given byte_len,
             * considering the given byte_order.
             *
             * The value is stored in the local storage format, see \ref bigint_storage_format
             *
             * @param buf the byte array holding the value
             * @param byte_len size of buf in bytes
             * @param littleEndian
             */
            BigInt(const uint8_t buf[], size_t byte_len, const lb_endian byte_order) {
                binary_decode(buf, byte_len, byte_order);
            }

            BigInt(std::vector<mp_word_t>&& other_reg) noexcept {
                this->swap_reg(other_reg);
            }

            BigInt(BigInt&& other) noexcept {
                this->swap(other);
            }

            BigInt& operator=(const BigInt& r) = default;

            BigInt& operator=(BigInt&& other) noexcept {
                if(this != &other) {
                    this->swap(other);
                }
                return *this;
            }

            /**
            * Swap this value with another
            * @param other big_int to swap values with
            */
            void swap(BigInt& other) noexcept {
               m_data.swap(other.m_data);
               std::swap(m_signedness, other.m_signedness);
            }

        private:
            void swap_reg(std::vector<mp_word_t>& reg) noexcept {
               m_data.swap(reg);
               // sign left unchanged
            }

        public:
            /** Unary negation operator, returns new negative instance of this. */
            BigInt operator-() const noexcept { return BigInt(*this).flip_sign(); }

            /**
             * @param n the offset to get a byte from
             * @result byte at offset n
             */
            uint8_t byte_at(size_t n) const noexcept {
                return get_byte_var_be(sizeof(mp_word_t) - (n % sizeof(mp_word_t)) - 1,
                                       word_at(n / sizeof(mp_word_t)));
            }

            /**
             * Return the mp_word_t at a specified position of the internal register
             * @param n position in the register
             * @return value at position n
             */
            mp_word_t word_at(size_t n) const noexcept {
                return m_data.get_word_at(n);
            }

            /**
            * Return a const pointer to the register
            * @result a pointer to the start of the internal register
            */
            const mp_word_t* data() const { return m_data.const_data(); }

            /**
             * Tests if the sign of the integer is negative
             * @result true, iff the integer has a negative sign
             */
            bool is_negative() const noexcept { return sign() == negative; }

            /**
             * Tests if the sign of the integer is positive
             * @result true, iff the integer has a positive sign
             */
            bool is_positive() const noexcept { return sign() == positive; }

            /**
             * Return the sign of the integer
             * @result the sign of the integer
             */
            sign_t sign() const noexcept { return m_signedness; }

            /**
             * @result the opposite sign of the represented integer value
             */
            sign_t reverse_sign() const noexcept {
                if(sign() == positive) {
                    return negative;
                }
                return positive;
            }

            /**
             * Flip the sign of this big_int
             */
            BigInt& flip_sign() noexcept {
                return set_sign(reverse_sign());
            }

            /**
             * Set sign of the integer
             * @param sign new Sign to set
             */
            BigInt& set_sign(sign_t sign) noexcept {
                if(sign == negative && is_zero()) {
                    sign = positive;
                }
                m_signedness = sign;
                return *this;
            }

            /** Returns absolute (positive) value of this instance */
            BigInt abs() const noexcept {
                return BigInt(*this).set_sign(positive);
            }

            /**
             * Give size of internal register
             * @result size of internal register in words
             */
            size_t size() const noexcept { return m_data.size(); }

            /**
             * Return how many words we need to hold this value
             * @result significant words of the represented integer value
             */
            size_t sig_words() const noexcept { return m_data.sig_words(); }

            /** Returns byte length of this integer */
            size_t bytes() const { return jau::round_up(bits(), 8U) / 8U; }

            /** Returns bit length of this integer */
            size_t bits() const noexcept {
                const size_t words = sig_words();
                if(words == 0) {
                    return 0;
                }
                const size_t full_words = (words - 1) * mp_word_bits;
                const size_t top_bits = mp_word_bits - top_bits_free();
                return full_words + top_bits;
            }

            /**
            * Zeroize the big_int. The size of the underlying register is not
            * modified.
            */
            void clear() { m_data.set_to_zero(); m_signedness = positive; }

            /**
             * Compares this instance against other considering both sign() value
             * Returns
             * - -1 if this < other
             * -  0 if this == other
             * -  1 if this > other
             */
            int compare(const BigInt& b) const noexcept {
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
            bool get_bit(size_t n) const noexcept {
                return (word_at(n / mp_word_bits) >> (n % mp_word_bits)) & 1;
            }

            /**
             * Set bit at specified position
             * @param n bit position to set
             */
            void set_bit(size_t n) noexcept {
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
            void conditionally_set_bit(size_t n, bool set_it) noexcept {
                const size_t which = n / mp_word_bits;
                const mp_word_t mask = static_cast<mp_word_t>(set_it) << (n % mp_word_bits);
                m_data.set_word_at(which, word_at(which) | mask);
            }

            /** ! operator, returns `true` iff this is zero, otherwise false. */
            bool operator !() const noexcept { return is_zero(); }

            bool operator==(const BigInt& b) const noexcept { return is_equal(b); }
            bool operator!=(const BigInt& b) const noexcept { return !is_equal(b); }
            bool operator<=(const BigInt& b) const noexcept { return cmp(b) <= 0; }
            bool operator>=(const BigInt& b) const noexcept { return cmp(b) >= 0; }
            bool operator<(const BigInt& b)  const noexcept { return is_less_than(b); }
            bool operator>(const BigInt& b)  const noexcept { return b.is_less_than(*this); }
#if 0
            std::strong_ordering operator<=>(const big_int_t& b) const noexcept {
                const int r = cmp(b);
                return 0 == r ? std::strong_ordering::equal : ( 0 > r ? std::strong_ordering::less : std::strong_ordering::greater);
            }
#endif

            BigInt& operator++() noexcept { return *this += 1; }

            BigInt& operator--() noexcept { return *this -= 1; }

            BigInt  operator++(int) noexcept { BigInt x = (*this); ++(*this); return x; }

            BigInt  operator--(int) noexcept { BigInt x = (*this); --(*this); return x; }

            BigInt& operator+=(const BigInt& y ) noexcept {
                return add(y.data(), y.sig_words(), y.sign());
            }

            BigInt& operator-=(const BigInt& y ) noexcept {
                return add(y.data(), y.sig_words(), y.sign() == positive ? negative : positive);
            }

            BigInt operator+(const BigInt& y ) const noexcept {
                return add2(*this, y.data(), y.sig_words(), y.sign());
            }

            BigInt operator-(const BigInt& y ) const noexcept {
                return add2(*this, y.data(), y.sig_words(), y.reverse_sign());
            }

            BigInt& operator<<=(size_t shift) noexcept {
                const size_t shift_words = shift / mp_word_bits;
                const size_t shift_bits  = shift % mp_word_bits;
                const size_t size = sig_words();

                const size_t bits_free = top_bits_free();

                const size_t new_size = size + shift_words + (bits_free < shift_bits);

                m_data.grow_to(new_size);

                ops::bigint_shl1(m_data.mutable_data(), new_size, size, shift_words, shift_bits);

                return *this;
            }

            BigInt& operator>>=(size_t shift) noexcept {
                const size_t shift_words = shift / mp_word_bits;
                const size_t shift_bits  = shift % mp_word_bits;

                ops::bigint_shr1(m_data.mutable_data(), m_data.size(), shift_words, shift_bits);

                if(is_negative() && is_zero()) {
                   set_sign(positive);
                }
                return *this;
            }

            BigInt operator<<(size_t shift) const {
               const size_t shift_words = shift / mp_word_bits;
               const size_t shift_bits  = shift % mp_word_bits;
               const size_t x_sw = sig_words();

               BigInt y = BigInt::with_capacity(x_sw + shift_words + (shift_bits ? 1 : 0));
               ops::bigint_shl2(y.mutable_data(), data(), x_sw, shift_words, shift_bits);
               y.set_sign(sign());
               return y;
            }

            BigInt operator>>(size_t shift) const {
                const size_t shift_words = shift / mp_word_bits;
                const size_t shift_bits  = shift % mp_word_bits;
                const size_t x_sw = sig_words();

                if(shift_words >= x_sw) {
                   return BigInt::zero();
                }

                BigInt y = BigInt::with_capacity(x_sw - shift_words);
                ops::bigint_shr2(y.mutable_data(), data(), x_sw, shift_words, shift_bits);

                if(is_negative() && y.is_zero()) {
                   y.set_sign(BigInt::positive);
                } else {
                   y.set_sign(sign());
                }
                return y;
            }


            BigInt& operator*=(const BigInt& y) noexcept {
                std::vector<mp_word_t> ws;
                return this->mul(y, ws);
            }

            BigInt operator*(const BigInt& y) noexcept
            {
                const size_t x_sw = sig_words();
                const size_t y_sw = y.sig_words();

                BigInt z;
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

            BigInt& operator/=(const BigInt& y) {
                if(y.sig_words() == 1 && jau::is_power_of_2(y.word_at(0))) {
                    (*this) >>= (y.bits() - 1);
                } else {
                    (*this) = (*this) / y;
                }
                return (*this);
            }

            BigInt operator/(const BigInt& y) const {
                if(y.sig_words() == 1) {
                    return *this / y.word_at(0);
                }
                BigInt q, r;
                vartime_divide(*this, y, q, r);
                return q;
            }
            /**
            * Modulo operator
            * @param y the modulus to reduce this by
            */
            BigInt& operator%=(const BigInt& mod) {
                return (*this = (*this) % mod);
            }

            BigInt operator%(const BigInt& mod) {
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
                BigInt q, r;
                vartime_divide(*this, mod, q, r);
                return r;
            }

            /**
             * Returns (*this)^e, or pow(*this, e)
             *
             * Implementation is not optimized and naive, i.e. O(n)
             *
             * @param e the exponent
             */
            BigInt pow(BigInt e) {
                const BigInt& b = *this;
                if( b.is_zero() ) {
                    return BigInt::zero();
                }
                const BigInt one_v = BigInt::one();
                BigInt r = one_v;
                bool is_negative;
                if( e.is_negative() ) {
                    is_negative = true;
                    e.flip_sign();
                } else {
                    is_negative = false;
                }

                while( e.is_nonzero() ) {
                    r *= b;
                    --e;
                }

                if( is_negative ) {
                    return one_v / r;
                } else {
                    return r;
                }
            }

            /**
             * Returns (*this)^e % m, or pow(*this, e) % m
             *
             * Implementation is not optimized and naive, i.e. O(n)
             *
             * @param e the exponent
             */
            BigInt mod_pow(BigInt e, BigInt m) {
                const BigInt& b = *this;
                if( b.is_zero() ) {
                    return BigInt::zero();
                }
                const BigInt one_v = BigInt::one();
                BigInt r = one_v;
                bool is_negative;
                if( e.is_negative() ) {
                    is_negative = true;
                    e.flip_sign();
                } else {
                    is_negative = false;
                }

                while( e.is_nonzero() ) {
                    r *= b;
                    r %= m;
                    --e;
                }

                if( is_negative ) {
                    return one_v / r;
                } else {
                    return r;
                }
            }

            /**
            * Square value of *this
            * @param ws a temp workspace
            */
            BigInt& square(std::vector<mp_word_t>& ws); // TODO

            /**
            * Set *this to y - *this
            * @param y the big_int_t to subtract from
            * @param ws a temp workspace
            */
            BigInt& rev_sub(const BigInt& y, std::vector<mp_word_t>& ws); // TODO

            /**
            * Set *this to (*this + y) % mod
            * This function assumes *this is >= 0 && < mod
            * @param y the big_int_t to add - assumed y >= 0 and y < mod
            * @param mod the positive modulus
            * @param ws a temp workspace
            */
            BigInt& mod_add(const BigInt& y, const BigInt& mod, std::vector<mp_word_t>& ws); // TODO

            /**
            * Set *this to (*this - y) % mod
            * This function assumes *this is >= 0 && < mod
            * @param y the big_int_t to subtract - assumed y >= 0 and y < mod
            * @param mod the positive modulus
            * @param ws a temp workspace
            */
            BigInt& mod_sub(const BigInt& y, const BigInt& mod, std::vector<mp_word_t>& ws); // TODO

            /**
            * Set *this to (*this * y) % mod
            * This function assumes *this is >= 0 && < mod
            * y should be small, less than 16
            * @param y the small integer to multiply by
            * @param mod the positive modulus
            * @param ws a temp workspace
            */
            BigInt& mod_mul(uint8_t y, const BigInt& mod, std::vector<mp_word_t>& ws); // TODO

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
                const size_t digit_estimate = static_cast<size_t>(1 + (this->bits() / 3.32));

                // (over-)estimate of db such that conversion_radix^db > *this
                const size_t digit_blocks = (digit_estimate + radix_digits - 1) / radix_digits;

                BigInt value = *this;
                value.set_sign(positive);

                // Extract groups of digits into words
                std::vector<mp_word_t> digit_groups(digit_blocks);

                for(size_t i = 0; i != digit_blocks; ++i) {
                    mp_word_t remainder = 0;
                    ct_divide_word(value, conversion_radix, value, remainder);
                    digit_groups[i] = remainder;
                }
                assert(value.is_zero());

                // Extract digits from the groups
                std::vector<uint8_t> digits(digit_blocks * radix_digits);

                for(size_t i = 0; i != digit_blocks; ++i) {
                    mp_word_t remainder = digit_groups[i];
                    for(size_t j = 0; j != radix_digits; ++j)
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
                size_t data_len;

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
            class data_t
            {
                public:
                    data_t() noexcept
                    : m_reg(), m_sig_words(sig_words_npos) {}

                    data_t(const data_t& o) noexcept = default;

                    data_t(data_t&& o) noexcept {
                        swap(o);
                    }
                    data_t(std::vector<mp_word_t>&& reg) noexcept {
                        swap(reg);
                    }

                    ~data_t() noexcept = default;

                    data_t& operator=(const data_t& r) noexcept = default;

                    data_t& operator=(data_t&& other) noexcept {
                        if(this != &other) {
                            this->swap(other);
                        }
                        return *this;
                    }
                    data_t& operator=(std::vector<mp_word_t>&& other_reg) noexcept {
                        if(&m_reg != &other_reg) {
                            this->swap(other_reg);
                        }
                        return *this;
                    }

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

                    mp_word_t get_word_at(size_t n) const noexcept {
                        if(n < m_reg.size()) {
                            return m_reg[n];
                        }
                        return 0;
                    }

                    void set_word_at(size_t i, mp_word_t w) {
                        invalidate_sig_words();
                        if(i >= m_reg.size()) {
                            if(w == 0) {
                                return;
                            }
                            grow_to(i + 1);
                        }
                        m_reg[i] = w;
                    }

                    void set_words(const mp_word_t w[], size_t len) {
                        invalidate_sig_words();
                        m_reg.assign(w, w + len);
                    }

                    void set_to_zero() {
                        m_reg.resize(m_reg.capacity());
                        clear_mem(m_reg.data(), m_reg.size());
                        m_sig_words = 0;
                    }

                    void set_size(size_t s) {
                        invalidate_sig_words();
                        clear_mem(m_reg.data(), m_reg.size());
                        m_reg.resize(s + (8 - (s % 8)));
                    }

                    void mask_bits(size_t n) noexcept {
                        if(n == 0) { return set_to_zero(); }

                        const size_t top_word = n / mp_word_bits;

                        // if(top_word < sig_words()) ?
                                if(top_word < size())
                                {
                                    const mp_word_t mask = (static_cast<mp_word_t>(1) << (n % mp_word_bits)) - 1;
                                    const size_t len = size() - (top_word + 1);
                                    if(len > 0)
                                    {
                                        clear_mem(&m_reg[top_word+1], len);
                                    }
                                    m_reg[top_word] &= mask;
                                    invalidate_sig_words();
                                }
                    }

                    void grow_to(size_t n) const {
                        if(n > size()) {
                            if(n <= m_reg.capacity()) {
                                m_reg.resize(n);
                            } else {
                                m_reg.resize(n + (8 - (n % 8)));
                            }
                        }
                    }

                    size_t size() const noexcept { return m_reg.size(); }

                    void shrink_to_fit(size_t min_size = 0) {
                        const size_t words = std::max(min_size, sig_words());
                        m_reg.resize(words);
                    }

                    void resize(size_t s) {
                        m_reg.resize(s);
                    }

                    void swap(data_t& other) noexcept {
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

                    size_t sig_words() const noexcept {
                        if(m_sig_words == sig_words_npos) {
                            m_sig_words = calc_sig_words();
                        } else {
                            assert(m_sig_words == calc_sig_words());
                        }
                        return m_sig_words;
                    }
                private:
                    static const size_t sig_words_npos = static_cast<size_t>(-1);

                    size_t calc_sig_words() const noexcept {
                        const size_t sz = m_reg.size();
                        size_t sig = sz;

                        mp_word_t sub = 1;

                        for(size_t i = 0; i != sz; ++i)
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
                    mutable size_t m_sig_words = sig_words_npos;
            };
            data_t m_data;
            sign_t m_signedness = positive;

            /**
            * Byte extraction of big-endian value
            * @param byte_num which byte to extract, 0 == highest byte
            * @param input the value to extract from
            * @return byte byte_num of input
            */
            template<typename T> static inline constexpr uint8_t get_byte_var_be(size_t byte_num, T input) noexcept {
               return static_cast<uint8_t>( input >> (((~byte_num)&(sizeof(T)-1)) << 3) );
            }
            /**
            * Byte extraction of little-endian value
            * @param byte_num which byte to extract, 0 == lowest byte
            * @param input the value to extract from
            * @return byte byte_num of input
            */
            template<typename T> static inline constexpr uint8_t get_byte_var_le(size_t byte_num, T input) noexcept {
               return static_cast<uint8_t>( input >> ( byte_num << 3 ) );
            }

            /**
            * Zero out some bytes. Warning: use secure_scrub_memory instead if the
            * memory is about to be freed or otherwise the compiler thinks it can
            * elide the writes.
            *
            * @param ptr a pointer to memory to zero
            * @param bytes the number of bytes to zero in ptr
            */
            static inline constexpr void clear_bytes(void* ptr, size_t bytes) noexcept {
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
            template<typename T> static inline constexpr void clear_mem(T* ptr, size_t n) noexcept {
               clear_bytes(ptr, sizeof(T)*n);
            }

            /**
            * Increase internal register buffer to at least n words
            * @param n new size of register
            */
            void grow_to(size_t n) const { m_data.grow_to(n); }

            void resize(size_t s) { m_data.resize(s); }

            void set_word_at(size_t i, mp_word_t w) {
                m_data.set_word_at(i, w);
            }

            void set_words(const mp_word_t w[], size_t len) {
                m_data.set_words(w, len);
            }
            /**
            * Return a mutable pointer to the register
            * @result a pointer to the start of the internal register
            */
            mp_word_t* mutable_data() { return m_data.mutable_data(); }

            /**
             * Set this number to the value in buf with given byte_len,
             * considering the given byte_order.
             *
             * The value is stored in the local storage format, see \ref bigint_storage_format
             */
            void binary_decode(const uint8_t buf[], size_t byte_len, const lb_endian byte_order) {
                const size_t full_words = byte_len / sizeof(mp_word_t);
                const size_t extra_bytes = byte_len % sizeof(mp_word_t);

                // clear() + setting size
                m_signedness = positive;
                m_data.set_size( jau::round_up(full_words + (extra_bytes > 0 ? 1U : 0U), 8U) );

                mp_word_t* sink = m_data.mutable_data();
                if( is_little_endian(byte_order) ) {
                    // little-endian to local (words arranged as little-endian w/ word itself in native-endian)
                    for(size_t i = 0; i < full_words; ++i) {
                        sink[i] = jau::get_value<mp_word_t>(buf + sizeof(mp_word_t)*i, byte_order);
                    }
                } else {
                    // big-endian to local (words arranged as little-endian w/ word itself in native-endian)
                    for(size_t i = 0; i < full_words; ++i) {
                        sink[i] = jau::get_value<mp_word_t>(buf + byte_len - sizeof(mp_word_t)*(i+1), byte_order);
                    }
                }
                mp_word_t le_w = 0;
                if( is_little_endian(byte_order) ) {
                    for(size_t i = 0; i < extra_bytes; ++i) {
                        le_w |= mp_word_t( buf[full_words*sizeof(mp_word_t) + i] ) << ( i * 8 ); // next lowest byte
                    }
                } else {
                    for(size_t i = 0; i < extra_bytes; ++i) {
                        le_w = (le_w << 8) | mp_word_t( buf[i] ); // buf[0] highest byte
                    }
                }
                sink[full_words] = jau::le_to_cpu( le_w );
            }

            /**
             * Set this number to the decoded hex-string value of buf with given str_len,
             * considering the given byte_order.
             *
             * The value is stored in the local storage format, see \ref bigint_storage_format
             */
            static BigInt hex_decode(const uint8_t buf[], size_t str_len, const lb_endian byte_order) {
                BigInt r;

                std::vector<uint8_t> bin_out;
                const size_t exp_blen = str_len / 2 + str_len % 2;
                const size_t blen = jau::hexStringBytes(bin_out, buf, str_len, is_little_endian(byte_order), false /* checkLeading0x */);
                if( exp_blen != blen ) {
                    throw jau::MathDomainError("invalid hexadecimal char @ "+std::to_string(blen)+"/"+std::to_string(exp_blen)+" of '"+
                            std::string(cast_uint8_ptr_to_char(buf), str_len)+"'", E_FILE_LINE);
                }
                r.binary_decode(bin_out.data(), bin_out.size(), lb_endian::little);
                return r;
            }

            static BigInt dec_decode(const uint8_t buf[], size_t str_len) {
                BigInt r;

                // This could be made faster using the same trick as to_dec_string
                for(size_t i = 0; i < str_len; ++i) {
                    const char c = buf[i];

                    if(c < '0' || c > '9') {
                        throw jau::MathDomainError("invalid decimal char", E_FILE_LINE);
                    }
                    const uint8_t x = c - '0';
                    assert(x < 10);

                    r *= 10;
                    r += x;
                }
                return r;
            }

        public:
            /**
             * Stores this number to the value in buf with given byte_len,
             * considering the given byte_order.
             *
             * The value is read from the local storage in its format, see \ref bigint_storage_format
             *
             * If byte_len is less than the byt-esize of this integer, i.e. bytes(),
             * then it will be truncated.
             *
             * If byte_len is greater than the byte-size of this integer, i.e. bytes(), it will be zero-padded.
             *
             * @return actual number of bytes copied, i.e. min(byte_len, bytes());
             */
            size_t binary_encode(uint8_t output[], size_t byte_len, const lb_endian byte_order) const noexcept {
                const size_t full_words = byte_len / sizeof(mp_word_t);
                const size_t extra_bytes = byte_len % sizeof(mp_word_t);

                if( is_little_endian( byte_order ) ) {
                    // to little-endian from local (words arranged as little-endian w/ word itself in native-endian)
                    for(size_t i = 0; i < full_words; ++i) {
                        jau::put_value<mp_word_t>(output + i*sizeof(mp_word_t), word_at(i), byte_order);
                    }
                } else {
                    // to big-endian from local (words arranged as little-endian w/ word itself in native-endian)
                    for(size_t i = 0; i < full_words; ++i) {
                        jau::put_value<mp_word_t>(output + byte_len - (i+1)*sizeof(mp_word_t), word_at(i), byte_order);
                    }
                }
                if(extra_bytes > 0) {
                    const mp_word_t le_w = jau::cpu_to_le( word_at(full_words) );
                    if( is_little_endian( byte_order ) ) {
                        for(size_t i = 0; i < extra_bytes; ++i) {
                            output[full_words*sizeof(mp_word_t) + i] = get_byte_var_le(i, le_w); // next lowest byte
                        }
                    } else {
                        for(size_t i = 0; i < extra_bytes; ++i) {
                            output[extra_bytes-1-i] = get_byte_var_le(i, le_w); // output[0] highest byte
                        }
                    }
                }
                return extra_bytes + full_words * sizeof(mp_word_t);
            }

        private:
            size_t top_bits_free() const noexcept {
                const size_t words = sig_words();

                const mp_word_t top_word = word_at(words - 1);
                const size_t bits_used = jau::high_bit(top_word);
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
            int cmp(const BigInt& other, bool check_signs = true) const noexcept {
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

            bool is_equal(const BigInt& other) const noexcept {
                if(this->sign() != other.sign()) {
                   return false;
                }
                return ops::bigint_ct_is_eq(this->data(), this->sig_words(),
                                            other.data(), other.sig_words()).is_set();
            }

            bool is_less_than(const BigInt& other) const noexcept {
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

            BigInt& add(const mp_word_t y[], size_t y_words, sign_t y_sign) {
                const size_t x_sw = sig_words();
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
                        set_sign(positive);
                    }
                }
                return (*this);
            }

            BigInt& operator+=(mp_word_t y) noexcept {
                return add(&y, 1, sign_t::positive);
            }
            BigInt& operator-=(mp_word_t y) noexcept {
                return add(&y, 1, sign_t::negative);
            }

            static BigInt add2(const BigInt& x, const mp_word_t y[], size_t y_words, sign_t y_sign) {
                const size_t x_sw = x.sig_words();

                BigInt z = BigInt::with_capacity(std::max(x_sw, y_words) + 1);

                if(x.sign() == y_sign) {
                    ops::bigint_add3(z.mutable_data(), x.data(), x_sw, y, y_words);
                    z.set_sign(x.sign());
                } else {
                    const int32_t relative_size = ops::bigint_sub_abs(z.mutable_data(), x.data(), x_sw, y, y_words);

                    //z.sign_fixup(relative_size, y_sign);
                    if(relative_size < 0) {
                        z.set_sign(y_sign);
                    } else if(relative_size == 0) {
                        z.set_sign(positive);
                    } else {
                        z.set_sign(x.sign());
                    }
                }
                return z;
            }

            BigInt& mul(const BigInt& y, std::vector<mp_word_t>& ws) noexcept {
                const size_t x_sw = sig_words();
                const size_t y_sw = y.sig_words();
                set_sign((sign() == y.sign()) ? positive : negative);

                if(x_sw == 0 || y_sw == 0)
                {
                    clear();
                    set_sign(positive);
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
                    const size_t new_size = x_sw + y_sw + 1;
                    // ws.resize(new_size);
                    (void)ws;
                    std::vector<mp_word_t> z_reg(new_size);

                    ops::basecase_mul(z_reg.data(), z_reg.size(), data(), x_sw, y.data(), y_sw);

                    this->swap_reg(z_reg);
                }

                return (*this);
            }

            BigInt operator*(mp_word_t y) {
                const size_t x_sw = sig_words();
                BigInt z = BigInt::with_capacity(x_sw + 1);

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
                set_sign(static_cast<sign_t>(new_sign));
            }

            /**
             * Return *this % mod
             *
             * Assumes that *this is (if anything) only slightly larger than
             * mod and performs repeated subtractions. It should not be used if
             * *this is much larger than mod, instead use modulo operator.
             */
            inline size_t reduce_below(const BigInt& p, std::vector<mp_word_t>& ws) {
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
                const size_t p_words = p.sig_words();

                if(size() < p_words + 1) {
                    grow_to(p_words + 1);
                }
                if(ws.size() < p_words + 1) {
                    ws.resize(p_words + 1);
                }
                clear_mem(ws.data(), ws.size());

                size_t reductions = 0;

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

            static void sign_fixup(const BigInt& x, const BigInt& y, BigInt& q, BigInt& r) {
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
            static void vartime_divide(const BigInt& x, const BigInt& y_arg, BigInt& q_out, BigInt& r_out) {
                if( y_arg.is_zero() ) {
                    throw jau::MathDivByZeroError("y_arg == 0", E_FILE_LINE);
                }
                const size_t y_words = y_arg.sig_words();

                assert(y_words > 0);

                BigInt y = y_arg;

                BigInt r = x;
                BigInt q = BigInt::zero();
                std::vector<mp_word_t> ws;

                r.set_sign(BigInt::positive);
                y.set_sign(BigInt::positive);

                // Calculate shifts needed to normalize y with high bit set
                const size_t shifts = y.top_bits_free();

                y <<= shifts;
                r <<= shifts;

                // we know y has not changed size, since we only shifted up to set high bit
                const size_t t = y_words - 1;
                const size_t n = std::max(y_words, r.sig_words()) - 1; // r may have changed size however
                assert(n >= t);

                q.grow_to(n - t + 1);

                mp_word_t* q_words = q.mutable_data();

                BigInt shifted_y = y << (mp_word_bits * (n-t));

                // Set q_{n-t} to number of times r > shifted_y
                q_words[n-t] = r.reduce_below(shifted_y, ws);

                const mp_word_t y_t0  = y.word_at(t);
                const mp_word_t y_t1  = y.word_at(t-1);
                assert((y_t0 >> (mp_word_bits-1)) == 1);

                for(size_t j = n; j != t; --j) {
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

            BigInt operator/(const mp_word_t& y) const {
                if(y == 0) {
                    throw jau::MathDivByZeroError("y == 0", E_FILE_LINE);
                }
                BigInt q;
                mp_word_t r;
                ct_divide_word(*this, y, q, r);
                return q;
            }

            static void ct_divide_word(const BigInt& x, mp_word_t y, BigInt& q_out, mp_word_t& r_out) {
                if(y == 0) {
                    throw jau::MathDivByZeroError("y == 0", E_FILE_LINE);
                }
                const size_t x_words = x.sig_words();
                const size_t x_bits = x.bits();

                BigInt q = BigInt::with_capacity(x_words);
                mp_word_t r = 0;

                for(size_t i = 0; i != x_bits; ++i)
                {
                    const size_t b = x_bits - 1 - i;
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
               if(remainder && sign() == BigInt::negative) {
                  return mod - remainder;
               }
               return remainder;
            }

            void append_detail(std::string& s) const noexcept {
                s.append(", bits ").append(std::to_string(bits())).append(", ").append(std::to_string(sig_words())).append(" word(s): ");
                for(size_t i=0; i<sig_words(); ++i) {
                    const mp_word_t w = word_at(i);
                    s.append( jau::bytesHexString(&w, 0, mp_word_bits/CHAR_BIT, false /* lsbFirst */, true /* lowerCase */) )
                     .append(", ");
                }
            }
    };

    /**@}*/
}

namespace jau {

    /** \addtogroup Integer
     *
     *  @{
     */

    inline mp::BigInt abs(mp::BigInt x) noexcept { return x.abs(); }
    inline mp::BigInt pow(mp::BigInt b, mp::BigInt e) { return b.pow(e); }

    inline const mp::BigInt& min(const mp::BigInt& x, const mp::BigInt& y) noexcept {
        return x < y ? x : y;
    }
    inline const mp::BigInt& max(const mp::BigInt& x, const mp::BigInt& y) noexcept {
        return x > y ? x : y;
    }
    inline const mp::BigInt& clamp(const mp::BigInt& x, const mp::BigInt& min_val, const mp::BigInt& max_val) noexcept {
        return min(max(x, min_val), max_val);
    }

    inline mp::BigInt& min(mp::BigInt& x, mp::BigInt& y) noexcept {
        return x < y ? x : y;
    }
    inline mp::BigInt& max(mp::BigInt& x, mp::BigInt& y) noexcept {
        return x > y ? x : y;
    }
    inline mp::BigInt& clamp(mp::BigInt& x, mp::BigInt& min_val, mp::BigInt& max_val) noexcept {
        return min(max(x, min_val), max_val);
    }

    inline mp::BigInt gcd(const mp::BigInt& a, const mp::BigInt& b) noexcept {
        mp::BigInt a_ = abs(a);
        mp::BigInt b_ = abs(b);
        while( b_.is_nonzero() ) {
            const mp::BigInt t = b_;
            b_ = a_ % b_;
            a_ = t;
        }
        return a_;
    }

    /**@}*/
}

namespace std {
    inline std::ostream& operator<<(std::ostream& out, const jau::mp::BigInt& v) {
        return out << v.to_dec_string();
    }
}
