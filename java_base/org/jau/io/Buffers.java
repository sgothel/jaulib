/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
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

import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.CharBuffer;
import java.nio.DoubleBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.nio.LongBuffer;
import java.nio.ShortBuffer;

import org.jau.lang.UnsafeUtil;
import org.jau.sys.Debug;
import org.jau.sys.PlatformProps;

/**
 * Utility methods allowing easy {@link java.nio.Buffer} manipulations.
 */
public class Buffers {
    static final boolean DEBUG = Debug.debug("NioUtil");

    public static final int SIZEOF_BYTE     = 1;
    public static final int SIZEOF_SHORT    = 2;
    public static final int SIZEOF_CHAR     = 2;
    public static final int SIZEOF_INT      = 4;
    public static final int SIZEOF_FLOAT    = 4;
    public static final int SIZEOF_LONG     = 8;
    public static final int SIZEOF_DOUBLE   = 8;

    private Buffers() {}

    /**
     * Allocates a new direct ByteBuffer with the specified number of
     * elements. The returned buffer will have its byte order set to
     * the host platform's native byte order.
     */
    public static ByteBuffer newDirectByteBuffer(final int size) {
        return ByteBuffer.allocateDirect( size ).order( ByteOrder.nativeOrder() );
    }

    /**
     * Helper routine to set a ByteBuffer to the native byte order, if
     * that operation is supported by the underlying NIO
     * implementation.
     */
    public static ByteBuffer nativeOrder(final ByteBuffer buf) {
        return buf.order(ByteOrder.nativeOrder());
    }

    /**
     * Helper routine to tell whether a buffer is direct or not. Null
     * pointers <b>are</b> considered direct.
     */
    public static boolean isDirect(final Object buf) {
        if (buf == null) {
            return true;
        }
        if (buf instanceof Buffer) {
            return ((Buffer) buf).isDirect();
        }
        throw new IllegalArgumentException("Unexpected buffer type " + buf.getClass().getName());
    }

    /**
     * Helper routine to get the Buffer byte offset by taking into
     * account the Buffer position and the underlying type.
     * This is the total offset for Direct Buffers.
     *
     * Return value is of type `long` only to cover the `int` multiple of the position and element type size.<br/>
     * For ByteBuffer, the return value can be safely cast to `int`.
     */
    public static long getDirectBufferByteOffset(final Object buf) {
        if (buf == null) {
            return 0;
        }
        if (buf instanceof Buffer) {
            final long pos = ((Buffer) buf).position();
            if (buf instanceof ByteBuffer) {
                return pos;
            } else if (buf instanceof FloatBuffer) {
                return pos * SIZEOF_FLOAT;
            } else if (buf instanceof IntBuffer) {
                return pos * SIZEOF_INT;
            } else if (buf instanceof ShortBuffer) {
                return pos * SIZEOF_SHORT;
            } else if (buf instanceof DoubleBuffer) {
                return pos * SIZEOF_DOUBLE;
            } else if (buf instanceof LongBuffer) {
                return pos * SIZEOF_LONG;
            } else if (buf instanceof CharBuffer) {
                return pos * SIZEOF_CHAR;
            }
        }
        throw new IllegalArgumentException("Disallowed array backing store type in buffer " + buf.getClass().getName());
    }

    /**
     * Helper routine to get the Buffer byte limit by taking into
     * account the Buffer limit and the underlying type.
     * This is the total limit for Direct Buffers.
     *
     * Return value is of type `long` only to cover the `int` multiple of the position and element type size.<br/>
     * For ByteBuffer, the return value can be safely cast to `int`.
     */
    public static long getDirectBufferByteLimit(final Object buf) {
        if (buf == null) {
            return 0;
        }
        if (buf instanceof Buffer) {
            final long limit = ((Buffer) buf).limit();
            if (buf instanceof ByteBuffer) {
                return limit;
            } else if (buf instanceof FloatBuffer) {
                return limit * SIZEOF_FLOAT;
            } else if (buf instanceof IntBuffer) {
                return limit * SIZEOF_INT;
            } else if (buf instanceof ShortBuffer) {
                return limit * SIZEOF_SHORT;
            } else if (buf instanceof DoubleBuffer) {
                return limit * SIZEOF_DOUBLE;
            } else if (buf instanceof LongBuffer) {
                return limit * SIZEOF_LONG;
            } else if (buf instanceof CharBuffer) {
                return limit * SIZEOF_CHAR;
            }
        }
        throw new IllegalArgumentException("Disallowed array backing store type in buffer " + buf.getClass().getName());
    }

    /**
     * Access to NIO {@link sun.misc.Cleaner}, allowing caller to deterministically clean a given {@link sun.nio.ch.DirectBuffer}.
     */
    public static class Cleaner {
        private static final boolean hasCleaner;
        /** OK to be lazy on thread synchronization, just for early out **/
        private static volatile boolean cleanerError;
        static {
            hasCleaner = UnsafeUtil.hasInvokeCleaner();
            cleanerError = !hasCleaner;
            if( DEBUG ) {
                System.err.println("Buffers.Cleaner.init: hasCleaner: "+hasCleaner+", cleanerError "+cleanerError);
            }
        }

        /**
         * If {@code b} is an direct NIO buffer, i.e {@link sun.nio.ch.DirectBuffer},
         * calls it's {@link sun.misc.Cleaner} instance {@code clean()} method once.
         * @return {@code true} if successful, otherwise {@code false}.
         */
        public static boolean clean(final ByteBuffer bb) {
            if( !hasCleaner && ( cleanerError || !bb.isDirect() ) ) {
                return false;
            }
            if( PlatformProps.JAVA_9 ) {
                UnsafeUtil.invokeCleaner(bb);
            } else {
                return false;
            }
            return true;
        }
    }

}
