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

import java.io.File;
import java.net.URISyntaxException;
import java.security.PrivilegedAction;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.StringTokenizer;

import org.jau.io.IOUtil;
import org.jau.lang.ReflectionUtil;
import org.jau.lang.UnsafeUtil;

/**
 * Static JNI Native Libraries handler.
 */
public class JNILibrary {
  public static final boolean DEBUG;
  protected static final boolean PERF;

  private static final String[] prefixes;
  private static final String[] suffixes;
  private static final boolean isOSX;

  private static final String tjc_name = "org.jau.pkg.cache.TempJarCache";
  private static final ReflectionUtil.MethodAccessor tjcIsInit;
  private static final ReflectionUtil.MethodAccessor tjcFindLib;
  private static final boolean tjcAvail;

  static {
      Debug.initSingleton();
      DEBUG = Debug.debug("JNILibrary");
      PERF = DEBUG || PropertyAccess.isPropertyDefined("jau.debug.JNILibrary.Perf", true);

      switch (PlatformProps.OS) {
          case WINDOWS:
              prefixes = new String[] { "" };
              suffixes = new String[] { ".dll" };
              isOSX = false;
              break;

          case MACOS:
          case IOS:
              prefixes = new String[] { "lib" };
              suffixes = new String[] { ".dylib" };
              isOSX = true;
              break;

              /*
          case ANDROID:
          case FREEBSD:
          case SUNOS:
          case HPUX:
          case OPENKODE:
          case LINUX: */
          default:
              prefixes = new String[] { "lib" };
              suffixes = new String[] { ".so" };
              isOSX = false;
              break;
      }

      Class<?> tjc = null;
      try {
          tjc = ReflectionUtil.getClass(tjc_name, false /* initializeClazz */, JNILibrary.class.getClassLoader());
      } catch (final Throwable t) {}
      if( null != tjc ) {
          tjcIsInit = new ReflectionUtil.MethodAccessor(tjc, "isInitialized", boolean.class);
          tjcFindLib = new ReflectionUtil.MethodAccessor(tjc, "findLibrary", String.class);
          tjcAvail = tjcIsInit.available() && tjcFindLib.available();
          if( DEBUG ) {
              System.err.println("JNILibrary: Available <"+tjc_name+">, fully avail "+tjcAvail+" (a "+tjcIsInit.available()+", b "+tjcFindLib.available()+")");
          }
      } else {
          tjcIsInit = null;
          tjcFindLib = null;
          tjcAvail = false;
          if( DEBUG ) {
              System.err.println("JNILibrary: Not available <"+tjc_name+">");
          }
      }
  }

  protected static final Object perfSync = new Object();
  protected static long perfTotal = 0;
  protected static long perfCount = 0;

  private static final HashSet<String> loaded = new HashSet<String>();

  public static synchronized boolean isLoaded(final String libName) {
    return loaded.contains(libName);
  }

  private static synchronized void addLoaded(final String libName) {
    loaded.add(libName);
    if(DEBUG) {
        System.err.println("JNILibrary: Loaded Native Library: "+libName);
    }
  }

  /**
   * Loads the library specified by libname.<br>
   * The implementation should ignore, if the library has been loaded already.<br>
   * @param libname the library to load
   * @param ignoreError if true, errors during loading the library should be ignored
   * @param cl optional ClassLoader, used to locate the library
   * @return true if library loaded successful
   */
  public static synchronized boolean loadLibrary(final String libname, final boolean ignoreError, final ClassLoader cl)
          throws SecurityException, UnsatisfiedLinkError
  {
      boolean res = true;
      if(!isLoaded(libname)) {
          try {
              loadLibraryImpl(libname, cl);
              addLoaded(libname);
              if(DEBUG) {
                  System.err.println("JNILibrary: loaded "+libname);
              }
          } catch (final UnsatisfiedLinkError e) {
              res = false;
              if(DEBUG) {
                  e.printStackTrace();
              }
              if (!ignoreError && e.getMessage().indexOf("already loaded") < 0) {
                  throw e;
              }
          }
      }
      return res;
  }

