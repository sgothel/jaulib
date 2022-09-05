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

    public static void main(final String args[]) {
        org.junit.runner.JUnitCore.main(TestBasicTypes.class.getName());
    }

}
