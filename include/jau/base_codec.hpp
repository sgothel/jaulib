/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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

#ifndef JAU_BASE_CODEC_HPP_
#define JAU_BASE_CODEC_HPP_

#include <string>
#include <vector>
#include <type_traits>

#include <jau/int_types.hpp>

/**
 * Base codecs, i.e. changing the decimal or binary values' base for a different representation.
 */
namespace jau::codec::base {

    /** @defgroup Codec Codec
     *  Data Stream Encoder and Decoder
     *
     *  Supported codecs:
     *  - jau::codec::base::alphabet enables variable integer base encode() and fixed binary base-64 encode64().
     *
     *  @{
     */

    /**
     * Base Alphabet Specification providing the alphabet for encode() and decode().
     *
     * Implementation delegates static code_point() function.
     *
     * @see encode()
     * @see decode()
     */
    class alphabet {
        public:
            typedef int (*code_point_func)(const char c) noexcept;

        private:
            std::string name_;
            int base_;
            std::string_view symbols_;
            char padding64_;
            code_point_func cpf;

        public:
            alphabet(std::string  _name, int _base, std::string_view _symbols, char _padding64, code_point_func _cpf) noexcept
            : name_(std::move(_name)), base_(_base), symbols_(_symbols), padding64_(_padding64), cpf(_cpf) {}

            /** Human readable name for this alphabet instance. */
            constexpr const std::string& name() const noexcept { return name_; }

            /** The fixed base used for this alphabet. */
            constexpr int base() const noexcept { return base_; }

            /** The string of symbols of this alphabet. */
            constexpr const std::string_view& symbols() const noexcept { return symbols_; }

            /** Padding symbol for base <= 64 and block encoding only. May return zero for no padding. */
            constexpr char padding64() const noexcept { return padding64_; }

            /** Returns the code-point of the given character or -1 if not element of this alphabet. */
            constexpr int code_point(const char c) const noexcept { return cpf(c); }

            /** Retrieve the character at given code-point of this alphabet. */
            constexpr char operator[]( size_t cp ) const noexcept { return symbols_[cp]; }

            std::string to_string() const noexcept {
                std::string res("alphabet[");
                res.append(name());
                res.append(", base <= "+std::to_string(base())+"]");
                return res;
            }
    };

    inline std::string to_string(const alphabet& v) noexcept { return v.to_string(); }

    inline bool operator!=(const alphabet& lhs, const alphabet& rhs ) noexcept {
        return lhs.base() != rhs.base() || lhs.name() != rhs.name() || lhs.symbols() != rhs.symbols();
    }

    inline bool operator==(const alphabet& lhs, const alphabet& rhs ) noexcept {
        return !( lhs != rhs );
    }

    /**
     * Safe canonical `base64` alphabet, without ASCII code-point sorting order.
     *
     * Representing the canonical `base64` [RFC 4648](https://www.rfc-editor.org/rfc/rfc4648.html) *Base 64 Alphabet*
     * including its code-point order `A` < `a` < `0` < `/`.
     *
     * - Value: `ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/`
     * - Padding: `=`
     *
     * ### Properties
     * - Base 64
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - [`base64` alphabet](https://www.rfc-editor.org/rfc/rfc4648.html), identical order
     * - Excludes quoting chars: "'$ and space
     * - Not supporting ASCII code-point sorting.
     * - Order: `A` < `a` < `0` < `/`
     */
    class base64_alphabet : public alphabet {
        private:
            static inline constexpr const std::string_view data  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

            static int s_code_point(const char c) noexcept {
                if ('A' <= c && c <= 'Z') {
                    return c - 'A';
                } else if ('a' <= c && c <= 'z') {
                    return c - 'a' + 26;
                } else if ('0' <= c && c <= '9') {
                    return c - '0' + 52;
                } else if ('+' == c) {
                    return 62;
                } else if ('/' == c) {
                    return 63;
                } else {
                    return -1;
                }
            }

        public:
            base64_alphabet() noexcept
            : alphabet("base64", 64, data, '=', s_code_point) {}
    };

