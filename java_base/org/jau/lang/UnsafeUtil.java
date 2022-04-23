/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2019 Gothel Software e.K.
 * Copyright (c) 2019 JogAmp Community.
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

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;
import java.security.PrivilegedAction;

import org.jau.sec.SecurityUtil;
import org.jau.sys.Debug;
import org.jau.sys.PlatformProps;

/**
 * Utility methods allowing easy access to certain {@link sun.misc.Unsafe} functionality.
 */
public class UnsafeUtil {

    static final boolean DEBUG;
    static {
        DEBUG = Debug.debug("UnsafeUtil");
    }

    protected UnsafeUtil() {}

    private static final Object theUnsafe;
    private static final Method unsafeCleanBB;
    private static volatile boolean hasUnsafeCleanBBError; /** OK to be lazy on thread synchronization, just for early out **/

    private static final Method staticFieldOffset;
    private static final Method getObjectVolatile;
    private static final Method putObjectVolatile;
    private static volatile boolean hasGetPutObjectVolatile;

    private static final Class<?> illegalAccessLoggerClass;
    private static final Long illegalAccessLoggerOffset;
    private static final Object illegalAccessLoggerSync = new Object();
    private static volatile boolean hasIllegalAccessError;

    static {
        final Object[] _theUnsafe = { null };
        final Method[] _cleanBB = { null };
        final Method[] _staticFieldOffset = { null };
        final Method[] _objectVolatile = { null, null }; // unsafeGetObjectVolatile, unsafePutObjectVolatile
        final Class<?>[] _illegalAccessLoggerClass = { null };
        final Long[] _loggerOffset = { null };

        SecurityUtil.doPrivileged(new PrivilegedAction<Object>() {
            @Override
            public Object run() {
                Class<?> unsafeClass = null;
                try {
                    // Using: sun.misc.Unsafe { public void invokeCleaner(java.nio.ByteBuffer directBuffer); }
                    unsafeClass = Class.forName("sun.misc.Unsafe");
                    {
                        final Field f = unsafeClass.getDeclaredField("theUnsafe");
                        f.setAccessible(true);
                        _theUnsafe[0] = f.get(null);
                    }
                    _cleanBB[0] = unsafeClass.getMethod("invokeCleaner", java.nio.ByteBuffer.class);
                    _cleanBB[0].setAccessible(true);
                } catch(final Throwable t) {
                    if( DEBUG ) {
                        ExceptionUtils.dumpThrowable("UnsafeUtil", t);
                    }
                }
                if( null != _theUnsafe[0] && PlatformProps.JAVA_9 ) {
                    try {
                        _staticFieldOffset[0] = unsafeClass.getDeclaredMethod("staticFieldOffset", Field.class);
                        _objectVolatile[0] = unsafeClass.getDeclaredMethod("getObjectVolatile", Object.class, long.class);
                        _objectVolatile[1] = unsafeClass.getDeclaredMethod("putObjectVolatile", Object.class, long.class, Object.class);

                        if( PlatformProps.JAVA_9 ) {
                            _illegalAccessLoggerClass[0] = Class.forName("jdk.internal.module.IllegalAccessLogger");
                            final Field loggerField = _illegalAccessLoggerClass[0].getDeclaredField("logger");
                            _loggerOffset[0] = (Long) _staticFieldOffset[0].invoke(_theUnsafe[0], loggerField);
                        }
                    } catch(final Throwable t) {
                        if( DEBUG ) {
                            ExceptionUtils.dumpThrowable("UnsafeUtil", t);
                        }
                    }
                }
                return null;
            } } );
        theUnsafe = _theUnsafe[0];
        unsafeCleanBB = _cleanBB[0];
        hasUnsafeCleanBBError = null == theUnsafe || null == unsafeCleanBB;
        if( DEBUG ) {
            System.err.println("UnsafeUtil.init: hasTheUnsafe: "+(null!=theUnsafe)+", hasInvokeCleaner: "+!hasUnsafeCleanBBError);
        }

        staticFieldOffset = _staticFieldOffset[0];
        getObjectVolatile = _objectVolatile[0];
        putObjectVolatile = _objectVolatile[1];
        hasGetPutObjectVolatile = null != staticFieldOffset && null != getObjectVolatile && null != putObjectVolatile;
        illegalAccessLoggerClass = _illegalAccessLoggerClass[0];
        illegalAccessLoggerOffset = _loggerOffset[0];
        hasIllegalAccessError = !hasGetPutObjectVolatile || null == illegalAccessLoggerClass || null == illegalAccessLoggerOffset;
        if( DEBUG ) {
            System.err.println("UnsafeUtil.init: hasUnsafeGetPutObjectVolatile: "+hasGetPutObjectVolatile+", hasUnsafeIllegalAccessLogger: "+!hasIllegalAccessError);
        }
    }

