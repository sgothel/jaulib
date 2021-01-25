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

import org.jau.util.Bitfield;

/**
 * Simple synchronized {@link Bitfield} by wrapping an existing {@link Bitfield}.
 */
public class SyncedBitfield implements Bitfield {
    private final Bitfield impl;

    public SyncedBitfield(final Bitfield impl) {
        this.impl = impl;
    }

    @Override
    public final synchronized int size() {
        return impl.size();
    }

    @Override
    public final synchronized void clearField(final boolean bit) {
        impl.clearField(bit);
    }

    @Override
    public final synchronized int get32(final int lowBitnum, final int length) throws IndexOutOfBoundsException {
        return impl.get32(lowBitnum, length);
    }

    @Override
    public final synchronized void put32(final int lowBitnum, final int length, final int data) throws IndexOutOfBoundsException {
        impl.put32(lowBitnum, length, data);
    }

    @Override
    public final synchronized int copy32(final int srcLowBitnum, final int dstLowBitnum, final int length) throws IndexOutOfBoundsException {
        return impl.copy32(srcLowBitnum, dstLowBitnum, length);
    }

    @Override
    public final synchronized boolean get(final int bitnum) throws IndexOutOfBoundsException {
        return impl.get(bitnum);
    }

    @Override
    public final synchronized boolean put(final int bitnum, final boolean bit) throws IndexOutOfBoundsException {
        return impl.put(bitnum, bit);
    }

    @Override
    public final synchronized void set(final int bitnum) throws IndexOutOfBoundsException {
        impl.set(bitnum);
    }

    @Override
    public final synchronized void clear(final int bitnum) throws IndexOutOfBoundsException {
        impl.clear(bitnum);
    }

    @Override
    public final synchronized boolean copy(final int srcBitnum, final int dstBitnum) throws IndexOutOfBoundsException {
        return impl.copy(srcBitnum, dstBitnum);
    }

    @Override
    public final synchronized int bitCount() {
        return impl.bitCount();
    }
}