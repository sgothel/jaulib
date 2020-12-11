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
package org.jau.util;

import jau.util.SyncedBitfield;

/**
 * Simple bitfield interface for efficient bit storage access in O(1).
 * @since 2.3.2
 */
public interface Bitfield {
    /**
     * Simple {@link Bitfield} factory for returning the efficient implementation.
     */
    public static class Factory {
        /**
         * Creates am efficient {@link Bitfield} instance based on the requested {@code storageBitSize}.
         * <p>
         * Implementation returns a plain 32 bit integer field implementation for
         * {@code storageBitSize} &le; 32 bits or an 32 bit integer array implementation otherwise.
         * </p>
         */
        public static Bitfield create(final int storageBitSize) {
            if( 32 >= storageBitSize ) {
                return new jau.util.Int32Bitfield();
            } else {
                return new jau.util.Int32ArrayBitfield(storageBitSize);
            }
        }
        /**
         * Creates a synchronized {@link Bitfield} by wrapping the given {@link Bitfield} instance.
         */
        public static Bitfield synchronize(final Bitfield impl) {
            return new SyncedBitfield(impl);
        }
    }

    /**
     * Returns the storage size in bit units, e.g. 32 bit for implementations using one {@code int} field.
     */
    int size();


    /**
     * Set all bits of this bitfield to the given value {@code bit}.
     */
    void clearField(final boolean bit);

    /**
     * Returns {@code length} bits from this storage,
     * starting with the lowest bit from the storage position {@code lowBitnum}.
     * @param lowBitnum storage bit position of the lowest bit, restricted to [0..{@link #size()}-{@code length}].
     * @param length number of bits to read, constrained to [0..32].
     * @throws IndexOutOfBoundsException if {@code rightBitnum} is out of bounds
     * @see #put32(int, int, int)
     */
    int get32(final int lowBitnum, final int length) throws IndexOutOfBoundsException;

    /**
     * Puts {@code length} bits of given {@code data} into this storage,
     * starting w/ the lowest bit to the storage position {@code lowBitnum}.
     * @param lowBitnum storage bit position of the lowest bit, restricted to [0..{@link #size()}-{@code length}].
     * @param length number of bits to write, constrained to [0..32].
     * @param data the actual bits to be put into this storage
     * @throws IndexOutOfBoundsException if {@code rightBitnum} is out of bounds
     * @see #get32(int, int)
     */
    void put32(final int lowBitnum, final int length, final int data) throws IndexOutOfBoundsException;

    /**
     * Copies {@code length} bits at position {@code srcLowBitnum} to position {@code dstLowBitnum}
     * and returning the bits.
     * <p>
     * Implementation shall operate as if invoking {@link #get32(int, int)}
     * and then {@link #put32(int, int, int)} sequentially.
     * </p>
     * @param srcLowBitnum source bit number, restricted to [0..{@link #size()}-1].
     * @param dstLowBitnum destination bit number, restricted to [0..{@link #size()}-1].
     * @throws IndexOutOfBoundsException if {@code bitnum} is out of bounds
     * @see #get32(int, int)
     * @see #put32(int, int, int)
     */
    int copy32(final int srcLowBitnum, final int dstLowBitnum, final int length) throws IndexOutOfBoundsException;

    /**
     * Return <code>true</code> if the bit at position <code>bitnum</code> is set, otherwise <code>false</code>.
     * @param bitnum bit number, restricted to [0..{@link #size()}-1].
     * @throws IndexOutOfBoundsException if {@code bitnum} is out of bounds
     */
    boolean get(final int bitnum) throws IndexOutOfBoundsException;

    /**
     * Set or clear the bit at position <code>bitnum</code> according to <code>bit</code>
     * and return the previous value.
     * @param bitnum bit number, restricted to [0..{@link #size()}-1].
     * @throws IndexOutOfBoundsException if {@code bitnum} is out of bounds
     */
    boolean put(final int bitnum, final boolean bit) throws IndexOutOfBoundsException;

    /**
     * Set the bit at position <code>bitnum</code> according to <code>bit</code>.
     * @param bitnum bit number, restricted to [0..{@link #size()}-1].
     * @throws IndexOutOfBoundsException if {@code bitnum} is out of bounds
     */
    void set(final int bitnum) throws IndexOutOfBoundsException;

    /**
     * Clear the bit at position <code>bitnum</code> according to <code>bit</code>.
     * @param bitnum bit number, restricted to [0..{@link #size()}-1].
     * @throws IndexOutOfBoundsException if {@code bitnum} is out of bounds
     */
    void clear(final int bitnum) throws IndexOutOfBoundsException;

    /**
     * Copies the bit at position {@code srcBitnum} to position {@code dstBitnum}
     * and returning <code>true</code> if the bit is set, otherwise <code>false</code>.
     * @param srcBitnum source bit number, restricted to [0..{@link #size()}-1].
     * @param dstBitnum destination bit number, restricted to [0..{@link #size()}-1].
     * @throws IndexOutOfBoundsException if {@code bitnum} is out of bounds
     */
    boolean copy(final int srcBitnum, final int dstBitnum) throws IndexOutOfBoundsException;

    /**
     * Returns the number of one bits within this bitfield.
     * <p>
     * Utilizes {#link {@link Bitfield.Util#bitCount(int)}}.
     * </p>
     */
    int bitCount();
}
