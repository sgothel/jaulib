/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
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

public class Hash64 {

    /**
     * <ul>
     * <li>31 - the traditional 'Java' prime for {@code 31 * x == (x << 5) - x}</li>
     * <li>92821 - result of Hans-Peter Stoerr's finding</li>
     * </ul>
     * @param prime
     * @param a
     * @return
     */
    public static long hash(final long prime, final byte a[]) {
        if (a == null) {
            return 0;
        }
        long h = 1;
        for (int i=0; i<a.length; i++) {
            h = prime * h + a[i];
        }
        return h;
    }

    public static long hash(final long prime, final int a, final int b) {
        final long h = prime + a;
        return prime * h + b;
    }

    public static long hash(final long prime, final long a, final long b) {
        final long h = prime + a;
        return prime * h + b;
    }

    /**
     * Generates a 64bit equally distributed hash value
     * from <code>addr</code> and <code>size</code> avoiding XOR collisions.
     */
    public static long hash31(final long a, final long b) {
        // 31 * x == (x << 5) - x
        final long hash = 31 + a;
        return ((hash << 5) - hash) + b;
    }
}
