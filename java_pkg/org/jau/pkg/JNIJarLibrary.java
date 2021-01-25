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
import java.net.URISyntaxException;
import java.net.URL;
import java.util.Arrays;
import java.util.Locale;

import org.jau.net.Uri;
import org.jau.pkg.cache.TempJarCache;
import org.jau.sys.JNILibrary;
import org.jau.sys.PlatformProps;

/**
 * Static JNI Native Libraries handler including functionality for native JAR files using {@link JarUtil} and {@link TempJarCache}.
 */
public class JNIJarLibrary extends JNILibrary {
  private static final String nativeJarTagPackage = "jau.nativetag"; // TODO: sync with gluegen-cpptasks-base.xml

  /**
   *
   * @param classFromJavaJar
   * @param classJarUri
   * @param jarBasename jar basename w/ suffix
   * @param nativeJarBasename native jar basename w/ suffix
   * @return
   * @throws IOException
   * @throws SecurityException
   * @throws URISyntaxException
   */
  private static final boolean addNativeJarLibsImpl(final Class<?> classFromJavaJar, final Uri classJarUri,
                                                    final Uri.Encoded jarBasename, final Uri.Encoded nativeJarBasename)
    throws IOException, SecurityException, URISyntaxException
  {
    if (DEBUG) {
        final StringBuilder msg = new StringBuilder();
        msg.append("JNILibrary: addNativeJarLibsImpl(").append(PlatformProps.NEWLINE);
        msg.append("  classFromJavaJar  = ").append(classFromJavaJar).append(PlatformProps.NEWLINE);
        msg.append("  classJarURI       = ").append(classJarUri).append(PlatformProps.NEWLINE);
        msg.append("  jarBasename       = ").append(jarBasename).append(PlatformProps.NEWLINE);
        msg.append("  os.and.arch       = ").append(PlatformProps.os_and_arch).append(PlatformProps.NEWLINE);
        msg.append("  nativeJarBasename = ").append(nativeJarBasename).append(PlatformProps.NEWLINE);
        msg.append(")");
        System.err.println(msg.toString());
    }
    final long t0 = PERF ? System.currentTimeMillis() : 0; // 'Platform.currentTimeMillis()' not yet available!

    boolean ok = false;

    final Uri jarSubURI = classJarUri.getContainedUri();
    if (null == jarSubURI) {
        throw new IllegalArgumentException("JarSubURI is null of: "+classJarUri);
    }

    final Uri jarSubUriRoot = jarSubURI.getDirectory();

    if (DEBUG) {
        System.err.printf("JNILibrary: addNativeJarLibsImpl: initial: %s -> %s%n", jarSubURI, jarSubUriRoot);
    }

    final String nativeLibraryPath = String.format((Locale)null, "natives/%s/", PlatformProps.os_and_arch);
    if (DEBUG) {
        System.err.printf("JNILibrary: addNativeJarLibsImpl: nativeLibraryPath: %s%n", nativeLibraryPath);
    }
    {
        // Attempt-1 a 'one slim native jar file' per 'os.and.arch' layout
        // with native platform libraries under 'natives/os.and.arch'!
        final Uri nativeJarURI = JarUtil.getJarFileUri( jarSubUriRoot.getEncoded().concat(nativeJarBasename) );

        if (DEBUG) {
            System.err.printf("JNILibrary: addNativeJarLibsImpl: module: %s -> %s%n", nativeJarBasename, nativeJarURI);
        }

        try {
            ok = TempJarCache.addNativeLibs(classFromJavaJar, nativeJarURI, nativeLibraryPath);
        } catch(final Exception e) {
            if(DEBUG) {
                System.err.printf("JNILibrary: addNativeJarLibsImpl: Caught %s%n", e.getMessage());
                e.printStackTrace();
            }
        }
    }
    if (!ok) {
        final ClassLoader cl = classFromJavaJar.getClassLoader();
        {
            // Attempt-2 a 'one big-fat jar file' layout, containing java classes
            // and all native platform libraries under 'natives/os.and.arch' per platform!
            final URL nativeLibraryURI = cl.getResource(nativeLibraryPath);
            if (null != nativeLibraryURI) {
                final Uri nativeJarURI = JarUtil.getJarFileUri( jarSubUriRoot.getEncoded().concat(jarBasename) );
                try {
                    if( TempJarCache.addNativeLibs(classFromJavaJar, nativeJarURI, nativeLibraryPath) ) {
                        ok = true;
                        if (DEBUG) {
                            System.err.printf("JNILibrary: addNativeJarLibsImpl: fat: %s -> %s%n", jarBasename, nativeJarURI);
                        }
                    }
                } catch(final Exception e) {
                    if(DEBUG) {
                        System.err.printf("JNILibrary: addNativeJarLibsImpl: Caught %s%n", e.getMessage());
                        e.printStackTrace();
                    }
                }
            }
        }
        if (!ok) {
            // Attempt-3 to find via ClassLoader and Native-Jar-Tag,
            // assuming one slim native jar file per 'os.and.arch'
            // and native platform libraries under 'natives/os.and.arch'!
            final String moduleName;
            {
                final String packageName = classFromJavaJar.getPackage().getName();
                final int idx = packageName.lastIndexOf('.');
                if( 0 <= idx ) {
                    moduleName = packageName.substring(idx+1);
                } else {
                    moduleName = packageName;
                }
            }
            final String os_and_arch_dot = PlatformProps.os_and_arch.replace('-', '.');
            final String nativeJarTagClassName = nativeJarTagPackage + "." + moduleName + "." + os_and_arch_dot + ".TAG"; // TODO: sync with gluegen-cpptasks-base.xml
            try {
                if(DEBUG) {
                    System.err.printf("JNILibrary: addNativeJarLibsImpl: ClassLoader/TAG: Locating module %s, os.and.arch %s: %s%n",
                            moduleName, os_and_arch_dot, nativeJarTagClassName);
                }
                final Uri nativeJarTagClassJarURI = JarUtil.getJarUri(nativeJarTagClassName, cl);
                if (DEBUG) {
                    System.err.printf("JNILibrary: addNativeJarLibsImpl: ClassLoader/TAG: %s -> %s%n", nativeJarTagClassName, nativeJarTagClassJarURI);
                }
                ok = TempJarCache.addNativeLibs(classFromJavaJar, nativeJarTagClassJarURI, nativeLibraryPath);
            } catch (final Exception e ) {
                if(DEBUG) {
                    System.err.printf("JNILibrary: addNativeJarLibsImpl: Caught %s%n", e.getMessage());
                    e.printStackTrace();
                }
            }
        }
    }

    if (DEBUG || PERF) {
        final long tNow = System.currentTimeMillis() - t0;
        final long tTotal, tCount;
        synchronized(perfSync) {
            tCount = perfCount+1;
            tTotal = perfTotal + tNow;
            perfTotal = tTotal;
            perfCount = tCount;
        }
        final double tAvrg = tTotal / (double)tCount;
        System.err.printf("JNILibrary: addNativeJarLibsImpl.X: %s / %s -> ok: %b; duration: now %d ms, total %d ms (count %d, avrg %.3f ms)%n",
                          jarBasename, nativeJarBasename, ok, tNow, tTotal, tCount, tAvrg);
    }
    return ok;
  }

