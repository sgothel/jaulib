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

package jau.test.util.parallel;

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;

import org.jau.lang.InterruptSource;
import org.jau.util.parallel.RunnableTask;
import org.junit.Assert;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

import jau.test.junit.util.JunitTracer;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestRunnableTask01 extends JunitTracer {

    @Test
    public void testInvokeAndWait00() throws IOException, InterruptedException, InvocationTargetException {
        final Object syncObject = new Object();
        final boolean[] done = {false};
        final Runnable clientAction = new Runnable() {
          @Override
        public void run() {
            synchronized(syncObject) {
                System.err.println("CA.1: "+syncObject);
                done[ 0 ] = true;
                System.err.println("CA.X");
                syncObject.notifyAll();
            }
          }
        };

        System.err.println("BB.0: "+syncObject);
        synchronized (syncObject) {
            System.err.println("BB.1: "+syncObject);
            new InterruptSource.Thread(null, clientAction, Thread.currentThread().getName()+"-clientAction").start();
            try {
                System.err.println("BB.2");
                syncObject.wait();
                System.err.println("BB.3");
            } catch (final InterruptedException e) {
                throw new RuntimeException(e);
            }
            Assert.assertTrue(done[0]);
            System.err.println("BB.X");
        }
    }

    @Test
    public void testInvokeAndWait01() throws IOException, InterruptedException, InvocationTargetException {
        final boolean[] done = {false};
        final Runnable clientAction = new Runnable() {
          @Override
        public void run() {
            System.err.println("CA.1");
            done[ 0 ] = true;
            System.err.println("CA.X");
          }
        };

        final RunnableTask rTask = new RunnableTask(clientAction, new Object(), false, null);
        System.err.println("BB.0: "+rTask.getSyncObject());
        synchronized (rTask.getSyncObject()) {
            System.err.println("BB.1: "+rTask.getSyncObject());
            new InterruptSource.Thread(null, rTask, Thread.currentThread().getName()+"-clientAction").start();
            try {
                System.err.println("BB.2");
                rTask.getSyncObject().wait();
                System.err.println("BB.3");
            } catch (final InterruptedException e) {
                throw new RuntimeException(e);
            }
            Assert.assertTrue(done[0]);
            System.err.println("BB.X");
        }
    }

    public static void main(final String args[]) throws IOException {
        final String tstname = TestRunnableTask01.class.getName();
        org.junit.runner.JUnitCore.main(tstname);
    }

}
