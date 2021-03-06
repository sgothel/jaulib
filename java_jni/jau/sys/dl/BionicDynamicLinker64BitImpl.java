/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2015 Gothel Software e.K.
 * Copyright (c) 2015 JogAmp Community.
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
package jau.sys.dl;

/**
 * Bionic 64bit specialization of {@link UnixDynamicLinkerImpl}
 * utilizing Bionic's non POSIX flags and mode values.
 * <p>
 * Bionic is used on Android.
 * </p>
 */
public final class BionicDynamicLinker64BitImpl extends UnixDynamicLinkerImpl {
  //      static final int RTLD_NOW      = 0x00002;
  private static final int RTLD_LAZY     = 0x00001;

  private static final int RTLD_LOCAL    = 0x00000;
  private static final int RTLD_GLOBAL   = 0x00100;
  //      static final int RTLD_NOLOAD   = 0x00004;

  private static final long RTLD_DEFAULT = 0x00000000L;
  //      static final long RTLD_NEXT    = -1L;

  @Override
  protected final long openLibraryLocalImpl(final String pathname) throws SecurityException {
    return dlopen(pathname, RTLD_LAZY | RTLD_LOCAL);
  }

  @Override
  protected final long openLibraryGlobalImpl(final String pathname) throws SecurityException {
    return dlopen(pathname, RTLD_LAZY | RTLD_GLOBAL);
  }

  @Override
  protected final long lookupSymbolGlobalImpl(final String symbolName) throws SecurityException {
    return dlsym(RTLD_DEFAULT, symbolName);
  }

}
