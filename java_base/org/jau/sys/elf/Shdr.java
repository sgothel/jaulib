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

public class Shdr {

  StructAccessor accessor;

  private final int mdIdx;
  private final MachineDataInfo md;

  private static final int[] Shdr_size = new int[] { 40 /* ARM_MIPS_32 */, 40 /* X86_32_UNIX */, 40 /* X86_32_MACOS */, 40 /* PPC_32_UNIX */, 40 /* SPARC_32_SUNOS */, 40 /* X86_32_WINDOWS */, 64 /* LP64_UNIX */, 64 /* X86_64_WINDOWS */  };
  private static final int[] sh_name_offset = new int[] { 0 /* ARM_MIPS_32 */, 0 /* X86_32_UNIX */, 0 /* X86_32_MACOS */, 0 /* PPC_32_UNIX */, 0 /* SPARC_32_SUNOS */, 0 /* X86_32_WINDOWS */, 0 /* LP64_UNIX */, 0 /* X86_64_WINDOWS */ };
//private static final int[] sh_name_size = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 4 /* LP64_UNIX */, 4 /* X86_64_WINDOWS */  };
  private static final int[] sh_type_offset = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 4 /* LP64_UNIX */, 4 /* X86_64_WINDOWS */ };
//private static final int[] sh_type_size = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 4 /* LP64_UNIX */, 4 /* X86_64_WINDOWS */  };
  private static final int[] sh_flags_offset = new int[] { 8 /* ARM_MIPS_32 */, 8 /* X86_32_UNIX */, 8 /* X86_32_MACOS */, 8 /* PPC_32_UNIX */, 8 /* SPARC_32_SUNOS */, 8 /* X86_32_WINDOWS */, 8 /* LP64_UNIX */, 8 /* X86_64_WINDOWS */ };
//private static final int[] sh_flags_size = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 8 /* LP64_UNIX */, 8 /* X86_64_WINDOWS */  };
  private static final int[] sh_addr_offset = new int[] { 12 /* ARM_MIPS_32 */, 12 /* X86_32_UNIX */, 12 /* X86_32_MACOS */, 12 /* PPC_32_UNIX */, 12 /* SPARC_32_SUNOS */, 12 /* X86_32_WINDOWS */, 16 /* LP64_UNIX */, 16 /* X86_64_WINDOWS */ };
//private static final int[] sh_addr_size = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 8 /* LP64_UNIX */, 8 /* X86_64_WINDOWS */  };
  private static final int[] sh_offset_offset = new int[] { 16 /* ARM_MIPS_32 */, 16 /* X86_32_UNIX */, 16 /* X86_32_MACOS */, 16 /* PPC_32_UNIX */, 16 /* SPARC_32_SUNOS */, 16 /* X86_32_WINDOWS */, 24 /* LP64_UNIX */, 24 /* X86_64_WINDOWS */ };
//private static final int[] sh_offset_size = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 8 /* LP64_UNIX */, 8 /* X86_64_WINDOWS */  };
  private static final int[] sh_size_offset = new int[] { 20 /* ARM_MIPS_32 */, 20 /* X86_32_UNIX */, 20 /* X86_32_MACOS */, 20 /* PPC_32_UNIX */, 20 /* SPARC_32_SUNOS */, 20 /* X86_32_WINDOWS */, 32 /* LP64_UNIX */, 32 /* X86_64_WINDOWS */ };
//private static final int[] sh_size_size = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 8 /* LP64_UNIX */, 8 /* X86_64_WINDOWS */  };
  private static final int[] sh_link_offset = new int[] { 24 /* ARM_MIPS_32 */, 24 /* X86_32_UNIX */, 24 /* X86_32_MACOS */, 24 /* PPC_32_UNIX */, 24 /* SPARC_32_SUNOS */, 24 /* X86_32_WINDOWS */, 40 /* LP64_UNIX */, 40 /* X86_64_WINDOWS */ };
//private static final int[] sh_link_size = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 4 /* LP64_UNIX */, 4 /* X86_64_WINDOWS */  };
  private static final int[] sh_info_offset = new int[] { 28 /* ARM_MIPS_32 */, 28 /* X86_32_UNIX */, 28 /* X86_32_MACOS */, 28 /* PPC_32_UNIX */, 28 /* SPARC_32_SUNOS */, 28 /* X86_32_WINDOWS */, 44 /* LP64_UNIX */, 44 /* X86_64_WINDOWS */ };
//private static final int[] sh_info_size = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 4 /* LP64_UNIX */, 4 /* X86_64_WINDOWS */  };
  private static final int[] sh_addralign_offset = new int[] { 32 /* ARM_MIPS_32 */, 32 /* X86_32_UNIX */, 32 /* X86_32_MACOS */, 32 /* PPC_32_UNIX */, 32 /* SPARC_32_SUNOS */, 32 /* X86_32_WINDOWS */, 48 /* LP64_UNIX */, 48 /* X86_64_WINDOWS */ };
//private static final int[] sh_addralign_size = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 8 /* LP64_UNIX */, 8 /* X86_64_WINDOWS */  };
  private static final int[] sh_entsize_offset = new int[] { 36 /* ARM_MIPS_32 */, 36 /* X86_32_UNIX */, 36 /* X86_32_MACOS */, 36 /* PPC_32_UNIX */, 36 /* SPARC_32_SUNOS */, 36 /* X86_32_WINDOWS */, 56 /* LP64_UNIX */, 56 /* X86_64_WINDOWS */ };
