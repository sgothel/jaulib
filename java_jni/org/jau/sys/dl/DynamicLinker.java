/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2013 Gothel Software e.K.
 * Copyright (c) 2013 JogAmp Community.
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

/** Low level secure dynamic linker access. */
public interface DynamicLinker {
  public static final boolean DEBUG = NativeLibrary.DEBUG;
  public static final boolean DEBUG_LOOKUP = NativeLibrary.DEBUG_LOOKUP;

  /**
   * @throws SecurityException if user is not granted global access
   */
  public void claimAllLinkPermission() throws SecurityException;

  /**
   * @throws SecurityException if user is not granted global access
   */
  public void releaseAllLinkPermission() throws SecurityException;

  /**
   * If a {@link SecurityManager} is installed, user needs link permissions
   * for the named library.
   * <p>
   * Opens the named library, allowing system wide access for other <i>users</i>.
   * </p>
   *
   * @param pathname the full pathname for the library to open
   * @param debug set to true to enable debugging
   * @return the library handle, maybe 0 if not found.
   * @throws SecurityException if user is not granted access for the named library.
   */
  public long openLibraryGlobal(String pathname, boolean debug) throws SecurityException;

  /**
   * If a {@link SecurityManager} is installed, user needs link permissions
   * for the named library.
   * <p>
   * Opens the named library, restricting access to this process.
   * </p>
   *
   * @param pathname the full pathname for the library to open
   * @param debug set to true to enable debugging
   * @return the library handle, maybe 0 if not found.
   * @throws SecurityException if user is not granted access for the named library.
   */
  public long openLibraryLocal(String pathname, boolean debug) throws SecurityException;

  /**
   * If a {@link SecurityManager} is installed, user needs link permissions
   * for <b>all</b> libraries, i.e. for <code>new RuntimePermission("loadLibrary.*");</code>!
   *
   * @param symbolName global symbol name to lookup up system wide.
   * @return the library handle, maybe 0 if not found.
   * @throws SecurityException if user is not granted access for all libraries.
   */
  public long lookupSymbolGlobal(String symbolName) throws SecurityException;

  /**
   * Security checks are implicit by previous call of
   * {@link #openLibraryLocal(String, boolean)} or {@link #openLibraryGlobal(String, boolean)}
   * retrieving the <code>librarHandle</code>.
   *
   * @param libraryHandle a library handle previously retrieved via {@link #openLibraryLocal(String, boolean)} or {@link #openLibraryGlobal(String, boolean)}.
   * @param symbolName global symbol name to lookup up system wide.
   * @return the library handle, maybe 0 if not found.
   * @throws IllegalArgumentException in case case <code>libraryHandle</code> is unknown.
   * @throws SecurityException if user is not granted access for the given library handle
   */
  public long lookupSymbol(long libraryHandle, String symbolName) throws SecurityException, IllegalArgumentException;

  /**
   * Security checks are implicit by previous call of
   * {@link #openLibraryLocal(String, boolean)} or {@link #openLibraryGlobal(String, boolean)}
   * retrieving the <code>librarHandle</code>.
   *
   * @param libraryHandle a library handle previously retrieved via {@link #openLibraryLocal(String, boolean)} or {@link #openLibraryGlobal(String, boolean)}.
   * @param debug set to true to enable debugging
   * @throws IllegalArgumentException in case case <code>libraryHandle</code> is unknown.
   * @throws SecurityException if user is not granted access for the given library handle
   */
  public void closeLibrary(long libraryHandle, boolean debug) throws SecurityException, IllegalArgumentException;

  /**
   * Returns a string containing the last error.
   * Maybe called for debuging purposed if any method fails.
   * @return error string, maybe null. A null or non-null value has no semantics.
   */
  public String getLastError();
}
