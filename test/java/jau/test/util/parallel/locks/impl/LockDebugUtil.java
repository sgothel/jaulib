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

package jau.test.util.parallel.locks.impl;

import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import jau.test.util.parallel.locks.Lock;

/**
 * Functionality enabled if {@link Lock#DEBUG} is <code>true</code>.
 */
public class LockDebugUtil {
    private static final ThreadLocal<ArrayList<Throwable>> tlsLockedStacks;
    private static final List<Throwable> dummy;
    static {
        if(Lock.DEBUG) {
            tlsLockedStacks = new ThreadLocal<ArrayList<Throwable>>();
            dummy = null;
        } else {
            tlsLockedStacks = null;
            dummy = new ArrayList<Throwable>(0);
        }
    }

    public static List<Throwable> getRecursiveLockTrace() {
        if(Lock.DEBUG) {
            ArrayList<Throwable> ls = tlsLockedStacks.get();
            if(null == ls) {
                ls = new ArrayList<Throwable>();
                tlsLockedStacks.set(ls);
            }
            return ls;
        } else {
            return dummy;
        }
    }

    public static void dumpRecursiveLockTrace(final PrintStream out) {
        if(Lock.DEBUG) {
            final List<Throwable> ls = getRecursiveLockTrace();
            if(null!=ls && ls.size()>0) {
                int j=0;
                out.println("TLSLockedStacks: locks "+ls.size());
                for(final Iterator<Throwable> i=ls.iterator(); i.hasNext(); j++) {
                    out.print(j+": ");
                    i.next().printStackTrace(out);
                }
            }
        }
    }
}
