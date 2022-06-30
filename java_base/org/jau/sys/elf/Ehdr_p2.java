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
package org.jau.sys.elf;

import org.jau.io.Buffers;
import org.jau.io.StructAccessor;
import org.jau.sys.MachineDataInfo;

public class Ehdr_p2 {

  StructAccessor accessor;

  private final int mdIdx;
  private final MachineDataInfo md;

  private static final int[] Ehdr_p2_size = new int[] { 28 /* ARM_MIPS_32 */, 28 /* X86_32_UNIX */, 28 /* X86_32_MACOS */, 28 /* PPC_32_UNIX */, 28 /* SPARC_32_SUNOS */, 28 /* X86_32_WINDOWS */, 40 /* LP64_UNIX */, 40 /* X86_64_WINDOWS */  };
  private static final int[] e_entry_offset = new int[] { 0 /* ARM_MIPS_32 */, 0 /* X86_32_UNIX */, 0 /* X86_32_MACOS */, 0 /* PPC_32_UNIX */, 0 /* SPARC_32_SUNOS */, 0 /* X86_32_WINDOWS */, 0 /* LP64_UNIX */, 0 /* X86_64_WINDOWS */ };
//private static final int[] e_entry_size = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 8 /* LP64_UNIX */, 8 /* X86_64_WINDOWS */  };
  private static final int[] e_phoff_offset = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 8 /* LP64_UNIX */, 8 /* X86_64_WINDOWS */ };
//private static final int[] e_phoff_size = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 8 /* LP64_UNIX */, 8 /* X86_64_WINDOWS */  };
  private static final int[] e_shoff_offset = new int[] { 8 /* ARM_MIPS_32 */, 8 /* X86_32_UNIX */, 8 /* X86_32_MACOS */, 8 /* PPC_32_UNIX */, 8 /* SPARC_32_SUNOS */, 8 /* X86_32_WINDOWS */, 16 /* LP64_UNIX */, 16 /* X86_64_WINDOWS */ };
//private static final int[] e_shoff_size = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 8 /* LP64_UNIX */, 8 /* X86_64_WINDOWS */  };
  private static final int[] e_flags_offset = new int[] { 12 /* ARM_MIPS_32 */, 12 /* X86_32_UNIX */, 12 /* X86_32_MACOS */, 12 /* PPC_32_UNIX */, 12 /* SPARC_32_SUNOS */, 12 /* X86_32_WINDOWS */, 24 /* LP64_UNIX */, 24 /* X86_64_WINDOWS */ };
//private static final int[] e_flags_size = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 4 /* LP64_UNIX */, 4 /* X86_64_WINDOWS */  };
  private static final int[] e_ehsize_offset = new int[] { 16 /* ARM_MIPS_32 */, 16 /* X86_32_UNIX */, 16 /* X86_32_MACOS */, 16 /* PPC_32_UNIX */, 16 /* SPARC_32_SUNOS */, 16 /* X86_32_WINDOWS */, 28 /* LP64_UNIX */, 28 /* X86_64_WINDOWS */ };
//private static final int[] e_ehsize_size = new int[] { 2 /* ARM_MIPS_32 */, 2 /* X86_32_UNIX */, 2 /* X86_32_MACOS */, 2 /* PPC_32_UNIX */, 2 /* SPARC_32_SUNOS */, 2 /* X86_32_WINDOWS */, 2 /* LP64_UNIX */, 2 /* X86_64_WINDOWS */  };
  private static final int[] e_phentsize_offset = new int[] { 18 /* ARM_MIPS_32 */, 18 /* X86_32_UNIX */, 18 /* X86_32_MACOS */, 18 /* PPC_32_UNIX */, 18 /* SPARC_32_SUNOS */, 18 /* X86_32_WINDOWS */, 30 /* LP64_UNIX */, 30 /* X86_64_WINDOWS */ };
