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

import java.nio.ByteBuffer;

import org.jau.lang.NioUtil;
import org.jau.lang.StructAccessor;
// import org.jau.sys.MachineDataInfo;

public class Ehdr_p1 {

  StructAccessor accessor;

  // private static final int mdIdx = 0;
  // private final MachineDataInfo md;

  private static final int Ehdr_p1_size = 24 /* ARM_MIPS_32 */; //  24 /* X86_32_UNIX */, 24 /* X86_32_MACOS */, 24 /* PPC_32_UNIX */, 24 /* SPARC_32_SUNOS */, 24 /* X86_32_WINDOWS */, 24 /* LP64_UNIX */, 24 /* X86_64_WINDOWS */
  private static final int e_ident_offset = 0 /* ARM_MIPS_32 */; // 0 /* X86_32_UNIX */, 0 /* X86_32_MACOS */, 0 /* PPC_32_UNIX */, 0 /* SPARC_32_SUNOS */, 0 /* X86_32_WINDOWS */, 0 /* LP64_UNIX */, 0 /* X86_64_WINDOWS */
// private static final int e_ident_size = 16 /* ARM_MIPS_32 */; // 16 /* X86_32_UNIX */, 16 /* X86_32_MACOS */, 16 /* PPC_32_UNIX */, 16 /* SPARC_32_SUNOS */, 16 /* X86_32_WINDOWS */, 16 /* LP64_UNIX */, 16 /* X86_64_WINDOWS */
  private static final int e_type_offset = 16 /* ARM_MIPS_32 */; // 16 /* X86_32_UNIX */, 16 /* X86_32_MACOS */, 16 /* PPC_32_UNIX */, 16 /* SPARC_32_SUNOS */, 16 /* X86_32_WINDOWS */, 16 /* LP64_UNIX */, 16 /* X86_64_WINDOWS */
//private static final int e_type_size = 2 /* ARM_MIPS_32 */; // 2 /* X86_32_UNIX */, 2 /* X86_32_MACOS */, 2 /* PPC_32_UNIX */, 2 /* SPARC_32_SUNOS */, 2 /* X86_32_WINDOWS */, 2 /* LP64_UNIX */, 2 /* X86_64_WINDOWS */
  private static final int e_machine_offset = 18 /* ARM_MIPS_32 */; // 18 /* X86_32_UNIX */, 18 /* X86_32_MACOS */, 18 /* PPC_32_UNIX */, 18 /* SPARC_32_SUNOS */, 18 /* X86_32_WINDOWS */, 18 /* LP64_UNIX */, 18 /* X86_64_WINDOWS */
//private static final int e_machine_size = 2 /* ARM_MIPS_32 */; // 2 /* X86_32_UNIX */, 2 /* X86_32_MACOS */, 2 /* PPC_32_UNIX */, 2 /* SPARC_32_SUNOS */, 2 /* X86_32_WINDOWS */, 2 /* LP64_UNIX */, 2 /* X86_64_WINDOWS */
  private static final int e_version_offset = 20 /* ARM_MIPS_32 */; // 20 /* X86_32_UNIX */, 20 /* X86_32_MACOS */, 20 /* PPC_32_UNIX */, 20 /* SPARC_32_SUNOS */, 20 /* X86_32_WINDOWS */, 20 /* LP64_UNIX */, 20 /* X86_64_WINDOWS */
//private static final int e_version_size = 4 /* ARM_MIPS_32 */; // 4 /* X86_32_UNIX */, 4 /* X86_32_MACOS */, 4 /* PPC_32_UNIX */, 4 /* SPARC_32_SUNOS */, 4 /* X86_32_WINDOWS */, 4 /* LP64_UNIX */, 4 /* X86_64_WINDOWS */

  public static int size() {
    return Ehdr_p1_size;
  }

  public static Ehdr_p1 create() {
    return create( NioUtil.newNativeByteBuffer( size() ) );
  }

  public static Ehdr_p1 create(final java.nio.ByteBuffer buf) {
      return new Ehdr_p1(buf);
  }

  Ehdr_p1(final java.nio.ByteBuffer buf) {
    // md = MachineDataInfo.StaticConfig.values()[mdIdx].md;
    accessor = new StructAccessor(buf);
  }

  public java.nio.ByteBuffer getBuffer() {
    return accessor.getBuffer();
  }

  /** Getter for native field: CType['char *', size [fixed false, lnx64 16], [array*1]], with array length of <code>16</code> */
  public static final int getE_identArrayLength() {
    return 16;
  }

  /** Setter for native field: CType['char *', size [fixed false, lnx64 16], [array*1]], with array length of <code>16</code> */
  public Ehdr_p1 setE_ident(final int offset, final byte[] val) {
    final int arrayLength = 16;
    if( offset + val.length > arrayLength ) { throw new IndexOutOfBoundsException("offset "+offset+" + val.length "+val.length+" > array-length "+arrayLength); };
    final int elemSize = NioUtil.SIZEOF_BYTE;
    final ByteBuffer destB = getBuffer();
    final int bTotal = arrayLength * elemSize;
    // if( bTotal > e_ident_size ) { throw new IndexOutOfBoundsException("bTotal "+bTotal+" > size "+e_ident_size+", elemSize "+elemSize+" * "+arrayLength); };
    int bOffset = e_ident_offset;
    final int bLimes = bOffset + bTotal;
    if( bLimes > destB.limit() ) { throw new IndexOutOfBoundsException("bLimes "+bLimes+" > buffer.limit "+destB.limit()+", elemOff "+bOffset+", elemSize "+elemSize+" * "+arrayLength); };
    bOffset += elemSize * offset;
    accessor.setBytesAt(bOffset, val);
    return this;
  }

  /** Getter for native field: CType['char *', size [fixed false, lnx64 16], [array*1]], with array length of <code>16</code> */
  public ByteBuffer getE_ident() {
    return accessor.slice(e_ident_offset,  NioUtil.SIZEOF_BYTE * 16);
  }

  /** Getter for native field: CType['char *', size [fixed false, lnx64 16], [array*1]], with array length of <code>16</code> */
  public byte[] getE_ident(final int offset, final byte result[]) {
    final int arrayLength = 16;
    if( offset + result.length > arrayLength ) { throw new IndexOutOfBoundsException("offset "+offset+" + result.length "+result.length+" > array-length "+arrayLength); };
    return accessor.getBytesAt(e_ident_offset + (NioUtil.SIZEOF_BYTE * offset), result);
  }


  /** Setter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public Ehdr_p1 setE_type(final short val) {
    accessor.setShortAt(e_type_offset, val);
    return this;
  }

  /** Getter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public short getE_type() {
    return accessor.getShortAt(e_type_offset);
  }

  /** Setter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public Ehdr_p1 setE_machine(final short val) {
    accessor.setShortAt(e_machine_offset, val);
    return this;
  }

  /** Getter for native field: CType['uint16_t', size [fixed true, lnx64 2], [int]] */
  public short getE_machine() {
    return accessor.getShortAt(e_machine_offset);
  }

  /** Setter for native field: CType['uint32_t', size [fixed true, lnx64 4], [int]] */
  public Ehdr_p1 setE_version(final int val) {
    accessor.setIntAt(e_version_offset, val);
    return this;
  }

  /** Getter for native field: CType['uint32_t', size [fixed true, lnx64 4], [int]] */
  public int getE_version() {
    return accessor.getIntAt(e_version_offset);
  }
}
