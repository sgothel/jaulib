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
package org.jau.lang;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import org.jau.sys.Debug;
import org.jau.sys.PlatformProps;

/**
 * Utility methods to simplify NIO buffers
 */
public class NioUtil {
    static final boolean DEBUG = Debug.debug("NioUtil");

    public static final int SIZEOF_BYTE     = 1;
    public static final int SIZEOF_SHORT    = 2;
    public static final int SIZEOF_CHAR     = 2;
    public static final int SIZEOF_INT      = 4;
    public static final int SIZEOF_FLOAT    = 4;
    public static final int SIZEOF_LONG     = 8;
    public static final int SIZEOF_DOUBLE   = 8;

    private NioUtil() {}

    public static ByteBuffer newNativeByteBuffer(final int size) {
        return ByteBuffer.allocateDirect( size ).order( ByteOrder.nativeOrder() );
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
