/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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
package org.jau.fs;

/**
 * Generic file type and POSIX protection mode bits as used in file_stats, touch(), mkdir() etc.
 *
 * The POSIX protection mode bits reside in the lower 16-bits and are bit-wise POSIX compliant
 * while the file type bits reside in the upper 16-bits and are platform agnostic.
 *
 * This `enum class` type fulfills `C++ named requirements: BitmaskType`.
 *
 * @see FileStats
 * @see FileStats#mode()
 */
public class FMode {
    public enum Bit {
        /** No mode bit set */
        none       ( 0 ),

        /** Protection bit: POSIX S_ISUID */
        set_uid    ( 04000 ),
        /** Protection bit: POSIX S_ISGID */
        set_gid    ( 02000 ),
        /** Protection bit: POSIX S_ISVTX */
        sticky     ( 01000 ),
        /** Protection bit: POSIX S_ISUID | S_ISGID | S_ISVTX */
        ugs_set    ( 07000 ),

        /** Protection bit: POSIX S_IRUSR */
        read_usr   ( 00400 ),
        /** Protection bit: POSIX S_IWUSR */
        write_usr  ( 00200 ),
        /** Protection bit: POSIX S_IXUSR */
        exec_usr   ( 00100 ),
        /** Protection bit: POSIX S_IRWXU */
        rwx_usr    ( 00700 ),

        /** Protection bit: POSIX S_IRGRP */
        read_grp   ( 00040 ),
        /** Protection bit: POSIX S_IWGRP */
        write_grp  ( 00020 ),
        /** Protection bit: POSIX S_IXGRP */
        exec_grp   ( 00010 ),
        /** Protection bit: POSIX S_IRWXG */
        rwx_grp    ( 00070 ),

        /** Protection bit: POSIX S_IROTH */
        read_oth   ( 00004 ),
        /** Protection bit: POSIX S_IWOTH */
        write_oth  ( 00002 ),
        /** Protection bit: POSIX S_IXOTH */
        exec_oth   ( 00001 ),
        /** Protection bit: POSIX S_IRWXO */
        rwx_oth    ( 00007 ),

        /** Protection bit: POSIX S_IRWXU | S_IRWXG | S_IRWXO or rwx_usr | rwx_grp | rwx_oth */
        rwx_all    ( 00777 ),

        /** Default directory protection bit: Safe default: POSIX S_IRWXU | S_IRGRP | S_IXGRP or rwx_usr | read_grp | exec_grp */
        def_dir_prot    ( 00750 ),

        /** Default file protection bit: Safe default: POSIX S_IRUSR | S_IWUSR | S_IRGRP or read_usr | write_usr | read_grp */
        def_file_prot   ( 00640 ),

        /** 12 bit protection bit mask 07777 for rwx_all | set_uid | set_gid | sticky . */
        protection_mask ( 0b000000000111111111111 ),

        /** Type: Entity is a file descriptor, might be in combination with link. */
        fd              ( 0b000001000000000000000 ),
        /** Type: Entity is a directory ), might be in combination with link. */
        dir             ( 0b000010000000000000000 ),
        /** Type: Entity is a file ), might be in combination with link. */
        file            ( 0b000100000000000000000 ),
        /** Type: Entity is a symbolic link ), might be in combination with file or dir. */
        link            ( 0b001000000000000000000 ),
        /** Type: Entity gives no access to user ), exclusive bit. */
        no_access       ( 0b010000000000000000000 ),
        /** Type: Entity does not exist ), exclusive bit. */
        not_existing    ( 0b100000000000000000000 ),
        /** Type mask for fd | dir | file | link | no_access | not_existing. */
        type_mask       ( 0b111111000000000000000 );

        Bit(final int v) { value = v; }
        public final int value;
    }
    public int mask;

    public FMode(final int v) {
        mask = v;
    }
    public FMode() {
        mask = 0;
    }

    public boolean isSet(final Bit bit) { return bit.value == ( mask & bit.value ); }
    public void set(final Bit bit) { mask = (short) ( mask | bit.value ); }
    public FMode mask(final int bits) {
        final int r = mask & bits;
        if( r == mask ) { return this; }
        else { return new FMode(r); }
    }

    private static native String to_string(final int mask, final boolean show_rwx);

    @Override
    public String toString() {
        return to_string(mask, false);
    }
    public String toString(final boolean show_rwx) {
        return to_string(mask, show_rwx);
    }

    @Override
    public boolean equals(final Object other) {
        if (this == other) {
            return true;
        }
        return (other instanceof FMode) &&
               this.mask == ((FMode)other).mask;
    }

    /** Default directory protection bit: Safe default: POSIX S_IRWXU | S_IRGRP | S_IXGRP or rwx_usr | read_grp | exec_grp */
    public static final FMode def_dir = new FMode(FMode.Bit.def_dir_prot.value);

    /** Default file protection bit: Safe default: POSIX S_IRUSR | S_IWUSR | S_IRGRP or read_usr | write_usr | read_grp */
    public static final FMode def_file = new FMode(FMode.Bit.def_file_prot.value);
}
