/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
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

import java.lang.reflect.Array;
import java.util.List;

public class BasicTypes {

    public static void bswap_6bytes(final byte[/*6*/] source, final int source_pos, final byte[/*6*/] sink, final int sink_pos) {
        sink[0+sink_pos] = source[5+source_pos];
        sink[1+sink_pos] = source[4+source_pos];
        sink[2+sink_pos] = source[3+source_pos];
        sink[3+sink_pos] = source[2+source_pos];
        sink[4+sink_pos] = source[1+source_pos];
        sink[5+sink_pos] = source[0+source_pos];
    }
    public static void bswap(final byte[] source, final int source_pos, final byte[] sink, final int sink_pos, final int len) {
        for(int i=0; i < len; ++i) {
            sink[i+sink_pos] = source[len-1-i+source_pos];
        }
    }

    /**
     * Converts a given hexadecimal string representation to a byte array.
     *
     * In case a non valid hexadecimal digit appears in the given string,
     * conversion ends and returns the byte array up until the violation.
     *
     * @param hexstr the hexadecimal string representation
     * @param lsbFirst low significant byte first
     * @param checkLeading0x if true, checks for a leading `0x` and removes it, otherwise not.
     * @return the matching byte array
     */
    public static byte[] hexStringBytes(final String hexstr, final boolean lsbFirst, final boolean checkLeading0x) {
        final int offset;
        if( checkLeading0x && hexstr.startsWith("0x") ) {
            offset = 2;
        } else {
            offset = 0;
        }
        final int size = ( hexstr.length() - offset ) / 2;
        final byte[] b = new byte[size];
        if( lsbFirst ) {
            for (int i = 0; i < b.length; i++) {
                final int idx = i * 2;
                final int h = Character.digit( hexstr.charAt( offset + idx ), 16 );
                final int l = Character.digit( hexstr.charAt( offset + idx + 1 ), 16 );
                if( 0 <= h && 0 <= l ) {
                    b[i] = (byte) ( (h << 4) + l );
                } else {
                    // invalid char
                    final byte[] b2 = new byte[i];
                    System.arraycopy(b, 0, b2, 0, i);
                    return b2;
                }
            }
        } else {
            int i=0;
            for (int idx = (b.length-1)*2; idx >= 0; idx-=2, i++) {
                final int h = Character.digit( hexstr.charAt( offset + idx ), 16);
                final int l = Character.digit( hexstr.charAt( offset + idx + 1 ), 16);
                if( 0 <= h && 0 <= l ) {
                    b[i] = (byte) ( (h << 4) + l );
                } else {
                    // invalid char
                    final byte[] b2 = new byte[i];
                    System.arraycopy(b, 0, b2, 0, i);
                    return b2;
                }
            }
        }
        return b;
    }

    /**
     * Produce a lower-case hexadecimal string representation of the given byte values.
     * <p>
     * If lsbFirst is true, orders LSB left -> MSB right, usual for byte streams. Result will not have a leading `0x`.<br>
     * Otherwise orders MSB left -> LSB right, usual for readable integer values. Result will have a leading `0x`.
     * </p>
     * @param bytes the byte array to represent
     * @param offset offset in byte array to the first byte to print.
     * @param length number of bytes to print. If negative, will use {@code bytes.length - offset}.
     * @param lsbFirst true having the least significant byte printed first (lowest addressed byte to highest),
     *                 otherwise have the most significant byte printed first (highest addressed byte to lowest).
     *                 A leading `0x` will be prepended if {@code lsbFirst == false}.
     * @return the hex-string representation of the data
     */
    public static String bytesHexString(final byte[] bytes, final int offset, final int length,
                                        final boolean lsbFirst)
    {
        final int byte_len = 0 <= length ? length : bytes.length - offset;
        if( byte_len > ( bytes.length - offset ) ) {
            throw new IllegalArgumentException("byte[] ( "+bytes.length+" - "+offset+" ) < "+length+" bytes");
        }
        // final char[] hex_array = lowerCase ? HEX_ARRAY_LOW : HEX_ARRAY_BIG;
        final char[] hex_array = HEX_ARRAY_LOW;
        final char[] hexChars;
        final int char_offset;

        if( lsbFirst ) {
            // LSB left -> MSB right, no leading `0x`
            char_offset = 0;
            hexChars = new char[byte_len * 2];
            for (int j = 0; j < byte_len; j++) {
                final int v = bytes[offset + j] & 0xFF;
                hexChars[char_offset + j * 2] = hex_array[v >>> 4];
                hexChars[char_offset + j * 2 + 1] = hex_array[v & 0x0F];
            }
        } else {
            // MSB left -> LSB right, with leading `0x`
            char_offset = 2;
            hexChars = new char[2 + byte_len * 2];
            hexChars[0] = '0';
            hexChars[1] = 'x';
            int k=0;
            for (int j = byte_len-1; j >= 0; j--, k++) {
                final int v = bytes[offset + j] & 0xFF;
                hexChars[char_offset + k * 2] = hex_array[v >>> 4];
                hexChars[char_offset + k * 2 + 1] = hex_array[v & 0x0F];
            }
        }
        return new String(hexChars);
    }
    private static final char[] HEX_ARRAY_LOW = "0123456789abcdef".toCharArray();
    private static final char[] HEX_ARRAY_BIG = "0123456789ABCDEF".toCharArray();