    /**
     * Safe canonical `base64url` alphabet, without ASCII code-point sorting order.
     *
     * Representing the canonical `base64url` [RFC 4648](https://www.rfc-editor.org/rfc/rfc4648.html) `URL and Filename safe` *Base 64 Alphabet*
     * including its code-point order `A` < `a` < `0` < `_`.
     *
     * - Value: `ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_`
     * - Padding: `=`
     *
     * ### Properties
     * - Base 64
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - [`base64url` alphabet](https://www.rfc-editor.org/rfc/rfc4648.html), identical order
     * - Safe URL and filename use
     * - Excludes forbidden [v]fat chars: `<>:"/\|?*`
     * - Excludes quoting chars: "'$ and space
     * - Not supporting ASCII code-point sorting.
     * - Order: `A` < `a` < `0` < `_`
     */
    class base64url_alphabet : public alphabet {
        private:
            static inline constexpr const std::string_view data  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

            static int s_code_point(const char c) noexcept {
                if ('A' <= c && c <= 'Z') {
                    return c - 'A';
                } else if ('a' <= c && c <= 'z') {
                    return c - 'a' + 26;
                } else if ('0' <= c && c <= '9') {
                    return c - '0' + 52;
                } else if ('-' == c) {
                    return 62;
                } else if ('_' == c) {
                    return 63;
                } else {
                    return -1;
                }
            }

        public:
            base64url_alphabet() noexcept
            : alphabet("base64url", 64, data, '=', s_code_point) {}
    };

    /**
     * Safe natural base 64 alphabet, both without ASCII code-point sorting order.
     *
     * Order is considered a natural extension of decimal symbols, i.e. `0` < `a` < `A` < `_`.
     *
     * - Value: `0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_`
     * - Padding: `=`
     *
     * ### Properties
     * - Base 64
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - [`base64url` alphabet](https://www.rfc-editor.org/rfc/rfc4648.html), but different order
     * - Safe URL and filename use
     * - Excludes forbidden [v]fat chars: `<>:"/\|?*`
     * - Excludes quoting chars: "'$ and space
     * - Not supporting ASCII code-point sorting.
     * - Order: `0` < `a` < `A` < `_`
     */
    class natural64_alphabet : public alphabet {
        private:
            static inline constexpr const std::string_view data = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_";

            static int s_code_point(const char c) noexcept {
                if ('0' <= c && c <= '9') {
                    return c - '0';
                } else if ('a' <= c && c <= 'z') {
                    return c - 'a' + 10;
                } else if ('A' <= c && c <= 'Z') {
                    return c - 'A' + 36;
                } else if ('-' == c) {
                    return 62;
                } else if ('_' == c) {
                    return 63;
                } else {
                    return -1;
                }
            }

        public:
            natural64_alphabet() noexcept
            : alphabet("natural64", 64, data, '=', s_code_point) {}
    };

    /**
     * Natural base 86 alphabet, without ASCII code-point sorting order.
     *
     * Order is considered a natural extension of decimal symbols, i.e. `0` < `a` < `A` < `_` < `~`
     *
     * - Value: `0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_!#%&()+,/:;<=>?@[]^{}~`
     * - Padding: none
     *
     * ### Properties
     * - Base 86
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - Excludes quoting chars: "'$ and space
     * - Not supporting ASCII code-point sorting.
     * - Order: `0` < `a` < `A` < `_` < `~`
     */
    class natural86_alphabet : public alphabet {
        private:
            static inline constexpr const std::string_view data = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_!#%&()+,/:;<=>?@[]^{}~";

            static int s_code_point(const char c) noexcept {
                if ('0' <= c && c <= '9') {
                    return c - '0';
                } else if ('a' <= c && c <= 'z') {
                    return c - 'a' + 10;
                } else if ('A' <= c && c <= 'Z') {
                    return c - 'A' + 36;
                } else {
                    switch( c ) {
                        case '-': return 62;
                        case '_': return 63;
                        case '!': return 64;
                        case '#': return 65;
                        case '%': return 66;
                        case '&': return 67;
                        case '(': return 68;
                        case ')': return 69;
                        case '+': return 70;
                        case ',': return 71;
                        case '/': return 72;
                        case ':': return 73;
                        case ';': return 74;
                        case '<': return 75;
                        case '=': return 76;
                        case '>': return 77;
                        case '?': return 78;
                        case '@': return 79;
                        case '[': return 80;
                        case ']': return 81;
                        case '^': return 82;
                        case '{': return 83;
                        case '}': return 84;
                        case '~': return 85;
                        default: return  -1;
                    }
                }
            }

