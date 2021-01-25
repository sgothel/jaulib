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

package org.jau.pkg;

import java.io.IOException;
import java.net.JarURLConnection;
import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.URLConnection;
import java.net.URLStreamHandler;
import java.util.Enumeration;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

import org.jau.junit.util.JunitTracer;
import org.jau.net.URIDumpUtil;
import org.jau.net.Uri;
import org.jau.pkg.JarUtil;
import org.jau.pkg.cache.TempCacheReg;
import org.jau.pkg.cache.TempFileCache;
import org.jau.pkg.cache.TempJarCache;
import org.jau.sys.AndroidVersion;
import org.jau.sys.PlatformProps;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestJarUtil extends JunitTracer {
    static TempFileCache fileCache;

    @BeforeClass
    public static void init() {
        if(AndroidVersion.isAvailable) {
            // ClassLoader -> JarURL doesn't work w/ Dalvik
            setTestSupported(false);
            // we allow basic TempFileCache initialization (test) ..
        }
        // may already been initialized by other test
        // Assert.assertFalse(TempCacheReg.isTempFileCacheUsed());
        Assert.assertTrue(TempFileCache.initSingleton());
        Assert.assertTrue(TempCacheReg.isTempFileCacheUsed());

        fileCache = new TempFileCache();
        Assert.assertTrue(fileCache.isValid(false));
        System.err.println("tmp dir: "+fileCache.getTempDir());
    }

    static class TestClassLoader extends URLClassLoader {
        public TestClassLoader(final URL[] urls) {
            super(urls);
        }
        public TestClassLoader(final URL[] urls, final ClassLoader parent) {
            super(urls, parent);
        }
    }

    void validateJarFile(final JarFile jarFile) throws IllegalArgumentException, IOException {
        Assert.assertNotNull(jarFile);
        Assert.assertTrue("jarFile has zero entries: "+jarFile, jarFile.size()>0);
        final Enumeration<JarEntry> entries = jarFile.entries();
        System.err.println("Entries of "+jarFile.getName()+": ");
        int i = 0;
        while(entries.hasMoreElements()) {
            System.err.println(i+": "+entries.nextElement().getName());
            i++;
        }
    }

    void validateJarFileURL(final Uri jarFileURI) throws IllegalArgumentException, IOException, URISyntaxException {
        Assert.assertNotNull(jarFileURI);
        final URL jarFileURL = jarFileURI.toURL();
        final URLConnection aURLc = jarFileURL.openConnection();
        Assert.assertTrue("jarFileURI/URL has zero content: "+jarFileURL, aURLc.getContentLength()>0);
        System.err.println("URLConnection: "+aURLc);
        Assert.assertTrue("Not a JarURLConnection: "+aURLc, (aURLc instanceof JarURLConnection) );
        final JarURLConnection jURLc = (JarURLConnection) aURLc;
        final JarFile jarFile = jURLc.getJarFile();
        validateJarFile(jarFile);
    }

    void validateJarUtil(final String expJarName, final String clazzBinName, final ClassLoader cl) throws IllegalArgumentException, IOException, URISyntaxException {
        final Uri.Encoded expJarNameE = Uri.Encoded.cast(expJarName);
        final Uri.Encoded jarName= JarUtil.getJarBasename(clazzBinName, cl);
        Assert.assertNotNull(jarName);
        Assert.assertEquals(expJarNameE, jarName);

        final Uri jarUri = JarUtil.getJarUri(clazzBinName, cl);
        Assert.assertNotNull(jarUri);
        System.err.println("1 - jarUri:");
        URIDumpUtil.showUri(jarUri);

        final Uri jarSubUri = jarUri.getContainedUri();
        Assert.assertNotNull(jarSubUri);
        System.err.println("2 - jarSubUri:");
        URIDumpUtil.showUri(jarSubUri);

        final URL jarSubURL= jarSubUri.toURL();
        final URLConnection urlConn = jarSubURL.openConnection();
        Assert.assertTrue("jarSubURL has zero content: "+jarSubURL, urlConn.getContentLength()>0);
        System.err.println("URLConnection of jarSubURL: "+urlConn);

        final Uri jarFileURL = JarUtil.getJarFileUri(clazzBinName, cl);
        validateJarFileURL(jarFileURL);

        final JarFile jarFile = JarUtil.getJarFile(clazzBinName, cl);
        validateJarFile(jarFile);
    }

    @Test
    public void testJarUtilFlat01() throws IOException, IllegalArgumentException, URISyntaxException {
        System.err.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXX");
        validateJarUtil("jaulib_fat.jar", "org.jau.base.JaulibVersion", this.getClass().getClassLoader());
        System.err.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    }

    // @Test // FIXME: Build ClassInJar0 and corresponding jar files
    public void testJarUtilJarInJar01() throws IOException, ClassNotFoundException, IllegalArgumentException, URISyntaxException {
        System.err.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXX");

        Assert.assertTrue(TempJarCache.initSingleton());
        Assert.assertTrue(TempCacheReg.isTempJarCacheUsed(false));
        Assert.assertTrue(TempJarCache.isInitialized(false));

        final ClassLoader rootCL = this.getClass().getClassLoader();

        // Get containing JAR file "TestJarsInJar.jar" and add it to the TempJarCache
        TempJarCache.addAll(PlatformProps.class, JarUtil.getJarFileUri("ClassInJar0", rootCL));

        // Fetch and load the contained "ClassInJar1.jar"
        final URL ClassInJar1_jarFileURL = JarUtil.getJarFileUri(TempJarCache.getResourceUri("ClassInJar1.jar")).toURL();
        final ClassLoader cl = new URLClassLoader(new URL[] { ClassInJar1_jarFileURL }, rootCL);
        Assert.assertNotNull(cl);
        validateJarUtil("ClassInJar1.jar", "ClassInJar1", cl);
        System.err.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    }

    // @Test // FIXME: Build ClassInJar0 and corresponding jar files
    public void testJarUtilJarInJar02() throws IOException, ClassNotFoundException, IllegalArgumentException, URISyntaxException {
        System.err.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXX");

        Assert.assertTrue(TempJarCache.initSingleton());
        Assert.assertTrue(TempCacheReg.isTempJarCacheUsed(false));
        Assert.assertTrue(TempJarCache.isInitialized(false));

        final ClassLoader rootCL = this.getClass().getClassLoader();

        // Get containing JAR file "TestJarsInJar.jar" and add it to the TempJarCache
        TempJarCache.addAll(PlatformProps.class, JarUtil.getJarFileUri("ClassInJar0", rootCL));

        // Fetch and load the contained "ClassInJar1.jar"
        final URL ClassInJar2_jarFileURL = JarUtil.getJarFileUri(TempJarCache.getResourceUri("sub/ClassInJar2.jar")).toURL();
        final ClassLoader cl = new URLClassLoader(new URL[] { ClassInJar2_jarFileURL }, rootCL);
        Assert.assertNotNull(cl);
        validateJarUtil("ClassInJar2.jar", "ClassInJar2", cl);
        System.err.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    }

    /**
     * Tests JarUtil's ability to resolve non-JAR URLs with a custom resolver. Meant to be used
     * in cases like an OSGi plugin, where all classes are loaded with custom classloaders and
     * therefore return URLs that don't start with "jar:". Adapted from test 02 above.
     * @throws URISyntaxException
     * @throws IllegalArgumentException
     */
    // @Test // FIXME: Build ClassInJar0 and corresponding jar files
    public void testJarUtilJarInJar03() throws IOException, ClassNotFoundException, IllegalArgumentException, URISyntaxException {
        System.err.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXX");

        Assert.assertTrue(TempJarCache.initSingleton());
        Assert.assertTrue(TempCacheReg.isTempJarCacheUsed(false));
        Assert.assertTrue(TempJarCache.isInitialized(false));

        /** This classloader mimics what OSGi's does -- it takes jar: URLs and makes them into bundleresource: URLs
         * where the JAR is not directly accessible anymore. Here I leave the JAR name at the end of the URL so I can
         * retrieve it later in the resolver, but OSGi obscures it completely and returns URLs like
         * "bundleresource:4.fwk1990213994:1/Something.class" where the JAR name not present. */
        class CustomClassLoader extends ClassLoader {
            CustomClassLoader() {
                super(TestJarUtil.this.getClass().getClassLoader());
            }

            /** Override normal method to return un-resolvable URL. */
            @Override
            public URL getResource(final String name) {
                final URL url = super.getResource(name);
                if(url == null)
                    return(null);
                URL urlReturn = null;
                try {
                    // numbers to mimic OSGi -- can be anything
                    urlReturn = new URL("bundleresource", "4.fwk1990213994", 1, url.getFile(),
                        new URLStreamHandler() {
                            @Override
                            protected URLConnection openConnection(final URL u) throws IOException {
                                return null;
                            }
                        });
                } catch(final MalformedURLException e) {
                    // shouldn't happen, since I create the URL correctly above
                    Assert.assertTrue(false);
                }
                return urlReturn;
            }
        };

        /* This resolver converts bundleresource: URLs back into jar: URLs. OSGi does this by consulting
         * opaque bundle data inside its custom classloader to find the stored JAR path; we do it here
         * by simply retrieving the JAR name from where we left it at the end of the URL. */
        JarUtil.setResolver( new JarUtil.Resolver() {
            @Override
            public URL resolve( final URL url ) {
                if( url.getProtocol().equals("bundleresource") ) {
                    try {
                        return new URL( Uri.JAR_SCHEME, "", url.getFile() );
                    } catch(final MalformedURLException e) {
                        return url;
                    }
                } else {
                    return url;
                }
            }
        } );

        final ClassLoader rootCL = new CustomClassLoader();

        // Get containing JAR file "TestJarsInJar.jar" and add it to the TempJarCache
        TempJarCache.addAll(PlatformProps.class, JarUtil.getJarFileUri("ClassInJar0", rootCL));

        // Fetch and load the contained "ClassInJar1.jar"
        final URL ClassInJar2_jarFileURL = JarUtil.getJarFileUri(TempJarCache.getResourceUri("sub/ClassInJar2.jar")).toURL();
        final ClassLoader cl = new URLClassLoader(new URL[] { ClassInJar2_jarFileURL }, rootCL);
        Assert.assertNotNull(cl);
        validateJarUtil("ClassInJar2.jar", "ClassInJar2", cl);
        System.err.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    }

    public static void main(final String args[]) throws IOException {
        final String tstname = TestJarUtil.class.getName();
        org.junit.runner.JUnitCore.main(tstname);
    }

}
