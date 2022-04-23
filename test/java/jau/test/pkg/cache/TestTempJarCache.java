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

package jau.test.pkg.cache;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Method;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.jar.JarFile;

import org.jau.io.IOUtil;
import org.jau.lang.ReflectionUtil;
import org.jau.net.Uri;
import org.jau.pkg.JarUtil;
import org.jau.pkg.cache.TempCacheReg;
import org.jau.pkg.cache.TempFileCache;
import org.jau.pkg.cache.TempJarCache;
import org.jau.sys.AndroidVersion;
import org.jau.sys.JNILibrary;
import org.jau.sys.PlatformProps;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

import jau.pkg.PlatformRuntime;
import jau.test.junit.util.JunitTracer;
import jau.test.net.URIDumpUtil;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestTempJarCache extends JunitTracer {
    static TempFileCache fileCache;

    static class TestClassLoader extends URLClassLoader {
        public TestClassLoader(final URL[] urls) {
            super(urls);
        }
        public TestClassLoader(final URL[] urls, final ClassLoader parent) {
            super(urls, parent);
        }
    }

    static void assertTempFileCachesIndividualInstances(final boolean shallBeSame, final TempFileCache fileCache2, final TempFileCache fileCache3) {
        Assert.assertTrue(fileCache2.getTempDir().exists());
        Assert.assertTrue(fileCache2.getTempDir().isDirectory());
        Assert.assertTrue(fileCache3.getTempDir().exists());
        Assert.assertTrue(fileCache3.getTempDir().isDirectory());

        Assert.assertEquals(TempFileCache.getBaseDir(), TempFileCache.getBaseDir());
        Assert.assertEquals(TempFileCache.getRootDir(), TempFileCache.getRootDir());

        if(shallBeSame) {
            Assert.assertTrue("file caches are not equal", fileCache2.getTempDir().equals(fileCache3.getTempDir()));
        } else {
            Assert.assertFalse("file caches are equal", fileCache2.getTempDir().equals(fileCache3.getTempDir()));
        }
        // also verify with diff classloader/reflection method,
        // to proof that methodology is valid!
        final ClassLoader cl = fileCache2.getClass().getClassLoader();
        assertTempFileCachesIndividualInstances(shallBeSame, fileCache2, cl, fileCache3, cl);
    }

    static void assertTempFileCachesIndividualInstances(final boolean shallBeSame, final Object fileCache2, final ClassLoader cl2, final Object fileCache3, final ClassLoader cl3) {
        final Class<?> fileCacheClazz2 = ReflectionUtil.getClass(TempFileCache.class.getName(), false, cl2);
        final Class<?> fileCacheClazz3 = ReflectionUtil.getClass(TempFileCache.class.getName(), false, cl3);

        final Method fc2GetBaseDir = ReflectionUtil.getMethod(fileCacheClazz2 , "getBaseDir");
        final Method fc3GetBaseDir = ReflectionUtil.getMethod(fileCacheClazz3 , "getBaseDir");
        final Object baseDir2 = ReflectionUtil.callMethod(fileCache2, fc2GetBaseDir);
        final Object baseDir3 = ReflectionUtil.callMethod(fileCache3, fc3GetBaseDir);
        Assert.assertEquals(baseDir2, baseDir3);

        final Method fc2GetRootDir = ReflectionUtil.getMethod(fileCacheClazz2 , "getRootDir");
        final Method fc3GetRootDir = ReflectionUtil.getMethod(fileCacheClazz3 , "getRootDir");
        final Object rootDir2 = ReflectionUtil.callMethod(fileCache2, fc2GetRootDir);
        final Object rootDir3 = ReflectionUtil.callMethod(fileCache3, fc3GetRootDir);
        Assert.assertEquals(rootDir2, rootDir3);

        final Method fc2GetTempDir = ReflectionUtil.getMethod(fileCacheClazz2 , "getTempDir");
        final Method fc3GetTempDir = ReflectionUtil.getMethod(fileCacheClazz3 , "getTempDir");
        final Object tempDir2 = ReflectionUtil.callMethod(fileCache2, fc2GetTempDir);
        final Object tempDir3 = ReflectionUtil.callMethod(fileCache3, fc3GetTempDir);

        if(shallBeSame) {
            Assert.assertTrue("file caches are not equal", tempDir2.equals(tempDir3));
        } else {
            Assert.assertFalse("file caches are equal", tempDir2.equals(tempDir3));
        }
    }

    @BeforeClass
    public static void init() {
        // may already been initialized by other test
        // Assert.assertFalse(TempCacheReg.isTempFileCacheUsed());
        PlatformRuntime.initSingleton();
        Assert.assertTrue(TempFileCache.initSingleton());
        Assert.assertTrue(TempCacheReg.isTempFileCacheUsed());

        fileCache = new TempFileCache();
        Assert.assertTrue(fileCache.isValid(false));
        System.err.println("tmp dir: "+fileCache.getTempDir());
    }

    @Test
    public void testTempFileCache01FileExist() throws IOException {
        Assert.assertTrue(fileCache.getTempDir().exists());
        Assert.assertTrue(fileCache.getTempDir().isDirectory());
    }

    @Test
    public void testTempFileCache02Instances() throws IOException {
        final TempFileCache fileCache2 = new TempFileCache();
        final TempFileCache fileCache3 = new TempFileCache();

        assertTempFileCachesIndividualInstances(false, fileCache2, fileCache3);
    }

    @Test
    public void testJarUtil01a() throws IOException, IllegalArgumentException, URISyntaxException {
        if(AndroidVersion.isAvailable) { System.err.println("n/a on Android"); return; }
        final JarFile jarFile = JarUtil.getJarFile(PlatformProps.class.getName(), this.getClass().getClassLoader());
        Assert.assertNotNull(jarFile);
        JarUtil.extract(fileCache.getTempDir(), null, jarFile, null, false, true, true);
        File f = new File(fileCache.getTempDir(), "META-INF/MANIFEST.MF");
        Assert.assertTrue(f.exists());
        f = new File(fileCache.getTempDir(), IOUtil.getClassFileName(PlatformProps.class.getName()));
        Assert.assertTrue(f.exists());
    }

    @Test
    public void testJarUtil01b() throws IOException {
        if(AndroidVersion.isAvailable) { System.err.println("n/a on Android"); return; }
        File f = new File(fileCache.getTempDir(), "META-INF/MANIFEST.MF");
        Assert.assertTrue(f.exists());
        f = new File(fileCache.getTempDir(), IOUtil.getClassFileName(PlatformProps.class.getName()));
        Assert.assertTrue(f.exists());
    }

    @Test
    public void testTempJarCache00Init() throws IOException {
        // may already been initialized by other test
        // Assert.assertFalse(TempCacheReg.isTempJarCacheUsed());
        // Assert.assertFalse(TempJarCache.isInitialized());
        Assert.assertTrue(TempJarCache.initSingleton());
        Assert.assertTrue(TempCacheReg.isTempJarCacheUsed(false));
        Assert.assertTrue(TempJarCache.isInitialized(false));
    }

    @Test
    public void testTempJarCache01LoadAllTestManifestAndClass() throws IOException, SecurityException, IllegalArgumentException, URISyntaxException {
        if(AndroidVersion.isAvailable) { System.err.println("n/a on Android"); return; }

        final ClassLoader cl = getClass().getClassLoader();
        TempJarCache.addAll(PlatformProps.class, JarUtil.getJarFileUri(PlatformProps.class.getName(), cl));

        File f0 = new File(TempJarCache.getTempFileCache().getTempDir(), "META-INF/MANIFEST.MF");
        Assert.assertTrue(f0.exists());

        File f1 = new File(TempJarCache.findResource("META-INF/MANIFEST.MF"));
        Assert.assertTrue(f1.exists());
        Assert.assertEquals(f0, f1);

        f0 = new File(TempJarCache.getTempFileCache().getTempDir(), IOUtil.getClassFileName(PlatformProps.class.getName()));
        Assert.assertTrue(f0.exists());

        f1 = new File(TempJarCache.findResource(IOUtil.getClassFileName(PlatformProps.class.getName())));
        Assert.assertTrue(f1.exists());
        Assert.assertEquals(f0, f1);
    }

    @Test
    public void testTempJarCache02AddNativeLibs() throws IOException, IllegalArgumentException, URISyntaxException {
        if(AndroidVersion.isAvailable) { System.err.println("n/a on Android"); return; }
        // FIXME ?? final Uri.Encoded nativeJarName = Uri.Encoded.cast("jaulib_jni-natives-"+PlatformProps.os_and_arch+".jar");
        final Uri.Encoded nativeJarName = Uri.Encoded.cast("jaulib-fat.jar");
        final String libBaseName = "jaulib_jni_jni";
        final ClassLoader cl = getClass().getClassLoader();

        final Uri jarUri = JarUtil.getJarUri(TempJarCache.class.getName(), cl);
        Assert.assertNotNull(jarUri);
        System.err.println("1 - jarUri:");
        URIDumpUtil.showUri(jarUri);

        final Uri jarFileUri = jarUri.getContainedUri();
        Assert.assertNotNull(jarFileUri);
        System.err.println("2 - jarFileUri:");
        URIDumpUtil.showUri(jarFileUri);

        final Uri jarFileDir = jarFileUri.getParent();
        Assert.assertNotNull(jarFileDir);
        System.err.println("3 - jarFileDir:");
        URIDumpUtil.showUri(jarFileDir);

        final Uri nativeJarURI = JarUtil.getJarFileUri(jarFileDir, nativeJarName);
        System.err.println("4 - nativeJarURI:");
        URIDumpUtil.showUri(nativeJarURI);

        TempJarCache.addNativeLibs(TempJarCache.class, nativeJarURI, null /* nativeLibraryPath */);
        final String libFullPath = TempJarCache.findLibrary(libBaseName);
        Assert.assertNotNull(libFullPath);
        Assert.assertEquals(libBaseName, JNILibrary.isValidNativeLibraryName(libFullPath, true));
        final File f = new File(libFullPath);
        Assert.assertTrue(f.exists());
    }

    @Test
    public void testTempJarCache04aSameClassLoader() throws IOException {
        assertTempFileCachesIndividualInstances(true, TempJarCache.getTempFileCache(), TempJarCache.getTempFileCache());

        final ClassLoader cl = getClass().getClassLoader();
        final TempFileCache fileCache2 = (TempFileCache) ReflectionUtil.callStaticMethod(TempJarCache.class.getName(), "getTempFileCache", null, null, cl);
        final TempFileCache fileCache3 = (TempFileCache) ReflectionUtil.callStaticMethod(TempJarCache.class.getName(), "getTempFileCache", null, null, cl);
        assertTempFileCachesIndividualInstances(true, fileCache2, fileCache3);
    }

    @Test
    public void testTempJarCache04bDiffClassLoader() throws IOException, IllegalArgumentException, URISyntaxException {
        if(AndroidVersion.isAvailable) { System.err.println("n/a on Android"); return; }
        final URL[] urls = new URL[] { JarUtil.getJarFileUri(TempJarCache.class.getName(), getClass().getClassLoader()).toURL() };
        System.err.println("url: "+urls[0]);
        final ClassLoader cl2 = new TestClassLoader(urls, null);
        final ClassLoader cl3 = new TestClassLoader(urls, null);

        Assert.assertFalse(( (Boolean) ReflectionUtil.callStaticMethod(TempJarCache.class.getName(), "isInitialized", new Class<?>[] { Boolean.TYPE }, new Object[] { Boolean.FALSE }, cl2)
                           ).booleanValue());
        Assert.assertFalse(( (Boolean) ReflectionUtil.callStaticMethod(TempJarCache.class.getName(), "isInitialized", new Class<?>[] { Boolean.TYPE }, new Object[] { Boolean.FALSE }, cl3)
                           ).booleanValue());
        Assert.assertTrue(( (Boolean) ReflectionUtil.callStaticMethod(TempJarCache.class.getName(), "initSingleton", null, null, cl2)
                           ).booleanValue());
        Assert.assertTrue(( (Boolean) ReflectionUtil.callStaticMethod(TempJarCache.class.getName(), "initSingleton", null, null, cl3)
                           ).booleanValue());
        Assert.assertTrue(( (Boolean) ReflectionUtil.callStaticMethod(TempJarCache.class.getName(), "isInitialized", new Class<?>[] { Boolean.TYPE }, new Object[] { Boolean.FALSE }, cl2)
                           ).booleanValue());
        Assert.assertTrue(( (Boolean) ReflectionUtil.callStaticMethod(TempJarCache.class.getName(), "isInitialized", new Class<?>[] { Boolean.TYPE }, new Object[] { Boolean.FALSE }, cl3)
                           ).booleanValue());

        final Object fileCache2 = ReflectionUtil.callStaticMethod(TempJarCache.class.getName(), "getTempFileCache", null, null, cl2);
        final Object fileCache3 = ReflectionUtil.callStaticMethod(TempJarCache.class.getName(), "getTempFileCache", null, null, cl3);

        assertTempFileCachesIndividualInstances(false, fileCache2, cl2, fileCache3, cl3);
    }

    public static void main(final String args[]) throws IOException {
        final String tstname = TestTempJarCache.class.getName();
        org.junit.runner.JUnitCore.main(tstname);
    }

}
