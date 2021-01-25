/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Author: Kenneth Bradley Russell
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2011 Gothel Software e.K.
 * Copyright (c) 2011 JogAmp Community.
 * Copyright (c) 2006 Sun Microsystems, Inc.
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

package org.jau.sys.dl;

import java.util.Iterator;
import java.util.List;

import org.jau.lang.ExceptionUtils;
import org.jau.sys.JNILibrary;
import org.jau.sys.PlatformProps;

import jau.sys.dl.BionicDynamicLinker32bitImpl;
import jau.sys.dl.BionicDynamicLinker64BitImpl;
import jau.sys.dl.MacOSXDynamicLinkerImpl;
import jau.sys.dl.PosixDynamicLinkerImpl;
import jau.sys.dl.WindowsDynamicLinkerImpl;

/** Provides low-level, relatively platform-independent access to
    shared ("native") libraries. The core library routines
    <code>System.load()</code> and <code>System.loadLibrary()</code>
    in general provide suitable functionality for applications using
    native code, but are not flexible enough to support certain kinds
    of glue code generation and deployment strategies. This class
    supports direct linking of native libraries to other shared
    objects not necessarily installed on the system (in particular,
    via the use of dlopen(RTLD_GLOBAL) on Unix platforms) as well as
    manual lookup of function names to support e.g. GlueGen's
    ProcAddressTable glue code generation style without additional
    supporting code needed in the generated library. */

public final class NativeLibrary implements DynamicLookupHelper {
  private final DynamicLinker dynLink;

  // Platform-specific representation for the handle to the open
  // library. This is an HMODULE on Windows and a void* (the result of
  // a dlopen() call) on Unix and Mac OS X platforms.
  private long libraryHandle;

  // May as well keep around the path to the library we opened
  private final String libraryPath;

  private final boolean global;

  // Private constructor to prevent arbitrary instances from floating around
  private NativeLibrary(final DynamicLinker dynLink, final long libraryHandle, final String libraryPath, final boolean global) {
    this.dynLink = dynLink;
    this.libraryHandle = libraryHandle;
    this.libraryPath   = libraryPath;
    this.global        = global;
    if (DEBUG) {
      System.err.println("NativeLibrary.open(): Successfully loaded: " + this);
    }
  }

  @Override
  public final String toString() {
    return "NativeLibrary[" + dynLink.getClass().getSimpleName() + ", " + libraryPath + ", 0x" + Long.toHexString(libraryHandle) + ", global " + global + "]";
  }

  /** Opens the given native library, assuming it has the same base
      name on all platforms.
      <p>
      The {@code searchSystemPath} argument changes the behavior to
      either use the default system path or not at all.
      </p>
      <p>
      Assuming {@code searchSystemPath} is {@code true},
      the {@code searchSystemPathFirst} argument changes the behavior to first
      search the default system path rather than searching it last.
      </p>
   * @param libName library name, with or without prefix and suffix
   * @param searchSystemPath if {@code true} library shall be searched in the system path <i>(default)</i>, otherwise {@code false}.
   * @param searchSystemPathFirst if {@code true} system path shall be searched <i>first</i> <i>(default)</i>, rather than searching it last.
   *                              if {@code searchSystemPath} is {@code false} this argument is ignored.
   * @param loader {@link ClassLoader} to locate the library
   * @param global if {@code true} allows system wide access of the loaded library, otherwise access is restricted to the process.
   * @return {@link NativeLibrary} instance or {@code null} if library could not be loaded.
   * @throws SecurityException if user is not granted access for the named library.
   * @since 2.4.0
   */
  public static final NativeLibrary open(final String libName,
                                         final boolean searchSystemPath,
                                         final boolean searchSystemPathFirst,
                                         final ClassLoader loader, final boolean global) throws SecurityException {
    return open(libName, libName, libName, searchSystemPath, searchSystemPathFirst, loader, global);
  }