  /**
   * Loads the library specified by libname.<br>
   * Optionally preloads the libraries specified by preload.<br>
   * The implementation should ignore, if any library has been loaded already.<br>
   * @param libname the library to load
   * @param preload the libraries to load before loading the main library if not null
   * @param preloadIgnoreError if true, errors during loading the preload-libraries should be ignored
   * @param cl optional ClassLoader, used to locate the library
   */
  public static synchronized void loadLibrary(final String libname, final String[] preload, final boolean preloadIgnoreError, final ClassLoader cl)
          throws SecurityException, UnsatisfiedLinkError
  {
      if(!isLoaded(libname)) {
          if (null!=preload) {
              for (int i=0; i<preload.length; i++) {
                  loadLibrary(preload[i], preloadIgnoreError, cl);
              }
          }
          loadLibrary(libname, false, cl);
      }
  }

  private static void loadLibraryImpl(final String libraryName, final ClassLoader cl) throws SecurityException, UnsatisfiedLinkError {
      // Note: special-casing JAWT which is built in to the JDK
      int mode = 0; // 2 - System.load( TempJarCache ), 3 - System.loadLibrary( name ), 4 - System.load( enumLibNames )
      // System.err.println("sun.boot.library.path=" + Debug.getProperty("sun.boot.library.path", false));
      final String libraryPath = findLibrary(libraryName, cl); // implicit TempJarCache usage if used/initialized
      if(DEBUG) {
          System.err.println("JNILibrary: loadLibraryImpl("+libraryName+"), TempJarCache: "+libraryPath);
      }
      if(null != libraryPath) {
          if(DEBUG) {
              System.err.println("JNILibrary: System.load("+libraryPath+") - mode 2");
          }
          System.load(libraryPath);
          mode = 2;
      } else {
          if(DEBUG) {
              System.err.println("JNILibrary: System.loadLibrary("+libraryName+") - mode 3");
          }
          try {
              System.loadLibrary(libraryName);
              mode = 3;
          } catch (final UnsatisfiedLinkError ex1) {
              if(DEBUG) {
                  System.err.println("ERROR (retry w/ enumLibPath) - "+ex1.getMessage());
              }
              final List<String> possiblePaths = enumerateLibraryPaths(libraryName,
                      false /* searchSystemPath */, false /* searchSystemPathFirst */, cl);
              // Iterate down these and see which one if any we can actually find.
              for (final Iterator<String> iter = possiblePaths.iterator(); 0 == mode && iter.hasNext(); ) {
                  final String path = iter.next();
                  if (DEBUG) {
                      System.err.println("JNILibrary: System.load("+path+") - mode 4");
                  }
                  try {
                      System.load(path);
                      mode = 4;
                  } catch (final UnsatisfiedLinkError ex2) {
                      if(DEBUG) {
                          System.err.println("n/a - "+ex2.getMessage());
                      }
                      if(!iter.hasNext()) {
                          throw ex2;
                      }
                  }
              }
          }
      }
      if(DEBUG) {
          System.err.println("JNILibrary: loadLibraryImpl("+libraryName+"): OK - mode "+mode);
      }
  }

  public static final String findLibrary(final String libName, final ClassLoader loader) {
      String res = null;
      if( tjcAvail ) { // TempJarCache ..
          final boolean _tjcIsInit = tjcIsInit.callStaticMethod(true);
          if( _tjcIsInit ) {
              res = tjcFindLib.callStaticMethod(libName);
              if (DEBUG) {
                  System.err.println("JNILibrary.findLibrary(<"+libName+">) (TempJarCache): "+res);
              }
          }
      }
      return res;
  }

