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

package org.jau.util;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.Set;
import java.util.jar.Attributes;
import java.util.jar.Manifest;

import org.jau.io.IOUtil;
import org.jau.sys.AndroidVersion;
import org.jau.sys.PlatformProps;
import org.jau.sys.PlatformTypes;

public class VersionUtil {

    public static final String SEPERATOR = "-----------------------------------------------------------------------------------------------------";

    /**
     * Appends environment information like OS, JVM and CPU architecture properties to the StringBuilder.
     */
    public static StringBuilder getPlatformInfo(StringBuilder sb) {
        if(null == sb) {
            sb = new StringBuilder();
        }

        sb.append(SEPERATOR).append(PlatformProps.NEWLINE);

        // environment
        sb.append("Platform: ").append(PlatformProps.OS).append(" / ").append(PlatformProps.os_name).append(' ').append(PlatformProps.os_version).append(", ");
        sb.append(PlatformProps.os_arch).append(" (").append(PlatformProps.CPU).append(", ").append(PlatformProps.ABI).append("), ");
        sb.append(Runtime.getRuntime().availableProcessors()).append(" cores, ").append("littleEndian ").append(PlatformProps.LITTLE_ENDIAN);
        sb.append(PlatformProps.NEWLINE);
        if( PlatformTypes.OSType.ANDROID == PlatformProps.OS ) {
            sb.append("Platform: Android Version: ").append(AndroidVersion.CODENAME).append(", ");
            sb.append(AndroidVersion.RELEASE).append(" [").append(AndroidVersion.RELEASE).append("], SDK: ").append(AndroidVersion.SDK_INT).append(", ").append(AndroidVersion.SDK_NAME);
            sb.append(PlatformProps.NEWLINE);
        }

        if( null != PlatformProps.MACH_DESC_RT ) {
            PlatformProps.MACH_DESC_RT.toString(sb).append("; (runtime)").append(PlatformProps.NEWLINE);
        } else {
            PlatformProps.MACH_DESC_STAT.toString(sb).append("; (static)").append(PlatformProps.NEWLINE);
        }

        // JVM/JRE
        sb.append("Platform: Java Version: ").append(PlatformProps.JAVA_VERSION_NUMBER).append(", VM: ").append(PlatformProps.JAVA_VM_NAME);
        sb.append(", Runtime: ").append(PlatformProps.JAVA_RUNTIME_NAME).append(PlatformProps.NEWLINE);
        sb.append("Platform: Java Vendor: ").append(PlatformProps.JAVA_VENDOR);
        if( PlatformProps.JAVA_21 ) {
            sb.append(", Java21");
        } else if( PlatformProps.JAVA_17 ) {
            sb.append(", Java17");
        } else if( PlatformProps.JAVA_9 ) {
            sb.append(", Java9");
        }
        // sb.append(", dynamicLib: ").append(PlatformProps.useDynamicLibraries);
        sb.append(PlatformProps.NEWLINE).append(SEPERATOR);

        return sb;
    }

    /**
     * Prints platform info.
     * @see #getPlatformInfo(java.lang.StringBuilder)
     */
    public static String getPlatformInfo() {
        return getPlatformInfo(null).toString();
    }

    /**
     * Returns the manifest of the jar which contains the specified extension.
     * The provided ClassLoader is used for resource loading.
     * @param cl A ClassLoader which should find the manifest.
     * @param extension The value of the 'Extension-Name' jar-manifest attribute; used to identify the manifest.
     * @return the requested manifest or null when not found.
     */
    public static Manifest getManifest(final ClassLoader cl, final String extension) {
        return getManifest(cl, new String[] { extension } );
    }

    /**
     * Returns the manifest of the jar which contains one of the specified extensions.
     * The provided ClassLoader is used for resource loading.
     * @param cl A ClassLoader which should find the manifest.
     * @param extensions The values of many 'Extension-Name's jar-manifest attribute; used to identify the manifest.
     *                   Matching is applied in decreasing order, i.e. first element is favored over the second, etc.
     * @return the requested manifest or null when not found.
     */
    public static Manifest getManifest(final ClassLoader cl, final String[] extensions) {
        final Manifest[] extManifests = new Manifest[extensions.length];
        try {
            final Enumeration<URL> resources = cl.getResources("META-INF/MANIFEST.MF");
            while (resources.hasMoreElements()) {
                final InputStream is = resources.nextElement().openStream();
                final Manifest manifest;
                try {
                    manifest = new Manifest(is);
                } finally {
                    IOUtil.close(is, false);
                }
                final Attributes attributes = manifest.getMainAttributes();
                if(attributes != null) {
                    for(int i=0; i < extensions.length && null == extManifests[i]; i++) {
                        final String extension = extensions[i];
                        if( extension.equals( attributes.getValue( Attributes.Name.EXTENSION_NAME ) ) ) {
                            if( 0 == i ) {
                                return manifest; // 1st one has highest prio - done
                            }
                            extManifests[i] = manifest;
                        }
                    }
                }
            }
        } catch (final IOException ex) {
            throw new RuntimeException("Unable to read manifest.", ex);
        }
        for(int i=1; i<extManifests.length; i++) {
            if( null != extManifests[i] ) {
                return extManifests[i];
            }
        }
        return null;
    }

    public static StringBuilder getFullManifestInfo(final Manifest mf, StringBuilder sb) {
        if(null==mf) {
            return sb;
        }

        if(null==sb) {
            sb = new StringBuilder();
        }

        final Attributes attr = mf.getMainAttributes();
        final Set<Object> keys = attr.keySet();
        for(final Iterator<Object> iter=keys.iterator(); iter.hasNext(); ) {
            final Attributes.Name key = (Attributes.Name) iter.next();
            final String val = attr.getValue(key);
            sb.append(" ");
            sb.append(key);
            sb.append(" = ");
            sb.append(val);
            sb.append(PlatformProps.NEWLINE);
        }
        return sb;
    }
}