//private static final int[] e_phentsize_size = new int[] { 2 /* ARM_MIPS_32 */, 2 /* X86_32_UNIX */, 2 /* X86_32_MACOS */, 2 /* PPC_32_UNIX */, 2 /* SPARC_32_SUNOS */, 2 /* X86_32_WINDOWS */, 2 /* LP64_UNIX */, 2 /* X86_64_WINDOWS */  };
  private static final int[] e_phnum_offset = new int[] { 20 /* ARM_MIPS_32 */, 20 /* X86_32_UNIX */, 20 /* X86_32_MACOS */, 20 /* PPC_32_UNIX */, 20 /* SPARC_32_SUNOS */, 20 /* X86_32_WINDOWS */, 32 /* LP64_UNIX */, 32 /* X86_64_WINDOWS */ };
//private static final int[] e_phnum_size = new int[] { 2 /* ARM_MIPS_32 */, 2 /* X86_32_UNIX */, 2 /* X86_32_MACOS */, 2 /* PPC_32_UNIX */, 2 /* SPARC_32_SUNOS */, 2 /* X86_32_WINDOWS */, 2 /* LP64_UNIX */, 2 /* X86_64_WINDOWS */  };
  private static final int[] e_shentsize_offset = new int[] { 22 /* ARM_MIPS_32 */, 22 /* X86_32_UNIX */, 22 /* X86_32_MACOS */, 22 /* PPC_32_UNIX */, 22 /* SPARC_32_SUNOS */, 22 /* X86_32_WINDOWS */, 34 /* LP64_UNIX */, 34 /* X86_64_WINDOWS */ };
//private static final int[] e_shentsize_size = new int[] { 2 /* ARM_MIPS_32 */, 2 /* X86_32_UNIX */, 2 /* X86_32_MACOS */, 2 /* PPC_32_UNIX */, 2 /* SPARC_32_SUNOS */, 2 /* X86_32_WINDOWS */, 2 /* LP64_UNIX */, 2 /* X86_64_WINDOWS */  };
  private static final int[] e_shnum_offset = new int[] { 24 /* ARM_MIPS_32 */, 24 /* X86_32_UNIX */, 24 /* X86_32_MACOS */, 24 /* PPC_32_UNIX */, 24 /* SPARC_32_SUNOS */, 24 /* X86_32_WINDOWS */, 36 /* LP64_UNIX */, 36 /* X86_64_WINDOWS */ };
//private static final int[] e_shnum_size = new int[] { 2 /* ARM_MIPS_32 */, 2 /* X86_32_UNIX */, 2 /* X86_32_MACOS */, 2 /* PPC_32_UNIX */, 2 /* SPARC_32_SUNOS */, 2 /* X86_32_WINDOWS */, 2 /* LP64_UNIX */, 2 /* X86_64_WINDOWS */  };
  private static final int[] e_shstrndx_offset = new int[] { 26 /* ARM_MIPS_32 */, 26 /* X86_32_UNIX */, 26 /* X86_32_MACOS */, 26 /* PPC_32_UNIX */, 26 /* SPARC_32_SUNOS */, 26 /* X86_32_WINDOWS */, 38 /* LP64_UNIX */, 38 /* X86_64_WINDOWS */ };
