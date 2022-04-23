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
package org.jau.pkg.cache;

import java.io.File;
import java.io.IOException;
import java.net.URISyntaxException;
import java.security.cert.Certificate;
import java.util.HashMap;
import java.util.Map;
import java.util.jar.JarFile;

import org.jau.net.Uri;
import org.jau.pkg.JarUtil;
import org.jau.sec.SecurityUtil;
import org.jau.sys.Debug;
import org.jau.sys.JNILibrary;

/**
 * Static Jar file cache handler using an underlying instance of {@link TempFileCache}, see {@link #getTempFileCache()}.
 * <p>
 * Lifecycle: Concurrently running JVMs and ClassLoader
 * </p>
 */
public class TempJarCache {
    private static final boolean DEBUG = Debug.debug("TempJarCache");

    // A HashMap of native libraries that can be loaded with System.load()
    // The key is the string name of the library as passed into the loadLibrary
    // call; it is the file name without the directory or the platform-dependent
    // library prefix and suffix. The value is the absolute path name to the
    // unpacked library file in nativeTmpDir.
    private static Map<String, String> nativeLibMap;

    public enum LoadState {
        LOOKED_UP, LOADED;

        public boolean compliesWith(final LoadState o2) {
            return null != o2 ? compareTo(o2) >= 0 : false;
        }
    }
    private static boolean testLoadState(final LoadState has, final LoadState exp) {
        if(null == has) {
            return null == exp;
        }
        return has.compliesWith(exp);
    }

    // Set of jar files added
    private static Map<Uri, LoadState> nativeLibJars;
    private static Map<Uri, LoadState> classFileJars;
    private static Map<Uri, LoadState> resourceFileJars;

    private static TempFileCache tmpFileCache;

    private static volatile boolean staticInitError = false;
    private static volatile boolean staticTempIsExecutable = true;
    private static volatile boolean isInit = false;

    /**
     * Documented way to kick off static initialization.
     *
     * @return true is static initialization was successful
     */
    public static boolean initSingleton() {
        if (!isInit) { // volatile: ok
            synchronized (TempJarCache.class) {
                if (!isInit) {
                    staticInitError = !TempFileCache.initSingleton();

                    if(!staticInitError) {
                        tmpFileCache = new TempFileCache();
                        staticInitError = !tmpFileCache.isValid(false);
                        staticTempIsExecutable = tmpFileCache.isValid(true);
                    }

                    if(!staticInitError) {
                        // Initialize the collections of resources
                        nativeLibMap = new HashMap<String, String>();
                        nativeLibJars = new HashMap<Uri, LoadState>();
                        classFileJars = new HashMap<Uri, LoadState>();
                        resourceFileJars = new HashMap<Uri, LoadState>();
                    }
                    if(DEBUG) {
                        final File tempDir = null != tmpFileCache ? tmpFileCache.getTempDir() : null;
                        final String tempDirAbsPath = null != tempDir ? tempDir.getAbsolutePath() : null;
                        System.err.println("TempJarCache.initSingleton(): ok "+(false==staticInitError)+", "+ tempDirAbsPath+", executable "+staticTempIsExecutable);
                    }
                    isInit = true;
                }
            }
        }
        return !staticInitError;
    }

    /**
     * This is <b>not recommended</b> since the JNI libraries may still be
     * in use by the ClassLoader they are loaded via {@link System#load(String)}.
     * </p>
     * <p>
     * In JogAmp, JNI native libraries loaded and registered by {@link JNIJarLibrary}
     * derivations, where the native JARs might be loaded via {@link JNIJarLibrary#addNativeJarLibs(Class, String) }.
     * </p>
     * <p>
     * The only valid use case to shutdown the TempJarCache is at bootstrapping,
     * i.e. when no native library is guaranteed to be loaded. This could be useful
     * if bootstrapping needs to find the proper native library type.
     * </p>
     *
    public static void shutdown() {
        if (isInit) { // volatile: ok
            synchronized (TempJarCache.class) {
                if (isInit) {
                    if(DEBUG) {
                        System.err.println("TempJarCache.shutdown(): real "+(false==staticInitError)+", "+ tmpFileCache.getTempDir());
                    }
                    isInit = false;
                    if(!staticInitError) {
                        nativeLibMap.clear();
                        nativeLibMap = null;
                        nativeLibJars.clear();
                        nativeLibJars = null;
                        classFileJars.clear();
                        classFileJars = null;
                        resourceFileJars.clear();
                        resourceFileJars = null;

                        tmpFileCache.destroy();
                        tmpFileCache = null;
                    }
                }
            }
        }
    } */

