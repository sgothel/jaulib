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

package jau.util.parallel.locks;

import java.util.List;
import java.util.concurrent.locks.AbstractOwnableSynchronizer;

import org.jau.util.parallel.locks.RecursiveLock;

/**
 * Reentrance locking toolkit, impl a non-complete fair FIFO scheduler.
 * <p>
 * Fair scheduling is not guaranteed due to the usage of {@link Object#notify()},
 * however new lock-applicants will wait if queue is not empty for {@link #lock()}
 * and {@link #tryLock(long) tryLock}(timeout>0).</p>
 *
 * <p>
 * Sync object extends {@link AbstractOwnableSynchronizer}, hence monitoring is possible.</p>
 */
public class RecursiveLockImpl01Unfairish implements RecursiveLock {

    /* package */ static interface Sync {
        Thread getOwner();
        boolean isOwner(Thread t);
        void setOwner(Thread t);

        Throwable getLockedStack();
        void setLockedStack(Throwable s);

        int getHoldCount();
        void incrHoldCount(Thread t);
        void decrHoldCount(Thread t);

        int getQSz();
        void incrQSz();
        void decrQSz();
    }

    @SuppressWarnings("serial")
    /* package */ static class SingleThreadSync extends AbstractOwnableSynchronizer implements Sync {
        /* package */ SingleThreadSync() {
            super();
        }
        @Override
        public final Thread getOwner() {
            return getExclusiveOwnerThread();
        }
        @Override
        public boolean isOwner(final Thread t) {
            return getExclusiveOwnerThread()==t;
        }
        @Override
        public final void setOwner(final Thread t) {
            setExclusiveOwnerThread(t);
        }
        @Override
        public final Throwable getLockedStack() {
            return lockedStack;
        }
        @Override
        public final void setLockedStack(final Throwable s) {
            final List<Throwable> ls = LockDebugUtil.getRecursiveLockTrace();
            if(s==null) {
                ls.remove(lockedStack);
            } else {
                ls.add(s);
            }
            lockedStack = s;
        }
        @Override
        public final int getHoldCount() { return holdCount; }
        @Override
        public void incrHoldCount(final Thread t) { holdCount++; }
        @Override
        public void decrHoldCount(final Thread t) { holdCount--; }

        @Override
        public final int getQSz() { return qsz; }
        @Override
        public final void incrQSz() { qsz++; }
        @Override
        public final void decrQSz() { qsz--; }

        /** lock count by same thread */
        private int holdCount = 0;
        /** queue size of waiting threads */
        private int qsz = 0;
        /** stack trace of the lock, only used if DEBUG */
        private Throwable lockedStack = null;
    }

    protected final Sync sync;

    public RecursiveLockImpl01Unfairish(final Sync sync) {
        this.sync = sync;
    }

    public RecursiveLockImpl01Unfairish() {
        this(new SingleThreadSync());
    }

    /**
     * Returns the Throwable instance generated when this lock was taken the 1st time
     * and if {@link org.jau.util.parallel.locks.Lock#DEBUG} is turned on, otherwise it returns always <code>null</code>.
     * @see org.jau.util.parallel.locks.Lock#DEBUG
     */
    public final Throwable getLockedStack() {
        synchronized(sync) {
            return sync.getLockedStack();
        }
    }

    @Override
    public final Thread getOwner() {
        synchronized(sync) {
            return sync.getOwner();
        }
    }

    @Override
    public final boolean isOwner(final Thread thread) {
        synchronized(sync) {
            return sync.isOwner(thread);
        }
    }

    @Override
    public final boolean isLocked() {
        synchronized(sync) {
            return null != sync.getOwner();
        }
    }

    @Override
    public final boolean isLockedByOtherThread() {
        synchronized(sync) {
            final Thread o = sync.getOwner();
            return null != o && Thread.currentThread() != o ;
        }
    }

    @Override
    public final int getHoldCount() {
        synchronized(sync) {
            return sync.getHoldCount();
        }
    }

    @Override
    public final void validateLocked() throws RuntimeException {
        synchronized(sync) {
            if ( !sync.isOwner(Thread.currentThread()) ) {
                if ( null == sync.getOwner() ) {
                    throw new RuntimeException(threadName(Thread.currentThread())+": Not locked: "+toString());
                }
                if(null!=sync.getLockedStack()) {
                    sync.getLockedStack().printStackTrace();
                }
                throw new RuntimeException(Thread.currentThread()+": Not owner: "+toString());
            }
        }
    }

