/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2014 Gothel Software e.K.
 * Copyright (c) 2014 JogAmp Community.
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

import static org.jau.util.BitDemoData.toBinaryString;
import static org.jau.util.BitDemoData.toHexBinaryString;

import java.io.IOException;
import java.nio.ByteBuffer;

import org.jau.io.Bitstream;
import org.jau.junit.util.SingletonJunitCase;
import org.jau.lang.NioUtil;
import org.junit.Assert;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

/**
 * Test {@link Bitstream} w/ int8 read/write access w/ semantics
 * as well as with aligned and unaligned access.
 * <ul>
 *  <li>{@link Bitstream#readInt8(boolean, boolean)}</li>
 *  <li>{@link Bitstream#writeInt8(boolean, boolean, byte)}</li>
 * </ul>
 */
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestBitstream02 extends SingletonJunitCase {

    @Test
    public void test01Int8BitsAligned() throws IOException {
        test01Int8BitsAlignedImpl((byte)0);
        test01Int8BitsAlignedImpl((byte)1);
        test01Int8BitsAlignedImpl((byte)7);
        test01Int8BitsAlignedImpl(Byte.MIN_VALUE);
        test01Int8BitsAlignedImpl(Byte.MAX_VALUE);
        test01Int8BitsAlignedImpl((byte)0xff);
    }
    void test01Int8BitsAlignedImpl(final byte val8) throws IOException {
        // Test with buffer defined value
        final ByteBuffer bb = ByteBuffer.allocate(NioUtil.SIZEOF_BYTE);
        System.err.println("XXX Test01Int8BitsAligned: value "+val8+", "+toHexBinaryString(val8, 8));
        bb.put(0, val8);

        final Bitstream.ByteBufferStream bbs = new Bitstream.ByteBufferStream(bb);
        final Bitstream<ByteBuffer> bs = new Bitstream<ByteBuffer>(bbs, false /* outputMode */);
        {
            final byte r8 = (byte) bs.readUInt8();
            System.err.println("Read8.1 "+r8+", "+toHexBinaryString(r8, 8));
            Assert.assertEquals(val8, r8);
        }

        // Test with written bitstream value
        bs.setStream(bs.getSubStream(), true /* outputMode */);
        bs.writeInt8(val8);
        bs.setStream(bs.getSubStream(), false /* outputMode */); // switch to input-mode, implies flush()
        {
            final byte r8 = (byte) bs.readUInt8();
            System.err.println("Read8.2 "+r8+", "+toHexBinaryString(r8, 8));
            Assert.assertEquals(val8, r8);
        }
    }

    @Test
    public void test02Int8BitsUnaligned() throws IOException {
        test02Int8BitsUnalignedImpl(0);
        test02Int8BitsUnalignedImpl(1);
        test02Int8BitsUnalignedImpl(7);
        test02Int8BitsUnalignedImpl(8);
        test02Int8BitsUnalignedImpl(15);
        test02Int8BitsUnalignedImpl(24);
        test02Int8BitsUnalignedImpl(25);
    }
    void test02Int8BitsUnalignedImpl(final int preBits) throws IOException {
        test02Int8BitsUnalignedImpl(preBits, (byte)0);
        test02Int8BitsUnalignedImpl(preBits, (byte)1);
        test02Int8BitsUnalignedImpl(preBits, (byte)7);
        test02Int8BitsUnalignedImpl(preBits, Byte.MIN_VALUE);
        test02Int8BitsUnalignedImpl(preBits, Byte.MAX_VALUE);
        test02Int8BitsUnalignedImpl(preBits, (byte)0xff);
    }
    void test02Int8BitsUnalignedImpl(final int preBits, final byte val8) throws IOException {
        final int preBytes = ( preBits + 7 ) >>> 3;
        final int byteCount = preBytes + NioUtil.SIZEOF_BYTE;
        final ByteBuffer bb = ByteBuffer.allocate(byteCount);
        System.err.println("XXX Test02Int8BitsUnaligned: preBits "+preBits+", value "+val8+", "+toHexBinaryString(val8, 8));

        // Test with written bitstream value
        final Bitstream.ByteBufferStream bbs = new Bitstream.ByteBufferStream(bb);
        final Bitstream<ByteBuffer> bs = new Bitstream<ByteBuffer>(bbs, true /* outputMode */);
        bs.writeBits31(preBits, 0);
        bs.writeInt8(val8);
        bs.setStream(bs.getSubStream(), false /* outputMode */); // switch to input-mode, implies flush()

        final int rPre = (short) bs.readBits31(preBits);
        final byte r8 = (byte) bs.readUInt8();
        System.err.println("ReadPre "+rPre+", "+toBinaryString(rPre, preBits));
        System.err.println("Read8 "+r8+", "+toHexBinaryString(r8, 8));
        Assert.assertEquals(val8, r8);
    }

    public static void main(final String args[]) throws IOException {
        final String tstname = TestBitstream02.class.getName();
        org.junit.runner.JUnitCore.main(tstname);
    }

}