    /**
     * Produce a hexadecimal string representation of the given byte value.
     * @param sb the StringBuilder destination to append
     * @param value the byte value to represent
     * @param lowerCase true to use lower case hex-chars, otherwise capital letters are being used.
     * @return the given StringBuilder for chaining
     */
    public static StringBuilder byteHexString(final StringBuilder sb, final byte value, final boolean lowerCase)
    {
        final char[] hex_array = lowerCase ? HEX_ARRAY_LOW : HEX_ARRAY_BIG;
        final int v = value & 0xFF;
        sb.append(hex_array[v >>> 4]);
        sb.append(hex_array[v & 0x0F]);
        return sb;
    }

    /**
     * Produce a decimal string representation of an integral integer value.
     * @tparam T an integral integer type
     * @param v the integral integer value
     * @param separator if not 0, use as separation character, otherwise no separation characters are being used
     * @param width the minimum number of characters to be printed. Add padding with blank space if result is shorter.
     * @return the string representation of the integral integer value
     */
    public static String int32DecString(final int v, final char separator, final int width) {
        final int v_sign = IntMath.sign(v);
        final int digit10_count1 = IntMath.digits10(v, v_sign, true /* sign_is_digit */);
        final int digit10_count2 = v_sign < 0 ? digit10_count1 - 1 : digit10_count1; // less sign

        final int comma_count = 0 == separator ? 0 : ( digit10_count1 - 1 ) / 3;
        final int net_chars = digit10_count1 + comma_count;
        final int total_chars = Math.max(width, net_chars);
        // std::string res(total_chars, ' ');
        final char res[] = new char[total_chars];

        int n = v;
        int char_iter = 0;

        for(int digit10_iter = 0; digit10_iter < digit10_count2 /* && char_iter < total_chars */; digit10_iter++ ) {
            final int digit = v_sign < 0 ? IntMath.invert_sign( n % 10 ) : n % 10;
            n /= 10;
            if( 0 < digit10_iter && 0 == digit10_iter % 3 ) {
                res[total_chars-1-(char_iter++)] = separator;
            }
            res[total_chars-1-(char_iter++)] = (char)('0' + digit);
        }
        if( v_sign < 0 /* && char_iter < total_chars */ ) {
            res[total_chars-1-(char_iter++)] = '-';
        }
        return new String(res);
    }

    private static final int radix_max_base = 143;
    private static final String radix_symbols = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!#%&()+,-.;=@[]^_{}~ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜ¢£¥₧ƒáíóúñÑªº¿½¼¡«»αßΓπΣσµτΦΘΩδ∞φε";

    /**
     * Converts a given positive decimal number to a symbolix string using given radix.
     *
     * ## Constraints
     * - Code page 437 compatible
     * - Forbidden [v]fat chars: <>:"/\|?*
     * - Further excluding quoting chars: "'$ and space to avoid any quoting issues
     *
     * Note: Beyond base 82, native C/C++ alike char encoding doesn't fit as code page 437 won't fit into ASCII. UTF-8 or wide chars would be required.
     * If compatibility with narrow byte chars using ASCII is required, don't exceed base 82.
     *
     * ## Examples
     * ### Base 62
     * - 62**3-1 = 238327, 238327/365d = 652.95 years
     * - 62 `0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ`
     *
     * ### Base 82
     * - 82**3-1 = 551367, 551367/365d = 1510.59 years
     * - 82 `0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!#%&()+,-.;=@[]^_{}~`
     *
     * ### Base 143
     * - 143**3-1 = 2924206, 2924206/365d = 8011.52 years
     * - 143 `0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!#%&()+,-.;=@[]^_{}~ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜ¢£¥₧ƒáíóúñÑªº¿½¼¡«»αßΓπΣσµτΦΘΩδ∞φε`
     *
     * @param num a positive decimal number
     * @param base radix to use, either 62, 82 or 143 where 143 is the maximum.
     * @param padding_width minimal width of the encoded radix string
     * @param padding_char padding character, usually '0'
     * @return the encoded radix string or an empty string if base > 143 or num is negative
     * @see #dec_to_radix(long, int, int, char)
     * @see #radix_to_dec(String, int)
     */
    public static String dec_to_radix(int num, final int base, final int padding_width, final char padding_char) {
        if( 0 > num || 0 >= base || base > radix_max_base ) {
            return "";
        }

        final StringBuilder res = new StringBuilder();
        do {
            res.insert( 0, radix_symbols.charAt( num % base ) );
            num /= base;
        } while ( 0 != num );

        for(int i=res.length(); i<padding_width; ++i) {
            res.insert(0, padding_char);
        }
        return res.toString();
    }

