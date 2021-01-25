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

package org.jau.junit.util;

import org.junit.BeforeClass;
import org.jau.util.parallel.locks.SingletonInstance;
import org.junit.AfterClass;
import org.junit.FixMethodOrder;
import org.junit.runners.MethodSorters;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public abstract class SingletonJunitCase extends JunitTracer {
    public static final String SINGLE_INSTANCE_LOCK_FILE = "SingletonTestCase.lock";
    public static final int SINGLE_INSTANCE_LOCK_PORT = 59999;

    public static final long SINGLE_INSTANCE_LOCK_TO   = 15*60*1000; // wait up to 15 mins
    public static final long SINGLE_INSTANCE_LOCK_POLL =        500; // poll every 500 ms

    private static SingletonInstance singletonInstance = null; // system wide lock via port locking
    private static final Object singletonSync = new Object();  // classloader wide lock
    private static boolean enabled = true;

    /**
     * Default is {@code true}.
     */
    public static final void enableSingletonLock(final boolean v) {
        enabled = v;
    }

    @BeforeClass
    public static final void oneTimeSetUpSingleton() {
        // one-time initialization code
        synchronized( singletonSync ) {
            if( enabled ) {
                if( null == singletonInstance )  {
                    System.err.println("++++ Test Singleton.ctor()");
                    // singletonInstance = SingletonInstance.createFileLock(SINGLE_INSTANCE_LOCK_POLL, SINGLE_INSTANCE_LOCK_FILE);
                    singletonInstance = SingletonInstance.createServerSocket(SINGLE_INSTANCE_LOCK_POLL, SINGLE_INSTANCE_LOCK_PORT);
                }
                System.err.println("++++ Test Singleton.lock()");
                if(!singletonInstance.tryLock(SINGLE_INSTANCE_LOCK_TO)) {
                    throw new RuntimeException("Fatal: Could not lock single instance: "+singletonInstance.getName());
                }
            }
        }
    }

    @AfterClass
    public static final void oneTimeTearDownSingleton() {
        // one-time cleanup code
        synchronized( singletonSync ) {
            System.gc(); // force cleanup
            if( enabled ) {
                System.err.println("++++ Test Singleton.unlock()");
                singletonInstance.unlock();
                try {
                    // allowing other JVM instances to pick-up socket
                    Thread.sleep( SINGLE_INSTANCE_LOCK_POLL );
                } catch (final InterruptedException e) { }
            }
        }
    }
}

