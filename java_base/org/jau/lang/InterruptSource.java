/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2015 Gothel Software e.K.
 * Copyright (c) 2015 JogAmp Community.
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

package org.jau.lang;

/**
 * Interface exposing {@link java.lang.Thread#interrupt()} source,
 * intended for {@link java.lang.Thread} specializations.
 * @since 0.3.0
 */
public interface InterruptSource {
    /**
     * Returns the source of the last {@link #interrupt()} call.
     * @param clear if true, issues {@link #clearInterruptSource()}
     */
    Throwable getInterruptSource(final boolean clear);

    /**
     * Returns the count of {@link java.lang.Thread#interrupt()} calls.
     * @param clear if true, issues {@link #clearInterruptSource()}
     */
    int getInterruptCounter(final boolean clear);

    /**
     * Clears source and count of {@link java.lang.Thread#interrupt()} calls, if any.
     */
    void clearInterruptSource();

    public static class Util {
        /**
         * Casts given {@link java.lang.Thread} to {@link InterruptSource}
         * if applicable, otherwise returns {@code null}.
         */
        public static InterruptSource get(final java.lang.Thread t) {
            if(t instanceof InterruptSource) {
                return (InterruptSource)t;
            } else {
                return null;
            }
        }
        /**
         * Casts current {@link java.lang.Thread} to {@link InterruptSource}
         * if applicable, otherwise returns {@code null}.
         */
        public static InterruptSource currentThread() {
            return get(java.lang.Thread.currentThread());
        }
    }

    /**
     * {@link java.lang.Thread} specialization implementing {@link InterruptSource}
     * to track {@link java.lang.Thread#interrupt()} calls.
     * @since 0.3.0
     */
    public static class Thread extends java.lang.Thread implements InterruptSource {
        volatile Throwable interruptSource = null;
        volatile int interruptCounter = 0;
        final Object sync = new Object();

        /**
         * See {@link Thread#Thread(} for details.
         */
        public Thread() {
            super();
        }
        /**
         * See {@link Thread#Thread(ThreadGroup, Runnable)} for details.
         * @param tg explicit {@link ThreadGroup}, may be {@code null}
         * @param target explicit {@link Runnable}, may be {@code null}
         */
        public Thread(final ThreadGroup tg, final Runnable target) {
            super(tg, target);
        }
        /**
         * See {@link Thread#Thread(ThreadGroup, Runnable, String)} for details.
         * @param tg explicit {@link ThreadGroup}, may be {@code null}
         * @param target explicit {@link Runnable}, may be {@code null}
         * @param name explicit name of thread, must not be {@code null}
         */
        public Thread(final ThreadGroup tg, final Runnable target, final String name) {
            super(tg, target, name);
        }

        /**
         * Depending on whether {@code name} is null, either
         * {@link #Thread(ThreadGroup, Runnable, String)} or
         * {@link #Thread(ThreadGroup, Runnable)} is being utilized.
         * @param tg explicit {@link ThreadGroup}, may be {@code null}
         * @param target explicit {@link Runnable}, may be {@code null}
         * @param name explicit name of thread, may be {@code null}
         */
        public static Thread create(final ThreadGroup tg, final Runnable target, final String name) {
            return null != name ? new Thread(tg, target, name) : new Thread(tg, target);
        }

        @Override
        public final Throwable getInterruptSource(final boolean clear) {
            synchronized(sync) {
                final Throwable r = interruptSource;
                if( clear ) {
                    clearInterruptSource();
                }
                return r;
            }
        }
        @Override
        public final int getInterruptCounter(final boolean clear) {
            synchronized(sync) {
                final int r = interruptCounter;
                if( clear ) {
                    clearInterruptSource();
                }
                return r;
            }
        }
        @Override
        public final void clearInterruptSource() {
            synchronized(sync) {
                interruptCounter = 0;
                interruptSource = null;
            }
        }
        @Override
        public final void interrupt() {
            synchronized(sync) {
                interruptCounter++;
                interruptSource = new Throwable(getName()+".interrupt() #"+interruptCounter);
            }
            super.interrupt();
        }
    }
}
