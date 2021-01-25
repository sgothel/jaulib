/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2012 Gothel Software e.K.
 * Copyright (c) 2012 JogAmp Community.
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

public interface RunnableExecutor {
    /** This {@link RunnableExecutor} implementation simply invokes {@link Runnable#run()}
     *  on the current thread.
     */
    public static final RunnableExecutor currentThreadExecutor = new CurrentThreadExecutor();

    /**
     * @param wait if true method waits until {@link Runnable#run()} is completed, otherwise don't wait.
     * @param r the {@link Runnable} to be executed.
     */
    void invoke(boolean wait, Runnable r);

    static class CurrentThreadExecutor implements RunnableExecutor {
        private CurrentThreadExecutor() {}

        @Override
        public void invoke(final boolean wait, final Runnable r) {
            r.run();
        }
    }
}