  /**
   * Comparison of prefix and suffix of the given libName's basename
   * is performed case insensitive <br>
   *
   * @param libName the full path library name with prefix and suffix
   * @param isLowerCaseAlready indicates if libName is already lower-case
   *
   * @return basename of libName w/o path, ie. /usr/lib/libDrinkBeer.so -> DrinkBeer on Unix systems, but null on Windows.
   */
  public static final String isValidNativeLibraryName(final String libName, final boolean isLowerCaseAlready) {
      final String libBaseName;
      try {
          libBaseName = IOUtil.getBasename(libName);
      } catch (final URISyntaxException uriEx) {
          throw new IllegalArgumentException(uriEx);
      }
      final String libBaseNameLC = isLowerCaseAlready ? libBaseName : libBaseName.toLowerCase();
      int prefixIdx = -1;
      for(int i=0; i < prefixes.length && 0 > prefixIdx; i++) {
          if ( libBaseNameLC.startsWith( prefixes[i] ) ) {
              prefixIdx = i;
          }
      }
      if( 0 <= prefixIdx ) {
          for(int i=0; i < suffixes.length; i++) {
              if ( libBaseNameLC.endsWith( suffixes[i] ) ) {
                  final int s = prefixes[prefixIdx].length();
                  final int e = suffixes[i].length();
                  return libBaseName.substring(s, libBaseName.length()-e);
              }
          }
      }
      return null;
  }

  /** Given the base library names (no prefixes/suffixes) for the
      various platforms, enumerate the possible locations and names of
      the indicated native library on the system using the system path. */
  public static final List<String> enumerateLibraryPaths(final String libName,
                                                   final boolean searchSystemPath,
                                                   final boolean searchSystemPathFirst,
                                                   final ClassLoader loader) {
      return enumerateLibraryPaths(libName, libName, libName,
                                  searchSystemPath, searchSystemPathFirst,
                                  loader);
  }

  /** Given the base library names (no prefixes/suffixes) for the
      various platforms, enumerate the possible locations and names of
      the indicated native library on the system using the system path. */
  public static final List<String> enumerateLibraryPaths(final String windowsLibName,
                                                   final String unixLibName,
                                                   final String macOSXLibName,
                                                   final boolean searchSystemPath,
                                                   final boolean searchSystemPathFirst,
                                                   final ClassLoader loader) {
    final List<String> paths = new ArrayList<String>();
    final String libName = selectName(windowsLibName, unixLibName, macOSXLibName);
    if (libName == null) {
      return paths;
    }

    // Allow user's full path specification to override our building of paths
    final File file = new File(libName);
    if (file.isAbsolute()) {
        paths.add(libName);
        return paths;
    }

    final String[] baseNames = buildNames(libName);

    if( searchSystemPath && searchSystemPathFirst ) {
        // Add just the library names to use the OS's search algorithm
        for (int i = 0; i < baseNames.length; i++) {
            paths.add(baseNames[i]);
        }
        // Add probable Mac OS X-specific paths
        if ( isOSX ) {
            // Add historical location
            addPaths("/Library/Frameworks/" + libName + ".framework", baseNames, paths);
            // Add current location
            addPaths("/System/Library/Frameworks/" + libName + ".framework", baseNames, paths);
        }
    }

    final String clPath = findLibrary(libName, loader);
    if (clPath != null) {
      paths.add(clPath);
    }

    // Add entries from java.library.path
    final String[] javaLibraryPaths =
      UnsafeUtil.doPrivileged(new PrivilegedAction<String[]>() {
          @Override
          public String[] run() {
            int count = 0;
            final String usrPath = System.getProperty("java.library.path");
            if(null != usrPath) {
                count++;
            }
            final String sysPath;
            if( searchSystemPath ) {
                sysPath = System.getProperty("sun.boot.library.path");
                if(null != sysPath) {
                    count++;
                }
            } else {
                sysPath = null;
            }
            final String[] res = new String[count];
            int i=0;
            if( null != sysPath && searchSystemPathFirst ) {
                res[i++] = sysPath;
            }
            if(null != usrPath) {
                res[i++] = usrPath;
            }
            if( null != sysPath && !searchSystemPathFirst ) {
                res[i++] = sysPath;
            }
            return res;
          }
        });
    if ( null != javaLibraryPaths ) {
        for( int i=0; i < javaLibraryPaths.length; i++ ) {
            final StringTokenizer tokenizer = new StringTokenizer(javaLibraryPaths[i], File.pathSeparator);
            while (tokenizer.hasMoreTokens()) {
                addPaths(tokenizer.nextToken(), baseNames, paths);
            }
        }
    }

    // Add current working directory
    final String userDir =
      UnsafeUtil.doPrivileged(new PrivilegedAction<String>() {
          @Override
          public String run() {
            return System.getProperty("user.dir");
          }
        });
    addPaths(userDir, baseNames, paths);

    // Add current working directory + natives/os-arch/ + library names
    // to handle Bug 1145 cc1 using an unpacked fat-jar
    addPaths(userDir+File.separator+"natives"+File.separator+PlatformProps.os_and_arch+File.separator, baseNames, paths);

    if( searchSystemPath && !searchSystemPathFirst ) {
        // Add just the library names to use the OS's search algorithm
        for (int i = 0; i < baseNames.length; i++) {
            paths.add(baseNames[i]);
        }
        // Add probable Mac OS X-specific paths
        if ( isOSX ) {
            // Add historical location
            addPaths("/Library/Frameworks/" + libName + ".Framework", baseNames, paths);
            // Add current location
            addPaths("/System/Library/Frameworks/" + libName + ".Framework", baseNames, paths);
        }
    }

    return paths;
  }