  /**
   * Loads and adds a JAR file's native library to the TempJarCache.<br>
   * The native library JAR file's URI is derived as follows:
   * <ul>
   *   <li> [1] <code>GLProfile.class</code> -> </li>
   *   <li> [2] <code>http://lala/gluegen-rt.jar</code> -> </li>
   *   <li> [3] <code>http://lala/gluegen-rt</code> -> </li>
   *   <li> [4] <code>http://lala/gluegen-rt-natives-'os.and.arch'.jar</code> </li>
   * </ul>
   * Where:
   * <ul>
   *   <li> [1] is one of <code>classesFromJavaJars</code></li>
   *   <li> [2] is it's complete URI</li>
   *   <li> [3] is it's <i>base URI</i></li>
   *   <li> [4] is the derived native JAR filename</li>
   * </ul>
   * <p>
   * Generic description:
   * <pre>
       final Class<?>[] classesFromJavaJars = new Class<?>[] { Class1.class, Class2.class };
       JNILibrary.addNativeJarLibs(classesFromJavaJars, "-all");
   * </pre>
   * If <code>Class1.class</code> is contained in a JAR file which name includes <code>singleJarMarker</code>, here <i>-all</i>,
   * implementation will attempt to resolve the native JAR file as follows:
   * <ul>
   *   <li><i>ClassJar-all</i>.jar to <i>ClassJar-all</i>-natives-<i>os.and.arch</i>.jar</li>
   * </ul>
   * Otherwise the native JAR files will be resolved for each class's JAR file:
   * <ul>
   *   <li><i>Class1Jar</i>.jar to <i>Class1Jar</i>-natives-<i>os.and.arch</i>.jar</li>
   *   <li><i>Class2Jar</i>.jar to <i>Class2Jar</i>-natives-<i>os.and.arch</i>.jar</li>
   * </ul>
   * </p>
   * <p>
   * Examples:
   * </p>
   * <p>
   * JOCL:
   * <pre>
        // only: jocl.jar -> jocl-natives-<i>os.and.arch</i>.jar
        addNativeJarLibs(new Class<?>[] { JOCLJNILibrary.class }, null, null );
   * </pre>
   * </p>
   * <p>
   * JOGL:
   * <pre>
       final ClassLoader cl = GLProfile.class.getClassLoader();
       // jogl-all.jar         -> jogl-all-natives-<i>os.and.arch</i>.jar
       // jogl-all-noawt.jar   -> jogl-all-noawt-natives-<i>os.and.arch</i>.jar
       // jogl-all-mobile.jar  -> jogl-all-mobile-natives-<i>os.and.arch</i>.jar
       // jogl-all-android.jar -> jogl-all-android-natives-<i>os.and.arch</i>.jar
       // nativewindow.jar     -> nativewindow-natives-<i>os.and.arch</i>.jar
       // jogl.jar             -> jogl-natives-<i>os.and.arch</i>.jar
       // newt.jar             -> newt-natives-<i>os.and.arch</i>.jar (if available)
       final String newtFactoryClassName = "com.jogamp.newt.NewtFactory";
       final Class<?>[] classesFromJavaJars = new Class<?>[] { NWJNILibrary.class, GLProfile.class, null };
       if( ReflectionUtil.isClassAvailable(newtFactoryClassName, cl) ) {
           classesFromJavaJars[2] = ReflectionUtil.getClass(newtFactoryClassName, false, cl);
       }
       JNILibrary.addNativeJarLibs(classesFromJavaJars, "-all");
   * </pre>
   * </p>
   *
   * @param classesFromJavaJars For each given Class, load the native library JAR.
   * @param singleJarMarker Optional string marker like "-all" to identify the single 'all-in-one' JAR file
   *                        after which processing of the class array shall stop.
   *
   * @return true if either the 'all-in-one' native JAR or all native JARs loaded successful or were loaded already,
   *         false in case of an error
   */
  public static boolean addNativeJarLibs(final Class<?>[] classesFromJavaJars, final String singleJarMarker) {
    if(DEBUG) {
        final StringBuilder msg = new StringBuilder();
        msg.append("JNILibrary: addNativeJarLibs(").append(PlatformProps.NEWLINE);
        msg.append("  classesFromJavaJars   = ").append(Arrays.asList(classesFromJavaJars)).append(PlatformProps.NEWLINE);
        msg.append("  singleJarMarker       = ").append(singleJarMarker).append(PlatformProps.NEWLINE);
        msg.append(")");
        System.err.println(msg.toString());
    }

    boolean ok = false;
    if ( TempJarCache.isInitialized(true) ) {
        ok = addNativeJarLibsWithTempJarCache(classesFromJavaJars, singleJarMarker);
    } else if(DEBUG) {
        System.err.println("JNILibrary: addNativeJarLibs0: disabled due to uninitialized TempJarCache");
    }
    return ok;
  }

