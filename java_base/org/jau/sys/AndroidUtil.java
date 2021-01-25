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
package org.jau.sys;

import java.io.File;
import java.lang.reflect.Method;

import org.jau.lang.ReflectionUtil;

public class AndroidUtil {

    private static final Method androidGetPackageInfoVersionNameMethod;
    private static final Method androidGetPackageInfoVersionCodeMethod;
    private static final Method androidGetTempRootMethod;

    static {
        if(AndroidVersion.isAvailable) {
            final ClassLoader cl = AndroidUtil.class.getClassLoader();
            final Class<?> androidAndroidUtilImplClz = ReflectionUtil.getClass("jau.sys.android.AndroidUtilImpl", true, cl);
            androidGetPackageInfoVersionCodeMethod = ReflectionUtil.getMethod(androidAndroidUtilImplClz, "getPackageInfoVersionCode", String.class);
            androidGetPackageInfoVersionNameMethod = ReflectionUtil.getMethod(androidAndroidUtilImplClz, "getPackageInfoVersionName", String.class);
            androidGetTempRootMethod = ReflectionUtil.getMethod(androidAndroidUtilImplClz, "getTempRoot");
        } else {
            androidGetPackageInfoVersionCodeMethod = null;
            androidGetPackageInfoVersionNameMethod = null;
            androidGetTempRootMethod = null;
        }
    }

    /**
     * @return null if platform is not Android or no Android Context is registered
     *         via {@link jogamp.common.os.android.StaticContext#init(android.content.Context) StaticContext.init(..)},
     *         otherwise the found package version code of <code>packageName</code> is returned.
     */
    public static final int getPackageInfoVersionCode(final String packageName) {
        if(null != androidGetPackageInfoVersionCodeMethod) {
            return ((Integer) ReflectionUtil.callStaticMethod(androidGetPackageInfoVersionCodeMethod, packageName)).intValue();
        }
        return -1;
    }

    /**
     * @return null if platform is not Android or no Android Context is registered
     *         via {@link jogamp.common.os.android.StaticContext#init(android.content.Context) StaticContext.init(..)},
     *         otherwise the found package version name of <code>packageName</code> is returned.
     */
    public static final String getPackageInfoVersionName(final String packageName) {
        if(null != androidGetPackageInfoVersionNameMethod) {
            return (String) ReflectionUtil.callStaticMethod(androidGetPackageInfoVersionNameMethod, packageName);
        }
        return null;
    }

    /**
     * @return null if platform is not Android or no Android Context is registered
     *         via {@link jogamp.common.os.android.StaticContext#init(android.content.Context) StaticContext.init(..)},
     *         otherwise the context relative world readable <code>temp</code> directory returned.
     */
    public static File getTempRoot()
        throws RuntimeException {
        if(null != androidGetTempRootMethod) {
            return (File) ReflectionUtil.callStaticMethod(androidGetTempRootMethod);
        }
        return null;
    }

}
