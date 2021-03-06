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

import static org.jau.sys.elf.IOUtils.getString;
import static org.jau.sys.elf.IOUtils.long2Int;
import static org.jau.sys.elf.IOUtils.readBytes;
import static org.jau.sys.elf.IOUtils.seek;
import static org.jau.sys.elf.IOUtils.toHexString;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;

/**
 * ELF ABI Section Header
 * <p>
 * References:
 * <ul>
 *   <li>http://linux.die.net/man/5/elf</li>
 *   <li>http://www.sco.com/developers/gabi/latest/contents.html</li>
 *   <li>http://infocenter.arm.com/
 *   <ul>
 *      <li>ARM IHI 0044E, current through ABI release 2.09</li>
 *   </ul></li>
 * </ul>
 * </p>
 */
public class SectionHeader {
    /**
     * {@value}
     */
    public static final int SHT_NULL        = 0;
    /**
     * {@value}
     */
    public static final int SHT_PROGBITS    = 1;
    /**
     * {@value}
     */
    public static final int SHT_SYMTAB      = 2;
    /**
     * {@value}
     */
    public static final int SHT_STRTAB      = 3;
    /**
     * {@value}
     */
    public static final int SHT_RELA        = 4;
    /**
     * {@value}
     */
    public static final int SHT_HASH        = 5;
    /**
     * {@value}
     */
    public static final int SHT_DYNAMIC     = 6;
    /**
     * {@value}
     */
    public static final int SHT_NOTE        = 7;
    /**
     * {@value}
     */
    public static final int SHT_NOBITS      = 8;
    /**
     * {@value}
     */
    public static final int SHT_REL         = 9;
    /**
     * {@value}
     */
    public static final int SHT_SHLIB       = 10;
    /**
     * {@value}
     */
    public static final int SHT_DYNSYM      = 11;
    /**
     * {@value}
     */
    public static final int SHT_NUM         = 12;
    /**
     * {@value}
     */
    public static final int SHT_LOPROC      = 0x70000000;
    /**
     * {@value}
     */
    public static final int SHT_HIPROC      = 0x7fffffff;
    /**
     * {@value}
     */
    public static final int SHT_LOUSER      = 0x80000000;
    /**
     * {@value}
     */
    public static final int SHT_HIUSER      = 0xffffffff;

    /**
     * {@value}
     */
    public static final int SHT_ARM_EXIDX          = 0x70000001;
    /**
     * {@value}
     */
    public static final int SHT_ARM_PREEMPTMAP     = 0x70000002;
    /**
     * {@value}
     */
    public static final int SHT_ARM_ATTRIBUTES     = 0x70000003;

    /**
     * {@value}. FIXME: Same as {@link #SHT_ARM_ATTRIBUTES}, ok?
     */
    public static final int SHT_AARCH64_ATTRIBUTES = 0x70000003;

    /**
     * {@value}
     */
    public static final int SHT_ARM_DEBUGOVERLAY   = 0x70000004;
    /**
     * {@value}
     */
    public static final int SHT_ARM_OVERLAYSECTION = 0x70000005;

    /**
     * {@value}
     */
    public static final short SHN_UNDEF     = (short)0;
    /**
     * {@value}
     */
    public static final short SHN_LORESERVE = (short)0xff00;
    /**
     * {@value}
     */
    public static final short SHN_LOPROC    = (short)0xff00;
    /**
     * {@value}
     */
    public static final short SHN_HIPROC    = (short)0xff1f;
    /**
     * {@value}
     */
    public static final short SHN_ABS       = (short)0xfff1;
    /**
     * {@value}
     */
    public static final short SHN_COMMON    = (short)0xfff2;
    /**
     * {@value}
     */
    public static final short SHN_HIRESERVE = (short)0xffff;

    /** Public access to the elf header */
    public final ElfHeaderPart2 eh2;

    /** Public access to the raw elf section header */
    public final Shdr raw;

    private final int idx;
    private String name;

