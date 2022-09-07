/**
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
package org.jau.util;

import java.nio.ByteBuffer;

/**
 * Base codecs, i.e. changing the decimal or binary values' base for a different representation.
 */
public class BaseCodec {

    /**
     * Base Alphabet Specification providing the alphabet for encode() and decode().
     *
     * @see {@link BaseCodec#encode(int, int, Alphabet, int)}
     * @see {@link BaseCodec#encode(long, int, Alphabet, int)}
     * @see {@link BaseCodec#decode(String, int, Alphabet)}
     */
    public static abstract class Alphabet {
        private final String name_;
        private final int max_base_;
        private final String symbols_;
        private final char padding64_;

        protected Alphabet(final String name, final int max_base, final String symbols, final char passing64) {
            this.name_ = name;
            this.max_base_ = max_base;
            this.symbols_ = symbols;
            this.padding64_ = passing64;
        }

        public final String name() { return name_; }
        public final int max_base() { return max_base_; }
        public final String symbols() { return symbols_; }

        /** Padding symbol for base <= 64 and block encoding only. May return zero for no padding. */
        public final char padding64() { return padding64_; }

        /** Returns the code-point of the given character or -1 if not element of this alphabet. */
        public abstract int code_point(final char c);

        public final char charAt( final int pos ) { return symbols().charAt(pos); }
        public final char symbol( final int pos ) { return symbols().charAt(pos); }

        @Override
        public boolean equals(final Object o) {
            if( this == o ) {
                return true;
            }
            if( o instanceof Alphabet ) {
                final Alphabet oa = (Alphabet)o;
                    return max_base() == max_base() && name().equals(oa.name()) && symbols().equals(oa.symbols());
            }
            return false;
        }

