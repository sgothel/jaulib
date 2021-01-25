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
package org.jau.util.parallel.locks;

import jau.util.parallel.locks.RecursiveLockImpl01CompleteFair;
import jau.util.parallel.locks.RecursiveLockImpl01Unfairish;
import jau.util.parallel.locks.RecursiveThreadGroupLockImpl01Unfairish;

public class LockFactory {

    public enum ImplType {
        Int01(0), Int02ThreadGroup(2);

        public final int id;

        ImplType(final int id){
            this.id = id;
        }
    }

    /** default is ImplType.Int01, unfair'ish (fastest w/ least deviation) */
    public static RecursiveLock createRecursiveLock() {
        return new RecursiveLockImpl01Unfairish();
    }

    /** default is ImplType.Int02ThreadGroup, unfair'ish (fastest w/ least deviation) */
    public static RecursiveThreadGroupLock createRecursiveThreadGroupLock() {
        return new RecursiveThreadGroupLockImpl01Unfairish();
    }

    public static RecursiveLock createRecursiveLock(final ImplType t, final boolean fair) {
        switch(t) {
            case Int01:
                return fair ? new RecursiveLockImpl01CompleteFair() : new RecursiveLockImpl01Unfairish();
            case Int02ThreadGroup:
                return new RecursiveThreadGroupLockImpl01Unfairish();
        }
        throw new InternalError("XXX");
    }

}
