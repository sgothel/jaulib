/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2010 Gothel Software e.K.
 * Copyright (c) 2010 JogAmp Community
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

package org.jau.base;

import java.util.jar.Manifest;

import org.jau.util.JauVersion;
import org.jau.util.VersionUtil;

public class JaulibVersion extends JauVersion {

    protected static volatile JaulibVersion jaulibVersionInfo;

    protected JaulibVersion(final String packageName, final Manifest mf) {
        super(packageName, mf);
    }

    public static JaulibVersion getInstance() {
        if(null == jaulibVersionInfo) { // volatile: ok
            synchronized(JaulibVersion.class) {
                if( null == jaulibVersionInfo ) {
                    final String packageNameCompileTime = "org.jau.base";
                    final String packageNameRuntime = "org.jau.base";
                    Manifest mf = VersionUtil.getManifest(JaulibVersion.class.getClassLoader(), packageNameRuntime);
                    if(null != mf) {
                        jaulibVersionInfo = new JaulibVersion(packageNameRuntime, mf);
                    } else {
                        mf = VersionUtil.getManifest(JaulibVersion.class.getClassLoader(), packageNameCompileTime);
                        jaulibVersionInfo = new JaulibVersion(packageNameCompileTime, mf);
                    }
                }
            }
        }
        return jaulibVersionInfo;
    }

    public static void main(final String args[]) {
        System.err.println(VersionUtil.getPlatformInfo());
        System.err.println(JaulibVersion.getInstance());
    }
}
