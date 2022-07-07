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
package org.jau.io;

import java.nio.ByteBuffer;

import org.jau.io.Buffers;

/**
 * This class represents a Ringbuffer-Based byte input stream with an externally provisioned data feed.
 *
 * Instance uses the native C++ object `jau::io::ByteInStream_Feed`.
 */
public final class ByteInStream_Feed implements ByteInStream  {
    private volatile long nativeInstance;
    /* pp */ long getNativeInstance() { return nativeInstance; }

    /**
     * Construct a ringbuffer backed externally provisioned byte input stream
     * @param id_name arbitrary identifier for this instance
     * @param timeoutMS maximum duration in milliseconds to wait @ check_available() and write(), zero waits infinitely
     */
    public ByteInStream_Feed(final String id_name, final long timeoutMS) {
        try {
            nativeInstance = ctorImpl(id_name, timeoutMS);
        } catch (final Throwable t) {
            System.err.println("ByteInStream_Feed.ctor: native ctor failed: "+t.getMessage());
            throw t;
        }
    }
    private native long ctorImpl(final String id_name, final long timeoutMS);

    @Override
    public native void closeStream();

    @Override
    public void close() {
        final long handle;
        synchronized( this ) {
            handle = nativeInstance;
            nativeInstance = 0;
        }
        if( 0 != handle ) {
            dtorImpl(handle);
        }
    }
    private static native void dtorImpl(final long nativeInstance);

    @Override
    public void finalize() {
        close();
    }

    @Override
    public native boolean check_available(final long n);

    @Override
    public native int read(final byte[] out, final int offset, final int length);

    @Override
    public int read(final ByteBuffer out) {
        if( !Buffers.isDirect(out) ) {
            throw new IllegalArgumentException("out buffer not direct");
        }
        final int res = read2Impl(out, (int)Buffers.getDirectBufferByteOffset(out));
        out.limit(out.position() + res);
        return res;
    }
    private native int read2Impl(Object out, int out_offset);

    @Override
    public native int peek(byte[] out, final int offset, final int length, final long peek_offset);

    @Override
    public native boolean end_of_data();

    @Override
    public native boolean error();

    @Override
    public native String id();

    @Override
    public native long discard_next(long N);

    @Override
    public native long bytes_read();

    @Override
    public native boolean has_content_size();

    @Override
    public native long content_size();

    /**
     * Interrupt a potentially blocked reader.
     *
     * Call this method if intended to abort streaming and to interrupt the reader thread's potentially blocked check_available() call,
     * i.e. done at set_eof()
     *
     * @see set_eof()
     */
    public native void interruptReader();

    /**
     * Write given bytes to the async ringbuffer.
     *
     * Wait up to timeout duration given in constructor until ringbuffer space is available, where fractions_i64::zero waits infinitely.
     *
     * This method is blocking.
     *
     * @param in the byte array to transfer to the async ringbuffer
     * @param offset offset to in byte array to write
     * @param length number of in bytes to write starting at offset
     */
    public native void write(final byte[] in, final int offset, final int length);

    /**
     * Write given bytes to the async ringbuffer.
     *
     * Wait up to timeout duration given in constructor until ringbuffer space is available, where fractions_i64::zero waits infinitely.
     *
     * This method is blocking.
     *
     * @param in the direct {@link ByteBuffer} to transfer to the async ringbuffer starting at its {@link ByteBuffer#position() position} up to its {@link ByteBuffer#limit() limit}.
     *            {@link ByteBuffer#limit() Limit} will be reset to {@link ByteBuffer#position() position}.
     */
    public void write(final ByteBuffer in) {
        if( !Buffers.isDirect(in) ) {
            throw new IllegalArgumentException("out buffer not direct");
        }
        write2Impl(in, (int)Buffers.getDirectBufferByteOffset(in), in.limit());
        in.limit(in.position());
    }
    private native void write2Impl(ByteBuffer out, int out_offset, int out_limit);

    /**
     * Set known content size, informal only.
     * @param content_length the content size in bytes
     */
    public native void set_content_size(final long size);

    /**
     * Set end-of-data (EOS), i.e. when feeder completed provisioning bytes.
     *
     * Implementation issues interruptReader() to unblock a potentially blocked reader thread.
     *
     * @param result should be either -1 for FAILED or 1 for SUCCESS.
     *
     * @see interruptReader()
     */
    public native void set_eof(final int result);

    @Override
    public native String toString();
}