  /** Opens the given native library, assuming it has the given base
      names (no "lib" prefix or ".dll/.so/.dylib" suffix) on the
      Windows, Unix and Mac OS X platforms, respectively, and in the
      context of the specified ClassLoader, which is used to help find
      the library in the case of e.g. Java Web Start.
      <p>
      The {@code searchSystemPath} argument changes the behavior to
      either use the default system path or not at all.
      </p>
      <p>
      Assuming {@code searchSystemPath} is {@code true},
      the {@code searchSystemPathFirst} argument changes the behavior to first
      search the default system path rather than searching it last.
      </p>
      Note that we do not currently handle DSO versioning on Unix.
      Experience with JOAL and OpenAL has shown that it is extremely
      problematic to rely on a specific .so version (for one thing,
      ClassLoader.findLibrary on Unix doesn't work with files not
      ending in .so, for example .so.0), and in general if this
      dynamic loading facility is used correctly the version number
      will be irrelevant.
   * @param libName windows library name, with or without prefix and suffix
   * @param unixLibName unix library name, with or without prefix and suffix
   * @param macOSXLibName mac-osx library name, with or without prefix and suffix
   * @param searchSystemPath if {@code true} library shall be searched in the system path <i>(default)</i>, otherwise {@code false}.
   * @param searchSystemPathFirst if {@code true} system path shall be searched <i>first</i> <i>(default)</i>, rather than searching it last.
   *                              if {@code searchSystemPath} is {@code false} this argument is ignored.
   * @param loader {@link ClassLoader} to locate the library
   * @param global if {@code true} allows system wide access of the loaded library, otherwise access is restricted to the process.
   * @return {@link NativeLibrary} instance or {@code null} if library could not be loaded.
   * @throws SecurityException if user is not granted access for the named library.
   */
  public static final NativeLibrary open(final String libName,
                                         final String unixLibName,
                                         final String macOSXLibName,
                                         final boolean searchSystemPath,
                                         final boolean searchSystemPathFirst,
                                         final ClassLoader loader, final boolean global) throws SecurityException {
    final List<String> possiblePaths = JNILibrary.enumerateLibraryPaths(libName,
                                                       unixLibName,
                                                       macOSXLibName,
                                                       searchSystemPath, searchSystemPathFirst,
                                                       loader);
    PlatformProps.initSingleton(); // loads native gluegen_rt library

    final DynamicLinker dynLink = getDynamicLinker();

    // Iterate down these and see which one if any we can actually find.
    for (final Iterator<String> iter = possiblePaths.iterator(); iter.hasNext(); ) {
        final String path = iter.next();
        if (DEBUG) {
            System.err.println("NativeLibrary.open(global "+global+"): Trying to load " + path);
        }
        long res;
        Throwable t = null;
        try {
            if(global) {
                res = dynLink.openLibraryGlobal(path, DEBUG);
            } else {
                res = dynLink.openLibraryLocal(path, DEBUG);
            }
        } catch (final Throwable t1) {
            t = t1;
            res = 0;
        }
        if ( 0 != res ) {
            return new NativeLibrary(dynLink, res, path, global);
        } else if( DEBUG ) {
            if( null != t ) {
                System.err.println("NativeLibrary.open: Caught "+t.getClass().getSimpleName()+": "+t.getMessage());
            }
            String errstr;
            try {
                errstr = dynLink.getLastError();
            } catch (final Throwable t2) { errstr=null; }
            System.err.println("NativeLibrary.open: Last error "+errstr);
            if( null != t ) {
                t.printStackTrace();
            }
        }
    }

    if (DEBUG) {
      System.err.println("NativeLibrary.open(global "+global+"): Did not succeed in loading (" + libName + ", " + unixLibName + ", " + macOSXLibName + ")");
    }

    // For now, just return null to indicate the open operation didn't
    // succeed (could also throw an exception if we could tell which
    // of the openLibrary operations actually failed)
    return null;
  }

  @Override
  public final void claimAllLinkPermission() throws SecurityException {
      dynLink.claimAllLinkPermission();
  }
  @Override
  public final void releaseAllLinkPermission() throws SecurityException {
      dynLink.releaseAllLinkPermission();
  }

  @Override
  public final long dynamicLookupFunction(final String funcName) throws SecurityException {
    if ( 0 == libraryHandle ) {
      throw new RuntimeException("Library is not open");
    }
    return dynLink.lookupSymbol(libraryHandle, funcName);
  }

  @Override
  public final boolean isFunctionAvailable(final String funcName) throws SecurityException {
    if ( 0 == libraryHandle ) {
      throw new RuntimeException("Library is not open");
    }
    return 0 != dynLink.lookupSymbol(libraryHandle, funcName);
  }

  /** Looks up the given function name in all loaded libraries.
   * @throws SecurityException if user is not granted access for the named library.
   */
  public final long dynamicLookupFunctionGlobal(final String funcName) throws SecurityException {
    return dynLink.lookupSymbolGlobal(funcName);
  }

  /* pp */ final DynamicLinker dynamicLinker() { return dynLink; }

  /* pp */ static DynamicLinker getDynamicLinker() {
      final DynamicLinker dynLink;
      switch (PlatformProps.OS) {
          case WINDOWS:
              dynLink = new WindowsDynamicLinkerImpl();
              break;

          case MACOS:
          case IOS:
              dynLink = new MacOSXDynamicLinkerImpl();
              break;

          case ANDROID:
              if( PlatformProps.CPU.is32Bit ) {
                  dynLink = new BionicDynamicLinker32bitImpl();
              } else {
                  dynLink = new BionicDynamicLinker64BitImpl();
              }
              break;

          default:
              dynLink = new PosixDynamicLinkerImpl();
              break;
      }
      return dynLink;
  }

  /** Retrieves the low-level library handle from this NativeLibrary
      object. On the Windows platform this is an HMODULE, and on Unix
      and Mac OS X platforms the void* result of calling dlopen(). */
  public final long getLibraryHandle() {
    return libraryHandle;
  }

  /** Retrieves the path under which this library was opened. */
  public final String getLibraryPath() {
    return libraryPath;
  }

  /** Closes this native library. Further lookup operations are not
      allowed after calling this method.
   * @throws SecurityException if user is not granted access for the named library.
   */
  public final void close() throws SecurityException {
    if (DEBUG) {
      System.err.println("NativeLibrary.close(): closing " + this);
    }
    if ( 0 == libraryHandle ) {
      throw new RuntimeException("Library already closed");
    }
    final long handle = libraryHandle;
    libraryHandle = 0;
    dynLink.closeLibrary(handle, DEBUG);
    if (DEBUG) {
      System.err.println("NativeLibrary.close(): Successfully closed " + this);
      ExceptionUtils.dumpStack(System.err);
    }
  }
}
