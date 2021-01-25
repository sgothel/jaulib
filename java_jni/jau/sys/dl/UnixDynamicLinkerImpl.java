/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
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
package jau.sys.dl;

/* pp */ abstract class UnixDynamicLinkerImpl extends DynamicLinkerImpl {

  //
  // Package private scope of class w/ protected native code access
  // and sealed jogamp.common.* package definition
  // ensuring no abuse via subclassing.
  //

  /** Interface to C language function: <br> <code> int dlclose(void * ); </code>    */
  protected static native int dlclose(long arg0);

  /** Interface to C language function: <br> <code> char *  dlerror(void); </code>    */
  protected static native java.lang.String dlerror();

  /** Interface to C language function: <br> <code> void *  dlopen(const char * , int); </code>    */
  protected static native long dlopen(java.lang.String arg0, int arg1);

  /** Interface to C language function: <br> <code> void *  dlsym(void * , const char * ); </code>    */
  protected static native long dlsym(long arg0, java.lang.String arg1);

  @Override
  protected final long lookupSymbolLocalImpl(final long libraryHandle, final String symbolName) throws SecurityException {
      return 0 != libraryHandle ? dlsym(libraryHandle, symbolName) : 0;
  }

  @Override
  protected final void closeLibraryImpl(final long libraryHandle) throws SecurityException {
      if( 0 != libraryHandle ) {
          dlclose(libraryHandle);
      }
  }

  @Override
  public final String getLastError() {
      return dlerror();
  }
}