    private static boolean isInitializedImpl() {
        if (!isInit) { // volatile: ok
            synchronized (TempJarCache.class) {
                if (!isInit) {
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * @param forExecutables if {@code true}, method also tests whether the underlying cache is suitable to load native libraries or launch executables
     * @return true if this class has been properly initialized, ie. is in use. Otherwise returns false.
     */
    public static boolean isInitialized(final boolean forExecutables) {
        return isInitializedImpl() && !staticInitError && ( !forExecutables || staticTempIsExecutable );
    }

    /**
     * @param forExecutables if {@code true}, method also tests whether the underlying cache is suitable to load native libraries or launch executables
     */
    /* package */ static void checkInitialized(final boolean forExecutables) {
        if(!isInitializedImpl()) {
            throw new RuntimeException("initSingleton() has to be called first.");
        }
        if(staticInitError) {
            throw new RuntimeException("initSingleton() failed.");
        }
        if( forExecutables && !staticTempIsExecutable ) {
            final File tempDir = null != tmpFileCache ? tmpFileCache.getTempDir() : null;
            final String tempDirAbsPath = null != tempDir ? tempDir.getAbsolutePath() : null;
            throw new RuntimeException("TempJarCache folder not suitable for executables: "+tempDirAbsPath);
        }
    }

    /**
     * @return the underlying {@link TempFileCache}
     * @throws RuntimeException if not {@link #isInitialized(boolean) isInitialized(false)}
     */
    public static TempFileCache getTempFileCache() {
        checkInitialized(false);
        return tmpFileCache;
    }

    /**
     * @param jarUri
     * @param exp
     * @return
     * @throws IOException
     * @throws RuntimeException if not {@link #isInitialized(boolean) isInitialized(false)}
     */
    public synchronized static boolean checkNativeLibs(final Uri jarUri, final LoadState exp) throws IOException {
        checkInitialized(false);
        if(null == jarUri) {
            throw new IllegalArgumentException("jarUri is null");
        }
        return testLoadState(nativeLibJars.get(jarUri), exp);
    }

    /**
     * @param jarUri
     * @param exp
     * @return
     * @throws IOException
     * @throws RuntimeException if not {@link #isInitialized(boolean) isInitialized(false)}
     */
    public synchronized static boolean checkClasses(final Uri jarUri, final LoadState exp) throws IOException {
        checkInitialized(false);
        if(null == jarUri) {
            throw new IllegalArgumentException("jarUri is null");
        }
        return testLoadState(classFileJars.get(jarUri), exp);
    }

    /**
     *
     * @param jarUri
     * @param exp
     * @return
     * @throws IOException
     * @throws RuntimeException if not {@link #isInitialized(boolean) isInitialized(false)}
     */
    public synchronized static boolean checkResources(final Uri jarUri, final LoadState exp) throws IOException {
        checkInitialized(false);
        if(null == jarUri) {
            throw new IllegalArgumentException("jarUri is null");
        }
        return testLoadState(resourceFileJars.get(jarUri), exp);
    }

    /**
     * Adds native libraries, if not yet added.
     *
     * @param certClass if class is certified, the JarFile entries needs to have the same certificate
     * @param jarUri
     * @param nativeLibraryPath if not null, only extracts native libraries within this path.
     * @return true if native libraries were added or previously loaded from given jarUri, otherwise false
     * @throws IOException if the <code>jarUri</code> could not be loaded or a previous load attempt failed
     * @throws SecurityException
     * @throws URISyntaxException
     * @throws IllegalArgumentException
     * @throws RuntimeException if not {@link #isInitialized(boolean) isInitialized(true)}
     */
    public synchronized static final boolean addNativeLibs(final Class<?> certClass, final Uri jarUri, final String nativeLibraryPath) throws IOException, SecurityException, IllegalArgumentException, URISyntaxException {
        checkInitialized(true);
        final LoadState nativeLibJarsLS = nativeLibJars.get(jarUri);
        if( !testLoadState(nativeLibJarsLS, LoadState.LOOKED_UP) ) {
            nativeLibJars.put(jarUri, LoadState.LOOKED_UP);
            final JarFile jarFile = JarUtil.getJarFile(jarUri);
            if(DEBUG) {
                System.err.println("TempJarCache: addNativeLibs: "+jarUri+": nativeJar "+jarFile.getName()+" (NEW)");
            }
            validateCertificates(certClass, jarFile);
            final int num = JarUtil.extract(tmpFileCache.getTempDir(), nativeLibMap, jarFile, nativeLibraryPath, true, false, false);
            nativeLibJars.put(jarUri, LoadState.LOADED);
            return num > 0;
        } else if( testLoadState(nativeLibJarsLS, LoadState.LOADED) ) {
            if(DEBUG) {
                System.err.println("TempJarCache: addNativeLibs: "+jarUri+": nativeJar "+jarUri+" (REUSE)");
            }
            return true;
        }
        throw new IOException("TempJarCache: addNativeLibs: "+jarUri+", previous load attempt failed");
    }

    /**
     * Adds native classes, if not yet added.
     *
     * TODO class access pending
     * needs Classloader.defineClass(..) access, ie. own derivation - will do when needed ..
     *
     * @param certClass if class is certified, the JarFile entries needs to have the same certificate
     * @param jarUri
     * @throws IOException if the <code>jarUri</code> could not be loaded or a previous load attempt failed
     * @throws SecurityException
     * @throws URISyntaxException
     * @throws IllegalArgumentException
     * @throws RuntimeException if not {@link #isInitialized(boolean) isInitialized(false)}
     */
    public synchronized static final void addClasses(final Class<?> certClass, final Uri jarUri) throws IOException, SecurityException, IllegalArgumentException, URISyntaxException {
        checkInitialized(false);
        final LoadState classFileJarsLS = classFileJars.get(jarUri);
        if( !testLoadState(classFileJarsLS, LoadState.LOOKED_UP) ) {
            classFileJars.put(jarUri, LoadState.LOOKED_UP);
            final JarFile jarFile = JarUtil.getJarFile(jarUri);
            if(DEBUG) {
                System.err.println("TempJarCache: addClasses: "+jarUri+": nativeJar "+jarFile.getName());
            }
            validateCertificates(certClass, jarFile);
            JarUtil.extract(tmpFileCache.getTempDir(), null, jarFile,
                            null /* nativeLibraryPath */, false, true, false);
            classFileJars.put(jarUri, LoadState.LOADED);
        } else if( !testLoadState(classFileJarsLS, LoadState.LOADED) ) {
            throw new IOException("TempJarCache: addClasses: "+jarUri+", previous load attempt failed");
        }
    }

    /**
     * Adds native resources, if not yet added.
     *
     * @param certClass if class is certified, the JarFile entries needs to have the same certificate
     * @param jarUri
     * @return
     * @throws IOException if the <code>jarUri</code> could not be loaded or a previous load attempt failed
     * @throws SecurityException
     * @throws URISyntaxException
     * @throws IllegalArgumentException
     * @throws RuntimeException if not {@link #isInitialized(boolean) isInitialized(false)}
     */
    public synchronized static final void addResources(final Class<?> certClass, final Uri jarUri) throws IOException, SecurityException, IllegalArgumentException, URISyntaxException {
        checkInitialized(false);
        final LoadState resourceFileJarsLS = resourceFileJars.get(jarUri);
        if( !testLoadState(resourceFileJarsLS, LoadState.LOOKED_UP) ) {
            resourceFileJars.put(jarUri, LoadState.LOOKED_UP);
            final JarFile jarFile = JarUtil.getJarFile(jarUri);
            if(DEBUG) {
                System.err.println("TempJarCache: addResources: "+jarUri+": nativeJar "+jarFile.getName());
            }
            validateCertificates(certClass, jarFile);
            JarUtil.extract(tmpFileCache.getTempDir(), null, jarFile,
                            null /* nativeLibraryPath */, false, false, true);
            resourceFileJars.put(jarUri, LoadState.LOADED);
        } else if( !testLoadState(resourceFileJarsLS, LoadState.LOADED) ) {
            throw new IOException("TempJarCache: addResources: "+jarUri+", previous load attempt failed");
        }
    }

    /**
     * Adds all types, native libraries, class files and other files (resources)
     * if not yet added.
     *
     * TODO class access pending
     * needs Classloader.defineClass(..) access, ie. own derivation - will do when needed ..
     *
     * @param certClass if class is certified, the JarFile entries needs to have the same certificate
     * @param jarUri
     * @throws IOException if the <code>jarUri</code> could not be loaded or a previous load attempt failed
     * @throws SecurityException
     * @throws URISyntaxException
     * @throws IllegalArgumentException
     * @throws RuntimeException if not {@link #isInitialized(boolean) isInitialized(false)}
     */
    public synchronized static final void addAll(final Class<?> certClass, final Uri jarUri) throws IOException, SecurityException, IllegalArgumentException, URISyntaxException {
        checkInitialized(false);
        if(null == jarUri) {
            throw new IllegalArgumentException("jarUri is null");
        }
        final LoadState nativeLibJarsLS = nativeLibJars.get(jarUri);
        final LoadState classFileJarsLS = classFileJars.get(jarUri);
        final LoadState resourceFileJarsLS = resourceFileJars.get(jarUri);
        if( !testLoadState(nativeLibJarsLS, LoadState.LOOKED_UP) ||
            !testLoadState(classFileJarsLS, LoadState.LOOKED_UP) ||
            !testLoadState(resourceFileJarsLS, LoadState.LOOKED_UP) ) {

            final boolean extractNativeLibraries = staticTempIsExecutable && !testLoadState(nativeLibJarsLS, LoadState.LOADED);
            final boolean extractClassFiles = !testLoadState(classFileJarsLS, LoadState.LOADED);
            final boolean extractOtherFiles = !testLoadState(resourceFileJarsLS, LoadState.LOOKED_UP);

            // mark looked-up (those who are not loaded)
            if(extractNativeLibraries) {
                nativeLibJars.put(jarUri, LoadState.LOOKED_UP);
            }
            if(extractClassFiles) {
                classFileJars.put(jarUri, LoadState.LOOKED_UP);
            }
            if(extractOtherFiles) {
                resourceFileJars.put(jarUri, LoadState.LOOKED_UP);
            }

            final JarFile jarFile = JarUtil.getJarFile(jarUri);
            if(DEBUG) {
                System.err.println("TempJarCache: addAll: "+jarUri+": nativeJar "+jarFile.getName());
            }
            validateCertificates(certClass, jarFile);
            JarUtil.extract(tmpFileCache.getTempDir(), nativeLibMap, jarFile,
                            null /* nativeLibraryPath */, extractNativeLibraries, extractClassFiles, extractOtherFiles);

            // mark loaded (those were just loaded)
            if(extractNativeLibraries) {
                nativeLibJars.put(jarUri, LoadState.LOADED);
            }
            if(extractClassFiles) {
                classFileJars.put(jarUri, LoadState.LOADED);
            }
            if(extractOtherFiles) {
                resourceFileJars.put(jarUri, LoadState.LOADED);
            }
        } else if( !testLoadState(nativeLibJarsLS, LoadState.LOADED) ||
                   !testLoadState(classFileJarsLS, LoadState.LOADED) ||
                   !testLoadState(resourceFileJarsLS, LoadState.LOADED) ) {
            throw new IOException("TempJarCache: addAll: "+jarUri+", previous load attempt failed");
        }
    }

    /**
     * If {@link #isInitialized(boolean) isInitialized(true)} is false due to lack of executable support only,
     * this method always returns false.
     * @param libName
     * @return the found native library path within this cache or null if not found
     * @throws RuntimeException if not {@link #isInitialized(boolean) isInitialized(false)}
     */
    public synchronized static final String findLibrary(final String libName) {
        checkInitialized(false);
        if( !staticTempIsExecutable ) {
            return null;
        }
        // try with mapped library basename first
        String path = nativeLibMap.get(libName);
        if(null == path) {
            // if valid library name, try absolute path in temp-dir
            if(null != JNILibrary.isValidNativeLibraryName(libName, false)) {
                final File f = new File(tmpFileCache.getTempDir(), libName);
                if(f.exists()) {
                    path = f.getAbsolutePath();
                }
            }
        }
        return path;
    }

    /** TODO class access pending
     * needs Classloader.defineClass(..) access, ie. own derivation - will do when needed ..
    public static Class<?> findClass(String name, ClassLoader cl) throws IOException, ClassFormatError {
        checkInitialized();
        final File f = new File(nativeTmpFileCache.getTempDir(), IOUtil.getClassFileName(name));
        if(f.exists()) {
            Class.forName(fname, initialize, loader)
            URL url = new URL(f.getAbsolutePath());
            byte[] b = IOUtil.copyStream2ByteArray(new BufferedInputStream( url.openStream() ));
            MyClassLoader mcl = new MyClassLoader(cl);
            return mcl.defineClass(name, b, 0, b.length);
        }
        return null;
    } */

    /**
     * Similar to {@link ClassLoader#getResource(String)}.
     * @param name
     * @return
     * @throws RuntimeException if not {@link #isInitialized(boolean) isInitialized(false)}
     */
    public synchronized static final String findResource(final String name) {
        checkInitialized(false);
        final File f = new File(tmpFileCache.getTempDir(), name);
        if(f.exists()) {
            return f.getAbsolutePath();
        }
        return null;
    }

    /**
     * Similar to {@link ClassLoader#getResource(String)}.
     * @param name
     * @return
     * @throws URISyntaxException
     * @throws RuntimeException if not {@link #isInitialized(boolean) isInitialized(false)}
     */
    public synchronized static final Uri getResourceUri(final String name) throws URISyntaxException {
        checkInitialized(false);
        final File f = new File(tmpFileCache.getTempDir(), name);
        if(f.exists()) {
            return Uri.valueOf(f);
        }
        return null;
    }

    private static void validateCertificates(final Class<?> certClass, final JarFile jarFile) throws IOException, SecurityException {
        if(null == certClass) {
            throw new IllegalArgumentException("certClass is null");
        }
        final Certificate[] rootCerts = SecurityUtil.getCerts(certClass);
        if( null != rootCerts ) {
            // Only validate the jarFile's certs with ours, if we have any.
            // Otherwise we may run uncertified JARs (application).
            // In case one tries to run uncertified JARs, the wrapping applet/JNLP
            // SecurityManager will kick in and throw a SecurityException.
            JarUtil.validateCertificates(rootCerts, jarFile);
            if(DEBUG) {
                System.err.println("TempJarCache: validateCertificates: OK - Matching rootCerts in given class "+certClass.getName()+", nativeJar "+jarFile.getName());
            }
        } else if(DEBUG) {
            System.err.println("TempJarCache: validateCertificates: OK - No rootCerts in given class "+certClass.getName()+", nativeJar "+jarFile.getName());
        }
    }
}
