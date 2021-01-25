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

package org.jau.junit.util;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import org.junit.Assume;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.FixMethodOrder;
import org.junit.Rule;
import org.junit.rules.TestName;
import org.junit.runners.MethodSorters;


@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public abstract class JunitTracer {
    @Rule public final TestName _unitTestName = new TestName();

    static volatile boolean testSupported = true;

    public static final boolean isTestSupported() {
        return testSupported;
    }

    public static final void setTestSupported(final boolean v) {
        System.err.println("setTestSupported: "+v);
        testSupported = v;
    }

    public final String getTestMethodName() {
        return _unitTestName.getMethodName();
    }

    public final String getSimpleTestName(final String separator) {
        return getClass().getSimpleName()+separator+getTestMethodName();
    }

    public final String getFullTestName(final String separator) {
        return getClass().getName()+separator+getTestMethodName();
    }

    @BeforeClass
    public static final void oneTimeSetUpBase() {
        // one-time initialization code
    }

    @AfterClass
    public static final void oneTimeTearDownBase() {
        // one-time cleanup code
        System.gc(); // force cleanup
    }

    @Before
    public final void setUpBase() {
        System.err.print("++++ TestCase.setUp: "+getFullTestName(" - "));
        if(!testSupported) {
            System.err.println(" - "+unsupportedTestMsg);
            Assume.assumeTrue(testSupported);
        }
        System.err.println();
    }

    @After
    public final void tearDownBase() {
        System.err.println("++++ TestCase.tearDown: "+getFullTestName(" - "));
    }

    static final String unsupportedTestMsg = "Test not supported on this platform.";

    public static void waitForKey(final String preMessage) {
        final BufferedReader stdin = new BufferedReader(new InputStreamReader(System.in));
        System.err.println(preMessage+"> Press enter to continue");
        try {
            System.err.println(stdin.readLine());
        } catch (final IOException e) { e.printStackTrace(); }
    }
}