        public:
            natural86_alphabet() noexcept
            : alphabet("natural86", 86, data, 0, s_code_point) {}
    };

    /**
     * Safe base 38 alphabet with ASCII code-point sorting order.
     *
     * - Value: `-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_`
     * - Padding: `=`
     *
     * ### Properties
     * - Base 38
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - Safe URL and filename use
     * - Excludes forbidden [v]fat chars: `<>:"/\|?*`
     * - Only using upper-case letters for unique filename under vfat
     * - Excludes quoting chars: "'$ and space
     * - Supporting ASCII code-point sorting.
     * - Order: `-` < `0` < `A` < `a` < `z`
     */
    class ascii38_alphabet : public alphabet {
        private:
            static inline constexpr const std::string_view data = "-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_";

            static int s_code_point(const char c) noexcept {
                if ('0' <= c && c <= '9') {
                    return c - '0' + 1;
                } else if ('A' <= c && c <= 'Z') {
                    return c - 'A' + 11;
                } else if ('-' == c) {
                    return 0;
                } else if ('_' == c) {
                    return 37;
                } else {
                    return -1;
                }
            }

        public:
            ascii38_alphabet() noexcept
            : alphabet("ascii38", 38, data, '=', s_code_point) {}
    };

    /**
     * Safe base 64 alphabet with ASCII code-point sorting order.
     *
     * - Value: `-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz`
     * - Padding: `=`
     *
     * ### Properties
     * - Base 64
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - [`base64url` alphabet](https://www.rfc-editor.org/rfc/rfc4648.html), but different order
     * - Safe URL and filename use
     * - Excludes forbidden [v]fat chars: `<>:"/\|?*`
     * - Excludes quoting chars: "'$ and space
     * - Supporting ASCII code-point sorting.
     * - Order: `-` < `0` < `A` < `a` < `z`
     */
    class ascii64_alphabet : public alphabet {
        private:
            static inline constexpr const std::string_view data = "-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";

            static int s_code_point(const char c) noexcept {
                if ('0' <= c && c <= '9') {
                    return c - '0' + 1;
                } else if ('A' <= c && c <= 'Z') {
                    return c - 'A' + 11;
                } else if ('a' <= c && c <= 'z') {
                    return c - 'a' + 38;
                } else if ('-' == c) {
                    return 0;
                } else if ('_' == c) {
                    return 37;
                } else {
                    return -1;
                }
            }

        public:
            ascii64_alphabet() noexcept
            : alphabet("ascii64", 64, data, '=', s_code_point) {}
    };

    /**
     * Base 86 alphabet with ASCII code-point sorting order.
     *
     * - Value: `!#%&()+,-/0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_abcdefghijklmnopqrstuvwxyz{}~`
     * - Padding: None
     *
     * ### Properties
     * - Base 86
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - Excludes quoting chars: "'$ and space
     * - Supporting ASCII code-point sorting.
     * - Order: `!` < `0` < `:` < `A` < `[` < `a` < `{` < `~`
     */
    class ascii86_alphabet : public alphabet {
        private:
            static inline constexpr const std::string_view data = "!#%&()+,-/0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_abcdefghijklmnopqrstuvwxyz{}~";

