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

package jau.test.util;

import static jau.test.util.BitDemoData.dumpData;
import static jau.test.util.BitDemoData.toBinaryString;
import static jau.test.util.BitDemoData.toHexBinaryString;
import static jau.test.util.BitDemoData.toHexString;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import org.jau.io.Bitstream;
import org.jau.io.Buffers;
import org.junit.Assert;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

import jau.test.junit.util.JunitTracer;

/**
 * Test {@link Bitstream} w/ int32 read/write access w/ semantics
 * as well as with aligned and unaligned access.
 * <ul>
 *  <li>{@link Bitstream#readUInt32(boolean)}</li>
 *  <li>{@link Bitstream#writeInt32(boolean, int)}</li>
 * </ul>
 */
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestBitstream04 extends JunitTracer {

    @Test
    public void test01Int32BitsAligned() throws IOException {
        test01Int32BitsImpl(null);
        test01Int32BitsImpl(ByteOrder.BIG_ENDIAN);
        test01Int32BitsImpl(ByteOrder.LITTLE_ENDIAN);
    }
    void test01Int32BitsImpl(final ByteOrder byteOrder) throws IOException {
        test01Int32BitsAlignedImpl(byteOrder, 0, 0);
        test01Int32BitsAlignedImpl(byteOrder, 1, 1);
        test01Int32BitsAlignedImpl(byteOrder, -1, -1);
        test01Int32BitsAlignedImpl(byteOrder, 7, 7);
        test01Int32BitsAlignedImpl(byteOrder, 0x0fffffff, 0x0fffffff);
        test01Int32BitsAlignedImpl(byteOrder, Integer.MIN_VALUE, -1);
        test01Int32BitsAlignedImpl(byteOrder, Integer.MAX_VALUE, Integer.MAX_VALUE);
        test01Int32BitsAlignedImpl(byteOrder, 0xffffffff, -1);
    }
    void test01Int32BitsAlignedImpl(final ByteOrder byteOrder, final int val32, final int expUInt32Int) throws IOException {
        // Test with buffer defined value
        final ByteBuffer bb = ByteBuffer.allocate(Buffers.SIZEOF_INT);
        if( null != byteOrder ) {
            bb.order(byteOrder);
        }
        final boolean bigEndian = ByteOrder.BIG_ENDIAN == bb.order();
        final String val32_hs = toHexString(val32);
        System.err.println("XXX Test01Int32BitsAligned: byteOrder "+byteOrder+" (bigEndian "+bigEndian+"), value "+val32+", "+toHexBinaryString(val32, 32));
        System.err.println("XXX Test01Int32BitsAligned: "+val32+", "+val32_hs);

        bb.putInt(0, val32);
        dumpData("TestData.1: ", bb, 0, 4);

        final Bitstream.ByteBufferStream bbs = new Bitstream.ByteBufferStream(bb);
        final Bitstream<ByteBuffer> bs = new Bitstream<ByteBuffer>(bbs, false /* outputMode */);
        {
            final long uint32_l = bs.readUInt32(bigEndian);
            final int int32_l = (int)uint32_l;
            final String uint32_l_hs = toHexString(uint32_l);
            final int uint32_i = Bitstream.uint32LongToInt(uint32_l);
            System.err.printf("Read32.1 uint32_l %012d, %10s; int32_l %012d %10s; uint32_i %012d %10s%n",
                    uint32_l, uint32_l_hs, int32_l, toHexString(int32_l), uint32_i, toHexString(uint32_i));
            Assert.assertEquals(val32_hs, uint32_l_hs);
            Assert.assertEquals(val32, int32_l);
            Assert.assertEquals(expUInt32Int, uint32_i);
        }

        // Test with written bitstream value
        bs.setStream(bs.getSubStream(), true /* outputMode */);
        bs.writeInt32(bigEndian, val32);
        bs.setStream(bs.getSubStream(), false /* outputMode */); // switch to input-mode, implies flush()
        dumpData("TestData.2: ", bb, 0, 4);
        {
            final long uint32_l = bs.readUInt32(bigEndian);
            final int int32_l = (int)uint32_l;
            final String uint32_l_hs = toHexString(uint32_l);
            final int uint32_i = Bitstream.uint32LongToInt(uint32_l);
            System.err.printf("Read32.2 uint32_l %012d, %10s; int32_l %012d %10s; uint32_i %012d %10s%n",
                    uint32_l, uint32_l_hs, int32_l, toHexString(int32_l), uint32_i, toHexString(uint32_i));
            Assert.assertEquals(val32_hs, uint32_l_hs);
            Assert.assertEquals(val32, int32_l);
            Assert.assertEquals(expUInt32Int, uint32_i);
        }
    }

    @Test
    public void test02Int32BitsUnaligned() throws IOException {
        test02Int32BitsUnalignedImpl(null);
        test02Int32BitsUnalignedImpl(ByteOrder.BIG_ENDIAN);
        test02Int32BitsUnalignedImpl(ByteOrder.LITTLE_ENDIAN);
    }
    void test02Int32BitsUnalignedImpl(final ByteOrder byteOrder) throws IOException {
        test02Int32BitsUnalignedImpl(byteOrder, 0);
        test02Int32BitsUnalignedImpl(byteOrder, 1);
        test02Int32BitsUnalignedImpl(byteOrder, 7);
        test02Int32BitsUnalignedImpl(byteOrder, 8);
        test02Int32BitsUnalignedImpl(byteOrder, 15);
        test02Int32BitsUnalignedImpl(byteOrder, 24);
        test02Int32BitsUnalignedImpl(byteOrder, 25);
    }
    void test02Int32BitsUnalignedImpl(final ByteOrder byteOrder, final int preBits) throws IOException {
        test02Int32BitsUnalignedImpl(byteOrder, preBits, 0, 0);
        test02Int32BitsUnalignedImpl(byteOrder, preBits, 1, 1);
        test02Int32BitsUnalignedImpl(byteOrder, preBits, -1, -1);
        test02Int32BitsUnalignedImpl(byteOrder, preBits, 7, 7);
        test02Int32BitsUnalignedImpl(byteOrder, preBits, 0x0fffffff, 0x0fffffff);
        test02Int32BitsUnalignedImpl(byteOrder, preBits, Integer.MIN_VALUE, -1);
        test02Int32BitsUnalignedImpl(byteOrder, preBits, Integer.MAX_VALUE, Integer.MAX_VALUE);
        test02Int32BitsUnalignedImpl(byteOrder, preBits, 0xffffffff, -1);
    }
    void test02Int32BitsUnalignedImpl(final ByteOrder byteOrder, final int preBits, final int val32, final int expUInt32Int) throws IOException {
        final int preBytes = ( preBits + 7 ) >>> 3;
        final int byteCount = preBytes + Buffers.SIZEOF_INT;
        final ByteBuffer bb = ByteBuffer.allocate(byteCount);
        if( null != byteOrder ) {
            bb.order(byteOrder);
        }
        final boolean bigEndian = ByteOrder.BIG_ENDIAN == bb.order();
        final String val32_hs = toHexString(val32);
        System.err.println("XXX Test02Int32BitsUnaligned: byteOrder "+byteOrder+" (bigEndian "+bigEndian+"), preBits "+preBits+", value "+val32+", "+toHexBinaryString(val32, 32));
        System.err.println("XXX Test02Int32BitsUnaligned: "+val32+", "+val32_hs);

        // Test with written bitstream value
        final Bitstream.ByteBufferStream bbs = new Bitstream.ByteBufferStream(bb);
        final Bitstream<ByteBuffer> bs = new Bitstream<ByteBuffer>(bbs, true /* outputMode */);
        bs.writeBits31(preBits, 0);
        bs.writeInt32(bigEndian, val32);
        bs.setStream(bs.getSubStream(), false /* outputMode */); // switch to input-mode, implies flush()

        final int rPre = bs.readBits31(preBits);
        final long uint32_l = bs.readUInt32(bigEndian);
        final int int32_l = (int)uint32_l;
        final String uint32_l_hs = toHexString(uint32_l);
        final int uint32_i = Bitstream.uint32LongToInt(uint32_l);
        System.err.println("ReadPre "+rPre+", "+toBinaryString(rPre, preBits));
        System.err.printf("Read32 uint32_l %012d, %10s; int32_l %012d %10s; uint32_i %012d %10s%n",
                uint32_l, uint32_l_hs, int32_l, toHexString(int32_l), uint32_i, toHexString(uint32_i));
        Assert.assertEquals(val32_hs, uint32_l_hs);
        Assert.assertEquals(val32, int32_l);
        Assert.assertEquals(expUInt32Int, uint32_i);
    }

    public static void main(final String args[]) throws IOException {
        final String tstname = TestBitstream04.class.getName();
        org.junit.runner.JUnitCore.main(tstname);
    }

}
