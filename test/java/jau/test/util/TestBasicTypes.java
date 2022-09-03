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

package jau.test.util;

import org.jau.util.BasicTypes;
import org.junit.Assert;
/**
 * Testing ValueConv's value conversion of primitive types
 */
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestBasicTypes {

    @Test
    public void testHexStrings() {
        final byte[] lalaSink1 = new byte[] { (byte)0x1a, (byte)0x1b, (byte)0x2a, (byte)0x2b, (byte)0xff };
        {
            final String value_s0 = "1a1b2a2bff";
            final String value_s1 = BasicTypes.bytesHexString(lalaSink1, 0, lalaSink1.length, true /* lsbFirst */);
            final byte[] lalaSink2 = BasicTypes.hexStringBytes(value_s1, true /* lsbFirst */, false);
            final String value_s2 = BasicTypes.bytesHexString(lalaSink2, 0, lalaSink2.length, true /* lsbFirst */);
            Assert.assertEquals(value_s0, value_s1);
            Assert.assertEquals(value_s0, value_s2);
            Assert.assertArrayEquals(lalaSink1, lalaSink2);
        }
        {
            final String value_s0 = "0xff2b2a1b1a";
            final String value_s1 = BasicTypes.bytesHexString(lalaSink1, 0, lalaSink1.length, false /* lsbFirst */);
            final byte[] lalaSink2 = BasicTypes.hexStringBytes(value_s1, false /* lsbFirst */, true);
            final String value_s2 = BasicTypes.bytesHexString(lalaSink2, 0, lalaSink2.length, false /* lsbFirst */);
            Assert.assertEquals(value_s0, value_s1);
            Assert.assertEquals(value_s0, value_s2);
            Assert.assertArrayEquals(lalaSink1, lalaSink2);
        }

    }

    private static void testRadix32(final int base) {
        final String r1_max = BasicTypes.dec_to_radix(base-1, base, 3, '0');
        final String r1_max_s = BasicTypes.dec_to_radix(base-1, base);
        final String r3_max = r1_max_s.concat(r1_max_s).concat(r1_max_s);
        final int min = (int)BasicTypes.radix_to_dec("0", base);
        final int max = (int)BasicTypes.radix_to_dec(r3_max, base);

        System.err.printf("Test base %d: [%d .. %d] <-> ['%s' .. '%s'], %d years (max/365d) \n",
                base, min, max, BasicTypes.dec_to_radix(min, base), BasicTypes.dec_to_radix(max, base), (max/365));

        Assert.assertEquals(0, min);
        Assert.assertEquals(0, BasicTypes.radix_to_dec("000", base));
        Assert.assertEquals("0", BasicTypes.dec_to_radix(0, base));
        Assert.assertEquals("000", BasicTypes.dec_to_radix(0, base, 3, '0'));

        Assert.assertEquals(1, BasicTypes.radix_to_dec("001", base));
        Assert.assertEquals("1", BasicTypes.dec_to_radix(1, base));
        Assert.assertEquals("001", BasicTypes.dec_to_radix(1, base, 3, '0'));
        {
            final int v0_d = (int)BasicTypes.radix_to_dec(r1_max, base);
            final String v1_s = BasicTypes.dec_to_radix(base-1, base, 3, '0');
            Assert.assertEquals(r1_max, v1_s);
            Assert.assertEquals(base-1, v0_d);
        }
        {
            final int v0_d = (int)BasicTypes.radix_to_dec(r3_max, base);
            final String v1_s = BasicTypes.dec_to_radix(max, base, 3, '0');
            Assert.assertEquals(r3_max, v1_s);
            Assert.assertEquals(max, v0_d);
        }
        for(int iter=min; iter<=max; ++iter) {
            final String rad = BasicTypes.dec_to_radix(iter, base, 3, '0');
            final int dec = (int)BasicTypes.radix_to_dec(rad, base);
            Assert.assertEquals(iter, dec);
        }
    }

    private static void testRadix64(final int base, final long min, final long max) {
        final int padding = 9;
        final String r1_max = BasicTypes.dec_to_radix(base-1, base, padding, '0');

        System.err.printf("Test base %d: [%d .. %d] <-> ['%s' .. '%s'], %d years (max/365d) \n",
                base, min, max, BasicTypes.dec_to_radix(min, base), BasicTypes.dec_to_radix(max, base), (max/365));

        Assert.assertEquals(0, BasicTypes.radix_to_dec("000", base));
        Assert.assertEquals("0", BasicTypes.dec_to_radix(0, base));

        Assert.assertEquals(1, BasicTypes.radix_to_dec("001", base));
        Assert.assertEquals("1", BasicTypes.dec_to_radix(1, base));
        {
            final long v0_d = BasicTypes.radix_to_dec(r1_max, base);
            final String v1_s = BasicTypes.dec_to_radix(base-1, base, padding, '0');
            Assert.assertEquals(r1_max, v1_s);
            Assert.assertEquals(base-1, v0_d);
        }
        for(long iter=Math.max(0L, min-1); iter<max; ) {
            ++iter;
            final String rad = BasicTypes.dec_to_radix(iter, base, padding, '0');
            final long dec = BasicTypes.radix_to_dec(rad, base);
            if( false ) {
                System.err.printf("test base %d: iter %d, rad '%s', dec %d\n", base, iter, rad, dec);
            }
            Assert.assertEquals(iter, dec);
        }
    }

    @Test
    public void test01RadixBase62() {
        testRadix32(62);
        testRadix64(62, 0x7fffff00L, 0x80000100L);
        testRadix64(62, 0xFFFFFFF0L, 0x100000010L);
        testRadix64(62, 0x7FFFFFFFFFFFFFF0L, 0x7FFFFFFFFFFFFFFFL);

    }
    @Test
    public void test02RadixBase82() {
        testRadix32(82);
        testRadix64(82, 0x7fffff00L, 0x80000100L);
        testRadix64(82, 0xFFFFFFF0L, 0x100000010L);
        testRadix64(82, 0x7FFFFFFFFFFFFFF0L, 0x7FFFFFFFFFFFFFFFL);
    }
    @Test
    public void test03RadixBase143() {
        testRadix32(143);
        testRadix64(143, 0x7fffff00L, 0x80000100L);
        testRadix64(143, 0xFFFFFFF0L, 0x100000010L);
        testRadix64(143, 0x7FFFFFFFFFFFFFF0L, 0x7FFFFFFFFFFFFFFFL);
    }

    public static void main(final String args[]) {
        org.junit.runner.JUnitCore.main(TestBasicTypes.class.getName());
    }

}
