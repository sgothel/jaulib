/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2010 Gothel Software e.K.
 * Copyright (c) 2010 JogAmp Community.
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

package org.jau.util.parallel.locks;

import org.jau.sys.Debug;

/**
 * Specifying a thread blocking lock implementation
 */
public interface Lock {

    /** Enable via the property <code>jogamp.debug.Lock</code> */
    public static final boolean DEBUG = Debug.debug("Lock");

    /** Enable via the property <code>jogamp.debug.Lock.TraceLock</code> */
    public static final boolean TRACE_LOCK = Debug.isPropertyDefined("jogamp.debug.Lock.TraceLock", true);

    /** The default {@link #TIMEOUT} value, of {@value} ms */
    public static final long DEFAULT_TIMEOUT = 5000; // 5s default timeout

    /**
     * The <code>TIMEOUT</code> for {@link #lock()} in ms,
     * defaults to {@link #DEFAULT_TIMEOUT}.
     * <p>
     * It can be overridden via the system property <code>jogamp.common.utils.locks.Lock.timeout</code>.
     * </p>
     */
    public static final long TIMEOUT = Debug.getLongProperty("jogamp.common.utils.locks.Lock.timeout", true, DEFAULT_TIMEOUT);

    /**
     * Blocking until the lock is acquired by this Thread or {@link #TIMEOUT} is reached.
     *
     * @throws RuntimeException in case of {@link #TIMEOUT}
     */
    void lock() throws RuntimeException;

    /**
     * Blocking until the lock is acquired by this Thread or <code>maxwait</code> in ms is reached.
     *
     * @param timeout Maximum time in ms to wait to acquire the lock. If this value is zero,
     *                the call returns immediately either without being able
     *                to acquire the lock, or with acquiring the lock directly while ignoring any scheduling order.
     * @return true if the lock has been acquired within <code>maxwait</code>, otherwise false
     *
     * @throws InterruptedException
     */
    boolean tryLock(long timeout) throws InterruptedException;

    /**
     * Release the lock.
     *
     * @throws RuntimeException in case the lock is not acquired by this thread.
     */
    void unlock() throws RuntimeException;

    /** Query if locked */
    boolean isLocked();
}
