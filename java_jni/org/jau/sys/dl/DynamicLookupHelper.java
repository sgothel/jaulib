/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Author: Kenneth Bradley Russell
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2011 Gothel Software e.K.
 * Copyright (c) 2011 JogAmp Community.
 * Copyright (c) 2003-2005 Sun Microsystems, Inc.
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

import org.jau.sys.Debug;

public interface DynamicLookupHelper {
  public static final boolean DEBUG = Debug.debug("NativeLibrary");
  public static final boolean DEBUG_LOOKUP = Debug.debug("NativeLibrary.Lookup");

  /**
   * @throws SecurityException if user is not granted access for the library set.
   */
  public void claimAllLinkPermission() throws SecurityException;
  /**
   * @throws SecurityException if user is not granted access for the library set.
   */
  public void releaseAllLinkPermission() throws SecurityException;

  /**
   * Returns the function handle for function 'funcName'.
   * @throws SecurityException if user is not granted access for the library set.
   */
  public long dynamicLookupFunction(String funcName) throws SecurityException;

  /**
   * Queries whether function 'funcName' is available.
   * @throws SecurityException if user is not granted access for the library set.
   */
  public boolean isFunctionAvailable(String funcName) throws SecurityException;
}
