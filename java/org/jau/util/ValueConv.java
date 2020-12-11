/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2012 Gothel Software e.K.
 * Copyright (c) 2012 JogAmp Community.
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

/**
 * Utility class providing simple signed and unsigned primitive value conversions
 * for byte, short, int, float and double.
 * <p>
 * Non float to non float conversions are handled via float or double,
 * depending on the value range.
 * </p>
 */
public class ValueConv {
    public static final byte float_to_byte(final float v, final boolean dSigned) {
        // lossy
        if( dSigned ) {
            return (byte) ( v * ( v > 0 ? 127.0f : 128.0f ) );
        } else {
            return (byte) ( v * 255.0f );
        }
    }
    public static final short float_to_short(final float v, final boolean dSigned) {
        if( dSigned ) {
            return (short) ( v * ( v > 0 ? 32767.0f : 32768.0f ) );
        } else {
            return (short) ( v * 65535.0f );
        }
    }
    public static final int float_to_int(final float v, final boolean dSigned) {
        // float significand          0x007fffff
        // double significand 0x000fffffffffffffL
        //                  int min = 0x80000000 = -2147483648
        //                  int max = 0x7fffffff = +2147483647
        if( dSigned ) {
            return (int) ( v * ( v > 0 ? 2147483647.0 : 2147483648.0 ) );
        } else {
            return (int) (long) ( v * 4294967295.0 );
        }
    }

    public static final byte double_to_byte(final double v, final boolean dSigned) {
        // lossy
        if( dSigned ) {
            return (byte) ( v * ( v > 0 ? 127.0 : 128.0 ) );
        } else {
            return (byte) ( v * 255.0 );
        }
    }
    public static final short double_to_short(final double v, final boolean dSigned) {
        // lossy
        if( dSigned ) {
            return (short) ( v * ( v > 0 ? 32767.0 : 32768.0 ) );
        } else {
            return (short) ( v * 65535.0 );
        }
    }
    public static final int double_to_int(final double v, final boolean dSigned) {
        // lossy
        if( dSigned ) {
            return (int) ( v * ( v > 0 ? 2147483647.0 : 2147483648.0 ) );
        } else {
            return (int) (long) ( v * 4294967295.0 );
        }
    }

    public static final float byte_to_float(final byte v, final boolean sSigned) {
        if( sSigned ) {
            return (v & 0xff) / ( v > 0 ? 127.0f : -128.0f ) ;
        } else {
            return (v & 0xff) / 255.0f ;
        }
    }
    public static final double byte_to_double(final byte v, final boolean sSigned) {
        if( sSigned ) {
            return (v & 0xff) / ( v > 0 ? 127.0 : -128.0 ) ;
        } else {
            return (v & 0xff) / 255.0 ;
        }
    }
    public static final float short_to_float(final short v, final boolean sSigned) {
        if( sSigned ) {
            return (v & 0xffff) / ( v > 0 ? 32767.0f : -32768.0f ) ;
        } else {
            return (v & 0xffff) / 65535.0f ;
        }
    }
    public static final double short_to_double(final short v, final boolean sSigned) {
        // lossy
        if( sSigned ) {
            return (v & 0xffff) / ( v > 0 ? 32767.0 : -32768.0 ) ;
        } else {
            return (v & 0xffff) / 65535.0 ;
        }
    }
    public static final float int_to_float(final int v, final boolean sSigned) {
        // lossy
        // float significand          0x007fffff
        // double significand 0x000fffffffffffffL
        //                  int min = 0x80000000 = -2147483648
        //                  int max = 0x7fffffff = +2147483647
        if( sSigned ) {
            return (float) ( v / ( v > 0 ? 2147483647.0 : 2147483648.0 ) );
        } else {
            return (float) ( (v & 0xffffffffL) / 4294967295.0 );
        }
    }
    public static final double int_to_double(final int v, final boolean sSigned) {
        if( sSigned ) {
            return v / ( v > 0 ? 2147483647.0 : 2147483648.0 ) ;
        } else {
            return (v & 0xffffffffL) / 4294967295.0 ;
        }
    }

    public static final short byte_to_short(final byte v, final boolean sSigned, final boolean dSigned) {
        return float_to_short(byte_to_float(v, sSigned), dSigned);
    }
    public static final int byte_to_int(final byte v, final boolean sSigned, final boolean dSigned) {
        return float_to_int(byte_to_float(v, sSigned), dSigned);
    }

    public static final byte short_to_byte(final short v, final boolean sSigned, final boolean dSigned) {
        return float_to_byte(short_to_float(v, sSigned), dSigned);
    }
    public static final int short_to_int(final short v, final boolean sSigned, final boolean dSigned) {
        return float_to_int(short_to_float(v, sSigned), dSigned);
    }

    public static final byte int_to_byte(final int v, final boolean sSigned, final boolean dSigned) {
        return float_to_byte(int_to_float(v, sSigned), dSigned);
    }
    public static final short int_to_short(final int v, final boolean sSigned, final boolean dSigned) {
        return float_to_short(int_to_float(v, sSigned), dSigned);
    }
}
