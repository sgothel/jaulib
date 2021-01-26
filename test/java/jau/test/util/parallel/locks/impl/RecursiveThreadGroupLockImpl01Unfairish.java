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
package jau.test.util.parallel.locks.impl;

import java.util.Arrays;

import jau.test.util.parallel.locks.RecursiveThreadGroupLock;

public class RecursiveThreadGroupLockImpl01Unfairish
    extends RecursiveLockImpl01Unfairish
    implements RecursiveThreadGroupLock
{
    /* package */ @SuppressWarnings("serial")
    static class ThreadGroupSync extends SingleThreadSync {
        /* package */ ThreadGroupSync() {
            super();
            threadNum = 0;
            threads = null;
            holdCountAdditionOwner = 0;
            waitingOrigOwner = null;
        }
        @Override
        public final void incrHoldCount(final Thread t) {
            super.incrHoldCount(t);
            if(!isOriginalOwner(t)) {
                holdCountAdditionOwner++;
            }
        }
        @Override
        public final void decrHoldCount(final Thread t) {
            super.decrHoldCount(t);
            if(!isOriginalOwner(t)) {
                holdCountAdditionOwner--;
            }
        }
        public final int getAdditionalOwnerHoldCount() {
            return holdCountAdditionOwner;
        }

        public final boolean isOriginalOwner(final Thread t) {
            return super.isOwner(t);
        }
        public final void setWaitingOrigOwner(final Thread origOwner) {
            waitingOrigOwner = origOwner;
        }
        public final Thread getWaitingOrigOwner() {
            return waitingOrigOwner;
        }
        @Override
        public final boolean isOwner(final Thread t) {
            if(getExclusiveOwnerThread()==t) {
                return true;
            }
            for(int i=threadNum-1; 0<=i; i--) {
                if(threads[i]==t) {
                    return true;
                }
            }
            return false;
        }

        public final int getAddOwnerCount() {
            return threadNum;
        }
        public final void addOwner(final Thread t) throws IllegalArgumentException {
            if(null == threads) {
                if(threadNum>0) {
                    throw new InternalError("XXX");
                }
                threads = new Thread[4];
            }
            for(int i=threadNum-1; 0<=i; i--) {
                if(threads[i]==t) {
                    throw new IllegalArgumentException("Thread already added: "+t);
                }
            }
            if (threadNum == threads.length) {
                threads = Arrays.copyOf(threads, threadNum * 2);
            }
            threads[threadNum] = t;
            threadNum++;
        }

        public final void removeAllOwners() {
            for(int i=threadNum-1; 0<=i; i--) {
                threads[i]=null;
            }
            threadNum=0;
        }

        public final void removeOwner(final Thread t) throws IllegalArgumentException {
            for (int i = 0 ; i < threadNum ; i++) {
                if (threads[i] == t) {
                    threadNum--;
                    System.arraycopy(threads, i + 1, threads, i, threadNum - i);
                    threads[threadNum] = null; // cleanup 'dead' [or duplicate] reference for GC
                    return;
                }
            }
            throw new IllegalArgumentException("Not an owner: "+t);
        }

        String addOwnerToString() {
            final StringBuilder sb = new StringBuilder();
            for(int i=0; i<threadNum; i++) {
                if(i>0) {
                    sb.append(", ");
                }
                sb.append(threads[i].getName());
            }
            return sb.toString();
        }

        // lock count by addition owner threads
        private int holdCountAdditionOwner;
        private Thread[] threads;
        private int threadNum;
        private Thread waitingOrigOwner;
    }

    public RecursiveThreadGroupLockImpl01Unfairish() {
        super(new ThreadGroupSync());
    }

    @Override
    public final boolean isOriginalOwner() {
        return isOriginalOwner(Thread.currentThread());
    }

    @Override
    public final boolean isOriginalOwner(final Thread thread) {
        synchronized(sync) {
            return ((ThreadGroupSync)sync).isOriginalOwner(thread) ;
        }
    }

    @Override
    public final void addOwner(final Thread t) throws RuntimeException, IllegalArgumentException {
        validateLocked();
        final Thread cur = Thread.currentThread();
        final ThreadGroupSync tgSync = (ThreadGroupSync)sync;
        if(!tgSync.isOriginalOwner(cur)) {
            throw new IllegalArgumentException("Current thread is not the original owner: orig-owner: "+tgSync.getOwner()+", current "+cur+": "+toString());
        }
        if(tgSync.isOriginalOwner(t)) {
            throw new IllegalArgumentException("Passed thread is original owner: "+t+", "+toString());
        }
        tgSync.addOwner(t);
    }

    @Override
    public final void unlock(final Runnable taskAfterUnlockBeforeNotify) {
        synchronized(sync) {
            final Thread cur = Thread.currentThread();
            final ThreadGroupSync tgSync = (ThreadGroupSync)sync;

            if( tgSync.getAddOwnerCount()>0 ) {
                if(TRACE_LOCK) {
                    System.err.println("--- LOCK XR (tg) "+toString()+", cur "+threadName(cur)+" -> owner...");
                }
                if( tgSync.isOriginalOwner(cur) ) {
                    // original locking owner thread
                    if( tgSync.getHoldCount() - tgSync.getAdditionalOwnerHoldCount() == 1 ) {
                        // release orig. lock
                        tgSync.setWaitingOrigOwner(cur);
                        try {
                            while ( tgSync.getAdditionalOwnerHoldCount() > 0 ) {
                                try {
                                    sync.wait();
                                } catch (final InterruptedException e) {
                                    // regular wake up!
                                }
                            }
                        } finally {
                            tgSync.setWaitingOrigOwner(null);
                            Thread.interrupted(); // clear slipped interrupt
                        }
                        tgSync.removeAllOwners();
                    }
                } else if( tgSync.getAdditionalOwnerHoldCount() == 1 ) {
                    // last additional owner thread wakes up original owner if waiting in unlock(..)
                    final Thread originalOwner = tgSync.getWaitingOrigOwner();
                    if( null != originalOwner ) {
                        originalOwner.interrupt();
                    }
                }
            }
            if(TRACE_LOCK) {
                System.err.println("++ unlock(X): currentThread "+cur.getName()+", lock: "+this.toString());
                System.err.println("--- LOCK X0 (tg) "+toString()+", cur "+threadName(cur)+" -> unlock!");
            }
            super.unlock(taskAfterUnlockBeforeNotify);
        }
    }

    @Override
    public final void removeOwner(final Thread t) throws RuntimeException, IllegalArgumentException {
        validateLocked();
        ((ThreadGroupSync)sync).removeOwner(t);
    }

    @Override
    public String toString() {
        final ThreadGroupSync tgSync = (ThreadGroupSync)sync;
        final int hc = sync.getHoldCount();
        final int addHC = tgSync.getAdditionalOwnerHoldCount();
        return syncName()+"[count "+hc+" [ add. "+addHC+", orig "+(hc-addHC)+
                           "], qsz "+sync.getQSz()+", owner "+threadName(sync.getOwner())+", add.owner "+tgSync.addOwnerToString()+"]";
    }
}
