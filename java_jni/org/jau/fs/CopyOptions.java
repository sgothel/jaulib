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
 * Filesystem copy options used to copy() path elements.
 *
 * By default, the fmode_t POSIX protection mode bits are preserved
 * while using the caller's uid and gid as well as current timestamps. <br />
 * Use {@link CopyOptions.Bit#preserve_all} to preserve uid and gid if allowed from the caller and access- and modification-timestamps.
 *
 * @see FileUtil#copy(String, String, CopyOptions)
 */
public class CopyOptions {
    public enum Bit {
        /** No option set */
        none ( (short) 0 ),

        /** Traverse through directories, i.e. perform visit, copy, remove etc actions recursively throughout the directory structure. */
        recursive ( (short)( 1 << 0 ) ),

        /** Copy referenced symbolic linked files or directories instead of just the symbolic link with property fmode_t::link set. */
        follow_symlinks ( (short)( 1 << 1 ) ),

        /**
         * Ignore errors from erroneous symlinks, i.e. non-existing link-targets or recursive loop-errors.
         *
         * This flag is required to copy erroneous symlinks using follow_symlinks, otherwise not.
         */
        ignore_symlink_errors ( (short)( 1 << 8 ) ),

        /** Overwrite existing destination files. */
        overwrite ( (short)( 1 << 9 ) ),

        /** Preserve uid and gid if allowed and access- and modification-timestamps, i.e. producing a most exact meta-data copy. */
        preserve_all ( (short)( 1 << 10 ) ),

        /** Ensure data and meta-data file synchronization is performed via ::fsync() after asynchronous copy operations of a file's content. */
        sync ( (short)( 1 << 11 ) ),

        /** Enable verbosity mode, show error messages on stderr. */
        verbose ( (short)( 1 << 15 ) );

        Bit(final short v) { value = v; }
        public final short value;
    }

    public short mask;

    public CopyOptions(final short v) {
        mask = v;
    }
    public CopyOptions() {
        mask = 0;
    }

    public boolean isSet(final Bit bit) { return bit.value == ( mask & bit.value ); }
    public void set(final Bit bit) { mask = (short) ( mask | bit.value ); }

    @Override
    public String toString() {
        int count = 0;
        final StringBuilder out = new StringBuilder();
        for (final Bit dt : Bit.values()) {
            if( isSet(dt) ) {
                if( 0 < count ) { out.append(", "); }
                out.append(dt.name()); count++;
            }
        }
        if( 1 < count ) {
            out.insert(0, "[");
            out.append("]");
        }
        return out.toString();
    }
    @Override
    public boolean equals(final Object other) {
        if (this == other) {
            return true;
        }
        return (other instanceof CopyOptions) &&
               this.mask == ((CopyOptions)other).mask;
    }
}
