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

package jau.test.io;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import org.jau.io.Buffers;
import org.jau.io.MemUtil;
import org.junit.Assert;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

import jau.pkg.PlatformRuntime;
import jau.test.junit.util.JunitTracer;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestMemBuffers00 extends JunitTracer {
    static final boolean DEBUG = false;

    @Test(timeout = 10000)
    public final void test01_ByteBuffer() {
        PlatformRuntime.checkInitialized();
        final int len = 10;

        final ByteBuffer b1 = Buffers.newDirectByteBuffer(len * 2);
        Assert.assertEquals(len * 2, b1.capacity());
        Assert.assertEquals(len * 2, b1.limit());
        Assert.assertEquals(0, b1.position());
        Assert.assertEquals(len * 2, b1.remaining());
        Assert.assertEquals(ByteOrder.nativeOrder(), b1.order());

        for(int i=0; i<len; ++i) {
            b1.put((byte)0xaa);
        }
        Assert.assertEquals(len, b1.position());
        b1.flip();
        Assert.assertEquals(len, b1.limit());
        Assert.assertEquals(0, b1.position());
        Assert.assertEquals(len, b1.remaining());

        Assert.assertEquals((byte)0xaa, b1.get(0));
        Assert.assertEquals((byte)0xaa, b1.get(len-1));
        MemUtil.zeroByteBuffer(b1);
        Assert.assertEquals((byte)0x00, b1.get(0));
        Assert.assertEquals((byte)0x00, b1.get(len-1));
    }

/**
    @Test(timeout = 10000)
    public final void test02_String_ByteBuffer() {
        PlatformRuntime.checkInitialized();
        final ByteBuffer b0;
        {
            final String s1 = "0123456789"; // len = 10
            b0 = CPUtils.to_ByteBuffer(s1, false);
            final String s2 = new String(s1.getBytes()); // true copy before deletion
            final boolean r2 = CPUtils.zeroString(s2);
            // Assert.assertTrue( r1 );
            CPUtils.fprintf_td(System.err, "test01_enc_dec_file_ok.1a: s1: '%s', len %d\n", s1, s1.length());
            CPUtils.fprintf_td(System.err, "test01_enc_dec_file_ok.1a: s2: '%s', len %d, zeroString result %b\n", s2, s2.length(), r2);

            final boolean r1 = CPUtils.zeroString(s1);
            CPUtils.fprintf_td(System.err, "test01_enc_dec_file_ok.1b: s1: '%s', len %d, zeroString result %b\n", s1, s1.length(), r1);
        }
        {
            // Ooops, the internalized string at compile time got zeroed out
            final String s1 = "0123456789"; // len = 10
            CPUtils.fprintf_td(System.err, "test01_enc_dec_file_ok.1c: s1: '%s', len %d\n", s1, s1.length());
        }
        {
            // That works, using dynamic storage
            final String s1 = CPUtils.to_String(b0, StandardCharsets.UTF_8);
            CPUtils.fprintf_td(System.err, "test01_enc_dec_file_ok.1d: s1: '%s', len %d\n", s1, s1.length());
        }
        {
            final String s1 = CPUtils.to_String(b0, StandardCharsets.UTF_8);
            CPUtils.fprintf_td(System.err, "test01_enc_dec_file_ok.2a: s1: '%s', len %d\n", s1, s1.length());

            final ByteBuffer b1 = CPUtils.newDirectByteBuffer(s1.length() * 2);
            Assert.assertEquals(s1.length() * 2, b1.capacity());
            Assert.assertEquals(s1.length() * 2, b1.limit());
            Assert.assertEquals(0, b1.position());
            Assert.assertEquals(s1.length() * 2, b1.remaining());

            b1.put(s1.getBytes(StandardCharsets.UTF_8));
            Assert.assertEquals(s1.length(), b1.position());
            b1.flip();
            Assert.assertEquals(s1.length(), b1.limit());
            Assert.assertEquals(0, b1.position());
            Assert.assertEquals(s1.length(), b1.remaining());
            {
                final String b1_str = CPUtils.to_String(b1, StandardCharsets.UTF_8);
                CPUtils.fprintf_td(System.err, "test01_enc_dec_file_ok.2b: b1: '%s', len %d/%d\n", b1_str, b1_str.length(), b1.limit());
            }
            Assert.assertEquals(s1.length(), b1.limit());
            Assert.assertEquals(0, b1.position());
            Assert.assertEquals(s1.length(), b1.remaining());

            final String s2 = new String(s1.getBytes()); // true copy
            final ByteBuffer b2 = CPUtils.to_ByteBuffer(s2, true);
            Assert.assertEquals(s1.length(), b2.capacity());
            Assert.assertEquals(s1.length(), b2.limit());
            Assert.assertEquals(0, b2.position());
            Assert.assertEquals(s1.length(), b2.remaining());
            {
                final byte[] b2_bytes = new byte[b2.remaining()];
                b2.get(b2_bytes);
                b2.rewind(); // yada yada yada (relative get, absolute @ JDK13
                final String b2_str = new String(b2_bytes, StandardCharsets.UTF_8);
                CPUtils.fprintf_td(System.err, "test01_enc_dec_file_ok.2c: b2: '%s', len %d/%d\n", b2_str, b2_str.length(), b2.limit());
            }
        }
    }
*/
    public static void main(final String args[]) {
        org.junit.runner.JUnitCore.main(TestMemBuffers00.class.getName());
    }
}