    @Override
    public final void lock() {
        synchronized(sync) {
            try {
                if(!tryLock(TIMEOUT)) {
                    if(null!=sync.getLockedStack()) {
                        sync.getLockedStack().printStackTrace();
                    }
                    throw new RuntimeException("Waited "+TIMEOUT+"ms for: "+toString()+" - "+threadName(Thread.currentThread()));
                }
            } catch (final InterruptedException e) {
                throw new RuntimeException("Interrupted", e);
            }
        }
    }

    @Override
    public final boolean tryLock(long timeout) throws InterruptedException {
        synchronized(sync) {
            final Thread cur = Thread.currentThread();
            if(TRACE_LOCK) {
                System.err.println("+++ LOCK 0 "+toString()+", timeout "+timeout+" ms, cur "+threadName(cur));
            }
            if (sync.isOwner(cur)) {
                sync.incrHoldCount(cur);
                if(TRACE_LOCK) {
                    System.err.println("+++ LOCK XR "+toString()+", cur "+threadName(cur));
                }
                return true;
            }

            if ( sync.getOwner() != null || ( 0<timeout && 0<sync.getQSz() ) ) {

                if ( 0 >= timeout ) {
                    // locked by other thread and no waiting requested
                    if(TRACE_LOCK) {
                        System.err.println("+++ LOCK XY "+toString()+", cur "+threadName(cur)+", left "+timeout+" ms");
                    }
                    return false;
                }

                sync.incrQSz();
                do {
                    final long t0 = System.currentTimeMillis();
                    sync.wait(timeout);
                    timeout -= System.currentTimeMillis() - t0;
                } while (null != sync.getOwner() && 0 < timeout) ;
                sync.decrQSz();

                if( 0 >= timeout && sync.getOwner() != null ) {
                    // timed out
                    if(TRACE_LOCK) {
                        System.err.println("+++ LOCK XX "+toString()+", cur "+threadName(cur)+", left "+timeout+" ms");
                    }
                    return false;
                }

                if(TRACE_LOCK) {
                    System.err.println("+++ LOCK X1 "+toString()+", cur "+threadName(cur)+", left "+timeout+" ms");
                }
            } else if(TRACE_LOCK) {
                System.err.println("+++ LOCK X0 "+toString()+", cur "+threadName(cur));
            }

            sync.setOwner(cur);
            sync.incrHoldCount(cur);

            if(DEBUG) {
                sync.setLockedStack(new Throwable("Previously locked by "+toString()));
            }
            return true;
        }
    }


    @Override
    public final void unlock() {
        synchronized(sync) {
            unlock(null);
        }
    }

    @Override
    public void unlock(final Runnable taskAfterUnlockBeforeNotify) {
        synchronized(sync) {
            validateLocked();
            final Thread cur = Thread.currentThread();

            sync.decrHoldCount(cur);

            if (sync.getHoldCount() > 0) {
                if(TRACE_LOCK) {
                    System.err.println("--- LOCK XR "+toString()+", cur "+threadName(cur));
                }
                return;
            }

            sync.setOwner(null);
            if(DEBUG) {
                sync.setLockedStack(null);
            }
            if(null!=taskAfterUnlockBeforeNotify) {
                taskAfterUnlockBeforeNotify.run();
            }

            if(TRACE_LOCK) {
                System.err.println("--- LOCK X0 "+toString()+", cur "+threadName(cur)+", signal any");
            }
            sync.notify();
        }
    }

    @Override
    public final int getQueueLength() {
        synchronized(sync) {
            return sync.getQSz();
        }
    }

    @Override
    public String toString() {
        return syncName()+"[count "+sync.getHoldCount()+
                           ", qsz "+sync.getQSz()+", owner "+threadName(sync.getOwner())+"]";
    }

    /* package */ final String syncName() {
        return "<"+Integer.toHexString(this.hashCode())+", "+Integer.toHexString(sync.hashCode())+">";
    }
    /* package */ final String threadName(final Thread t) { return null!=t ? "<"+t.getName()+">" : "<NULL>" ; }
}

