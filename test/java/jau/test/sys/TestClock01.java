/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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

package jau.test.sys;

import java.time.Instant;
import java.time.ZoneOffset;
import java.time.temporal.ChronoUnit;

import org.jau.io.PrintUtil;
import org.jau.sys.Clock;
import org.junit.Assert;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

import jau.pkg.PlatformRuntime;
import jau.test.junit.util.JunitTracer;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestClock01 extends JunitTracer {

    @Test
    public void test00()  {
        PlatformRuntime.checkInitialized();
        PrintUtil.fprintf_td(System.err, "test00\n");

        final Instant m_t0 = Instant.ofEpochMilli(Clock.currentTimeMillis());
        final Instant m_t1 = Clock.getMonotonicTime();
        final Instant w_t2 = Instant.ofEpochSecond(Clock.wallClockSeconds());
        final Instant w_t3 = Clock.getWallClockTime();

        try {
            Thread.sleep(100);
        } catch (final InterruptedException e) { }

        final Instant m_t0_b = Instant.ofEpochMilli(Clock.currentTimeMillis());
        final Instant m_t1_b = Clock.getMonotonicTime();
        final Instant w_t2_b = Instant.ofEpochSecond(Clock.wallClockSeconds());
        final Instant w_t3_b = Clock.getWallClockTime();

        final long m_t0_d = m_t0.until(m_t0_b, ChronoUnit.MILLIS);
        final long m_t1_d = m_t1.until(m_t1_b, ChronoUnit.MILLIS);
        final long w_t2_d = w_t2.until(w_t2_b, ChronoUnit.MILLIS);
        final long w_t3_d = w_t3.until(w_t3_b, ChronoUnit.MILLIS);

        PrintUtil.fprintf_td(System.err, "mono t0 %s, %d ms\n", m_t0.atZone(ZoneOffset.UTC), m_t0_d);
        PrintUtil.fprintf_td(System.err, "mono t1 %s, %d ms\n", m_t1.atZone(ZoneOffset.UTC), m_t1_d);
        PrintUtil.fprintf_td(System.err, "wall t2 %s, %d ms\n", w_t2.atZone(ZoneOffset.UTC), w_t2_d);
        PrintUtil.fprintf_td(System.err, "wall t3 %s, %d ms\n", w_t3.atZone(ZoneOffset.UTC), w_t3_d);

        final long td_min = 50;
        final long td_max = 150;
        Assert.assertTrue( td_min <= m_t0_d );
        Assert.assertTrue( td_max >= m_t0_d );

        Assert.assertTrue( td_min <= m_t1_d );
        Assert.assertTrue( td_max >= m_t1_d );

        Assert.assertTrue( 0      <= w_t2_d ); // sec granularity only
        Assert.assertTrue( td_max >= w_t2_d );

        Assert.assertTrue( td_min <= w_t3_d );
        Assert.assertTrue( td_max >= w_t3_d );
    }

    public static void main(final String args[]) {
        final String tstname = TestClock01.class.getName();
        org.junit.runner.JUnitCore.main(tstname);
    }

}
