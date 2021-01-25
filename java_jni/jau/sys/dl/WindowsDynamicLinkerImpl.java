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

public final class WindowsDynamicLinkerImpl extends DynamicLinkerImpl {

  /** Interface to C language function: <br> <code> BOOL FreeLibrary(HANDLE hLibModule); </code>    */
  private static native int FreeLibrary(long hLibModule);

  /** Interface to C language function: <br> <code> DWORD GetLastError(void); </code>    */
  private static native int GetLastError();

  /** Interface to C language function: <br> <code> PROC GetProcAddressA(HANDLE hModule, LPCSTR lpProcName); </code>    */
  private static native long GetProcAddressA(long hModule, java.lang.String lpProcName);

  /** Interface to C language function: <br> <code> HANDLE LoadLibraryW(LPCWSTR lpLibFileName); </code>    */
  private static native long LoadLibraryW(java.lang.String lpLibFileName);

  @Override
  protected final long openLibraryLocalImpl(final String libraryName) throws SecurityException {
    // How does that work under Windows ?
    // Don't know .. so it's an alias to global, for the time being
    return LoadLibraryW(libraryName);
  }

  @Override
  protected final long openLibraryGlobalImpl(final String libraryName) throws SecurityException {
    return LoadLibraryW(libraryName);
  }

  @Override
  protected final long lookupSymbolGlobalImpl(final String symbolName) throws SecurityException {
    if(DEBUG_LOOKUP) {
        System.err.println("lookupSymbolGlobal: Not supported on Windows");
    }
    // allow DynamicLibraryBundle to continue w/ local libs
    return 0;
  }

  private static final int symbolArgAlignment=4;  // 4 byte alignment of each argument
  private static final int symbolMaxArguments=12; // experience ..

  @Override
  protected final long lookupSymbolLocalImpl(final long libraryHandle, final String symbolName) throws IllegalArgumentException {
    String _symbolName = symbolName;
    long addr = GetProcAddressA(libraryHandle, _symbolName);
    if( 0 == addr ) {
        // __stdcall hack: try some @nn decorations,
        //                 the leading '_' must not be added (same with cdecl)
        for(int arg=0; 0==addr && arg<=symbolMaxArguments; arg++) {
            _symbolName = symbolName+"@"+(arg*symbolArgAlignment);
            addr = GetProcAddressA(libraryHandle, _symbolName);
        }
    }
    return addr;
  }

  @Override
  protected final void closeLibraryImpl(final long libraryHandle) throws IllegalArgumentException {
    FreeLibrary(libraryHandle);
  }

  @Override
  public final String getLastError() {
      final int err = GetLastError();
      return "Last error: 0x"+Integer.toHexString(err)+" ("+err+")";
  }

}