    /**
     * Converts a given positive decimal number to a symbolix string using given radix.
     *
     * See {@link #dec_to_radix(int, int, int)} for details.
     *
     * @param num a positive decimal number
     * @param base radix to use, either 62, 82 or 143 where 143 is the maximum.
     * @param padding_width minimal width of the encoded radix string
     * @param padding_char padding character, usually '0'
     * @return the encoded radix string or an empty string if base > 143 or num is negative
     * @see #dec_to_radix(int, int, int, char)
     * @see #radix_to_dec(String, int)
     */
    public static String dec_to_radix(long num, final int base, final int padding_width, final char padding_char) {
        if( 0 > num || 0 >= base || base > radix_max_base ) {
            return "";
        }

        final StringBuilder res = new StringBuilder();
        final long lbase = base;
        do {
            res.insert( 0, radix_symbols.charAt( (int)( num % lbase ) ) );
            num /= lbase;
        } while ( 0 != num );

        for(int i=res.length(); i<padding_width; ++i) {
            res.insert(0, padding_char);
        }
        return res.toString();
    }

    /**
     * Converts a given positive decimal number to a symbolix string using given radix.
     *
     * See {@link #dec_to_radix(int, int, int)} for details.
     *
     * @param num a positive decimal number
     * @param base radix to use, either 62, 82 or 143 where 143 is the maximum.
     * @return the encoded radix string or an empty string if base > 143 or num is negative
     * @see #dec_to_radix(int, int, int, char)
     * @see #radix_to_dec(String, int)
     */
    public static String dec_to_radix(final int num, final int base) {
        return dec_to_radix(num, base, 0 /* padding_width */, '0');
    }

    /**
     * Converts a given positive decimal number to a symbolix string using given radix.
     *
     * See {@link #dec_to_radix(int, int, int)} for details.
     *
     * @param num a positive decimal number
     * @param base radix to use, either 62, 82 or 143 where 143 is the maximum.
     * @return the encoded radix string or an empty string if base > 143 or num is negative
     * @see #dec_to_radix(long, int, int, char)
     * @see #radix_to_dec(String, int)
     */
    public static String dec_to_radix(final long num, final int base) {
        return dec_to_radix(num, base, 0 /* padding_width */, '0');
    }

    /**
     * Converts a given positive decimal number to a symbolic string using given radix.
     *
     * See {@link #dec_to_radix(int, int, int)} for details.
     *
     * @param str an encoded radix string
     * @param base radix to use, either 62, 82 or 143 where 143 is the maximum.
     * @return the decoded radix decimal value or -1 if base > 143
     * @see #dec_to_radix(int, int)
     */
    public static long radix_to_dec(final String str, final int base) {
        if( base > radix_max_base ) {
            return -1;
        }
        final int str_len = str.length();
        long res = 0;
        for (int i = 0; i < str_len; ++i) {
            res = res * base + radix_symbols.indexOf(str.charAt(i));
        }
        return res;
    }

    /**
     * Returns all valid consecutive UTF-8 characters within buffer
     * in the range offset -> size or until EOS.
     * <p>
     * In case a non UTF-8 character has been detected,
     * the content will be cut off and the decoding loop ends.
     * </p>
     * <p>
     * Method utilizes a finite state machine detecting variable length UTF-8 codes.
     * See <a href="http://bjoern.hoehrmann.de/utf-8/decoder/dfa/">Bjoern Hoehrmann's site</a> for details.
     * </p>
     */
    // public static native String decodeUTF8String(final byte[] buffer, final int offset, final int size);

    /**
     * Convert a non empty list to an array of same type.
     *
     * @param <E> the element type of the list
     * @param list the list instance
     * @throws IllegalArgumentException if given list instance is empty
     */
    @SuppressWarnings("unchecked")
    public static <E> E[] toArray(final List<E> list) throws IllegalArgumentException {
        if( 0 == list.size() ) {
            throw new IllegalArgumentException("Given list is empty");
        }
        final Class<E> clazz;
        {
            final E e0 = list.get(0);
            clazz = (Class<E>) e0.getClass();
        }
        final E[] res = (E[]) Array.newInstance(clazz, list.size());
        for(int i=0; i < res.length; ++i) {
            res[i] = list.get(i);
        }
        return res;
    }

}
