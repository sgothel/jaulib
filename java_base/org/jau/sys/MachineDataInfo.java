/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Author: Kenneth Bradley Russell
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2010 Gothel Software e.K.
 * Copyright (c) 2010 JogAmp Community.
 * Copyright (c) 2003 Sun Microsystems
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

/**
 * Machine data description for alignment and size onle, see {@link com.jogamp.gluegen}.
 * <p>
 * {@code little-endian} / {@code big/endian} description is left,
 * allowing re-using instances in {@link MachineDataInfo.StaticConfig StaticConfig}.
 * Use {@link {@link RuntimeProps#LITTLE_ENDIAN}.
 * </p>
 * <p>
 * Further more, the value {@ MachineDataInfo#pageSizeInBytes} shall be ignored
 * in {@link MachineDataInfo.StaticConfig StaticConfig}, see {@link MachineDataInfo#compatible(MachineDataInfo)}.
 * </p>
 */
public class MachineDataInfo {
  /*                              arch   os          int, long, float, doubl, ldoubl,  ptr,   page */
  private final static int[] size_arm_mips_32     =  { 4,    4,     4,     8,      8,    4,   4096 };
  private final static int[] size_x86_32_unix     =  { 4,    4,     4,     8,     12,    4,   4096 };
  private final static int[] size_x86_32_android  =  { 4,    4,     4,     8,      8,    4,   4096 };
  private final static int[] size_x86_32_macos    =  { 4,    4,     4,     8,     16,    4,   4096 };
  private final static int[] size_ppc_32_unix     =  { 4,    4,     4,     8,     16,    4,   4096 };
  private final static int[] size_sparc_32_sunos  =  { 4,    4,     4,     8,     16,    4,   8192 };
  private final static int[] size_x86_32_windows  =  { 4,    4,     4,     8,     12,    4,   4096 };
  private final static int[] size_lp64_unix       =  { 4,    8,     4,     8,     16,    8,   4096 };
  private final static int[] size_x86_64_windows  =  { 4,    4,     4,     8,     16,    8,   4096 };
  private final static int[] size_arm64_ios       =  { 4,    8,     4,     8,      8,    8,   8192 };

  /*                               arch   os          i8, i16, i32, i64, int, long, float, doubl, ldoubl, ptr */
  private final static int[] align_arm_mips_32    =  { 1,   2,   4,   8,   4,    4,     4,     8,      8,   4 };
  private final static int[] align_x86_32_unix    =  { 1,   2,   4,   4,   4,    4,     4,     4,      4,   4 };
  private final static int[] align_x86_32_macos   =  { 1,   2,   4,   4,   4,    4,     4,     4,     16,   4 };
  private final static int[] align_ppc_32_unix    =  { 1,   2,   4,   8,   4,    4,     4,     8,     16,   4 };
  private final static int[] align_sparc_32_sunos =  { 1,   2,   4,   8,   4,    4,     4,     8,      8,   4 };
  private final static int[] align_x86_32_windows =  { 1,   2,   4,   8,   4,    4,     4,     8,      4,   4 };
  private final static int[] align_lp64_unix      =  { 1,   2,   4,   8,   4,    8,     4,     8,     16,   8 };
  private final static int[] align_x86_64_windows =  { 1,   2,   4,   8,   4,    4,     4,     8,     16,   8 };
  private final static int[] align_arm64_ios      =  { 1,   2,   4,   8,   4,    8,     4,     8,      8,   8 };

  /**
   * Static enumeration of {@link MachineDataInfo} instances
   * used for high performance data size and alignment lookups,
   * e.g. for generated structures using the {@link MachineDataInfo.StaticConfig} index.
   * <p>
   * The value {@link MachineDataInfo#pageSizeInBytes} shall be ignored
   * for static instances!
   * </p>
   * <p>
   * If changing this table, you need to:
   * <ul>
   *   <li>Rebuild GlueGen.</li>
   *   <li>Run ant {@code build.xml} target {@code generate.os.sources}.</li>
   *   <li>Rebuild everything.</li>
   * </ul>
   * .. b/c the generated code for glued structures must reflect this change!
   * </p>
   */
  public enum StaticConfig {
      /** {@link Platform.CPUType#ARM} or {@link Platform.CPUType#MIPS_32} */
      ARM_MIPS_32(     size_arm_mips_32,   align_arm_mips_32),
      /** {@link Platform.CPUType#X86_32} Unix */
      X86_32_UNIX(    size_x86_32_unix,    align_x86_32_unix),
      /** {@link Platform.CPUType#X86_32} Android/Bionic */
      X86_32_ANDROID( size_x86_32_android, align_x86_32_unix),
      /** {@link Platform.CPUType#X86_32} MacOS (Special case gcc4/OSX) */
      X86_32_MACOS(   size_x86_32_macos,   align_x86_32_macos),
      /** {@link Platform.CPUType#PPC} Unix */
      PPC_32_UNIX(    size_ppc_32_unix,   align_ppc_32_unix),
      /** {@link Platform.CPUType#SPARC_32} Solaris */
      SPARC_32_SUNOS( size_sparc_32_sunos, align_sparc_32_sunos),
      /** {@link Platform.CPUType#X86_32} Windows */
      X86_32_WINDOWS( size_x86_32_windows, align_x86_32_windows),
      /** LP64 Unix, e.g.: {@link Platform.CPUType#X86_64} Unix, {@link Platform.CPUType#ARM64} EABI, {@link Platform.CPUType#PPC64} Unix, .. */
      LP64_UNIX(      size_lp64_unix,    align_lp64_unix),
      /** {@link Platform.CPUType#X86_64} Windows */
      X86_64_WINDOWS( size_x86_64_windows, align_x86_64_windows),
      /** {@link Platform.CPUType#ARM64 } iOS */
      ARM64_IOS( size_arm64_ios, align_arm64_ios);
      // 9

