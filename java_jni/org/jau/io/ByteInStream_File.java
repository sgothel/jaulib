/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022-2023 Gothel Software e.K.
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

/**
 * File based byte input stream, including named file descriptor.
 *
 * Implementation mimics std::ifstream via OS level file descriptor (FD) operations,
 * giving more flexibility, allowing reusing existing FD and enabling openat() operations.
 *
 * If source path denotes a named file descriptor, i.e. jau::fs::file_stats::is_fd() returns true,
 * has_content_size() returns false and available() returns true as long the stream is open and EOS hasn't occurred.
 *
 * Instance uses the native C++ object `jau::io::ByteInStream_File`.
 */
public final class ByteInStream_File implements ByteInStream {
    private volatile long nativeInstance;
    /* pp */ long getNativeInstance() { return nativeInstance; }

    /**
     * Construct a Stream-Based byte input stream from filesystem path
     *
     * In case the given path is a local file URI starting with `file://`, see {@link org.jau.io.UriTk#is_local_file_protocol(String)},
     * the leading `file://` is cut off and the remainder being used.
     *
     * @param path the path to the file, maybe a local file URI
     */
    public ByteInStream_File(final String path) {
        try {
            nativeInstance = ctorImpl1(path);
        } catch (final Throwable t) {
            System.err.println("ByteInStream_File.ctor: native ctor failed: "+t.getMessage());
            throw t;
        }
    }
    private static native long ctorImpl1(final String path);

    /**
     * Construct a stream based byte input stream from filesystem path and parent directory file descriptor
     *
     * In case the given path is a local file URI starting with `file://`, see jau::io::uri::is_local_file_protocol(),
     * the leading `file://` is cut off and the remainder being used.
     *
     * @param dirfd parent directory file descriptor
     * @param path the path to the file, maybe a local file URI
     */
    public ByteInStream_File(final int dirfd, final String path) {
        try {
            nativeInstance = ctorImpl2(dirfd, path);
        } catch (final Throwable t) {
            System.err.println("ByteInStream_File.ctor: native ctor failed: "+t.getMessage());
            throw t;
        }
    }
    private static native long ctorImpl2(final int dirfd, final String path);

    /**
     * Construct a stream based byte input stream by duplicating given file descriptor
     *
     * In case the given path is a local file URI starting with `file://`, see jau::io::uri::is_local_file_protocol(),
     * the leading `file://` is cut off and the remainder being used.
     *
     * @param fd file descriptor to duplicate leaving the given `fd` untouched
     */
    public ByteInStream_File(final int fd) {
        try {
            nativeInstance = ctorImpl3(fd);
        } catch (final Throwable t) {
            System.err.println("ByteInStream_File.ctor: native ctor failed: "+t.getMessage());
            throw t;
        }
    }
    private static native long ctorImpl3(final int fd);

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
    public native boolean is_open();

    @Override
    public void clear(final IOState state) {
        clearImpl( state.mask );
    }
    private native void clearImpl(int s);

    /**
     * Returns the file descriptor if is_open(), otherwise -1 for no file descriptor.
     *
     * @see is_open()
     */
    public native int fd();

    @Override
    public IOState rdState() {
        return new IOState( rdStateImpl() );
    }
    private native int rdStateImpl();

    @Override
    public void setState(final IOState state) {
        setStateImpl( state.mask );
    }
    private native void setStateImpl(int s);

    @Override
    public native boolean good();

    @Override
    public native boolean eof();

    @Override
    public native boolean fail();

    @Override
    public native boolean bad();

    @Override
    public native boolean timeout();

    @Override
    public boolean end_of_data() { return !good(); }

    @Override
    public native boolean available(final long n);

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
    public native String id();

    @Override
    public native long discard_next(long N);

    @Override
    public native long tellg();

    @Override
    public native boolean has_content_size();

    @Override
    public native long content_size();

    @Override
    public native String toString();
}
