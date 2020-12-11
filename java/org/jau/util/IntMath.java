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

public class IntMath {

    /**
     * Returns the value of the sign function.
     * <pre>
     * -1 for x < 0
     *  0 for x = 0
     *  1 for x > 0
     * </pre>
     * Implementation is type safe.
     * @param x the integral number
     * @return function result
     */
    public static int sign(final int x) {
        return (0 < x ? 1 : 0) - (x < 0 ? 1 : 0);
    }

    /**
     * See {@link #sign(int)}.
     */
    public static int sign(final long x) {
        return (0 < x ? 1 : 0) - (x < 0 ? 1 : 0);
    }
    /**
     * See {@link #sign(int)}.
     */
    public static short sign(final short x) {
        return (short) ( (0 < x ? 1 : 0) - (x < 0 ? 1 : 0) );
    }

    /**
     * Safely inverts the sign of an integral number.
     * <p>
     * Implementation takes special care to have T_MIN, i.e. std::numeric_limits<T>::min(),
     * converted to T_MAX, i.e. std::numeric_limits<T>::max().<br>
     * This is necessary since <code>T_MAX < | -T_MIN |</code> and the result would
     * not fit in the return type T otherwise.
     * </p>
     * Hence for the extreme minimum case:
     * <pre>
     * jau::invert_sign<int32_t>(INT32_MIN) = | INT32_MIN | - 1 = INT32_MAX
     * </pre>
     * Otherwise with x < 0:
     * <pre>
     * jau::invert_sign<int32_t>(x) = | x | = -x
     * </pre>
     * and x >= 0:
     * <pre>
     * jau::invert_sign<int32_t>(x) = -x
     * </pre>
     * @param x
     * @return
     */
    public static int invert_sign(final int x) {
        return Integer.MIN_VALUE == x ? Integer.MAX_VALUE  : -x;
    }

    /**
     * See {@link #invert_sign(int)}.
     */
    public static long invert_sign(final long x) {
        return Long.MIN_VALUE == x ? Long.MAX_VALUE  : -x;
    }
    /**
     * See {@link #invert_sign(int)}.
     */
    public static short invert_sign(final short x) {
        return Short.MIN_VALUE == x ? Short.MAX_VALUE  : (short)-x;
    }

    /**
     * Returns the absolute value of an integral number
     * <p>
     * Implementation uses jau::invert_sign() to have a safe absolute value conversion, if required.
     * </p>
     * @tparam T an integral number type
     * @param x the integral number
     * @return function result
     */
    public static int abs(final int x) {
        return sign(x) < 0 ? invert_sign( x ) : x;
    }

    /**
     * See {@link #abs(int)}
     */
    public static long abs(final long x) {
        return sign(x) < 0 ? invert_sign( x ) : x;
    }
    /**
     * See {@link #abs(int)}
     */
    public static short abs(final short x) {
        return sign(x) < 0 ? invert_sign( x ) : x;
    }

    /**
     * Returns the number of decimal digits of the given integral value number using std::log10<T>().<br>
     * If sign_is_digit == true (default), treats a potential negative sign as a digit.
     * <pre>
     * x < 0: 1 + (int) ( log10( -x ) ) + ( sign_is_digit ? 1 : 0 )
     * x = 0: 1
     * x > 0: 1 + (int) ( log10(  x ) )
     * </pre>
     * Implementation uses jau::invert_sign() to have a safe absolute value conversion, if required.
     * <p>
     * Convenience method, reusing precomputed sign of value to avoid redundant computations.
     * </p>
     * @param x the integral integer
     * @param x_sign the pre-determined sign of the given value x
     * @param sign_is_digit if true and value is negative, adds one to result for sign. Defaults to true.
     * @return digit count
     */
    public static int digits10(final int x, final int x_sign, final boolean sign_is_digit) {
        if( x_sign == 0 ) {
            return 1;
        }

        if( x_sign < 0 ) {
            return 1 + (int)( Math.log10( invert_sign( x ) ) ) + ( sign_is_digit ? 1 : 0 );
        } else {
            return 1 + (int)( Math.log10(              x   ) );
        }
    }

    /**
     * See {@link #digits10(int, int, boolean)
     */
    public static int digits10(final long x, final int x_sign, final boolean sign_is_digit) {
        if( x_sign == 0 ) {
            return 1;
        }

        if( x_sign < 0 ) {
            return 1 + (int)( Math.log10( invert_sign( x ) ) ) + ( sign_is_digit ? 1 : 0 );
        } else {
            return 1 + (int)( Math.log10(              x   ) );
        }
    }

    /**
     * Returns the number of decimal digits of the given integral value number using std::log10<T>().
     * If sign_is_digit == true (default), treats a potential negative sign as a digit.
     * <pre>
     * x < 0: 1 + (int) ( log10( -x ) ) + ( sign_is_digit ? 1 : 0 )
     * x = 0: 1
     * x > 0: 1 + (int) ( log10(  x ) )
     * </pre>
     * Implementation uses jau::invert_sign() to have a safe absolute value conversion, if required.
     * @tparam T an integral integer type
     * @param x the integral integer
     * @param sign_is_digit if true and value is negative, adds one to result for sign. Defaults to true.
     * @return digit count
     */
    public static int digits10(final int x, final boolean sign_is_digit) {
        return digits10(x, sign(x), sign_is_digit);
    }

    /**
     * See {@link #digits10(int, boolean)
     */
    public static int digits10(final long x, final boolean sign_is_digit) {
        return digits10(x, sign(x), sign_is_digit);
    }

}
