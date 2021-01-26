/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2012 Gothel Software e.K.
 * Copyright (c) 2012 JogAmp Community.
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

import java.io.IOException;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

import jau.test.junit.util.JunitTracer;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestSystemPropsAndEnvs extends JunitTracer {

    @Test
    public void dumpProperties() {
        int i=0;
        final Properties props = System.getProperties();
        final Iterator<Map.Entry<Object,Object>> iter = props.entrySet().iterator();
        while (iter.hasNext()) {
          i++;
          final Map.Entry<Object, Object> entry = iter.next();
          System.out.format("%4d: %s = %s%n", i, entry.getKey(), entry.getValue());
        }
        System.out.println("Property count: "+i);
    }

    private static String[] suppress_envs = new String[] { "COOKIE", "SSH", "GPG" };

    private static boolean contains(final String data, final String[] search) {
        if(null != data && null != search) {
            for(int i=0; i<search.length; i++) {
                if(data.indexOf(search[i]) >= 0) {
                    return true;
                }
            }
        }
        return false;
    }

    @Test
    public void dumpEnvironment() {
        int i=0;
        final Map<String, String> env = System.getenv();
        for (final String envName : env.keySet()) {
            if(!contains(envName, suppress_envs)) {
                i++;
                System.out.format("%4d: %s = %s%n",
                                  i, envName,
                                  env.get(envName));
            }
        }
        System.out.println("Environment count: "+i);
    }

    public static void main(final String args[]) throws IOException {
        final String tstname = TestSystemPropsAndEnvs.class.getName();
        org.junit.runner.JUnitCore.main(tstname);
    }

}