            static int s_code_point(const char c) noexcept {
                if ('0' <= c && c <= '9') {
                    return c - '0' + 10;
                } else if ('A' <= c && c <= 'Z') {
                    return c - 'A' + 27;
                } else if ('a' <= c && c <= 'z') {
                    return c - 'a' + 57;
                } else {
                    switch( c ) {
                        case '!': return  0;
                        case '#': return  1;
                        case '%': return  2;
                        case '&': return  3;
                        case '(': return  4;
                        case ')': return  5;
                        case '+': return  6;
                        case ',': return  7;
                        case '-': return  8;
                        case '/': return  9;

                        case ':': return 20;
                        case ';': return 21;
                        case '<': return 22;
                        case '=': return 23;
                        case '>': return 24;
                        case '?': return 25;
                        case '@': return 26;

                        case '[': return 53;
                        case ']': return 54;
                        case '^': return 55;
                        case '_': return 56;

                        case '{': return 83;
                        case '}': return 84;
                        case '~': return 85;
                        default: return  -1;
                    }
                }
            }

        public:
            ascii86_alphabet() noexcept
            : alphabet("ascii86", 86, data, 0, s_code_point) {}
    };

    /**
     * Encodes a given positive decimal number to a symbolic string representing a given alphabet and its base.
     *
     * Besides using a custom alphabet, the following build-in alphabets are provided
     * - jau::codec::base::base64_alphabet
     * - jau::codec::base::base64url_alphabet
     * - jau::codec::base::natural86_alphabet
     * - jau::codec::base::ascii64_alphabet
     * - jau::codec::base::ascii86_alphabet
     *
     * @param num a positive decimal number
     * @param aspec the used alphabet specification
     * @param min_width minimum width of the encoded string, encoded zero is used for padding
     * @return the encoded string or an empty string if base exceeds alphabet::max_base() or invalid arguments
     *
     * @see encodeBase()
     * @see decodeBase()
     */
    std::string encode(int num, const alphabet& aspec, const unsigned int min_width=0) noexcept;

    /**
     * Encodes a given positive decimal number to a symbolic string representing a given alphabet and its base.
     *
     * Besides using a custom alphabet, the following build-in alphabets are provided
     * - jau::codec::base::base64_alphabet
     * - jau::codec::base::base64url_alphabet
     * - jau::codec::base::natural86_alphabet
     * - jau::codec::base::ascii64_alphabet
     * - jau::codec::base::ascii86_alphabet
     *
     * @param num a positive decimal number
     * @param aspec the used alphabet specification
     * @param min_width minimum width of the encoded string, encoded zero is used for padding
     * @return the encoded string or an empty string if base exceeds alphabet::max_base() or invalid arguments
     *
     * @see encodeBase()
     * @see decodeBase()
     */
    std::string encode(int64_t num, const alphabet& aspec, const unsigned int min_width=0) noexcept;

    /**
     * Decodes a given symbolic string representing a given alphabet and its base to a positive decimal number.
     *
     * Besides using a custom alphabet, the following build-in alphabets are provided
     * - jau::codec::base::base64_alphabet
     * - jau::codec::base::base64url_alphabet
     * - jau::codec::base::natural86_alphabet
     * - jau::codec::base::ascii64_alphabet
     * - jau::codec::base::ascii86_alphabet
     *
     * @param str an encoded string
     * @param aspec the used alphabet specification
     * @return the decoded decimal value or -1 if base exceeds alphabet::max_base(), unknown code-point or invalid arguments
     *
     * @see encodeBase()
     */
    int64_t decode(const std::string_view& str, const alphabet& aspec) noexcept;

    /**
     * Encodes given octets using the given alphabet and fixed base 64 encoding
     * according to `base64` [RFC 4648](https://www.rfc-editor.org/rfc/rfc4648.html).
     *
     * An error only occurs if in_len > 0 and resulting encoded string is empty.
     *
     * @param in_octets pointer to octets start
     * @param in_len length of octets in bytes
     * @param aspec the used base 64 alphabet specification
     * @return the encoded string, empty if base exceeds alphabet::max_base() or invalid arguments
     */
    std::string encode64(const void* in_octets, size_t in_len, const alphabet& aspec) noexcept;

    /**
     * Decodes a given symbolic string representing using given alphabet and fixed base 64 to octets
     * according to `base64` [RFC 4648](https://www.rfc-editor.org/rfc/rfc4648.html).
     *
     * An error only occurs if the encoded string length > 0 and resulting decoded octets size is empty.
     *
     * @param str encoded string
     * @param aspec the used base 64 alphabet specification
     * @return the decoded octets, empty if base exceeds alphabet::max_base(), unknown code-point or invalid arguments
     */
    std::vector<uint8_t> decode64(const std::string_view& str, const alphabet& aspec) noexcept;

