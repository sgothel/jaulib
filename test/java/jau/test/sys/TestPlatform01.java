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

package jau.test.sys;

import org.jau.sys.PlatformProps;
import org.jau.sys.MachineDataInfo;
import org.jau.sys.RuntimeProps;
import org.junit.Assert;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

import jau.test.junit.util.JunitTracer;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestPlatform01 extends JunitTracer {

    @Test
    public void testInfo00()  {
        System.err.println();
        System.err.print(PlatformProps.NEWLINE);
        System.err.println("OS name/type: "+PlatformProps.os_name+", "+PlatformProps.OS);
        System.err.println("OS version: "+PlatformProps.os_version);
        System.err.println();
        System.err.println("Arch, CPU: "+PlatformProps.os_arch+", "+PlatformProps.CPU+"/"+PlatformProps.CPU.family);
        System.err.println("OS/Arch: "+PlatformProps.os_and_arch);
        System.err.println();
        System.err.println("Java runtime: "+PlatformProps.JAVA_RUNTIME_NAME);
        System.err.println("Java version, vm: "+PlatformProps.JAVA_VERSION_NUMBER);
        System.err.println();
        System.err.println("MD.ST: "+PlatformProps.MACH_DESC_STAT);
        System.err.println("MD.RT: "+RuntimeProps.MACH_DESC_RT);
        System.err.println();
        System.err.println();
    }

    @Test
    public void testPageSize01()  {
        final MachineDataInfo machine = PlatformProps.MACH_DESC_STAT;
        final int ps = machine.pageSizeInBytes();
        System.err.println("PageSize: "+ps);
        Assert.assertTrue("PageSize is 0", 0 < ps );

        final int ps_pages = machine.pageCount(ps);
        Assert.assertTrue("PageNumber of PageSize is not 1, but "+ps_pages, 1 == ps_pages);

        final int sz0 = ps - 10;
        final int sz0_pages = machine.pageCount(sz0);
        Assert.assertTrue("PageNumber of PageSize-10 is not 1, but "+sz0_pages, 1 == sz0_pages);

        final int sz1 = ps + 10;
        final int sz1_pages = machine.pageCount(sz1);
        Assert.assertTrue("PageNumber of PageSize+10 is not 2, but "+sz1_pages, 2 == sz1_pages);

        final int ps_psa = machine.pageAlignedSize(ps);
        Assert.assertTrue("PageAlignedSize of PageSize is not PageSize, but "+ps_psa, ps == ps_psa);

        final int sz0_psa = machine.pageAlignedSize(sz0);
        Assert.assertTrue("PageAlignedSize of PageSize-10 is not PageSize, but "+sz0_psa, ps == sz0_psa);

        final int sz1_psa = machine.pageAlignedSize(sz1);
        Assert.assertTrue("PageAlignedSize of PageSize+10 is not 2*PageSize, but "+sz1_psa, ps*2 == sz1_psa);
    }

    public static void main(final String args[]) {
        final String tstname = TestPlatform01.class.getName();
        org.junit.runner.JUnitCore.main(tstname);
    }

}