  private static boolean addNativeJarLibsWithTempJarCache(final Class<?>[] classesFromJavaJars, final String singleJarMarker) {
      boolean ok;
      int count = 0;
      try {
          boolean done = false;
          ok = true;

          for (int i = 0; i < classesFromJavaJars.length; ++i) {
              final Class<?> c = classesFromJavaJars[i];
              if (c == null) {
                  continue;
              }

              final ClassLoader cl = c.getClassLoader();
              final Uri classJarURI = JarUtil.getJarUri(c.getName(), cl);
              final Uri.Encoded jarName = JarUtil.getJarBasename(classJarURI);

              if (jarName == null) {
                  continue;
              }

              final Uri.Encoded jarBasename = jarName.substring(0, jarName.indexOf(".jar"));

              if(DEBUG) {
                  System.err.printf("JNILibrary: jarBasename: %s%n", jarBasename);
              }

              /**
               * If a jar marker was specified, and the basename contains the
               * marker, we're done.
               */

              if (singleJarMarker != null) {
                  if (jarBasename.indexOf(singleJarMarker) >= 0) {
                      done = true;
                  }
              }

              final Uri.Encoded nativeJarBasename =
                      Uri.Encoded.cast( String.format((Locale)null, "%s-natives-%s.jar", jarBasename.get(), PlatformProps.os_and_arch) );

              ok = JNIJarLibrary.addNativeJarLibsImpl(c, classJarURI, jarName, nativeJarBasename);
              if (ok) {
                  count++;
              }
              if (DEBUG && done) {
                  System.err.printf("JNILibrary: addNativeJarLibs0: done: %s%n", jarBasename);
              }
          }
      } catch (final Exception x) {
          System.err.printf("JNILibrary: Caught %s: %s%n", x.getClass().getSimpleName(), x.getMessage());
          if(DEBUG) {
              x.printStackTrace();
          }
          ok = false;
      }
      if(DEBUG) {
          System.err.printf("JNILibrary: addNativeJarLibsWhenInitialized: count %d, ok %b%n", count, ok);
      }
      return ok;
  }

}
