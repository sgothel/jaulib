/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2010 Gothel Software e.K.
 * Copyright (c) 2010 JogAmp Community.
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

package jau.sys;

import org.jau.sys.PlatformProps;
import org.jau.sys.MachineDataInfo;
import org.jau.sys.RuntimeProps;

/**
 * Runtime operations of {@link MachineDataInfo}.
 */
public class MachineDataInfoRuntime {

  static volatile boolean initialized = false;
  static volatile MachineDataInfo runtimeMD = null;
  static volatile MachineDataInfo.StaticConfig staticMD = null;

  public static void initialize() {
      if( !initialized ) {
          synchronized(MachineDataInfo.class) { // volatile dbl-checked-locking OK
              if( !initialized ) {
                  MachineDataInfo.StaticConfig.validateUniqueMachineDataInfo();

                  final MachineDataInfo runtimeMD = getRuntimeImpl();
                  final MachineDataInfo.StaticConfig staticMD = MachineDataInfo.StaticConfig.findCompatible(runtimeMD);
                  if( null == staticMD ) {
                      throw new RuntimeException("No compatible MachineDataInfo.StaticConfig for runtime:"+PlatformProps.NEWLINE+runtimeMD);
                  }
                  if( !staticMD.md.compatible(runtimeMD) ) {
                      throw new RuntimeException("Incompatible MachineDataInfo:"+PlatformProps.NEWLINE+
                                                 " Static "+staticMD+PlatformProps.NEWLINE+
                                                 " Runtime "+runtimeMD);
                  }
                  MachineDataInfoRuntime.runtimeMD = runtimeMD;
                  MachineDataInfoRuntime.staticMD = staticMD;
                  initialized=true;
                  if( PlatformProps.DEBUG ) {
                      System.err.println("MachineDataInfoRuntime.initialize():"+PlatformProps.NEWLINE+
                                         " Static "+staticMD+PlatformProps.NEWLINE+
                                         " Runtime "+runtimeMD);
                  }
                  return;
              }
          }
      }
      throw new InternalError("Already initialized");
  }
  /**
   * The static {@link MachineDataInfo} is utilized for high performance
   * precompiled size, offset, etc table lookup within generated structures
   * using the {@link MachineDataInfo.StaticConfig} index.
   */
  public static MachineDataInfo.StaticConfig getStatic() {
      if(!initialized) {
          synchronized(MachineDataInfo.class) { // volatile dbl-checked-locking OK
              if(!initialized) {
                  throw new InternalError("Not set");
              }
          }
      }
      return staticMD;
  }
  public static MachineDataInfo getRuntime() {
      if(!initialized) {
          synchronized(MachineDataInfo.class) { // volatile dbl-checked-locking OK
              if(!initialized) {
                  throw new InternalError("Not set");
              }
          }
      }
      return runtimeMD;
  }

  private static MachineDataInfo getRuntimeImpl() {
        try {
            RuntimeProps.initSingleton();
        } catch (final Throwable err) {
            return null;
        }

        final int pointerSizeInBytes = getPointerSizeInBytesImpl();
        switch(pointerSizeInBytes) {
            case 4:
            case 8:
                break;
            default:
                throw new RuntimeException("Unsupported pointer size "+pointerSizeInBytes+"bytes, please implement.");
        }

        final long pageSizeL =  getPageSizeInBytesImpl();
        if(Integer.MAX_VALUE < pageSizeL) {
            throw new InternalError("PageSize exceeds integer value: " + pageSizeL);
        }

        // size:      int, long, float, double, pointer, pageSize
        // alignment: int8, int16, int32, int64, int, long, float, double, pointer
        return new MachineDataInfo(
            true /* runtime validated */,

            getSizeOfIntImpl(), getSizeOfLongImpl(),
            getSizeOfFloatImpl(), getSizeOfDoubleImpl(), getSizeOfLongDoubleImpl(),
            pointerSizeInBytes, (int)pageSizeL,

            getAlignmentInt8Impl(), getAlignmentInt16Impl(), getAlignmentInt32Impl(), getAlignmentInt64Impl(),
            getAlignmentIntImpl(), getAlignmentLongImpl(),
            getAlignmentFloatImpl(), getAlignmentDoubleImpl(), getAlignmentLongDoubleImpl(),
            getAlignmentPointerImpl());
    }

    private static native int getPointerSizeInBytesImpl();
    private static native long getPageSizeInBytesImpl();

    private static native int getAlignmentInt8Impl();
    private static native int getAlignmentInt16Impl();
    private static native int getAlignmentInt32Impl();
    private static native int getAlignmentInt64Impl();
    private static native int getAlignmentIntImpl();
    private static native int getAlignmentLongImpl();
    private static native int getAlignmentPointerImpl();
    private static native int getAlignmentFloatImpl();
    private static native int getAlignmentDoubleImpl();
    private static native int getAlignmentLongDoubleImpl();
    private static native int getSizeOfIntImpl();
    private static native int getSizeOfLongImpl();
    private static native int getSizeOfPointerImpl();
    private static native int getSizeOfFloatImpl();
    private static native int getSizeOfDoubleImpl();
    private static native int getSizeOfLongDoubleImpl();
}

