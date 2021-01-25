/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2015 Gothel Software e.K.
 * Copyright (c) 2015 JogAmp Community.
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
package jau.util;

import org.jau.util.BitMath;
import org.jau.util.Bitfield;

/**
 * Simple bitfield interface for efficient storage access in O(1).
 * <p>
 * Implementation uses one 32bit integer field for storage.
 * </p>
 */
public class Int32Bitfield implements Bitfield {
    /** Unit size in bits, here 32 bits for one int unit. */
    private static final int UNIT_SIZE = 32;
    private int storage;

    public Int32Bitfield() {
        this.storage = 0;
    }

    @Override
    public int size() {
        return UNIT_SIZE;
    }

    @Override
    public final void clearField(final boolean bit) {
        if( bit ) {
            storage = BitMath.UNSIGNED_INT_MAX_VALUE;
        } else {
            storage = 0;
        }
    }

    private static final void check(final int size, final int bitnum) throws IndexOutOfBoundsException {
        if( 0 > bitnum || bitnum >= size ) {
            throw new IndexOutOfBoundsException("Bitnum should be within [0.."+(size-1)+"], but is "+bitnum);
        }
    }

    @Override
    public final int get32(final int lowBitnum, final int length) throws IndexOutOfBoundsException {
        if( 0 > length || length > 32 ) {
            throw new IndexOutOfBoundsException("length should be within [0..32], but is "+length);
        }
        check(UNIT_SIZE-length+1, lowBitnum);
        final int left = 32 - lowBitnum;             // remaining bits of first chunk
        if( 32 == left ) {
            // fast path
            final int m = BitMath.getBitMask(length);// mask of chunk
            return m & storage;
        } else {
            // slow path
            final int l = Math.min(length, left);    // length of first chunk < 32
            final int m = ( 1 << l ) - 1;            // mask of first chunk
            return m & ( storage >>> lowBitnum );
        }
    }
    @Override
    public final void put32(final int lowBitnum, final int length, final int data) throws IndexOutOfBoundsException {
        if( 0 > length || length > 32 ) {
            throw new IndexOutOfBoundsException("length should be within [0..32], but is "+length);
        }
        check(UNIT_SIZE-length+1, lowBitnum);
        final int left = 32 - lowBitnum;             // remaining bits of first chunk storage
        if( 32 == left ) {
            // fast path
            final int m = BitMath.getBitMask(length);// mask of chunk
            storage = ( ( ~m ) & storage )           // keep non-written storage bits
                      | ( m & data );                // overwrite storage w/ used data bits
        } else {
            // slow path
            final int l = Math.min(length, left);    // length of first chunk < 32
            final int m = ( 1 << l ) - 1;            // mask of first chunk
            storage = ( ( ~( m << lowBitnum ) ) & storage ) // keep non-written storage bits
                      | ( ( m & data ) << lowBitnum );      // overwrite storage w/ used data bits
        }
    }
    @Override
    public final int copy32(final int srcBitnum, final int dstBitnum, final int length) throws IndexOutOfBoundsException {
        final int data = get32(srcBitnum, length);
        put32(dstBitnum, length, data);
        return data;
    }

    @Override
    public final boolean get(final int bitnum) throws IndexOutOfBoundsException {
        check(UNIT_SIZE, bitnum);
        return 0 != ( storage & ( 1 << bitnum ) ) ;
    }
    @Override
    public final boolean put(final int bitnum, final boolean bit) throws IndexOutOfBoundsException {
        check(UNIT_SIZE, bitnum);
        final int m = 1 << bitnum;
        final boolean prev = 0 != ( storage & m ) ;
        if( prev != bit ) {
            if( bit ) {
                storage |=  m;
            } else {
                storage &= ~m;
            }
        }
        return prev;
    }
    @Override
    public final void set(final int bitnum) throws IndexOutOfBoundsException {
        check(UNIT_SIZE, bitnum);
        final int m = 1 << bitnum;
        storage |=  m;
    }
    @Override
    public final void clear (final int bitnum) throws IndexOutOfBoundsException {
        check(UNIT_SIZE, bitnum);
        final int m = 1 << bitnum;
        storage &= ~m;
    }
    @Override
    public final boolean copy(final int srcBitnum, final int dstBitnum) throws IndexOutOfBoundsException {
        check(UNIT_SIZE, srcBitnum);
        check(UNIT_SIZE, dstBitnum);
        // get
        final boolean bit = 0 != ( storage & ( 1 << srcBitnum ) ) ;
        // put
        final int m = 1 << dstBitnum;
        if( bit ) {
            storage |=  m;
        } else {
            storage &= ~m;
        }
        return bit;
    }

    @Override
    public int bitCount() {
        return BitMath.bitCount(storage);
    }
}
