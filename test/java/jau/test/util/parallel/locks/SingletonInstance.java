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

package jau.test.util.parallel.locks;

import java.io.File;

import jau.test.util.parallel.locks.impl.SingletonInstanceFileLock;
import jau.test.util.parallel.locks.impl.SingletonInstanceServerSocket;

public abstract class SingletonInstance implements Lock {

    protected static final boolean DEBUG = true;

    public static SingletonInstance createFileLock(final long poll_ms, final String lockFileBasename) {
        return new SingletonInstanceFileLock(poll_ms, lockFileBasename);
    }

    public static SingletonInstance createFileLock(final long poll_ms, final File lockFile) {
        return new SingletonInstanceFileLock(poll_ms, lockFile);
    }

    /**
     * A user shall use <b>ephemeral ports</b>:
     * <ul>
     *  <li>IANA suggests 49152 to 65535 as "dynamic and/or private ports".</li>
     *  <li>Many GNU/Linux kernels use 32768 to 61000.</li>
     *  <li>FreeBSD >= 4.6 uses the IANA port range.</li>
     *  <li>FreeBSD < 4.6 and BSD use ports 1024 through 4999.</li>
     *  <li>Microsoft Windows operating systems through Server 2003 use the range 1025 to 5000</li>
     *  <li>Windows Vista, Windows 7, and Server 2008 use the IANA range.</li>
     * </ul>
     * @param pollPeriod
     * @param portNumber to be used for this single instance server socket.
     */
    public static SingletonInstance createServerSocket(final long poll_ms, final int portNumber) {
        return new SingletonInstanceServerSocket(poll_ms, portNumber);
    }

    protected SingletonInstance(final long poll_ms) {
        this.poll_ms = Math.max(10, poll_ms);
    }

    public final long getPollPeriod() { return poll_ms; }
    public abstract String getName();
    @Override
    public final String toString() { return getName(); }

    @Override
    public synchronized void lock() throws RuntimeException {
        try {
            do {
                if(tryLock(TIMEOUT)) {
                    return;
                }
            } while ( true ) ;
        } catch ( final RuntimeException ie ) {
            throw new  RuntimeException(ie);
        }
    }

    @Override
    public synchronized boolean tryLock(long maxwait) throws RuntimeException {
        if(locked) {
            return true;
        }
        final long t0 = System.currentTimeMillis();
        int i=0;
        try {
            do {
                final long t1 = System.currentTimeMillis();
                locked = tryLockImpl();
                if(locked) {
                    if( DEBUG ) {
                        final long t2 = System.currentTimeMillis();
                        System.err.println(infoPrefix(t2)+" +++ "+getName()+" - Locked within "+(t2-t0)+" ms, "+(i+1)+" attempts");
                    }
                    return true;
                }
                if( DEBUG && 0==i ) {
                    System.err.println(infoPrefix(System.currentTimeMillis())+" III "+getName()+" - Wait for lock");
                }
                Thread.sleep(poll_ms);
                maxwait -= System.currentTimeMillis()-t1;
                i++;
            } while ( 0 < maxwait ) ;
        } catch ( final InterruptedException ie ) {
            final long t2 = System.currentTimeMillis();
            throw new RuntimeException(infoPrefix(t2)+" EEE (1) "+getName()+" - couldn't get lock within "+(t2-t0)+" ms, "+i+" attempts", ie);
        }
        if( DEBUG ) {
            final long t2 = System.currentTimeMillis();
            System.err.println(infoPrefix(t2)+" +++ EEE (2) "+getName()+" - couldn't get lock within "+(t2-t0)+" ms, "+i+" attempts");
        }
        return false;
    }
    protected abstract boolean tryLockImpl();

    @Override
    public void unlock() throws RuntimeException {
        final long t0 = System.currentTimeMillis();
        if(locked) {
            locked = !unlockImpl();
            if( DEBUG ) {
                final long t2 = System.currentTimeMillis();
                System.err.println(infoPrefix(t2)+" --- "+getName()+" - Unlock "+ ( locked ? "failed" : "ok" ) + " within "+(t2-t0)+" ms");
            }
        }
    }
    protected abstract boolean unlockImpl();

    @Override
    public synchronized boolean isLocked() {
        return locked;
    }

    protected String infoPrefix(final long currentMillis) {
        return "SLOCK [T "+Thread.currentThread().getName()+" @ "+currentMillis+" ms";
    }
    protected String infoPrefix() {
        return infoPrefix(System.currentTimeMillis());
    }

    private final long poll_ms;
    private boolean locked = false;
}
