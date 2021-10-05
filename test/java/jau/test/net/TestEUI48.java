/**
 * Copyright 2015 JogAmp Community. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice, this list of
 *       conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright notice, this list
 *       of conditions and the following disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY JogAmp Community ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL JogAmp Community OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 * or implied, of JogAmp Community.
 */

package jau.test.net;

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.nio.ByteOrder;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

import org.jau.net.EUI48;
import org.jau.net.EUI48Sub;

import jau.test.junit.util.JunitTracer;

import org.junit.Assert;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

/**
 * Test basic EUI48 functionality
 */
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestEUI48 extends JunitTracer {

    static void test_sub01(final ByteOrder byte_order, final String mac_str, final List<String> mac_sub_strs, final List<Integer> indices) {
        final EUI48 mac = new EUI48(mac_str);

        System.out.printf("Test EUI48 mac: '%s' -> '%s'\n", mac_str, mac.toString());
        Assert.assertEquals(mac_str, mac.toString());

        int i=0;
        for(final Iterator<String> iter=mac_sub_strs.iterator(); iter.hasNext(); ++i) {
            final String mac_sub_str = iter.next();
            final EUI48Sub mac_sub = new EUI48Sub(mac_sub_str);
            System.out.printf("EUI48Sub mac02_sub: '%s' -> '%s'\n", mac_sub_str, mac_sub.toString());
            // cut-off pre- and post-colon in test string, but leave single colon
            String sub_str = new String(mac_sub_str);
            if( sub_str.isEmpty() ) {
                sub_str = ":";
            } else if( !sub_str.equals(":") )  {
                if( sub_str.length() > 0 && sub_str.charAt(0) == ':' ) {
                    sub_str = sub_str.substring(1, sub_str.length());
                }
                if( sub_str.length() > 0 && sub_str.charAt(sub_str.length()-1) == ':' ) {
                    sub_str = sub_str.substring(0, sub_str.length()-1);
                }
            }
            Assert.assertEquals(sub_str, mac_sub.toString());

            final int idx = mac.indexOf(mac_sub, byte_order);
            Assert.assertEquals( idx, indices.get(i).intValue());
            if( idx >= 0 ) {
                Assert.assertTrue( mac.contains(mac_sub) );
            } else {
                Assert.assertFalse( mac.contains(mac_sub) );
            }
        }
    }
    static void test_sub02(final String mac_sub_str_exp, final String mac_sub_str, final boolean expected_result) {
        final StringBuilder errmsg = new StringBuilder();
        final EUI48Sub mac_sub = new EUI48Sub ();
        final boolean res = EUI48Sub.scanEUI48Sub(mac_sub_str, mac_sub, errmsg);
        if( res ) {
            System.out.printf("EUI48Sub mac_sub: '%s' -> '%s'\n", mac_sub_str, mac_sub.toString());
            if( expected_result ) {
                Assert.assertEquals(mac_sub_str_exp, mac_sub.toString());
            }
        } else {
            System.out.printf("EUI48Sub mac_sub: '%s' -> Error '%s'\n", mac_sub_str, errmsg.toString());
        }
        Assert.assertEquals(expected_result, res);
    }

    @Test
    public void test01_EUI48AndSub() {
        {
            // index                      [high=5 ...   low=0]
            final String mac02_str = "C0:10:22:A0:10:00";
            final String[] mac02_sub_strs =    { "C0", "C0:10", ":10:22", "10:22", ":10:22:", "10:22:", "10", "10:00", "00", ":", "", "00:10", mac02_str};
            final Integer[] mac02_sub_idxs_le = {  5,       4,        3,       3,         3,        3,    1,       0,    0,   0,  0,      -1,         0};
            final Integer[] mac02_sub_idxs_be = {  0,       0,        1,       1,         1,        1,    4,       4,    5,   0,  0,      -1,         0};
            test_sub01(ByteOrder.LITTLE_ENDIAN, mac02_str, Arrays.asList(mac02_sub_strs), Arrays.asList(mac02_sub_idxs_le));
            test_sub01(ByteOrder.BIG_ENDIAN, mac02_str, Arrays.asList(mac02_sub_strs), Arrays.asList(mac02_sub_idxs_be));
        }

        {
            // index                      [high=5 ...   low=0]
            final String mac03_str = "01:02:03:04:05:06";
            final String[] mac03_sub_strs =    { "01", "01:02", ":03:04", "03:04", ":04:05:", "04:05:", "04", "05:06", "06", ":", "", "06:05", mac03_str};
            final Integer[] mac03_sub_idxs_le = {  5,       4,        2,       2,         1,        1,    2,       0,    0,   0,  0,      -1,         0};
            final Integer[] mac03_sub_idxs_be = {  0,       0,        2,       2,         3,        3,    3,       4,    5,   0,  0,      -1,         0};
            test_sub01(ByteOrder.LITTLE_ENDIAN, mac03_str, Arrays.asList(mac03_sub_strs), Arrays.asList(mac03_sub_idxs_le));
            test_sub01(ByteOrder.BIG_ENDIAN, mac03_str, Arrays.asList(mac03_sub_strs), Arrays.asList(mac03_sub_idxs_be));
        }
        {
            final String mac_sub_str = "C0:10:22:A0:10:00";
            test_sub02(mac_sub_str, mac_sub_str, true /* expected_result */);
        }
        {
            final String mac_sub_str = "0600106";
            test_sub02(null, mac_sub_str, false /* expected_result */);
        }
        {
            final EUI48 h = new EUI48("01:02:03:04:05:06");
            final EUI48Sub n = new EUI48Sub("01:02");
            Assert.assertEquals(0, h.indexOf(n, ByteOrder.BIG_ENDIAN));
            Assert.assertEquals(4, h.indexOf(n, ByteOrder.LITTLE_ENDIAN));
        }
        {
            final EUI48 h = new EUI48("01:02:03:04:05:06");
            final EUI48Sub n = new EUI48Sub("05:06");
            Assert.assertEquals(4, h.indexOf(n, ByteOrder.BIG_ENDIAN));
            Assert.assertEquals(0, h.indexOf(n, ByteOrder.LITTLE_ENDIAN));
        }
    }

    public static void main(final String args[]) throws IOException {
        final String tstname = TestEUI48.class.getName();
        org.junit.runner.JUnitCore.main(tstname);
    }

}
