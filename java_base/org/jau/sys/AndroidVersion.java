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
package org.jau.sys;

import java.lang.reflect.Field;
import java.util.HashMap;

import org.jau.lang.ReflectionUtil;
import org.jau.sys.PlatformTypes.ABIType;
import org.jau.sys.PlatformTypes.CPUType;

public class AndroidVersion {
    public static final boolean isAvailable;

    /** The name of the instruction set (CPU type + ABI convention) of native code. API-4. All lower case.*/
    public static final String CPU_ABI_NAME;
    public static final CPUType CPU;
    public static final ABIType ABI;

    /** The name of the second instruction set (CPU type + ABI convention) of native code. API-8. All lower case.*/
    public static final String CPU_ABI2_NAME;
    public static final CPUType CPU2;
    public static final ABIType ABI2;

    /** Development codename, or the string "REL" for official release */
    public static final String CODENAME;

    /** internal build value used by the underlying source control. */
    public static final String INCREMENTAL;

    /** official build version string */
    public static final String RELEASE;

    /** SDK Version number, key to VERSION_CODES */
    public static final int SDK_INT;

    /** SDK Version string */
    public static final String SDK_NAME;

    private static final String androidBuild = "android.os.Build";
    private static final String androidBuildVersion = "android.os.Build$VERSION";
    private static final String androidBuildVersionCodes = "android.os.Build$VERSION_CODES";

    static {
        final ClassLoader cl = AndroidVersion.class.getClassLoader();
        Class<?> abClass = null;
        Object abObject= null;
        Class<?> abvClass = null;
        Object abvObject= null;
        Class<?> abvcClass = null;
        Object abvcObject= null;

        final boolean isDalvikVm = "Dalvik".equals(System.getProperty("java.vm.name"));

        if (isDalvikVm) {
          try {
              abClass = ReflectionUtil.getClass(androidBuild, true, cl);
              abObject = abClass.getDeclaredConstructor().newInstance();
              abvClass = ReflectionUtil.getClass(androidBuildVersion, true, cl);
              abvObject = abvClass.getDeclaredConstructor().newInstance();
              abvcClass = ReflectionUtil.getClass(androidBuildVersionCodes, true, cl);
              abvcObject = abvcClass.getDeclaredConstructor().newInstance();
          } catch (final Exception e) { /* n/a */ }
        }
        isAvailable = isDalvikVm && null != abObject && null != abvObject;
        if(isAvailable) {
            CPU_ABI_NAME = getString(abClass, abObject, "CPU_ABI", true);
            CPU_ABI2_NAME = getString(abClass, abObject, "CPU_ABI2", true);
            CODENAME = getString(abvClass, abvObject, "CODENAME", false);
            INCREMENTAL = getString(abvClass, abvObject, "INCREMENTAL", false);
            RELEASE = getString(abvClass, abvObject, "RELEASE", false);
            SDK_INT = getInt(abvClass, abvObject, "SDK_INT");
            final String sdk_name;
            if( null != abvcObject ) {
                final HashMap<Integer, String> version_codes = getVersionCodes(abvcClass, abvcObject);
                sdk_name = version_codes.get(SDK_INT);
            } else {
                sdk_name = null;
            }
            SDK_NAME = ( null != sdk_name ) ? sdk_name : "SDK_"+SDK_INT ;

            /**
             * <p>
             * FIXME: Where is a comprehensive list of known 'android.os.Build.CPU_ABI' and 'android.os.Build.CPU_ABI2' strings ?<br/>
             * Fount this one: <code>http://www.kandroid.org/ndk/docs/CPU-ARCH-ABIS.html</code>
             * <pre>
             *  lib/armeabi/libfoo.so
             *  lib/armeabi-v7a/libfoo.so
             *  lib/arm64-v8a/libfoo.so
             *  lib/x86/libfoo.so
             *  lib/mips/libfoo.so
             * </pre>
             * </p>
             */
            CPU = PlatformTypes.CPUType.query(CPU_ABI_NAME);
            ABI = PlatformTypes.ABIType.query(CPU, CPU_ABI_NAME);
            if( null != CPU_ABI2_NAME && CPU_ABI2_NAME.length() > 0 ) {
                CPU2 = PlatformTypes.CPUType.query(CPU_ABI2_NAME);
                ABI2 = PlatformTypes.ABIType.query(CPU2, CPU_ABI2_NAME);
            } else {
                CPU2 = null;
                ABI2 = null;
            }
        } else {
            CPU_ABI_NAME = null;
            CPU_ABI2_NAME = null;
            CODENAME = null;
            INCREMENTAL = null;
            RELEASE = null;
            SDK_INT = -1;
            SDK_NAME = null;
            CPU = null;
            ABI = null;
            CPU2 = null;
            ABI2 = null;
        }
    }

    private static final HashMap<Integer, String> getVersionCodes(final Class<?> cls, final Object obj) {
        final Field[] fields = cls.getFields();
        final HashMap<Integer, String> map = new HashMap<Integer, String>( 3 * fields.length / 2, 0.75f );
        for(int i=0; i<fields.length; i++) {
            try {
                final int version = fields[i].getInt(obj);
                final String version_name = fields[i].getName();
                // System.err.println(i+": "+version+": "+version_name);
                map.put(Integer.valueOf(version), version_name);
            } catch (final Exception e) { e.printStackTrace(); /* n/a */ }
        }
        return map;
    }

    private static final String getString(final Class<?> cls, final Object obj, final String name, final boolean lowerCase) {
        try {
            final Field f = cls.getField(name);
            final String s = (String) f.get(obj);
            if( lowerCase && null != s ) {
                return s.toLowerCase();
            } else {
                return s;
            }
        } catch (final Exception e) { e.printStackTrace(); /* n/a */ }
        return null;
    }

    private static final int getInt(final Class<?> cls, final Object obj, final String name) {
        try {
            final Field f = cls.getField(name);
            return f.getInt(obj);
        } catch (final Exception e) { e.printStackTrace(); /* n/a */ }
        return -1;
    }

    // android.os.Build.VERSION
}