//private static final int[] e_shstrndx_size = new int[] { 2 /* ARM_MIPS_32 */, 2 /* X86_32_UNIX */, 2 /* X86_32_MACOS */, 2 /* PPC_32_UNIX */, 2 /* SPARC_32_SUNOS */, 2 /* X86_32_WINDOWS */, 2 /* LP64_UNIX */, 2 /* X86_64_WINDOWS */  };

  public java.nio.ByteBuffer getBuffer() {
    return accessor.getBuffer();
  }

  /** Setter for native field: CType['ElfN_Addr' (typedef), size [fixed false, lnx64 8], [int]] */
  public Ehdr_p2 setE_entry(final long val) {
    accessor.setLongAt(e_entry_offset[mdIdx], val, md.longSizeInBytes());
    return this;
  }

  /** Getter for native field: CType['ElfN_Addr' (typedef), size [fixed false, lnx64 8], [int]] */
  public long getE_entry() {
    return accessor.getLongAt(e_entry_offset[mdIdx], md.longSizeInBytes());
  }

  /** Setter for native field: CType['ElfN_Off' (typedef), size [fixed false, lnx64 8], [int]] */
  public Ehdr_p2 setE_phoff(final long val) {
    accessor.setLongAt(e_phoff_offset[mdIdx], val, md.longSizeInBytes());
    return this;
  }

  /** Getter for native field: CType['ElfN_Off' (typedef), size [fixed false, lnx64 8], [int]] */
  public long getE_phoff() {
    return accessor.getLongAt(e_phoff_offset[mdIdx], md.longSizeInBytes());
  }

  /** Setter for native field: CType['ElfN_Off' (typedef), size [fixed false, lnx64 8], [int]] */
  public Ehdr_p2 setE_shoff(final long val) {
    accessor.setLongAt(e_shoff_offset[mdIdx], val, md.longSizeInBytes());
    return this;
  }

  /** Getter for native field: CType['ElfN_Off' (typedef), size [fixed false, lnx64 8], [int]] */
  public long getE_shoff() {
    return accessor.getLongAt(e_shoff_offset[mdIdx], md.longSizeInBytes());
  }

  /** Setter for native field: CType['uint32_t', size [fixed true, lnx64 4], [int]] */
  public Ehdr_p2 setE_flags(final int val) {
    accessor.setIntAt(e_flags_offset[mdIdx], val);
    return this;
  }

  /** Getter for native field: CType['uint32_t', size [fixed true, lnx64 4], [int]] */
  public int getE_flags() {
    return accessor.getIntAt(e_flags_offset[mdIdx]);
  }

  /** Setter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public Ehdr_p2 setE_ehsize(final short val) {
    accessor.setShortAt(e_ehsize_offset[mdIdx], val);
    return this;
  }

  /** Getter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public short getE_ehsize() {
    return accessor.getShortAt(e_ehsize_offset[mdIdx]);
  }

  /** Setter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public Ehdr_p2 setE_phentsize(final short val) {
    accessor.setShortAt(e_phentsize_offset[mdIdx], val);
    return this;
  }

  /** Getter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public short getE_phentsize() {
    return accessor.getShortAt(e_phentsize_offset[mdIdx]);
  }

  /** Setter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public Ehdr_p2 setE_phnum(final short val) {
    accessor.setShortAt(e_phnum_offset[mdIdx], val);
    return this;
  }

  /** Getter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public short getE_phnum() {
    return accessor.getShortAt(e_phnum_offset[mdIdx]);
  }

  /** Setter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public Ehdr_p2 setE_shentsize(final short val) {
    accessor.setShortAt(e_shentsize_offset[mdIdx], val);
    return this;
  }

  /** Getter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public short getE_shentsize() {
    return accessor.getShortAt(e_shentsize_offset[mdIdx]);
  }

  /** Setter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public Ehdr_p2 setE_shnum(final short val) {
    accessor.setShortAt(e_shnum_offset[mdIdx], val);
    return this;
  }

  /** Getter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public short getE_shnum() {
    return accessor.getShortAt(e_shnum_offset[mdIdx]);
  }

  /** Setter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public Ehdr_p2 setE_shstrndx(final short val) {
    accessor.setShortAt(e_shstrndx_offset[mdIdx], val);
    return this;
  }

  /** Getter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public short getE_shstrndx() {
    return accessor.getShortAt(e_shstrndx_offset[mdIdx]);
  }

  // --- Begin CustomJavaCode .cfg declarations
  public static int size(final int mdIdx) {
      return Ehdr_p2_size[mdIdx];
  }

  public static Ehdr_p2 create(final int mdIdx) {
      return create(mdIdx, Buffers.newDirectByteBuffer( size(mdIdx) ) );
  }

  public static Ehdr_p2 create(final int mdIdx, final java.nio.ByteBuffer buf) {
      return new Ehdr_p2(mdIdx, buf);
  }

  Ehdr_p2(final int mdIdx, final java.nio.ByteBuffer buf) {
      this.mdIdx = mdIdx;
      this.md = MachineDataInfo.StaticConfig.values()[mdIdx].md;
      this.accessor = new StructAccessor(buf);
  }
  // ---- End CustomJavaCode .cfg declarations
}
