/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
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

package jau.test.util;

import static org.jau.util.ValueConv.*;

import org.junit.Assert;
/**
 * Testing ValueConv's value conversion of primitive types
 */
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestValueConversion {

    @Test
    public void testBaseFloat() {
        Assert.assertEquals(Byte.MAX_VALUE, float_to_byte( 1.0f, true));
        Assert.assertEquals(Byte.MIN_VALUE, float_to_byte(-1.0f, true));
        Assert.assertEquals( 1.0f, byte_to_float( Byte.MAX_VALUE, true), 0.0);
        Assert.assertEquals(-1.0f, byte_to_float( Byte.MIN_VALUE, true), 0.0);

        Assert.assertEquals(Short.MAX_VALUE, float_to_short( 1.0f, true));
        Assert.assertEquals(Short.MIN_VALUE, float_to_short(-1.0f, true));
        Assert.assertEquals( 1.0f, short_to_float( Short.MAX_VALUE, true), 0.0);
        Assert.assertEquals(-1.0f, short_to_float( Short.MIN_VALUE, true), 0.0);

        Assert.assertEquals(Integer.MAX_VALUE, float_to_int( 1.0f, true));
        Assert.assertEquals(Integer.MIN_VALUE, float_to_int(-1.0f, true));
        Assert.assertEquals( 1.0f, int_to_float( Integer.MAX_VALUE, true), 0.0);
        Assert.assertEquals(-1.0f, int_to_float( Integer.MIN_VALUE, true), 0.0);

        Assert.assertEquals((byte)0xff, float_to_byte( 1.0f, false));
        Assert.assertEquals( 1.0f, byte_to_float( (byte)0xff, false), 0.0);

        Assert.assertEquals((short)0xffff, float_to_short( 1.0f, false));
        Assert.assertEquals( 1.0f, short_to_float( (short)0xffff, false), 0.0);

        Assert.assertEquals(0xffffffff, float_to_int( 1.0f, false));
        Assert.assertEquals( 1.0f, int_to_float( 0xffffffff, false), 0.0);
    }

    @Test
    public void testBaseDouble() {
        Assert.assertEquals(Byte.MAX_VALUE, double_to_byte( 1.0, true));
        Assert.assertEquals(Byte.MIN_VALUE, double_to_byte(-1.0, true));
        Assert.assertEquals( 1.0, byte_to_double( Byte.MAX_VALUE, true), 0.0);
        Assert.assertEquals(-1.0, byte_to_double( Byte.MIN_VALUE, true), 0.0);

        Assert.assertEquals(Short.MAX_VALUE, double_to_short( 1.0, true));
        Assert.assertEquals(Short.MIN_VALUE, double_to_short(-1.0, true));
        Assert.assertEquals( 1.0, short_to_double( Short.MAX_VALUE, true), 0.0);
        Assert.assertEquals(-1.0, short_to_double( Short.MIN_VALUE, true), 0.0);

        Assert.assertEquals(Integer.MAX_VALUE, double_to_int( 1.0, true));
        Assert.assertEquals(Integer.MIN_VALUE, double_to_int(-1.0, true));
        Assert.assertEquals( 1.0, int_to_double( Integer.MAX_VALUE, true), 0.0);
        Assert.assertEquals(-1.0, int_to_double( Integer.MIN_VALUE, true), 0.0);

        Assert.assertEquals((byte)0xff, double_to_byte( 1.0, false));
        Assert.assertEquals( 1.0, byte_to_double( (byte)0xff, false), 0.0);

        Assert.assertEquals((short)0xffff, double_to_short( 1.0, false));
        Assert.assertEquals( 1.0, short_to_double( (short)0xffff, false), 0.0);

        Assert.assertEquals(0xffffffff, double_to_int( 1.0, false));
        Assert.assertEquals( 1.0, int_to_double( 0xffffffff, false), 0.0);
    }

    @Test
    public void testConversion() {
        final byte sb0 = 127;
        final byte sb1 = -128;

        final float sf0 = byte_to_float(sb0, true);
        final float sf1 = byte_to_float(sb1, true);
        final short ss0 = byte_to_short(sb0, true, true);
        final short ss1 = byte_to_short(sb1, true, true);
        final int si0 = byte_to_int(sb0, true, true);
        final int si1 = byte_to_int(sb1, true, true);

        Assert.assertEquals(1.0f, sf0, 0.0);
        Assert.assertEquals(-1.0f, sf1, 0.0);
        Assert.assertEquals(Short.MAX_VALUE, ss0);
        Assert.assertEquals(Short.MIN_VALUE, ss1);
        Assert.assertEquals(Integer.MAX_VALUE, si0);
        Assert.assertEquals(Integer.MIN_VALUE, si1);

        Assert.assertEquals(sb0, short_to_byte(ss0, true, true));
        Assert.assertEquals(sb1, short_to_byte(ss1, true, true));
        Assert.assertEquals(sb0, int_to_byte(si0, true, true));
        Assert.assertEquals(sb1, int_to_byte(si1, true, true));

        final byte ub0 = (byte) 0xff;
        final float uf0 = byte_to_float(ub0, false);
        final short us0 = byte_to_short(ub0, false, false);
        final int ui0 = byte_to_int(ub0, false, false);

        Assert.assertEquals(1.0f, uf0, 0.0);
        Assert.assertEquals((short)0xffff, us0);
        Assert.assertEquals(0xffffffff, ui0);

        Assert.assertEquals(ub0, short_to_byte(us0, false, false));
        Assert.assertEquals(us0, int_to_short(ui0, false, false));
    }

    public static void main(final String args[]) {
        org.junit.runner.JUnitCore.main(TestValueConversion.class.getName());
    }

}
