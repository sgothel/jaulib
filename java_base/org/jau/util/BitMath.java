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

public class BitMath {
    /** Maximum 32 bit Unsigned Integer Value: {@code 0xffffffff} == {@value}. */
    public static final int UNSIGNED_INT_MAX_VALUE = 0xffffffff;

    /**
     * Maximum 32bit integer value being of {@link #isPowerOf2(int)}.
     * <p>
     * We rely on the JVM spec {@link Integer#SIZE} == 32.
     * </p>
     */
    public static final int MAX_POWER_OF_2 = 1 << ( Integer.SIZE - 2 );

    /**
     * Returns the 32 bit mask of n-bits, i.e. n low order 1's.
     * <p>
     * Implementation handles n == 32.
     * </p>
     * @throws IndexOutOfBoundsException if {@code b} is out of bounds, i.e. &gt; 32
     */
    public static int getBitMask(final int n) {
        if( 32 > n ) {
            return ( 1 << n ) - 1;
        } else if ( 32 == n ) {
            return UNSIGNED_INT_MAX_VALUE;
        } else {
            throw new IndexOutOfBoundsException("n <= 32 expected, is "+n);
        }
    }

    /**
     * Returns the number of set bits within given 32bit integer in O(1)
     * using a <i>HAKEM 169 Bit Count</i> inspired implementation:
     * <pre>
     *   http://www.inwap.com/pdp10/hbaker/hakmem/hakmem.html
     *   http://home.pipeline.com/~hbaker1/hakmem/hacks.html#item169
     *   http://tekpool.wordpress.com/category/bit-count/
     *   http://www.hackersdelight.org/
     * </pre>
     * <p>
     * We rely on the JVM spec {@link Integer#SIZE} == 32.
     * </p>
     */
    public static final int bitCount(int n) {
        // Note: Original used 'unsigned int',
        // hence we use the unsigned right-shift '>>>'
        /**
         * Original does not work due to lack of 'unsigned' right-shift and modulo,
         * we need 2-complementary solution, i.e. 'signed'.
                int c = n;
                c -= (n >>> 1) & 033333333333;
                c -= (n >>> 2) & 011111111111;
                return ( (c + ( c >>> 3 ) ) & 030707070707 ) & 0x3f; // % 63
         */
        // Hackers Delight, Figure 5-2, pop1 of pop.c.txt
        n = n - ((n >>> 1) & 0x55555555);
        n = (n & 0x33333333) + ((n >>> 2) & 0x33333333);
        n = (n + (n >>> 4)) & 0x0f0f0f0f;
        n = n + (n >>> 8);
        n = n + (n >>> 16);
        return n & 0x3f;
    }

    /**
     * Returns {@code true} if the given integer is a power of 2
     * <p>
     * Source: bithacks: http://www.graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
     * </p>
     */
    public static final boolean isPowerOf2(final int n) {
        return 0<n && 0 == (n & (n - 1));
    }

    /**
     * Returns the next higher power of 2 of 32-bit of given {@code n}
     * <p>
     * Source: bithacks: http://www.graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
     * </p>
     * <p>
     * We rely on the JVM spec {@link Integer#SIZE} == 32.
     * </p>
     */
    public static final int nextPowerOf2(int n) {
        n--;
        n |= n >>> 1;
        n |= n >>> 2;
        n |= n >>> 4;
        n |= n >>> 8;
        n |= n >>> 16;
        return (n < 0) ? 1 : n + 1; // avoid edge case where n is 0, it would return 0, which isn't a power of 2
    }

    /**
     * If the given {@code n} is not {@link #isPowerOf2(int)} return {@link #nextPowerOf2(int)},
     * otherwise return {@code n} unchanged.
     * <pre>
     * return isPowerOf2(n) ? n : nextPowerOf2(n);
     * </pre>
     * <p>
     * We rely on the JVM spec {@link Integer#SIZE} == 32.
     * </p>
     */
    public static final int roundToPowerOf2(final int n) {
        return isPowerOf2(n) ? n : nextPowerOf2(n);
    }

}
