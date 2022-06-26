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
package org.jau.nio;

/**
 * This class represents a file based byte input stream.
 */
public final class ByteInStream_File implements ByteInStream, AutoCloseable  {
    private volatile long nativeInstance;
    /* pp */ long getNativeInstance() { return nativeInstance; }

    /**
     * Construct a Stream-Based byte input stream from filesystem path treating file as binary.
     *
     * In case the given path is a local file URI starting with `file://`, see {@link org.jau.nio.Uri#is_local_file_protocol(String)},
     * the leading `file://` is cut off and the remainder being used.
     *
     * @param path the path to the file, maybe a local file URI
     */
    public ByteInStream_File(final String path) {
        this(path, true);
    }

    /**
     * Construct a Stream-Based byte input stream from filesystem path
     *
     * In case the given path is a local file URI starting with `file://`, see {@link org.jau.nio.Uri#is_local_file_protocol(String)},
     * the leading `file://` is cut off and the remainder being used.
     *
     * @param path the path to the file, maybe a local file URI
     * @param use_binary whether to treat the file as binary (default) or use platform character conversion
     */
    public ByteInStream_File(final String path, final boolean use_binary) {
        try {
            nativeInstance = ctorImpl(path, use_binary);
        } catch (final Throwable t) {
            System.err.println("ByteInStream_File.ctor: native ctor failed: "+t.getMessage());
            throw t;
        }
    }
    private native long ctorImpl(final String path, final boolean use_binary);

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
    public native long read(final byte[] out, final int offset, final int length);

    @Override
    public native long peek(byte[] out, final int offset, final int length, final long peek_offset);

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

    @Override
    public native String toString();
}