      public final MachineDataInfo md;

      StaticConfig(final int[] sizes, final int[] alignments) {
          int i=0, j=0;
          this.md = new MachineDataInfo(false,
                                           sizes[i++],
                                           sizes[i++],
                                           sizes[i++],
                                           sizes[i++],
                                           sizes[i++],
                                           sizes[i++],
                                           sizes[i++],
                                           alignments[j++],
                                           alignments[j++],
                                           alignments[j++],
                                           alignments[j++],
                                           alignments[j++],
                                           alignments[j++],
                                           alignments[j++],
                                           alignments[j++],
                                           alignments[j++],
                                           alignments[j++]);
      }

      public final StringBuilder toString(StringBuilder sb) {
        if(null==sb) {
            sb = new StringBuilder();
        }
        sb.append("MachineDataInfoStatic: ").append(this.name()).append("(").append(this.ordinal()).append("): ");
        md.toString(sb);
        return sb;
      }
      public final String toShortString() {
          return this.name()+"("+this.ordinal()+")";
      }
      @Override
      public String toString() {
        return toString(null).toString();
      }

      /**
       * Static's {@link MachineDataInfo} shall be unique by the
       * {@link MachineDataInfo#compatible(MachineDataInfo) compatible} criteria.
       */
      public static final void validateUniqueMachineDataInfo() {
          final StaticConfig[] scs = StaticConfig.values();
          for(int i=scs.length-1; i>=0; i--) {
              final StaticConfig a = scs[i];
              for(int j=scs.length-1; j>=0; j--) {
                  if( i != j ) {
                      final StaticConfig b = scs[j];
                      if( a.md.compatible(b.md) ) {
                          // oops
                          final String msg = "Duplicate/Compatible MachineDataInfo in StaticConfigs: Elements ["+i+": "+a.toShortString()+"] and ["+j+": "+b.toShortString()+"]";
                          System.err.println(msg);
                          System.err.println(a);
                          System.err.println(b);
                          throw new InternalError(msg);
                      }
                  }
              }
          }
      }
      public static final StaticConfig findCompatible(final MachineDataInfo md) {
          final StaticConfig[] scs = StaticConfig.values();
          for(int i=scs.length-1; i>=0; i--) {
              final StaticConfig a = scs[i];
              if( a.md.compatible(md) ) {
                  return a;
              }
          }
          return null;
      }
  }

  final private boolean runtimeValidated;

  final private int int8SizeInBytes = 1;
  final private int int16SizeInBytes = 2;
  final private int int32SizeInBytes = 4;
  final private int int64SizeInBytes = 8;

  final private int intSizeInBytes;
  final private int longSizeInBytes;
  final private int floatSizeInBytes;
  final private int doubleSizeInBytes;
  final private int ldoubleSizeInBytes;
  final private int pointerSizeInBytes;
  final private int pageSizeInBytes;

  final private int int8AlignmentInBytes;
  final private int int16AlignmentInBytes;
  final private int int32AlignmentInBytes;
  final private int int64AlignmentInBytes;
  final private int intAlignmentInBytes;
  final private int longAlignmentInBytes;
  final private int floatAlignmentInBytes;
  final private int doubleAlignmentInBytes;
  final private int ldoubleAlignmentInBytes;
  final private int pointerAlignmentInBytes;

