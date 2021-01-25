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
package org.jau.io;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.FileChannel.MapMode;

import org.jau.io.MappedByteBufferInputStream.CacheMode;
import org.jau.io.MappedByteBufferInputStream.FileResizeOp;

/**
 * An {@link OutputStream} implementation based on an underlying {@link FileChannel}'s memory mapped {@link ByteBuffer}.
 * <p>
 * Implementation is based on {@link MappedByteBufferInputStream}, using it as its parent instance.
 * </p>
 * <p>
 * An instance maybe created via its parent {@link MappedByteBufferInputStream#getOutputStream(FileResizeOp)}
 * or directly {@link #MappedByteBufferOutputStream(FileChannel, MapMode, CacheMode, int, FileResizeOp)}.
 * </p>
 * @since 0.3.0
 */
public class MappedByteBufferOutputStream extends OutputStream {
    private final MappedByteBufferInputStream parent;

    MappedByteBufferOutputStream(final MappedByteBufferInputStream parent,
                                 final FileResizeOp fileResizeOp) throws IOException {
        if( FileChannel.MapMode.READ_ONLY == parent.getMapMode() ) {
            throw new IOException("FileChannel map-mode is read-only");
        }
        this.parent = parent;
        this.parent.setFileResizeOp(fileResizeOp);
    }

    /**
     * Creates a new instance using the given {@link FileChannel}.
     * <p>
     * The {@link ByteBuffer} slices will be mapped lazily at first usage.
     * </p>
     * @param fileChannel the file channel to be mapped lazily.
     * @param mmode the map mode, default is {@link FileChannel.MapMode#READ_WRITE}.
     * @param cmode the caching mode, default is {@link MappedByteBufferInputStream.CacheMode#FLUSH_PRE_SOFT}.
     * @param sliceShift the pow2 slice size, default is {@link MappedByteBufferInputStream#DEFAULT_SLICE_SHIFT}.
     * @param fileResizeOp {@link MappedByteBufferInputStream.FileResizeOp} as described on {@link MappedByteBufferInputStream#setFileResizeOp(FileResizeOp)}.
     * @throws IOException
     */
    public MappedByteBufferOutputStream(final FileChannel fileChannel,
                                        final FileChannel.MapMode mmode,
                                        final CacheMode cmode,
                                        final int sliceShift, final FileResizeOp fileResizeOp) throws IOException {
        this(new MappedByteBufferInputStream(fileChannel, mmode, cmode, sliceShift, fileChannel.size(), 0), fileResizeOp);
    }

    /**
     * See {@link MappedByteBufferInputStream#setSynchronous(boolean)}.
     */
    public final synchronized void setSynchronous(final boolean s) {
        parent.setSynchronous(s);
    }
    /**
     * See {@link MappedByteBufferInputStream#getSynchronous()}.
     */
    public final synchronized boolean getSynchronous() {
        return parent.getSynchronous();
    }

    /**
     * See {@link MappedByteBufferInputStream#setLength(long)}.
     */
    public final synchronized void setLength(final long newTotalSize) throws IOException {
        parent.setLength(newTotalSize);
    }

    /**
     * See {@link MappedByteBufferInputStream#notifyLengthChange(long)}.
     */
    public final synchronized void notifyLengthChange(final long newTotalSize) throws IOException {
        parent.notifyLengthChange(newTotalSize);
    }

    /**
     * See {@link MappedByteBufferInputStream#length()}.
     */
    public final synchronized long length() {
        return parent.length();
    }

    /**
     * See {@link MappedByteBufferInputStream#remaining()}.
     */
    public final synchronized long remaining() throws IOException {
        return parent.remaining();
    }

    /**
     * See {@link MappedByteBufferInputStream#position()}.
     */
    public final synchronized long position() throws IOException {
        return parent.position();
    }

    /**
     * See {@link MappedByteBufferInputStream#position(long)}.
     */
    public final synchronized MappedByteBufferInputStream position( final long newPosition ) throws IOException {
        return parent.position(newPosition);
    }

    /**
     * See {@link MappedByteBufferInputStream#skip(long)}.
     */
    public final synchronized long skip( final long n ) throws IOException {
        return parent.skip(n);
    }

    @Override
    public final synchronized void flush() throws IOException {
        parent.flush( true /* metaData */);
    }

    /**
     * See {@link MappedByteBufferInputStream#flush(boolean)}.
     */
    // @Override
    public final synchronized void flush(final boolean metaData) throws IOException {
        parent.flush(metaData);
    }

    @Override
    public final synchronized void close() throws IOException {
        parent.close();
    }

    @Override
    public final synchronized void write(final int b) throws IOException {
        parent.checkOpen();
        final long totalRem = parent.remaining();
        if ( totalRem < 1 ) { // grow if required
            parent.setLength( parent.length() + 1 );
        }
        ByteBuffer slice = parent.currentSlice();
        final int currRem = slice.remaining();
        if ( 0 == currRem ) {
            if ( null == ( slice = parent.nextSlice() ) ) {
                if( MappedByteBufferInputStream.DEBUG ) {
                    System.err.println("EOT write: "+parent.currentSlice());
                    parent.dbgDump("EOT write:", System.err);
                }
                throw new IOException("EOT"); // 'end-of-tape'
            }
        }
        slice.put( (byte)(b & 0xFF) );

        // sync last buffer (happens only in synchronous mode)
        if( null != slice ) {
            parent.syncSlice(slice);
        }
    }

