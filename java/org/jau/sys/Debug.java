/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2012 Gothel Software e.K.
 * Copyright (c) 2012 JogAmp Community.
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

import java.security.AccessController;
import java.security.PrivilegedAction;

import com.jogamp.common.util.PropertyAccess;

/** Helper routines for logging and debugging. */

public class Debug extends PropertyAccess {
  // Some common properties
  private static final boolean verbose;
  private static final boolean debugAll;

  static {
    AccessController.doPrivileged(new PrivilegedAction<Object>() {
        @Override
        public Object run() {
            PropertyAccess.addTrustedPrefix("jogamp.");
            return null;
    } } );

    verbose = isPropertyDefined("jogamp.verbose", true);
    debugAll = isPropertyDefined("jogamp.debug", true);
  }

  /** Ensures static init block has been issues, i.e. if calling through to {@link PropertyAccess#isPropertyDefined(String, boolean)}. */
  public static final void initSingleton() {}

  public static final boolean verbose() {
    return verbose;
  }

  public static final boolean debugAll() {
    return debugAll;
  }

  public static final boolean debug(final String subcomponent) {
    return debugAll() || isPropertyDefined("jogamp.debug." + subcomponent, true);
  }
}