  public MachineDataInfo(final boolean runtimeValidated,

                            final int intSizeInBytes,
                            final int longSizeInBytes,
                            final int floatSizeInBytes,
                            final int doubleSizeInBytes,
                            final int ldoubleSizeInBytes,
                            final int pointerSizeInBytes,
                            final int pageSizeInBytes,

                            final int int8AlignmentInBytes,
                            final int int16AlignmentInBytes,
                            final int int32AlignmentInBytes,
                            final int int64AlignmentInBytes,
                            final int intAlignmentInBytes,
                            final int longAlignmentInBytes,
                            final int floatAlignmentInBytes,
                            final int doubleAlignmentInBytes,
                            final int ldoubleAlignmentInBytes,
                            final int pointerAlignmentInBytes) {
    this.runtimeValidated = runtimeValidated;

    this.intSizeInBytes     = intSizeInBytes;
    this.longSizeInBytes    = longSizeInBytes;
    this.floatSizeInBytes   = floatSizeInBytes;
    this.doubleSizeInBytes  = doubleSizeInBytes;
    this.ldoubleSizeInBytes = ldoubleSizeInBytes;
    this.pointerSizeInBytes = pointerSizeInBytes;
    this.pageSizeInBytes    = pageSizeInBytes;

    this.int8AlignmentInBytes    = int8AlignmentInBytes;
    this.int16AlignmentInBytes   = int16AlignmentInBytes;
    this.int32AlignmentInBytes   = int32AlignmentInBytes;
    this.int64AlignmentInBytes   = int64AlignmentInBytes;
    this.intAlignmentInBytes     = intAlignmentInBytes;
    this.longAlignmentInBytes    = longAlignmentInBytes;
    this.floatAlignmentInBytes   = floatAlignmentInBytes;
    this.doubleAlignmentInBytes  = doubleAlignmentInBytes;
    this.ldoubleAlignmentInBytes = ldoubleAlignmentInBytes;
    this.pointerAlignmentInBytes = pointerAlignmentInBytes;
  }

  /**
   * @return true if all values are validated at runtime, otherwise false (i.e. for static compilation w/ preset values)
   */
  public final boolean isRuntimeValidated() {
      return runtimeValidated;
  }

  public final int intSizeInBytes()     { return intSizeInBytes;    }
  public final int longSizeInBytes()    { return longSizeInBytes;   }
  public final int int8SizeInBytes()    { return int8SizeInBytes;  }
  public final int int16SizeInBytes()   { return int16SizeInBytes;  }
  public final int int32SizeInBytes()   { return int32SizeInBytes;  }
  public final int int64SizeInBytes()   { return int64SizeInBytes;  }
  public final int floatSizeInBytes()   { return floatSizeInBytes;  }
  public final int doubleSizeInBytes()  { return doubleSizeInBytes; }
  public final int ldoubleSizeInBytes() { return ldoubleSizeInBytes; }
  public final int pointerSizeInBytes() { return pointerSizeInBytes; }
  public final int pageSizeInBytes()    { return pageSizeInBytes; }

  public final int intAlignmentInBytes()     { return intAlignmentInBytes;    }
  public final int longAlignmentInBytes()    { return longAlignmentInBytes;   }
  public final int int8AlignmentInBytes()    { return int8AlignmentInBytes;  }
  public final int int16AlignmentInBytes()   { return int16AlignmentInBytes;  }
  public final int int32AlignmentInBytes()   { return int32AlignmentInBytes;  }
  public final int int64AlignmentInBytes()   { return int64AlignmentInBytes;  }
  public final int floatAlignmentInBytes()   { return floatAlignmentInBytes;  }
  public final int doubleAlignmentInBytes()  { return doubleAlignmentInBytes; }
  public final int ldoubleAlignmentInBytes() { return ldoubleAlignmentInBytes; }
  public final int pointerAlignmentInBytes() { return pointerAlignmentInBytes; }

  /**
   * @return number of pages required for size in bytes
   */
  public int pageCount(final int size) {
    return ( size + ( pageSizeInBytes - 1) ) / pageSizeInBytes ; // integer arithmetic
  }

  /**
   * @return page aligned size in bytes
   */
  public int pageAlignedSize(final int size) {
    return pageCount(size) * pageSizeInBytes;
  }

  /**
   * Checks whether two size objects are equal. Two instances
   * of <code>MachineDataInfo</code> are considered equal if all components
   * match but {@link #runtimeValidated},  {@link #isRuntimeValidated()}.
   * @return  <code>true</code> if the two MachineDataInfo are equal;
   *          otherwise <code>false</code>.
   */
  @Override
  public final boolean equals(final Object obj) {
      if (this == obj) { return true; }
      if ( !(obj instanceof MachineDataInfo) ) { return false; }
      final MachineDataInfo md = (MachineDataInfo) obj;

      return pageSizeInBytes == md.pageSizeInBytes &&
             compatible(md);
  }

