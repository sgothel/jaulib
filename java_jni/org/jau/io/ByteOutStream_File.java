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

import org.jau.fs.FMode;

/**
 * File based byte output stream, including named file descriptor.
 *
 * Implementation mimics std::ifstream via OS level file descriptor (FD) operations,
 * giving more flexibility, allowing reusing existing FD and enabling openat() operations.
 *
 * Instance uses the native C++ object `jau::io::ByteOutStream_File`.
 */
public final class ByteOutStream_File implements ByteOutStream {
    private volatile long nativeInstance;
    /* pp */ long getNativeInstance() { return nativeInstance; }

    /**
     * Construct a stream based byte output stream from filesystem path,
     * either an existing or new file.
     *
     * In case the file already exists, the underlying file offset is positioned at the end of the file.
     *
     * In case the given path is a local file URI starting with `file://`, see jau::io::uri::is_local_file_protocol(),
     * the leading `file://` is cut off and the remainder being used.
     *
     * @param path the path to the file, maybe a local file URI
     * @param mode file protection mode for a new file, otherwise ignored, may use {@link FMode#def_file}.
     */
    public ByteOutStream_File(final String path, final FMode mode) {
        try {
            nativeInstance = ctorImpl1(path, mode.mask);
        } catch (final Throwable t) {
            System.err.println("ByteOutStream_File.ctor: native ctor failed: "+t.getMessage());
            throw t;
        }
    }
    private static native long ctorImpl1(final String path, int mode);

    /**
     * Construct a stream based byte output stream from filesystem path and parent directory file descriptor,
     * either an existing or new file.
     *
     * In case the file already exists, the underlying file offset is positioned at the end of the file.
     *
     * In case the given path is a local file URI starting with `file://`, see jau::io::uri::is_local_file_protocol(),
     * the leading `file://` is cut off and the remainder being used.
     *
     * @param dirfd parent directory file descriptor
     * @param path the path to the file, maybe a local file URI
     * @param mode file protection mode for a new file, otherwise ignored, may use {@link FMode#def_file}.
     */
    public ByteOutStream_File(final int dirfd, final String path, final FMode mode) {
        try {
            nativeInstance = ctorImpl2(dirfd, path, mode.mask);
        } catch (final Throwable t) {
            System.err.println("ByteOutStream_File.ctor: native ctor failed: "+t.getMessage());
            throw t;
        }
    }
    private static native long ctorImpl2(final int dirfd, final String path, int mode);

    /**
     * Construct a stream based byte output stream by duplicating given file descriptor
     *
     * In case the given path is a local file URI starting with `file://`, see jau::io::uri::is_local_file_protocol(),
     * the leading `file://` is cut off and the remainder being used.
     *
     * @param fd file descriptor to duplicate leaving the given `fd` untouched
     */
    public ByteOutStream_File(final int fd) {
        try {
            nativeInstance = ctorImpl3(fd);
        } catch (final Throwable t) {
            System.err.println("ByteOutStream_File.ctor: native ctor failed: "+t.getMessage());
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
    public native int write(final byte[] in, final int offset, final int length);

    @Override
    public int write(final ByteBuffer in) {
        if( !Buffers.isDirect(in) ) {
            throw new IllegalArgumentException("in buffer not direct");
        }
        final int res = write2Impl(in, (int)Buffers.getDirectBufferByteOffset(in), (int)Buffers.getDirectBufferByteLimit(in));
        in.position(in.position() + res);
        return res;
    }
    private native int write2Impl(Object in, int in_offset, int in_limit);

    @Override
    public native String id();

    @Override
    public native long tellp();

    @Override
    public native String toString();
}
