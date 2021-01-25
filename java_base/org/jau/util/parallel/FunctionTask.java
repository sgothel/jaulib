/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2013 Gothel Software e.K.
 * Copyright (c) 2013 JogAmp Community.
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
import org.jau.util.Function;

/**
 * Helper class to provide a Runnable queue implementation with a Runnable wrapper
 * which notifies after execution for the <code>invokeAndWait()</code> semantics.
 */
public class FunctionTask<R,A> extends TaskBase implements Function<R,A> {
    protected Function<R,A> runnable;
    protected R result;
    protected A[] args;

    /**
     * Invokes <code>func</code> on the current {@link Thread}.
     * <p>
     * The result can be retrieved via {@link FunctionTask#getResult()},
     * using the returned instance.
     * </p>
     * @param func the {@link Function} to execute.
     * @param args the {@link Function} arguments
     * @return the newly created and invoked {@link FunctionTask}
     * @since 2.4.0
     */
    public static <U,V> FunctionTask<U,V> invokeOnCurrentThread(final Function<U,V> func, final V... args) {
        final FunctionTask<U,V> rt = new FunctionTask<U,V>( func, null, false, null);
        rt.args = args;
        rt.run();
        return rt;
    }

    /**
     * Invokes <code>func</code> on a new {@link InterruptSource.Thread},
     * see {@link InterruptSource.Thread#Thread(ThreadGroup, Runnable, String)} for details.
     * <p>
     * The result can be retrieved via {@link FunctionTask#getResult()},
     * using the returned instance.
     * </p>
     * @param tg the {@link ThreadGroup} for the new thread, maybe <code>null</code>
     * @param threadName the name for the new thread, maybe <code>null</code>
     * @param waitUntilDone if <code>true</code>, waits until <code>func</code> execution is completed, otherwise returns immediately.
     * @param func the {@link Function} to execute.
     * @param args the {@link Function} arguments
     * @return the newly created and invoked {@link FunctionTask}
     * @since 2.3.2
     */
    public static <U,V> FunctionTask<U,V> invokeOnNewThread(final ThreadGroup tg, final String threadName,
                                                            final boolean waitUntilDone, final Function<U,V> func, final V... args) {
        final FunctionTask<U,V> rt;
        if( !waitUntilDone ) {
            rt = new FunctionTask<U,V>( func, null, true, System.err );
            final InterruptSource.Thread t = InterruptSource.Thread.create(tg, rt, threadName);
            rt.args = args;
            t.start();
        } else {
            final Object sync = new Object();
            rt = new FunctionTask<U,V>( func, sync, true, null );
            final InterruptSource.Thread t = InterruptSource.Thread.create(tg, rt, threadName);
            synchronized(sync) {
                rt.args = args;
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
     * ie. suitable for <code>invokeAndWait()</code>.
     *
     * @param runnable the user action
     * @param syncObject the synchronization object the caller shall wait until <code>runnable</code> execution is completed,
     *                   or <code>null</code> if waiting is not desired.
     * @param catchExceptions Influence an occurring exception during <code>runnable</code> execution.
     *                        If <code>true</code>, the exception is silenced and can be retrieved via {@link #getThrowable()},
     *                        otherwise the exception is thrown.
     * @param exceptionOut If not <code>null</code>, exceptions are written to this {@link PrintStream}.
     */
    public FunctionTask(final Function<R,A> runnable, final Object syncObject, final boolean catchExceptions, final PrintStream exceptionOut) {
        super(syncObject, catchExceptions, exceptionOut);
        this.runnable = runnable ;
        result = null;
        args = null;
    }

    /** Return the user action */
    public final Function<R,A> getRunnable() {
        return runnable;
    }

    /**
     * Sets the arguments for {@link #run()}.
     * They will be cleared after calling {@link #run()} or {@link #eval(Object...)}.
     */
    public final void setArgs(final A... args) {
        this.args = args;
    }

    /**
     * Retrieves the cached result of {@link #run()}
     * and is cleared within this method.
     */
    public final R getResult() {
        final R res = result;
        result = null;
        return res;
    }

    /**
     * {@inheritDoc}
     * <p>
     * Calls {@link #eval(Object...)}.
     * </p>
     * <p>
     * You may set the {@link #eval(Object...)} arguments via {@link #setArgs(Object...)}
     * and retrieve the result via {@link #getResult()}.
     * </p>
     */
    @Override
    public final void run() {
        execThread = Thread.currentThread();

        final A[] args = this.args;
        this.args = null;
        this.result = null;
        runnableException = null;
        tStarted = System.currentTimeMillis();
        if(null == syncObject) {
            try {
                this.result = runnable.eval(args);
            } catch (final Throwable t) {
                runnableException = t;
                if(null != exceptionOut) {
                    exceptionOut.println("FunctionTask.run(): "+getExceptionOutIntro()+" exception occured on thread "+Thread.currentThread().getName()+": "+toString());
                    printSourceTrace();
                    t.printStackTrace(exceptionOut);
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
                    this.result = runnable.eval(args);
                } catch (final Throwable t) {
                    runnableException = t;
                    if(null != exceptionOut) {
                        exceptionOut.println("FunctionTask.run(): "+getExceptionOutIntro()+" exception occured on thread "+Thread.currentThread().getName()+": "+toString());
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

    @Override
    public final R eval(final A... args) {
        this.args = args;
        run();
        final R res = result;
        result = null;
        return res;
    }
}

