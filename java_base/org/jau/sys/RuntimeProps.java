/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2010-2023 Gothel Software e.K.
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
package org.jau.sys;

import org.jau.lang.ReflectionUtil;
import org.jau.sys.PlatformTypes.OSType;

/**
 * Runtime platform properties derived from {@link PlatformProps} and runtime query.
 *
 * Static initializer loads the native jaulib library, if available.
 */
public class RuntimeProps {
    private static final String prt_name = "jau.pkg.PlatformRuntime";

    //
    // static initialization order:
    //

    public static final boolean DEBUG;

    /** Runtime determined {@link MachineDataInfo}, null if not available (i.e. no JNI libs loaded). */
    public static final MachineDataInfo MACH_DESC_RT;

    /**
     * true if enabled and in use.
     * <p>
     * System property: 'jau.pkg.UseTempJarCache',
     * defaults to true if {@link #OS_TYPE} is not {@link OSType#ANDROID}.
     * </p>
     */
    public static final boolean USE_TEMP_JAR_CACHE;

    static {
        DEBUG = PlatformProps.DEBUG;

        {
            boolean _USE_TEMP_JAR_CACHE = false;
            Class<?> prt = null;
            try {
                prt = ReflectionUtil.getClass(prt_name, true /* initializeClazz */, RuntimeProps.class.getClassLoader());
            } catch (final Throwable t) {
                if( DEBUG ) {
                    System.err.println("Platform.RT: Exception: "+t.getMessage());
                    t.printStackTrace();
                }
            }
            if( null != prt ) {
                final ReflectionUtil.MethodAccessor prtGetMachDesc = new ReflectionUtil.MethodAccessor(prt, "getMachineDataInfo");
                final ReflectionUtil.MethodAccessor prtGetUseTempJarCache = new ReflectionUtil.MethodAccessor(prt, "getUseTempJarCache");
                if( null != prtGetMachDesc && prtGetMachDesc.available() ) {
                    MACH_DESC_RT = prtGetMachDesc.callStaticMethod();
                    _USE_TEMP_JAR_CACHE = null != MACH_DESC_RT;
                    if( DEBUG ) {
                        System.err.println("Platform.RT: Available <"+prt_name+".getMachineDataInfo()>");
                    }
                } else {
                    MACH_DESC_RT = null;
                    if( DEBUG ) {
                        System.err.println("Platform.RT: Not available (2) <"+prt_name+".getMachineDataInfo()>");
                    }
                }
                if( null != prtGetUseTempJarCache && prtGetUseTempJarCache.available() ) {
                    _USE_TEMP_JAR_CACHE = prtGetUseTempJarCache.callStaticMethod();
                    if( DEBUG ) {
                        System.err.println("Platform.RT: Available <"+prt_name+".getUseTempJarCache()> = "+_USE_TEMP_JAR_CACHE);
                    }
                } else {
                    if( DEBUG ) {
                        System.err.println("Platform.RT: Not available (3) <"+prt_name+".getUseTempJarCache()>");
                    }
                }
            } else {
                MACH_DESC_RT = null;
                if( DEBUG ) {
                    System.err.println("Platform.RT: Not available (1) <"+prt_name+">");
                }
            }
            USE_TEMP_JAR_CACHE = _USE_TEMP_JAR_CACHE;
        }

        if( DEBUG ) {
            System.err.println("Platform.MD.RT: "+MACH_DESC_RT);
            System.err.println("Platform.sys: USE_TEMP_JAR_CACHE "+USE_TEMP_JAR_CACHE);
        }
    }
    public static void initSingleton() { }

    /**
     * Returns true if enabled and in use.
     * <p>
     * System property: 'jau.pkg.UseTempJarCache',
     * defaults to true if {@link #O} is not {@link OSType#ANDROID}.
     * </p>
     */
    public static final boolean getUseTempJarCache() { return USE_TEMP_JAR_CACHE; }

    protected RuntimeProps() {}

}
