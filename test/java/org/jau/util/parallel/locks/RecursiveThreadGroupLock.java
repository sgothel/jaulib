/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2011 Gothel Software e.K.
 * Copyright (c) 2011 JogAmp Community.
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

/**
 * Reentrance capable locking toolkit, supporting multiple threads as owner.
 * <p>
 * See use case description at {@link #addOwner(Thread)}.
 * </p>
 */
public interface RecursiveThreadGroupLock extends RecursiveLock {
    /**
     * Returns true if the current thread is the original lock owner, ie.
     * successfully claimed this lock the first time, ie. {@link #getHoldCount()} == 1.
     */
    boolean isOriginalOwner();

    /**
     * Returns true if the passed thread is the original lock owner, ie.
     * successfully claimed this lock the first time, ie. {@link #getHoldCount()} == 1.
     */
    boolean isOriginalOwner(Thread thread);

    /**
     * Add a thread to the list of additional lock owners, which enables them to recursively claim this lock.
     * <p>
     * The caller must hold this lock and be the original lock owner, see {@link #isOriginalOwner()}.
     * </p>
     * <p>
     * If the original owner releases this lock via {@link #unlock()}
     * all additional lock owners are released as well.
     * This ensures consistency of spawn off additional lock owner threads and it's release.
     * </p>
     * Use case:
     * <pre>
     * Thread2 thread2 = new Thread2();
     *
     * Thread1 {
     *
     *   // Claim this lock and become the original lock owner.
     *   lock.lock();
     *
     *   try {
     *
     *     // Allow Thread2 to claim the lock, ie. make thread2 an additional lock owner
     *     addOwner(thread2);
     *
     *     // Start thread2
     *     thread2.start();
     *
     *     // Wait until thread2 has finished requiring this lock, but keep thread2 running
     *     while(!thread2.waitForResult()) sleep();
     *
     *     // Optional: Only if sure that this thread doesn't hold the lock anymore,
     *     // otherwise just release the lock via unlock().
     *     removeOwner(thread2);
     *
     *   } finally {
     *
     *     // Release this lock and remove all additional lock owners.
     *     // Implicit wait until thread2 gets off the lock.
     *     lock.unlock();
     *
     *   }
     *
     * }.start();
     * </pre>
     *
     * @param t the thread to be added to the list of additional owning threads
     * @throws RuntimeException if the current thread does not hold the lock.
     * @throws IllegalArgumentException if the passed thread is the lock owner or already added.
     *
     * @see #removeOwner(Thread)
     * @see #unlock()
     * @see #lock()
     */
    void addOwner(Thread t) throws RuntimeException, IllegalArgumentException;

    /**
     * Remove a thread from the list of additional lock owner threads.
     * <p>
     * The caller must hold this lock and be the original lock owner, see {@link #isOriginalOwner()}.
     * </p>
     * <p>
     * Only use this method if sure that the thread doesn't hold the lock anymore.
     * </p>
     *
     * @param t the thread to be removed from the list of additional owning threads
     * @throws RuntimeException if the current thread does not hold the lock.
     * @throws IllegalArgumentException if the passed thread is not added by {@link #addOwner(Thread)}
     */
    void removeOwner(Thread t) throws RuntimeException, IllegalArgumentException;

    /**
     * <p>
     * Wait's until all additional owners released this lock before releasing it.
     * </p>
     *
     * {@inheritDoc}
     */
    @Override
    void unlock() throws RuntimeException;

    /**
     * <p>
     * Wait's until all additional owners released this lock before releasing it.
     * </p>
     *
     * {@inheritDoc}
     */
    @Override
    void unlock(Runnable taskAfterUnlockBeforeNotify);

}
