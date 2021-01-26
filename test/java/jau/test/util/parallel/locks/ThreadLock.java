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

package jau.test.util.parallel.locks;

/**
 * Extending the {@link Lock} features with convenient functionality.
 */
public interface ThreadLock extends Lock {

    /** Query whether the lock is hold by the a thread other than the current thread. */
    boolean isLockedByOtherThread();

    /** Query whether the lock is hold by the given thread. */
    boolean isOwner(Thread thread);

    /**
     * @return the Thread owning this lock if locked, otherwise null
     */
    Thread getOwner();

    /**
     * @throws RuntimeException if current thread does not hold the lock
     */
    void validateLocked() throws RuntimeException;

    /**
     * Execute the {@link Runnable Runnable taskAfterUnlockBeforeNotify} while holding the exclusive lock.
     * <p>
     * Then release the lock.
     * </p>
     */
    void unlock(Runnable taskAfterUnlockBeforeNotify);
}
