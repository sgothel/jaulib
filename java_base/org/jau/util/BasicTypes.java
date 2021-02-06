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

public class BasicTypes {

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
            for (int j = byte_len-1; j >= 0; j--) {
                final int v = bytes[offset + j] & 0xFF;
                hexChars[char_offset + j * 2] = hex_array[v >>> 4];
                hexChars[char_offset + j * 2 + 1] = hex_array[v & 0x0F];
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

}