    @Override
    public final synchronized void write(final byte b[], final int off, final int len) throws IOException {
        parent.checkOpen();
        if (b == null) {
            throw new NullPointerException();
        } else if( off < 0 ||
                   len < 0 ||
                   off > b.length ||
                   off + len > b.length ||
                   off + len < 0
                 ) {
            throw new IndexOutOfBoundsException("offset "+off+", length "+len+", b.length "+b.length);
        } else if( 0 == len ) {
            return;
        }
        final long totalRem = parent.remaining();
        if ( totalRem < len ) { // grow if required
            parent.setLength( parent.length() + len - totalRem );
        }
        int written = 0;
        ByteBuffer slice = null;
        while( written < len ) {
            slice = parent.currentSlice();
            int currRem = slice.remaining();
            if ( 0 == currRem ) {
                if ( null == ( slice = parent.nextSlice() ) ) {
                    if( MappedByteBufferInputStream.DEBUG ) {
                        System.err.println("EOT write: offset "+off+", length "+len+", b.length "+b.length);
                        System.err.println("EOT write: written "+written+" / "+len+", currRem "+currRem);
                        System.err.println("EOT write: "+parent.currentSlice());
                        parent.dbgDump("EOT write:", System.err);
                    }
                    throw new InternalError("EOT"); // 'end-of-tape'
                }
                currRem = slice.remaining();
            }
            final int currLen = Math.min( len - written, currRem );
            slice.put( b, off + written, currLen );
            written += currLen;
        }
        // sync last buffer (happens only in synchronous mode)
        if( null != slice ) {
            parent.syncSlice(slice);
        }
    }

    /**
     * Perform similar to {@link #write(byte[], int, int)}
     * with {@link ByteBuffer} instead of byte array.
     * @param b the {@link ByteBuffer} source, data is read from current {@link ByteBuffer#position()}
     * @param len the number of bytes to write
     * @throws IOException if a buffer slice operation failed or stream has been {@link #close() closed}.
     */
    // @Override
    public final synchronized void write(final ByteBuffer b, final int len) throws IOException {
        parent.checkOpen();
        if (b == null) {
            throw new NullPointerException();
        } else if (len < 0 || len > b.remaining()) {
            throw new IndexOutOfBoundsException("length "+len+", b "+b);
        } else if( 0 == len ) {
            return;
        }
        final long totalRem = parent.remaining();
        if ( totalRem < len ) { // grow if required
            parent.setLength( parent.length() + len - totalRem );
        }
        int written = 0;
        ByteBuffer slice = null;
        while( written < len ) {
            slice = parent.currentSlice();
            int currRem = slice.remaining();
            if ( 0 == currRem ) {
                if ( null == ( slice = parent.nextSlice() ) ) {
                    if( MappedByteBufferInputStream.DEBUG ) {
                        System.err.println("EOT write: length "+len+", b "+b);
                        System.err.println("EOT write: written "+written+" / "+len+", currRem "+currRem);
                        System.err.println("EOT write: "+parent.currentSlice());
                        parent.dbgDump("EOT write:", System.err);
                    }
                    throw new InternalError("EOT"); // 'end-of-tape'
                }
                currRem = slice.remaining();
            }
            final int currLen = Math.min( len - written, currRem );

            if( slice.hasArray() && b.hasArray() ) {
                System.arraycopy(b.array(), b.arrayOffset() + b.position(),
                                 slice.array(), slice.arrayOffset() + slice.position(),
                                 currLen);
                b.position( b.position() + currLen );
                slice.position( slice.position() + currLen );
            } else if( currLen == currRem ) {
                slice.put(b);
            } else {
                final int _limit = b.limit();
                b.limit(currLen);
                try {
                    slice.put(b);
                } finally {
                    b.limit(_limit);
                }
            }
            written += currLen;
        }
        // sync last buffer (happens only in synchronous mode)
        if( null != slice ) {
            parent.syncSlice(slice);
        }
    }

    /**
     * Perform similar to {@link #write(ByteBuffer, int)}
     * with {@link MappedByteBufferInputStream} instead of byte array.
     * <p>
     * Method directly copies memory mapped {@link ByteBuffer}'ed data
     * from the given input stream to this stream without extra data copy.
     * </p>
     * @param b the {@link ByteBuffer} source, data is read from current {@link MappedByteBufferInputStream#position()}
     * @param len the number of bytes to write
     * @throws IOException if a buffer slice operation failed or stream has been {@link #close() closed}.
     */
    // @Override
    public final synchronized void write(final MappedByteBufferInputStream b, final long len) throws IOException {
        parent.checkOpen();
        if (b == null) {
            throw new NullPointerException();
        } else if (len < 0 || len > b.remaining()) {
            throw new IndexOutOfBoundsException("length "+len+", b "+b);
        } else if( 0 == len ) {
            return;
        }
        final long totalRem = parent.remaining();
        if ( totalRem < len ) { // grow if required
            parent.setLength( parent.length() + len - totalRem );
        }
        long written = 0;
        ByteBuffer slice = null;
        while( written < len ) {
            slice = parent.currentSlice();
            int currRem = slice.remaining();
            if ( 0 == currRem ) {
                if ( null == ( slice = parent.nextSlice() ) ) {
                    if( MappedByteBufferInputStream.DEBUG ) {
                        System.err.println("EOT write: length "+len+", b "+b);
                        System.err.println("EOT write: written "+written+" / "+len+", currRem "+currRem);
                        System.err.println("EOT write: "+parent.currentSlice());
                        parent.dbgDump("EOT write:", System.err);
                    }
                    throw new InternalError("EOT"); // 'end-of-tape'
                }
                currRem = slice.remaining();
            }
            final int currLen = b.read(slice, (int)Math.min( len - written, currRem ));
            if( 0 > currLen ) {
                throw new InternalError("Unexpected InputStream EOT"); // 'end-of-tape'
            }
            written += currLen;
        }
        // sync last buffer (happens only in synchronous mode)
        if( null != slice ) {
            parent.syncSlice(slice);
        }
    }
}