  private static final String selectName(final String windowsLibName,
                                   final String unixLibName,
                                   final String macOSXLibName) {
    switch (PlatformProps.OS) {
      case WINDOWS:
        return windowsLibName;

      case MACOS:
      case IOS:
        return macOSXLibName;

      default:
        return unixLibName;
    }
  }

  private static final String[] buildNames(final String libName) {
      // If the library name already has the prefix / suffix added
      // (principally because we want to force a version number on Unix
      // operating systems) then just return the library name.
      final String libBaseNameLC;
      try {
          libBaseNameLC = IOUtil.getBasename(libName).toLowerCase();
      } catch (final URISyntaxException uriEx) {
          throw new IllegalArgumentException(uriEx);
      }

      int prefixIdx = -1;
      for(int i=0; i<prefixes.length && 0 > prefixIdx; i++) {
          if (libBaseNameLC.startsWith(prefixes[i])) {
              prefixIdx = i;
          }
      }
      if( 0 <= prefixIdx ) {
          for(int i=0; i<suffixes.length; i++) {
              if (libBaseNameLC.endsWith(suffixes[i])) {
                  return new String[] { libName };
              }
          }
          int suffixIdx = -1;
          for(int i=0; i<suffixes.length && 0 > suffixIdx; i++) {
              suffixIdx = libBaseNameLC.indexOf(suffixes[i]);
          }
          boolean ok = true;
          if (suffixIdx >= 0) {
              // Check to see if everything after it is a Unix version number
              for (int i = suffixIdx + suffixes[0].length();
                      i < libName.length();
                      i++) {
                  final char c = libName.charAt(i);
                  if (!(c == '.' || (c >= '0' && c <= '9'))) {
                      ok = false;
                      break;
                  }
              }
              if (ok) {
                  return new String[] { libName };
              }
          }
      }

      final String[] res = new String[prefixes.length * suffixes.length + ( isOSX ? 1 : 0 )];
      int idx = 0;
      for (int i = 0; i < prefixes.length; i++) {
          for (int j = 0; j < suffixes.length; j++) {
              res[idx++] = prefixes[i] + libName + suffixes[j];
          }
      }
      if ( isOSX ) {
          // Plain library-base-name in Framework folder
          res[idx++] = libName;
      }
      return res;
  }

  private static final void addPaths(final String path, final String[] baseNames, final List<String> paths) {
    for (int j = 0; j < baseNames.length; j++) {
      paths.add(path + File.separator + baseNames[j]);
    }
  }
}