    SectionHeader(final ElfHeaderPart2 eh, final byte[] buf, final int offset, final int length, final int sectionIdx) {
        this( eh, ByteBuffer.wrap(buf, 0, buf.length), sectionIdx );
    }
    SectionHeader(final ElfHeaderPart2 eh, final java.nio.ByteBuffer buf, final int idx) {
        this.eh2 = eh;
        this.raw = Shdr.create(eh.eh1.machDesc.ordinal(), buf);
        this.idx = idx;
        this.name = null;
    }

    @Override
    public String toString() {
        return "SectionHeader[idx "+idx+", name "+name+", type "+toHexString(getType())+", link "+raw.getSh_link()+", info "+toHexString(raw.getSh_info())+", flags "+toHexString(getFlags())+"]";
    }

    /**
     * @param strS the {@link SectionHeader#SHT_STRTAB} section containing all strings
     * @param nameOffset name offset within strS
     */
    void initName(final Section strS, final int nameOffset) throws IndexOutOfBoundsException {
        name = getString(strS.data, strS.offset + nameOffset, strS.length - nameOffset, null);
    }

    /** Returns the index of this section within the Elf section header table. */
    public int getIndex() {
        return idx;
    }

    /** Returns the type of this section. */
    public int getType() {
        return raw.getSh_type();
    }

    /** Returns the flags of this section. */
    public long getFlags() {
        return raw.getSh_flags();
    }

    /** Returns the size of this section. */
    public long getSize() {
        return raw.getSh_size();
    }

    /** Returns this section name, maybe <code>null</code> if not read. */
    public String getName() {
        return name;
    }

    /**
     * Returns the Section referenced w/ this section header
     *
     * @param in file owning the section
     * @throws IOException if read error occurs
     * @throws IllegalArgumentException if section offset or size mismatch including size &gt; {@link Integer#MAX_VALUE}
     */
    public Section readSection(final RandomAccessFile in) throws IOException, IllegalArgumentException {
        final int s_size = long2Int(raw.getSh_size());
        if( 0 == s_size || 0 > s_size ) {
            throw new IllegalArgumentException("Shdr["+idx+"] has invalid int size: "+raw.getSh_size()+" -> "+s_size);
        }
        final byte[] s_buf = new byte[s_size];
        return readSectionImpl(in, s_buf, 0, s_size);
    }

    /**
     * Returns the Section referenced w/ this section header using given byte array.
     *
     * @param in file owning the section
     * @param b destination buffer
     * @param b_off offset in destination buffer
     * @param r_len requested read length in bytes, which shall be &le; than this section size
     * @throws IOException if read error occurs
     * @throws IllegalArgumentException if section offset or size mismatch including size &gt; {@link Integer#MAX_VALUE}
     * @throws IllegalArgumentException if requested read length is &gt; section size
     */
    public Section readSection(final RandomAccessFile in, final byte[] b, final int b_off, final int r_len) throws IOException, IllegalArgumentException {
        final int s_size = long2Int(raw.getSh_size());
        if( 0 == s_size || 0 > s_size ) {
            throw new IllegalArgumentException("Shdr["+idx+"] has invalid int size: "+raw.getSh_size()+" -> "+s_size);
        }
        if( r_len > s_size ) {
            throw new IllegalArgumentException("Shdr["+idx+"] has only "+s_size+" bytes, while read request is of "+r_len+" bytes");
        }
        return readSectionImpl(in, b, b_off, r_len);
    }

    Section readSectionImpl(final RandomAccessFile in, final byte[] b, final int b_off, final int r_len) throws IOException, IllegalArgumentException {
        final long s_off = raw.getSh_offset();
        seek(in, s_off);
        readBytes(in, b, b_off, r_len);
        if( SectionHeader.SHT_ARM_ATTRIBUTES == getType() ) {
            return new SectionArmAttributes(this, b, b_off, r_len);
        } else {
            return new Section(this, b, b_off, r_len);
        }
    }
}
