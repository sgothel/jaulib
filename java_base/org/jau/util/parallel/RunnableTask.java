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

package org.jau.util.parallel;

import java.io.PrintStream;

import org.jau.lang.InterruptSource;
import org.jau.lang.InterruptedRuntimeException;

/**
 * Helper class to provide a Runnable queue implementation with a Runnable wrapper
 * which notifies after execution for the <code>invokeAndWait()</code> semantics.
 */
public class RunnableTask extends TaskBase {
    protected final Runnable runnable;

    /**
     * Invokes <code>runnable</code> on the current {@link Thread}.
     * @param runnable the {@link Runnable} to execute on the current thread.
     *                 The runnable <b>must exit</b>, i.e. not loop forever.
     * @return the newly created and invoked {@link RunnableTask}
     * @since 2.4.0
     */
    public static RunnableTask invokeOnCurrentThread(final Runnable runnable) {
        final RunnableTask rt = new RunnableTask( runnable, null, false, null );
        rt.run();
        return rt;
    }

    /**
     * Invokes <code>runnable</code> on a new {@link InterruptSource.Thread},
     * see {@link InterruptSource.Thread#Thread(ThreadGroup, Runnable, String)} for details.
     * @param tg the {@link ThreadGroup} for the new thread, maybe <code>null</code>
     * @param threadName the name for the new thread, maybe <code>null</code>
     * @param waitUntilDone if <code>true</code>, waits until <code>runnable</code> execution is completed, otherwise returns immediately.
     * @param runnable the {@link Runnable} to execute on the new thread. If <code>waitUntilDone</code> is <code>true</code>,
     *                 the runnable <b>must exit</b>, i.e. not loop forever.
     * @return the newly created and invoked {@link RunnableTask}
     * @since 2.3.2
     */
    public static RunnableTask invokeOnNewThread(final ThreadGroup tg, final String threadName,
                                                 final boolean waitUntilDone, final Runnable runnable) {
        final RunnableTask rt;
        if( !waitUntilDone ) {
            rt = new RunnableTask( runnable, null, true, System.err );
            final InterruptSource.Thread t = InterruptSource.Thread.create(tg, rt, threadName);
            t.start();
        } else {
            final Object sync = new Object();
            rt = new RunnableTask( runnable, sync, true, null );
            final InterruptSource.Thread t = InterruptSource.Thread.create(tg, rt, threadName);
            synchronized(sync) {
                t.start();
                while( rt.isInQueue() ) {
                    try {
                        sync.wait();
                    } catch (final InterruptedException ie) {
                        throw new InterruptedRuntimeException(ie);
                    }
                    final Throwable throwable = rt.getThrowable();
                    if(null!=throwable) {
                        throw new RuntimeException(throwable);
                    }
                }
            }
        }
        return rt;
    }

    /**
     * Create a RunnableTask object w/ synchronization,
     * ie. suitable for <code>invokeAndWait()</code>, i.e. {@link #invoke(boolean, Runnable) invoke(true, runnable)}.
     *
     * @param runnable The user action
     * @param syncObject The synchronization object if caller wait until <code>runnable</code> execution is completed,
     *                   or <code>null</code> if waiting is not desired.
     * @param catchExceptions Influence an occurring exception during <code>runnable</code> execution.
     *                        If <code>true</code>, the exception is silenced and can be retrieved via {@link #getThrowable()},
     *                        otherwise the exception is thrown.
     * @param exceptionOut If not <code>null</code>, exceptions are written to this {@link PrintStream}.
     */
    public RunnableTask(final Runnable runnable, final Object syncObject, final boolean catchExceptions, final PrintStream exceptionOut) {
        super(syncObject, catchExceptions, exceptionOut);
        this.runnable = runnable ;
    }

    /** Return the user action */
    public final Runnable getRunnable() {
        return runnable;
    }

    @Override
    public final void run() {
        execThread = Thread.currentThread();

        runnableException = null;
        tStarted = System.currentTimeMillis();
        if(null == syncObject) {
            try {
                runnable.run();
            } catch (final Throwable t) {
                runnableException = t;
                if(null != exceptionOut) {
                    exceptionOut.println("RunnableTask.run(): "+getExceptionOutIntro()+" exception occured on thread "+Thread.currentThread().getName()+": "+toString());
                    printSourceTrace();
                    runnableException.printStackTrace(exceptionOut);
                }
                if(!catchExceptions) {
                    throw new RuntimeException(runnableException);
                }
            } finally {
                tExecuted = System.currentTimeMillis();
                isExecuted = true;
            }
        } else {
            synchronized (syncObject) {
                try {
                    runnable.run();
                } catch (final Throwable t) {
                    runnableException = t;
                    if(null != exceptionOut) {
                        exceptionOut.println("RunnableTask.run(): "+getExceptionOutIntro()+" exception occured on thread "+Thread.currentThread().getName()+": "+toString());
                        printSourceTrace();
                        t.printStackTrace(exceptionOut);
                    }
                    if(!catchExceptions) {
                        throw new RuntimeException(runnableException);
                    }
                } finally {
                    tExecuted = System.currentTimeMillis();
                    isExecuted = true;
                    syncObject.notifyAll();
                }
            }
        }
    }
}

