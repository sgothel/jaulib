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

import org.jau.sys.Debug;
import org.jau.sys.PropertyAccess;

/**
 * Helper class to provide a Runnable queue implementation with a Runnable wrapper
 * which notifies after execution for the <code>invokeAndWait()</code> semantics.
 */
public abstract class TaskBase implements Runnable {
    /** Enable via the property <code>jogamp.debug.TaskBase.TraceSource</code> */
    private static final boolean TRACE_SOURCE;

    static {
        Debug.initSingleton();
        TRACE_SOURCE = PropertyAccess.isPropertyDefined("jau.debug.TaskBase.TraceSource", true);
    }

    protected final Object syncObject;
    protected final boolean catchExceptions;
    protected final PrintStream exceptionOut;
    protected final Throwable sourceStack;

    protected Object attachment;
    protected Throwable runnableException;
    protected long tCreated, tStarted;
    protected volatile long tExecuted;
    protected volatile boolean isExecuted;
    protected volatile boolean isFlushed;
    protected volatile Thread execThread;

    /**
     * @param syncObject The synchronization object if caller wait until <code>runnable</code> execution is completed,
     *                   or <code>null</code> if waiting is not desired.
     * @param catchExceptions Influence an occurring exception during <code>runnable</code> execution.
     *                        If <code>true</code>, the exception is silenced and can be retrieved via {@link #getThrowable()},
     *                        otherwise the exception is thrown.
     * @param exceptionOut If not <code>null</code>, exceptions are written to this {@link PrintStream}.
     */
    protected TaskBase(final Object syncObject, final boolean catchExceptions, final PrintStream exceptionOut) {
        this.syncObject = syncObject;
        this.catchExceptions = catchExceptions;
        this.exceptionOut = exceptionOut;
        this.sourceStack = TRACE_SOURCE ? new Throwable("Creation @") : null;
        this.tCreated = System.currentTimeMillis();
        this.tStarted = 0;
        this.tExecuted = 0;
        this.isExecuted = false;
        this.isFlushed = false;
        this.execThread = null;
    }

    protected final String getExceptionOutIntro() {
        return catchExceptions ? "A caught" : "An uncaught";
    }
    protected final void printSourceTrace() {
        if( null != sourceStack && null != exceptionOut ) {
            sourceStack.printStackTrace(exceptionOut);
        }
    }

    /**
     * Returns the execution thread or {@code null} if not yet {@link #run()}.
     * @since 0.3.0
     */
    public final Thread getExecutionThread() {
        return execThread;
    }

    /**
     * Return the synchronization object if any.
     * @see #RunnableTask(Runnable, Object, boolean)
     */
    public final Object getSyncObject() {
        return syncObject;
    }

    /**
     * Attach a custom object to this task.
     * Useful to piggybag further information, ie tag a task final.
     */
    public final void setAttachment(final Object o) {
        attachment = o;
    }

    /**
     * Return the attachment object if any.
     * @see #setAttachment(Object)
     */
    public final Object getAttachment() {
        return attachment;
    }

    @Override
    public abstract void run();

    /**
     * Simply flush this task and notify a waiting executor.
     * The executor which might have been blocked until notified
     * will be unblocked and the task removed from the queue.
     *
     * @param t optional Throwable to be assigned for later {@link #getThrowable()} query in case of an error.
     *
     * @see #isFlushed()
     * @see #isInQueue()
     */
    public final void flush(final Throwable t) {
        if(!isExecuted() && hasWaiter()) {
            runnableException = t;
            synchronized (syncObject) {
                isFlushed = true;
                syncObject.notifyAll();
            }
        }
    }

    /**
     * @return !{@link #isExecuted()} && !{@link #isFlushed()}
     */
    public final boolean isInQueue() { return !isExecuted && !isFlushed; }

    /**
     * @return True if executed, otherwise false;
     */
    public final boolean isExecuted() { return isExecuted; }

    /**
     * @return True if flushed, otherwise false;
     */
    public final boolean isFlushed() { return isFlushed; }

    /**
     * @return True if invoking thread waits until done,
     *         ie a <code>notifyObject</code> was passed, otherwise false;
     */
    public final boolean hasWaiter() { return null != syncObject; }

    /**
     * @return A thrown exception while execution of the user action, if any and if caught
     * @see #RunnableTask(Runnable, Object, boolean)
     */
    public final Throwable getThrowable() { return runnableException; }

    public final long getTimestampCreate() { return tCreated; }
    public final long getTimestampBeforeExec() { return tStarted; }
    public final long getTimestampAfterExec() { return tExecuted; }
    public final long getDurationInQueue() { return tStarted - tCreated; }
    public final long getDurationInExec() { return 0 < tExecuted ? tExecuted - tStarted : 0; }
    public final long getDurationTotal() { return 0 < tExecuted ? tExecuted - tCreated : tStarted - tCreated; }

    @Override
    public String toString() {
        final String etn;
        final String eth;
        if( null != execThread ) {
            etn = execThread.getName();
            eth = "0x"+Integer.toHexString(execThread.hashCode());
        } else {
            etn = "n/a";
            eth = "n/a";
        }
        return "RunnableTask[enqueued "+isInQueue()+"[executed "+isExecuted()+", flushed "+isFlushed()+", thread["+eth+", "+etn+"]], tTotal "+getDurationTotal()+" ms, tExec "+getDurationInExec()+" ms, tQueue "+getDurationInQueue()+" ms, attachment "+attachment+", throwable "+getThrowable()+"]";
    }
}

