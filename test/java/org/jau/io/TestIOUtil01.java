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

package org.jau.io;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.URISyntaxException;
import java.net.URLConnection;
import java.nio.ByteBuffer;
import java.util.Arrays;

import org.jau.junit.util.JunitTracer;
import org.jau.lang.ExceptionUtils;
import org.jau.sys.MachineDataInfo;
import org.jau.sys.PlatformProps;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestIOUtil01 extends JunitTracer {

    static final MachineDataInfo machine = PlatformProps.MACH_DESC_STAT;
    static final int tsz = machine.pageSizeInBytes() + machine.pageSizeInBytes() / 2 ;
    static final byte[] orig = new byte[tsz];
    static final String tfilename = "./test.bin" ;

    @BeforeClass
    public static void setup() throws IOException {
        final File tfile = new File(tfilename);
        tfile.deleteOnExit();
        final OutputStream tout = new BufferedOutputStream(new FileOutputStream(tfile));
        for(int i=0; i<tsz; i++) {
            final byte b = (byte) (i%256);
            orig[i] = b;
            tout.write(b);
        }
        tout.close();
    }

    @AfterClass
    public static void cleanup() {
        final File tfile = new File(tfilename);
        tfile.delete();
    }

    @Test
    public void test01CleanPathString() throws IOException, URISyntaxException {
        {
            final String input    = "./dummy/nop/../a.txt";
            final String expected = "dummy/a.txt";
            Assert.assertEquals(expected, IOUtil.cleanPathString(input));
        }
        {
            final String input    = "../dummy/nop/../a.txt";
            final String expected = "../dummy/a.txt";
            Assert.assertEquals(expected, IOUtil.cleanPathString(input));
        }
        {
            final String input    = ".././dummy/nop/../a.txt";
            final String expected = "../dummy/a.txt";
            Assert.assertEquals(expected, IOUtil.cleanPathString(input));
        }
        {
            final String input    = "./../dummy/nop/../a.txt";
            final String expected = "../dummy/a.txt";
            Assert.assertEquals(expected, IOUtil.cleanPathString(input));
        }
        {
            final String input    = "../dummy/./nop/../a.txt";
            final String expected = "../dummy/a.txt";
            Assert.assertEquals(expected, IOUtil.cleanPathString(input));
        }
        {
            final String input    = "/dummy/nop/./../a.txt";
            final String expected = "/dummy/a.txt";
            Assert.assertEquals(expected, IOUtil.cleanPathString(input));
        }
        {
            final String input    = "dummy/../nop/./.././aaa/bbb/../../a.txt";
            final String expected = "a.txt";
            Assert.assertEquals(expected, IOUtil.cleanPathString(input));
        }
        {
            final String input    = "/dummy/../nop/./.././aaa/bbb/././ccc/../../../a.txt";
            final String expected = "/a.txt";
            Assert.assertEquals(expected, IOUtil.cleanPathString(input));
        }
        {
            URISyntaxException use = null;
            try {
                // Error case!
                final String input    = "../../error.txt";
                final String expected = "error.txt";
                final String result = IOUtil.cleanPathString(input); // URISyntaxException
                System.err.println("input   : "+input);
                System.err.println("expected: "+expected);
                System.err.println("result  : "+result);
                Assert.assertEquals(expected, result);
            } catch (final URISyntaxException _use) {
                use = _use;
                ExceptionUtils.dumpThrowable("", _use, 0, 3);
            }
            Assert.assertNotNull("URISyntaxException expected", use);
        }
        {
            URISyntaxException use = null;
            try {
                // Error case!
                final String input    = ".././a/../../error.txt";
                final String expected = "error.txt";
                final String result = IOUtil.cleanPathString(input); // URISyntaxException
                System.err.println("input   : "+input);
                System.err.println("expected: "+expected);
                System.err.println("result  : "+result);
                Assert.assertEquals(expected, result);
            } catch (final URISyntaxException _use) {
                use = _use;
                ExceptionUtils.dumpThrowable("", _use, 0, 3);
            }
            Assert.assertNotNull("URISyntaxException expected", use);
        }
    }

    @Test
    public void test11CopyStream01Array() throws IOException {
        final URLConnection urlConn = IOUtil.getResource(tfilename, this.getClass().getClassLoader(), this.getClass());
        Assert.assertNotNull(urlConn);
        final BufferedInputStream bis = new BufferedInputStream( urlConn.getInputStream() );
        final byte[] bb;
        try {
            bb = IOUtil.copyStream2ByteArray( bis );
        } finally {
            IOUtil.close(bis, false);
        }
        Assert.assertEquals("Byte number not equal orig vs array", orig.length, bb.length);
        Assert.assertTrue("Bytes not equal orig vs array", Arrays.equals(orig, bb));

    }

    @Test
    public void test12CopyStream02Buffer() throws IOException {
        final URLConnection urlConn = IOUtil.getResource(tfilename, this.getClass().getClassLoader(), this.getClass());
        Assert.assertNotNull(urlConn);
        final BufferedInputStream bis = new BufferedInputStream( urlConn.getInputStream() );
        final ByteBuffer bb;
        try {
            bb = IOUtil.copyStream2ByteBuffer( bis );
        } finally {
            IOUtil.close(bis, false);
        }
        Assert.assertEquals("Byte number not equal orig vs buffer", orig.length, bb.limit());
        int i;
        for(i=tsz-1; i>=0 && orig[i]==bb.get(i); i--) ;
        Assert.assertTrue("Bytes not equal orig vs array", 0>i);
    }

    @Test
    public void test13CopyStream03Buffer() throws IOException {
        final String tfilename2 = "./test2.bin" ;
        final URLConnection urlConn1 = IOUtil.getResource(tfilename, this.getClass().getClassLoader(), this.getClass());
        Assert.assertNotNull(urlConn1);

        final File file2 = new File(tfilename2);
        file2.deleteOnExit();
        try {
            IOUtil.copyURLConn2File(urlConn1, file2);
            final URLConnection urlConn2 = IOUtil.getResource(tfilename2, this.getClass().getClassLoader(), this.getClass());
            Assert.assertNotNull(urlConn2);

            final BufferedInputStream bis = new BufferedInputStream( urlConn2.getInputStream() );
            final ByteBuffer bb;
            try {
                bb = IOUtil.copyStream2ByteBuffer( bis );
            } finally {
                IOUtil.close(bis, false);
            }
            Assert.assertEquals("Byte number not equal orig vs buffer", orig.length, bb.limit());
            int i;
            for(i=tsz-1; i>=0 && orig[i]==bb.get(i); i--) ;
            Assert.assertTrue("Bytes not equal orig vs array", 0>i);
        } finally {
            file2.delete();
        }
    }

    public static void main(final String args[]) throws IOException {
        final String tstname = TestIOUtil01.class.getName();
        org.junit.runner.JUnitCore.main(tstname);
    }

}
