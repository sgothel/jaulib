/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2014 Gothel Software e.K.
 * Copyright (c) 2014 JogAmp Community.
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
package org.jau.util;

/**
 * Generic function interface to perform an action w/ given optional arguments
 * producing an optional result.
 * <p>
 * For <code>void</code> functions, simply use type <code>Object</code>
 * and ignore the result and/or arguments.
 * </p>
 *
 * @param <T> the result type of {@link #eval(Object...)}
 */
public interface Function<R,A> {
    /**
     * Implementation may compute variable <code>args</code> list
     * and returns a result.
     *
     * @param args variable argument list, <code>A[]</code>, maybe null
     * @return the result.
     */
    R eval(A... args);
}