//private static final int[] sh_entsize_size = new int[] { 4 /* ARM_MIPS_32 */, 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 8 /* LP64_UNIX */, 8 /* X86_64_WINDOWS */  };

  public java.nio.ByteBuffer getBuffer() {
    return accessor.getBuffer();
  }

  /** Setter for native field: CType['uint32_t', size [fixed true, lnx64 4], [int]] */
  public Shdr setSh_name(final int val) {
    accessor.setIntAt(sh_name_offset[mdIdx], val);
    return this;
  }

  /** Getter for native field: CType['uint32_t', size [fixed true, lnx64 4], [int]] */
  public int getSh_name() {
    return accessor.getIntAt(sh_name_offset[mdIdx]);
  }

  /** Setter for native field: CType['uint32_t', size [fixed true, lnx64 4], [int]] */
  public Shdr setSh_type(final int val) {
    accessor.setIntAt(sh_type_offset[mdIdx], val);
    return this;
  }

  /** Getter for native field: CType['uint32_t', size [fixed true, lnx64 4], [int]] */
  public int getSh_type() {
    return accessor.getIntAt(sh_type_offset[mdIdx]);
  }

  /** Setter for native field: CType['ElfN_size' (typedef), size [fixed false, lnx64 8], [int]] */
  public Shdr setSh_flags(final long val) {
    accessor.setLongAt(sh_flags_offset[mdIdx], val, md.longSizeInBytes());
    return this;
  }

  /** Getter for native field: CType['ElfN_size' (typedef), size [fixed false, lnx64 8], [int]] */
  public long getSh_flags() {
    return accessor.getLongAt(sh_flags_offset[mdIdx], md.longSizeInBytes());
  }

  /** Setter for native field: CType['ElfN_Addr' (typedef), size [fixed false, lnx64 8], [int]] */
  public Shdr setSh_addr(final long val) {
    accessor.setLongAt(sh_addr_offset[mdIdx], val, md.longSizeInBytes());
    return this;
  }

  /** Getter for native field: CType['ElfN_Addr' (typedef), size [fixed false, lnx64 8], [int]] */
  public long getSh_addr() {
    return accessor.getLongAt(sh_addr_offset[mdIdx], md.longSizeInBytes());
  }

  /** Setter for native field: CType['ElfN_Off' (typedef), size [fixed false, lnx64 8], [int]] */
  public Shdr setSh_offset(final long val) {
    accessor.setLongAt(sh_offset_offset[mdIdx], val, md.longSizeInBytes());
    return this;
  }

  /** Getter for native field: CType['ElfN_Off' (typedef), size [fixed false, lnx64 8], [int]] */
  public long getSh_offset() {
    return accessor.getLongAt(sh_offset_offset[mdIdx], md.longSizeInBytes());
  }

  /** Setter for native field: CType['ElfN_size' (typedef), size [fixed false, lnx64 8], [int]] */
  public Shdr setSh_size(final long val) {
    accessor.setLongAt(sh_size_offset[mdIdx], val, md.longSizeInBytes());
    return this;
  }

  /** Getter for native field: CType['ElfN_size' (typedef), size [fixed false, lnx64 8], [int]] */
  public long getSh_size() {
    return accessor.getLongAt(sh_size_offset[mdIdx], md.longSizeInBytes());
  }

  /** Setter for native field: CType['uint32_t', size [fixed true, lnx64 4], [int]] */
  public Shdr setSh_link(final int val) {
    accessor.setIntAt(sh_link_offset[mdIdx], val);
    return this;
  }

  /** Getter for native field: CType['uint32_t', size [fixed true, lnx64 4], [int]] */
  public int getSh_link() {
    return accessor.getIntAt(sh_link_offset[mdIdx]);
  }

  /** Setter for native field: CType['uint32_t', size [fixed true, lnx64 4], [int]] */
  public Shdr setSh_info(final int val) {
    accessor.setIntAt(sh_info_offset[mdIdx], val);
    return this;
  }

  /** Getter for native field: CType['uint32_t', size [fixed true, lnx64 4], [int]] */
  public int getSh_info() {
    return accessor.getIntAt(sh_info_offset[mdIdx]);
  }

  /** Setter for native field: CType['ElfN_size' (typedef), size [fixed false, lnx64 8], [int]] */
  public Shdr setSh_addralign(final long val) {
    accessor.setLongAt(sh_addralign_offset[mdIdx], val, md.longSizeInBytes());
    return this;
  }

  /** Getter for native field: CType['ElfN_size' (typedef), size [fixed false, lnx64 8], [int]] */
  public long getSh_addralign() {
    return accessor.getLongAt(sh_addralign_offset[mdIdx], md.longSizeInBytes());
  }

  /** Setter for native field: CType['ElfN_size' (typedef), size [fixed false, lnx64 8], [int]] */
  public Shdr setSh_entsize(final long val) {
    accessor.setLongAt(sh_entsize_offset[mdIdx], val, md.longSizeInBytes());
    return this;
  }

  /** Getter for native field: CType['ElfN_size' (typedef), size [fixed false, lnx64 8], [int]] */
  public long getSh_entsize() {
    return accessor.getLongAt(sh_entsize_offset[mdIdx], md.longSizeInBytes());
  }

  // --- Begin CustomJavaCode .cfg declarations
  public static int size(final int mdIdx) {
      return Shdr_size[mdIdx];
  }

  public static Shdr create(final int mdIdx) {
      return create(mdIdx, Buffers.newDirectByteBuffer( size(mdIdx) ) );
  }

  public static Shdr create(final int mdIdx, final java.nio.ByteBuffer buf) {
      return new Shdr(mdIdx, buf);
  }

  Shdr(final int mdIdx, final java.nio.ByteBuffer buf) {
      this.mdIdx = mdIdx;
      this.md = MachineDataInfo.StaticConfig.values()[mdIdx].md;
      this.accessor = new StructAccessor(buf);
  }
  // ---- End CustomJavaCode .cfg declarations
}
