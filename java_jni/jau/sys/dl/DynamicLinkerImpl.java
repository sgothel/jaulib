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

import java.util.HashMap;

import org.jau.sec.SecurityUtil;
import org.jau.sys.dl.DynamicLinker;

/* pp */ abstract class DynamicLinkerImpl implements DynamicLinker {

  //
  // Package private scope of class w/ protected native code access
  // and sealed jogamp.common.* package definition
  // ensuring no abuse via subclassing.
  //

  private final Object secSync = new Object();
  private boolean allLinkPermissionGranted = false;

  /**
   * @throws SecurityException if user is not granted global access
   */
  @Override
public final void claimAllLinkPermission() throws SecurityException {
      synchronized( secSync ) {
          allLinkPermissionGranted = true;
      }
  }

  /**
   * @throws SecurityException if user is not granted global access
   */
  @Override
public final void releaseAllLinkPermission() throws SecurityException {
      synchronized( secSync ) {
          allLinkPermissionGranted = false;
      }
  }

  private final void checkLinkPermission(final String pathname) throws SecurityException {
      synchronized( secSync ) {
          if( !allLinkPermissionGranted ) {
              SecurityUtil.checkLinkPermission(pathname);
          }
      }
  }
  private final void checkLinkPermission(final long libraryHandle) throws SecurityException {
      synchronized( secSync ) {
          if( !allLinkPermissionGranted ) {
              final LibRef libRef = getLibRef( libraryHandle );
              if( null == libRef ) {
                  throw new IllegalArgumentException("Library handle 0x"+Long.toHexString(libraryHandle)+" unknown.");
              }
              SecurityUtil.checkLinkPermission(libRef.getName());
          }
      }
  }

  private final void checkAllLinkPermission() throws SecurityException {
      synchronized( secSync ) {
          if( !allLinkPermissionGranted ) {
              SecurityUtil.checkAllLinkPermission();
          }
      }
  }

  @Override
  public final long openLibraryGlobal(final String pathname, final boolean debug) throws SecurityException {
    checkLinkPermission(pathname);
    final long handle = openLibraryGlobalImpl(pathname);
    if( 0 != handle ) {
        final LibRef libRef = incrLibRefCount(handle, pathname);
        if( DEBUG || debug ) {
            System.err.println("DynamicLinkerImpl.openLibraryGlobal \""+pathname+"\": 0x"+Long.toHexString(handle)+" -> "+libRef+")");
        }
    } else if ( DEBUG || debug ) {
        System.err.println("DynamicLinkerImpl.openLibraryGlobal \""+pathname+"\" failed, error: "+getLastError());
    }
    return handle;
  }
  protected abstract long openLibraryGlobalImpl(final String pathname) throws SecurityException;

  @Override
  public final long openLibraryLocal(final String pathname, final boolean debug) throws SecurityException {
    checkLinkPermission(pathname);
    final long handle = openLibraryLocalImpl(pathname);
    if( 0 != handle ) {
        final LibRef libRef = incrLibRefCount(handle, pathname);
        if( DEBUG || debug ) {
            System.err.println("DynamicLinkerImpl.openLibraryLocal \""+pathname+"\": 0x"+Long.toHexString(handle)+" -> "+libRef+")");
        }
    } else if ( DEBUG || debug ) {
        System.err.println("DynamicLinkerImpl.openLibraryLocal \""+pathname+"\" failed, error: "+getLastError());
    }
    return handle;
  }
  protected abstract long openLibraryLocalImpl(final String pathname) throws SecurityException;

  @Override
  public final long lookupSymbolGlobal(final String symbolName) throws SecurityException {
    checkAllLinkPermission();
    final long addr = lookupSymbolGlobalImpl(symbolName);
    if(DEBUG_LOOKUP) {
        System.err.println("DynamicLinkerImpl.lookupSymbolGlobal("+symbolName+") -> 0x"+Long.toHexString(addr));
    }
    return addr;
  }
  protected abstract long lookupSymbolGlobalImpl(final String symbolName) throws SecurityException;

  @Override
  public final long lookupSymbol(final long libraryHandle, final String symbolName) throws SecurityException, IllegalArgumentException {
    checkLinkPermission(libraryHandle);
    final long addr = lookupSymbolLocalImpl(libraryHandle, symbolName);
    if(DEBUG_LOOKUP) {
        System.err.println("DynamicLinkerImpl.lookupSymbol(0x"+Long.toHexString(libraryHandle)+", "+symbolName+") -> 0x"+Long.toHexString(addr));
    }
    return addr;
  }
  protected abstract long lookupSymbolLocalImpl(final long libraryHandle, final String symbolName) throws SecurityException;

  @Override
  public final void closeLibrary(final long libraryHandle, final boolean debug) throws SecurityException, IllegalArgumentException {
    final LibRef libRef = decrLibRefCount( libraryHandle );
    if( null != libRef ) {
        checkLinkPermission(libRef.getName());
    } // else null libRef is OK for global lookup
    if( DEBUG || debug ) {
        System.err.println("DynamicLinkerImpl.closeLibrary(0x"+Long.toHexString(libraryHandle)+" -> "+libRef+")");
    }
    if( 0 != libraryHandle ) {
        closeLibraryImpl(libraryHandle);
    }
  }
  protected abstract void closeLibraryImpl(final long libraryHandle) throws SecurityException;

  private static final HashMap<Long,Object> libHandle2Name = new HashMap<Long,Object>( 16 /* initialCapacity */ );

  static final class LibRef {
      LibRef(final String name) {
          this.name = name;
          this.refCount = 1;
      }
      final int incrRefCount() { return ++refCount; }
      final int decrRefCount() { return --refCount; }
      final int getRefCount() { return refCount; }

      final String getName() { return name; }
      @Override
      public final String toString() { return "LibRef["+name+", refCount "+refCount+"]"; }

      private final String name;
      private int refCount;
  }

  private final LibRef getLibRef(final long handle) {
      synchronized( libHandle2Name ) {
          return (LibRef) libHandle2Name.get(handle);
      }
  }

  private final LibRef incrLibRefCount(final long handle, final String libName) {
      synchronized( libHandle2Name ) {
          LibRef libRef = getLibRef(handle);
          if( null == libRef ) {
              libRef = new LibRef(libName);
              libHandle2Name.put(handle, libRef);
          } else {
              libRef.incrRefCount();
          }
          if(DEBUG) {
              System.err.println("DynamicLinkerImpl.incrLibRefCount 0x"+Long.toHexString(handle)+ " -> "+libRef+", libs loaded "+libHandle2Name.size());
          }
          return libRef;
      }
  }

  private final LibRef decrLibRefCount(final long handle) {
      synchronized( libHandle2Name ) {
          final LibRef libRef = getLibRef(handle);
          if( null != libRef ) {
              if( 0 == libRef.decrRefCount() ) {
                  libHandle2Name.remove(handle);
              }
          }
          if(DEBUG) {
              System.err.println("DynamicLinkerImpl.decrLibRefCount 0x"+Long.toHexString(handle)+ " -> "+libRef+", libs loaded "+libHandle2Name.size());
          }
          return libRef;
      }
  }
}