  /**
   * Checks whether two {@link MachineDataInfo} objects are equal.
   * <p>
   * Two {@link MachineDataInfo} instances are considered equal if all components
   * match but {@link #isRuntimeValidated()} and {@link #pageSizeInBytes()}.
   * </p>
   * @return  <code>true</code> if the two {@link MachineDataInfo} are equal;
   *          otherwise <code>false</code>.
   */
  public final boolean compatible(final MachineDataInfo md) {
      return intSizeInBytes == md.intSizeInBytes &&
             longSizeInBytes == md.longSizeInBytes &&
             floatSizeInBytes == md.floatSizeInBytes &&
             doubleSizeInBytes == md.doubleSizeInBytes &&
             ldoubleSizeInBytes == md.ldoubleSizeInBytes &&
             pointerSizeInBytes == md.pointerSizeInBytes &&

             int8AlignmentInBytes == md.int8AlignmentInBytes &&
             int16AlignmentInBytes == md.int16AlignmentInBytes &&
             int32AlignmentInBytes == md.int32AlignmentInBytes &&
             int64AlignmentInBytes == md.int64AlignmentInBytes &&
             intAlignmentInBytes == md.intAlignmentInBytes &&
             longAlignmentInBytes == md.longAlignmentInBytes &&
             floatAlignmentInBytes == md.floatAlignmentInBytes &&
             doubleAlignmentInBytes == md.doubleAlignmentInBytes &&
             ldoubleAlignmentInBytes == md.ldoubleAlignmentInBytes &&
             pointerAlignmentInBytes == md.pointerAlignmentInBytes ;
  }

  public StringBuilder toString(StringBuilder sb) {
    if(null==sb) {
        sb = new StringBuilder();
    }
    sb.append("MachineDataInfo: runtimeValidated ").append(isRuntimeValidated()).append(", 32Bit ").append(4 == pointerAlignmentInBytes).append(", primitive size / alignment:").append(PlatformProps.NEWLINE);
    sb.append("  int8    ").append(int8SizeInBytes)   .append(" / ").append(int8AlignmentInBytes);
    sb.append(", int16   ").append(int16SizeInBytes)  .append(" / ").append(int16AlignmentInBytes).append(PlatformProps.NEWLINE);
    sb.append("  int     ").append(intSizeInBytes)    .append(" / ").append(intAlignmentInBytes);
    sb.append(", long    ").append(longSizeInBytes)   .append(" / ").append(longAlignmentInBytes).append(PlatformProps.NEWLINE);
    sb.append("  int32   ").append(int32SizeInBytes)  .append(" / ").append(int32AlignmentInBytes);
    sb.append(", int64   ").append(int64SizeInBytes)  .append(" / ").append(int64AlignmentInBytes).append(PlatformProps.NEWLINE);
    sb.append("  float   ").append(floatSizeInBytes)  .append(" / ").append(floatAlignmentInBytes);
    sb.append(", double  ").append(doubleSizeInBytes) .append(" / ").append(doubleAlignmentInBytes);
    sb.append(", ldouble ").append(ldoubleSizeInBytes).append(" / ").append(ldoubleAlignmentInBytes).append(PlatformProps.NEWLINE);
    sb.append("  pointer ").append(pointerSizeInBytes).append(" / ").append(pointerAlignmentInBytes);
    sb.append(", page    ").append(pageSizeInBytes);
    return sb;
  }

  @Override
  public String toString() {
    return toString(null).toString();
  }

  public static StaticConfig guessStaticMachineDataInfo(final PlatformTypes.OSType osType, final PlatformTypes.CPUType cpuType) {
      if( cpuType.is32Bit ) {
          if( PlatformTypes.CPUFamily.ARM32 == cpuType.family ||
              PlatformTypes.CPUType.MIPS_32 == cpuType ) {
              return StaticConfig.ARM_MIPS_32;
          } else if( PlatformTypes.OSType.WINDOWS == osType ) {
              return StaticConfig.X86_32_WINDOWS;
          } else if( PlatformTypes.OSType.MACOS == osType ) {
              return StaticConfig.X86_32_MACOS;
          } else if ( PlatformTypes.OSType.SUNOS == osType &&
                      PlatformTypes.CPUType.SPARC_32 == cpuType ) {
              return StaticConfig.SPARC_32_SUNOS;
          } else if ( PlatformTypes.CPUType.PPC == cpuType ) {
              return StaticConfig.PPC_32_UNIX;
          } else {
              return StaticConfig.X86_32_UNIX;
          }
      } else {
          if( PlatformTypes.OSType.WINDOWS == osType ) {
              return StaticConfig.X86_64_WINDOWS;
          } else if( PlatformTypes.OSType.IOS == osType &&  PlatformTypes.CPUType.ARM64 == cpuType ) {
              return StaticConfig.ARM64_IOS;
          } else {
              // for all 64bit unix types (x86_64, aarch64, sparcv9, ..)
              return StaticConfig.LP64_UNIX;
          }
      }
  }
}