        @Override
        public String toString() {
            return "Alphabet["+name_+", base <= "+max_base_+"]";
        }
    };

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
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - [`base64` alphabet](https://www.rfc-editor.org/rfc/rfc4648.html), identical order
     * - Excludes quoting chars: "'$ and space
     * - Not supporting ASCII code-point sorting.
     * - Order: `A` < `a` < `0` < `/`
     */
    public static class Base64Alphabet extends Alphabet {
        private static final String data = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        @Override
        public int code_point(final char c) {
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

        public Base64Alphabet() {
            super("base64", 64, data, '=');
        }
    }

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
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - [`base64url` alphabet](https://www.rfc-editor.org/rfc/rfc4648.html), identical order
     * - Safe URL and filename use
     * - Excludes forbidden [v]fat chars: `<>:"/\|?*`
     * - Excludes quoting chars: "'$ and space
     * - Not supporting ASCII code-point sorting.
     * - Order: `A` < `a` < `0` < `_`
     *
     * @see encodeBase()
     * @see decodeBase()
     */
    public static class Base64urlAlphabet extends Alphabet {
        private static final String data  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

        @Override
        public int code_point(final char c) {
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

        public Base64urlAlphabet() {
            super("base64url", 64, data, '=');
        }
    }

    /**
     * Natural base 86 alphabet including a safe base 64 subset, both without ASCII code-point sorting order.
     *
     * Order is considered a natural extension of decimal symbols, i.e. `0` < `a` < `A` < `_` < `~`
     *
     * - Value: `0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_!#%&()+,/:;<=>?@[]^{}~`
     * - Padding: `=` (base <= 64)
     *
     * ### Properties up to base <= 64
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - [`base64url` alphabet](https://www.rfc-editor.org/rfc/rfc4648.html), but different order
     * - Safe URL and filename use
     * - Excludes forbidden [v]fat chars: `<>:"/\|?*`
     * - Excludes quoting chars: "'$ and space
     * - Not supporting ASCII code-point sorting.
     * - Order: `0` < `a` < `A` < `_`
     *
     * ### Properties base range [65 .. 86]
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - Excludes quoting chars: "'$ and space
     * - Not supporting ASCII code-point sorting.
     * - Order: `0` < `a` < `A` < `_` < `~`
     *
     * @see encodeBase()
     * @see decodeBase()
     */
    public static class Natural86Alphabet extends Alphabet {
        private static final String data = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_!#%&()+,/:;<=>?@[]^{}~";

        @Override
        public int code_point(final char c) {
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

        public Natural86Alphabet() {
            super("natural86", 86, data, '=');
        }
    }

    /**
     * Safe base 38 alphabet with ASCII code-point sorting order.
     *
     * - Value: `-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_`
     * - Padding: `=`
     *
     * ### Properties
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - Safe URL and filename use
     * - Excludes forbidden [v]fat chars: `<>:"/\|?*`
     * - Only using upper-case letters for unique filename under vfat
     * - Excludes quoting chars: "'$ and space
     * - Supporting ASCII code-point sorting.
     * - Order: `-` < `0` < `A` < `a` < `z`
     *
     * @see encodeBase()
     * @see decodeBase()
     */
    public static class Ascii38Alphabet extends Alphabet {
        private static final String data = "-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_";

        @Override
        public int code_point(final char c) {
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

        public Ascii38Alphabet() {
            super("ascii38", 38, data, '=');
        }
    }

    /**
     * Safe base 64 alphabet with ASCII code-point sorting order.
     *
     * - Value: `-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz`
     * - Padding: `=`
     *
     * ### Properties
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - [`base64url` alphabet](https://www.rfc-editor.org/rfc/rfc4648.html), but different order
     * - Safe URL and filename use
     * - Excludes forbidden [v]fat chars: `<>:"/\|?*`
     * - Excludes quoting chars: "'$ and space
     * - Supporting ASCII code-point sorting.
     * - Order: `-` < `0` < `A` < `a` < `z`
     *
     * @see encodeBase()
     * @see decodeBase()
     */
    public static class Ascii64Alphabet extends Alphabet {
        private static final String data = "-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";

        @Override
        public int code_point(final char c) {
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

        public Ascii64Alphabet() {
            super("ascii64", 64, data, '=');
        }
    }

    /**
     * Base 86 alphabet with ASCII code-point sorting order.
     *
     * - Value: `!#%&()+,-/0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_abcdefghijklmnopqrstuvwxyz{}~`
     * - Padding: None
     *
     * ### Properties
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - Excludes quoting chars: "'$ and space
     * - Supporting ASCII code-point sorting.
     * - Order: `!` < `0` < `:` < `A` < `[` < `a` < `{` < `~`
     *
     * @see encodeBase()
     * @see decodeBase()
     */
    public static class Ascii86Alphabet extends Alphabet {
        private static final String data = "!#%&()+,-/0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_abcdefghijklmnopqrstuvwxyz{}~";

        @Override
        public int code_point(final char c) {
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

        public Ascii86Alphabet() {
            super("ascii86", 86, data, (char)0);
        }
    }

    /**
     * Encodes a given positive decimal number to a symbolic string representing a given base and alphabet.
     *
     * Besides using a custom alphabet, the following build-in alphabets are provided
     * - {@link BaseCodec.Base64Alphabet}
     * - {@link BaseCodec.Base64urlAlphabet}
     * - {@link BaseCodec.Natural86Alphabet}
     * - {@link BaseCodec.Ascii64Alphabet}
     * - {@link BaseCodec.Ascii86Alphabet}
     *
     * @param num a positive decimal number
     * @param base positive radix to use <= Alphabet.max_base()
     * @param aspec the used alphabet specification
     * @param min_width minimum width of the encoded string, encoded zero is used for padding
     * @return the encoded string or an empty string if base exceeds Alphabet.max_base() or invalid arguments
     *
     * @see {@link BaseCodec#encode(long, int, Alphabet, int)}
     * @see {@link BaseCodec#decode(String, int, Alphabet)}
     */
    public static String encode(int num, final int base, final Alphabet aspec, final int min_width) {
        if( 0 > num || 1 >= base || base > aspec.max_base() ) {
            return "";
        }
        final StringBuilder res = new StringBuilder();
        do {
            res.insert( 0, aspec.charAt( num % base ) ); // safe: base <= alphabet.length()
            num /= base;
        } while ( 0 != num );

        final char s0 = aspec.charAt(0);
        for(int i=res.length(); i<min_width; ++i) {
            res.insert(0, s0);
        }
        return res.toString();
    }

    /**
     * Encodes a given positive decimal number to a symbolic string representing a given base and alphabet.
     *
     * Besides using a custom alphabet, the following build-in alphabets are provided
     * - {@link BaseCodec.Base64Alphabet}
     * - {@link BaseCodec.Base64urlAlphabet}
     * - {@link BaseCodec.Natural86Alphabet}
     * - {@link BaseCodec.Ascii64Alphabet}
     * - {@link BaseCodec.Ascii86Alphabet}
     *
     * @param num a positive decimal number
     * @param base positive radix to use <= Alphabet.max_base()
     * @param aspec the used alphabet specification
     * @param min_width minimum width of the encoded string, encoded zero is used for padding
     * @return the encoded string or an empty string if base exceeds Alphabet.max_base() or invalid arguments
     *
     * @see {@link BaseCodec#encode(int, int, Alphabet, int)}
     * @see {@link BaseCodec#decode(String, int, Alphabet)}
     */
    public static String encode(long num, final int base, final Alphabet aspec, final int min_width) {
        if( 0 > num || 1 >= base || base > aspec.max_base() ) {
            return "";
        }
        final StringBuilder res = new StringBuilder();
        final long lbase = base;
        do {
            res.insert( 0, aspec.charAt( (int)( num % lbase ) ) ); // safe: base <= alphabet.length()
            num /= lbase;
        } while ( 0 != num );

        final char s0 = aspec.charAt(0);
        for(int i=res.length(); i<min_width; ++i) {
            res.insert(0, s0);
        }
        return res.toString();
    }

    /**
     * Encodes a given positive decimal number to a symbolic string representing a given base and alphabet.
     *
     * Besides using a custom alphabet, the following build-in alphabets are provided
     * - {@link BaseCodec.Base64Alphabet}
     * - {@link BaseCodec.Base64urlAlphabet}
     * - {@link BaseCodec.Natural86Alphabet}
     * - {@link BaseCodec.Ascii64Alphabet}
     * - {@link BaseCodec.Ascii86Alphabet}
     *
     * @param num a positive decimal number
     * @param base positive radix to use <= Alphabet.max_base()
     * @param aspec the used alphabet specification
     * @return the encoded string or an empty string if base exceeds Alphabet.max_base() or invalid arguments
     *
     * @see {@link BaseCodec#encode(int, int, Alphabet, int)}
     * @see {@link BaseCodec#decode(String, int, Alphabet)}
     */
    public static String encode(final int num, final int base, final Alphabet aspec) {
        return encode(num, base, aspec, 0 /* min_width */);
    }

    /**
     * Encodes a given positive decimal number to a symbolic string representing a given base and alphabet.
     *
     * Besides using a custom alphabet, the following build-in alphabets are provided
     * - {@link BaseCodec.Base64Alphabet}
     * - {@link BaseCodec.Base64urlAlphabet}
     * - {@link BaseCodec.Natural86Alphabet}
     * - {@link BaseCodec.Ascii64Alphabet}
     * - {@link BaseCodec.Ascii86Alphabet}
     *
     * @param num a positive decimal number
     * @param base positive radix to use <= Alphabet.max_base()
     * @param aspec the used alphabet specification
     * @return the encoded string or an empty string if base exceeds Alphabet.max_base() or invalid arguments
     *
     * @see {@link BaseCodec#encode(long, int, Alphabet, int)}
     * @see {@link BaseCodec#decode(String, int, Alphabet)}
     */
    public static String encode(final long num, final int base, final Alphabet aspec) {
        return encode(num, base, aspec, 0 /* min_width */);
    }

    /**
     * Decodes a given symbolic string representing a given base and alphabet to a positive decimal number.
     *
     * Besides using a custom alphabet, the following build-in alphabets are provided
     * - {@link BaseCodec.Base64Alphabet}
     * - {@link BaseCodec.Base64urlAlphabet}
     * - {@link BaseCodec.Natural86Alphabet}
     * - {@link BaseCodec.Ascii64Alphabet}
     * - {@link BaseCodec.Ascii86Alphabet}
     *
     * @param str an encoded string
     * @param base positive radix to use <= Alphabet.max_base()
     * @param aspec the used alphabet specification
     * @return the decoded radix decimal value or -1 if base exceeds Alphabet.max_base(), unknown code-point or invalid arguments
     *
     * @see {@link BaseCodec#encode(int, int, Alphabet, int)}
     * @see {@link BaseCodec#encode(long, int, Alphabet, int)}
     */
    public static long decode(final String str, final int base, final Alphabet aspec) {
        if( 1 >= base || base > aspec.max_base() ) {
            return -1;
        }
        final int str_len = str.length();
        long res = 0;
        for (int i = 0; i < str_len; ++i) {
            final int d = aspec.code_point( str.charAt(i) );
            if( 0 > d || d >= base ) {
                return -1; // encoded value not found
            }
            res = res * base + d;
        }
        return res;
    }

    private static int to_int(final byte b) { return b & 0xff; }

    /**
     * Encodes given octets using the given alphabet and fixed base 64 encoding
     * according to `base64` [RFC 4648](https://www.rfc-editor.org/rfc/rfc4648.html).
     *
     * An error only occurs if in_len > 0 and resulting encoded string is empty.
     *
     * @param in_octets source byte array
     * @param in_pos index to octets start
     * @param in_len length of octets in bytes
     * @param aspec the used alphabet specification
     * @return the encoded string, empty if base exceeds alphabet::max_base() or invalid arguments
     */
    public static StringBuilder encode64(final byte[] in_octets, int in_pos, int in_len, final Alphabet aspec) {
        if( 64 > aspec.max_base() || in_pos + in_len > in_octets.length ) {
            return new StringBuilder(0);
        }
        final char padding = aspec.padding64();

        final int out_len = ( in_len + 2 ) / 3 * 4; // estimate ..
        final StringBuilder res = new StringBuilder(out_len);

        while( 0 < in_len && 0 < out_len ) {
            // Note: Addition is basically a bitwise XOR, plus carry bit

            // 1st symbol
            res.append( aspec.charAt( ( to_int(in_octets[in_pos+0]) >> 2 ) & 0x3f ) ); // take in[0] 6 bits[7..2] -> symbol[5..0]
            if( 0 == --in_len ) {
                // len == 1 bytes
                // 2nd symbol
                res.append( aspec.charAt(   ( to_int(in_octets[in_pos+0]) << 4 ) & 0x3f ) ); // take in[0] 2 bits[1..0] -> symbol[5..4]
                if( 0 != padding ) {
                    res.append(padding);
                    res.append(padding);
                }
                break;
            } else {
                // len >= 2 bytes
                // 2nd symbol
                res.append( aspec.charAt( ( ( to_int(in_octets[in_pos+0]) << 4 ) + ( to_int(in_octets[in_pos+1]) >> 4) ) & 0x3f ) ); // take ( in[0] 2 bits[1..0] -> symbol[5..4] ) + ( int[1] 4 bits[7..4] -> symbol[3..0] )
            }
            if( 0 == --in_len ) {
                // len == 2 bytes
                // 3rd symbol
                res.append( aspec.charAt(   ( to_int(in_octets[in_pos+1]) << 2 ) & 0x3f ) ); // take in[1] 4 bits[3..0] -> symbol[5..2]
                if( 0 != padding ) {
                    res.append(padding);
                }
                break;
            } else {
                // len >= 3 bytes
                // 3rd symbol
                res.append( aspec.charAt( ( ( to_int(in_octets[in_pos+1]) << 2 ) + ( to_int(in_octets[in_pos+2]) >> 6) ) & 0x3f ) ); // take ( in[1] 4 bits[3..0] -> symbol[5..2] ) + ( int[2] 2 bits[7..6] -> symbol[1..0] )
                // 4th symbol
                res.append( aspec.charAt(     to_int(in_octets[in_pos+2]) & 0x3f ) ); // take in[2] 6 bits[5..0] -> symbol[5..0]
                --in_len;
                in_pos+=3;
            }
        }
        return res;
    }

    /**
     * Decodes a given symbolic string representing using given alphabet and fixed base 64 to octets
     * according to `base64` [RFC 4648](https://www.rfc-editor.org/rfc/rfc4648.html).
     *
     * An error only occurs if the encoded string length > 0 and resulting decoded octets size is empty.
     *
     * @param in_code encoded string
     * @param aspec the used alphabet specification
     * @return the decoded octets, empty if base exceeds alphabet::max_base(), unknown code-point or invalid arguments
     */
    public static ByteBuffer decode64(final String in_code, final Alphabet aspec) {
        if( 64 > aspec.max_base() ) {
            return ByteBuffer.allocate(0); // Error
        }
        int in_len = in_code.length();
        if( in_len == 0 ) {
            return ByteBuffer.allocate(0); // OK
        }
        final char padding = aspec.padding64();

        final int out_len = 3 * ( in_len / 4 ) + 2; // estimate w/ potentially up to 2 additional bytes
        final ByteBuffer res = ByteBuffer.allocate(out_len);
        int in_pos = 0;

        while( in_len >= 2 ) {
            final int cp0 = aspec.code_point( in_code.charAt( in_pos + 0 ) );
            final int cp1 = aspec.code_point( in_code.charAt( in_pos + 1 ) );
            if( 0 > cp0 || cp0 >= 64 || 0 > cp1 || cp1 >= 64 ) {
                break;
            }
            res.put( (byte)(cp0 << 2 | cp1 >> 4) );
            if( 2 == in_len ) {
                if( 0 == padding ) {
                    in_len = 0; // accept w/o padding
                }
                break;
            }
            if( padding == in_code.charAt( in_pos + 2 ) ) {
                if( 4 != in_len ) {
                    break;
                }
                if( padding != in_code.charAt( in_pos + 3 ) ) {
                    break;
                }
            } else {
                final int cp2 = aspec.code_point( in_code.charAt( in_pos + 2 ) );
                if( 0 > cp2 || cp2 >= 64 ) {
                    break;
                }
                res.put( (byte)( ( ( cp1 << 4 ) & 0xf0 ) | ( cp2 >> 2 ) ) );
                if( 3 == in_len ) {
                    if( 0 == padding ) {
                        in_len = 0; // accept w/o padding
                    }
                    break;
                }
                if( padding == in_code.charAt( in_pos + 3 ) ) {
                    if( 4 != in_len ) {
                        break;
                    }
                } else {
                    final int cp3 = aspec.code_point( in_code.charAt( in_pos + 3 ) );
                    if( 0 > cp3 || cp3 >= 64 ) {
                        break;
                    }
                    res.put( (byte)( ( ( cp2 << 6 ) & 0xc0 ) | cp3 ) );
                }
            }
            in_pos += 4;
            in_len -= 4;
        }

        if( 0 != in_len ) {
            // System.err.printf("in_len %d/%d at '%s', out_len %d/%d\n", in_pos, in_code.length(), in_code, res.position(), out_len);
            res.clear(); // decoding error, position = 0, limit = capacity
        }
        res.flip(); // limit = position, position = 0, remaining() = limit - position
        return res;
    }

    /**
     * Inserts a line feed (LF) character `\n` (ASCII 0x0a) after every period of characters.
     *
     * @param str the input string of characters, which will be mutated.
     * @param period period of characters after which one LF will be inserted.
     * @return count of inserted LF characters
     */
    public static int insert_lf(final StringBuilder str, final int period) {
        int count = 0;
        for(int i = period; i < str.length(); i += period + 1) {
            str.insert(i, "\n");
            ++count;
        }
        return count;
    }

    /**
     * Removes line feed character from str.
     *
     * @param str the input string of characters, which will be mutated.
     * @return count of removed LF characters
     */
    public static int remove_lf(final StringBuilder str) {
        int count = 0;
        int pos = 0;
        pos = str.indexOf("\n", 0);
        while( 0 < pos && pos <= str.length() ) {
            str.replace(pos, pos+1, "");
            ++count;
            pos = str.indexOf("\n", pos);
        }
        return count;
    }

    /**
     * Encodes given octets using the given alphabet and fixed base 64 encoding
     * according to `base64` [RFC 4648](https://www.rfc-editor.org/rfc/rfc4648.html)
     * and adds line-feeds every 64 characters as required for PEM.
     *
     * An error only occurs if in_len > 0 and resulting encoded string is empty.
     *
     * @param in_octets pointer to octets start
     * @param in_len length of octets in bytes
     * @param aspec the used alphabet specification
     * @return the encoded string, empty if base exceeds alphabet::max_base() or invalid arguments
     */
    public static StringBuilder encode64_pem(final byte[] in_octets, final int in_pos, final int in_len, final Alphabet aspec) {
        final StringBuilder e = encode64(in_octets, in_pos, in_len, aspec);
        insert_lf(e, 64);
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
     * @param aspec the used alphabet specification
     * @return the encoded string, empty if base exceeds alphabet::max_base() or invalid arguments
     */
    public static StringBuilder encode64_mime(final byte[] in_octets, final int in_pos, final int in_len, final Alphabet aspec) {
        final StringBuilder e = encode64(in_octets, in_pos, in_len, aspec);
        insert_lf(e, 76);
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
     * @param aspec the used alphabet specification
     * @return the decoded octets, empty if base exceeds alphabet::max_base(), unknown code-point or invalid arguments
     */
    public static ByteBuffer decode64_lf(final String str, final Alphabet aspec) {
        final StringBuilder e = new StringBuilder(str); // costly copy
        remove_lf(e);
        return decode64(e.toString(), aspec);
    }

    /**
     * Decodes a given symbolic string representing using given alphabet and fixed base 64 to octets
     * according to `base64` [RFC 4648](https://www.rfc-editor.org/rfc/rfc4648.html)
     * and removes all linefeeds before decoding as required for PEM and MIME.
     *
     * An error only occurs if the encoded string length > 0 and resulting decoded octets size is empty.
     *
     * @param str and encoded string, no copy, will be mutated
     * @param aspec the used alphabet specification
     * @return the decoded octets, empty if base exceeds alphabet::max_base(), unknown code-point or invalid arguments
     */
    public static ByteBuffer decode64_lf(final StringBuilder str, final Alphabet aspec) {
        remove_lf(str);
        return decode64(str.toString(), aspec);
    }

}