    /**
     * Inserts a line feed (LF) character `\n` (ASCII 0x0a) after every period of characters.
     *
     * @param str the input string of characters, which will be mutated.
     * @param period period of characters after which one LF will be inserted.
     * @return count of inserted LF characters
     */
    size_t insert_lf(std::string& str, const size_t period) noexcept;

    /**
     * Removes line feed character from str.
     *
     * @param str the input string of characters, which will be mutated.
     * @return count of removed LF characters
     */
    size_t remove_lf(std::string& str) noexcept;

    /**
     * Encodes given octets using the given alphabet and fixed base 64 encoding
     * according to `base64` [RFC 4648](https://www.rfc-editor.org/rfc/rfc4648.html)
     * and adds line-feeds every 64 characters as required for PEM.
     *
     * An error only occurs if in_len > 0 and resulting encoded string is empty.
     *
     * @param in_octets pointer to octets start
     * @param in_len length of octets in bytes
     * @param aspec the used base 64 alphabet specification
     * @return the encoded string, empty if base exceeds alphabet::max_base() or invalid arguments
     */
    inline std::string encode64_pem(const void* in_octets, size_t in_len, const alphabet& aspec) noexcept {
        std::string e = encode64(in_octets, in_len, aspec);
        (void)insert_lf(e, 64);
        return e;
    }

    /**
     * Encodes given octets using the given alphabet and fixed base 64 encoding
     * according to `base64` [RFC 4648](https://www.rfc-editor.org/rfc/rfc4648.html)
     * and adds line-feeds every 76 characters as required for MIME.
     *
     * An error only occurs if in_len > 0 and resulting encoded string is empty.
     *
     * @param in_octets pointer to octets start
     * @param in_len length of octets in bytes
     * @param aspec the used base 64 alphabet specification
     * @return the encoded string, empty if base exceeds alphabet::max_base() or invalid arguments
     */
    inline std::string encode64_mime(const void* in_octets, size_t in_len, const alphabet& aspec) noexcept {
        std::string e = encode64(in_octets, in_len, aspec);
        (void)insert_lf(e, 76);
        return e;
    }

    /**
     * Decodes a given symbolic string representing using given alphabet and fixed base 64 to octets
     * according to `base64` [RFC 4648](https://www.rfc-editor.org/rfc/rfc4648.html)
     * and removes all linefeeds before decoding as required for PEM and MIME.
     *
     * An error only occurs if the encoded string length > 0 and resulting decoded octets size is empty.
     *
     * @param str and encoded string, will be copied
     * @param aspec the used base 64 alphabet specification
     * @return the decoded octets, empty if base exceeds alphabet::max_base(), unknown code-point or invalid arguments
     */
    inline std::vector<uint8_t> decode64_lf(const std::string_view& str, const alphabet& aspec) noexcept {
        std::string e(str); // costly copy
        (void)remove_lf(e);
        return decode64(e, aspec);
    }

    /**
     * Decodes a given symbolic string representing using given alphabet and fixed base 64 to octets
     * according to `base64` [RFC 4648](https://www.rfc-editor.org/rfc/rfc4648.html)
     * and removes all linefeeds before decoding as required for PEM and MIME.
     *
     * An error only occurs if the encoded string length > 0 and resulting decoded octets size is empty.
     *
     * @param str and encoded string, no copy, will be mutated
     * @param aspec the used base 64 alphabet specification
     * @return the decoded octets, empty if base exceeds alphabet::max_base(), unknown code-point or invalid arguments
     */
    inline std::vector<uint8_t> decode64_lf(std::string& str, const alphabet& aspec) noexcept {
        (void)remove_lf(str);
        return decode64(str, aspec);
    }

    /**@}*/

} // namespace jau::codec::base

/** \example test_intdecstring01.cpp
 * This C++ unit test validates the jau::to_decstring implementation
 */

#endif /* JAU_BASE_CODEC_HPP_ */