    /**
     * Returns {@code true} if {@code sun.misc.Unsafe.invokeCleaner(java.nio.ByteBuffer)}
     * is available and has not caused an exception.
     * @see #invokeCleaner(ByteBuffer)
     */
    public static boolean hasInvokeCleaner() { return !hasUnsafeCleanBBError; }

    /**
     * Access to {@code sun.misc.Unsafe.invokeCleaner(java.nio.ByteBuffer)}.
     * <p>
     * If {@code b} is an direct NIO buffer, i.e {@link sun.nio.ch.DirectBuffer},
     * calls it's {@link sun.misc.Cleaner} instance {@code clean()} method once.
     * </p>
     * @return {@code true} if successful, otherwise {@code false}.
     * @see #hasInvokeCleaner()
     */
    public static boolean invokeCleaner(final ByteBuffer bb) {
        if( hasUnsafeCleanBBError || !bb.isDirect() ) {
            return false;
        }
        try {
            unsafeCleanBB.invoke(theUnsafe, bb);
            return true;
        } catch(final Throwable t) {
            hasUnsafeCleanBBError = true;
            if( DEBUG ) {
                ExceptionUtils.dumpThrowable("UnsafeUtil", t);
            }
            return false;
        }
    }

    /**
     * Returns {@code true} if access to {@code jdk.internal.module.IllegalAcessLogger}'s {@code logger} field
     * is available and has not caused an exception.
     * @see #doWithoutIllegalAccessLogger(PrivilegedAction)
     */
    public static boolean hasIllegalAccessLoggerAccess() { return !hasIllegalAccessError; }

    /**
     * Issue the given user {@code action} while {@code jdk.internal.module.IllegalAcessLogger}'s {@code logger} has been temporarily disabled.
     * <p>
     * The caller shall place this call into their own {@link AccessController#doPrivileged(PrivilegedAction)} block.
     * </p>
     * <p>
     * In case the runtime is not {@link PlatformProps#JAVA_9} or the logger is not accessible or disabling caused an exception,
     * the user {@code action} is just executed w/o temporary logger modifications.
     * </p>
     * @param action the user action task
     * @throws RuntimeException is thrown for a caught {@link Throwable} while executing the user {@code action}
     * @see #hasIllegalAccessLoggerAccess()
     */
    public static <T> T doWithoutIllegalAccessLogger(final PrivilegedAction<T> action) throws RuntimeException {
        if( !hasIllegalAccessError ) {
            synchronized(illegalAccessLoggerSync) {
                final Object newLogger = null;
                Object oldLogger = null;
                try {
                    oldLogger = getObjectVolatile.invoke(theUnsafe, illegalAccessLoggerClass, illegalAccessLoggerOffset);
                    putObjectVolatile.invoke(theUnsafe, illegalAccessLoggerClass, illegalAccessLoggerOffset, newLogger);
                } catch (IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
                    // unaccessible ..
                    hasIllegalAccessError = true;
                    if( DEBUG ) {
                        ExceptionUtils.dumpThrowable("UnsafeUtil", e);
                    }
                    return action.run();
                }
                try {
                    return action.run();
                } catch (final Throwable t) {
                    if( DEBUG ) {
                        t.printStackTrace();
                    }
                    throw new RuntimeException(t);
                } finally {
                    try {
                        putObjectVolatile.invoke(theUnsafe, illegalAccessLoggerClass, illegalAccessLoggerOffset, oldLogger);
                    } catch (IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
                        // should not happen, worked above @ logger setup
                        hasIllegalAccessError = true;
                        throw new InternalError(e);
                    }
                }
            }
        } else {
            return action.run();
        }
    }
}
