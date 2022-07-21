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
 * Filesystem traverse options used to visit() path elements.
 *
 * This `enum class` type fulfills `C++ named requirements: BitmaskType`.
 *
 * @see FileUtil#visit(FileStats, TraverseOptions, org.jau.fs.FileUtil.PathVisitor)
 * @see FileUtil#remove(String, TraverseOptions)
 */
public class TraverseOptions {
    public static final TraverseOptions none = new TraverseOptions();
    public static final TraverseOptions recursive = new TraverseOptions(TraverseOptions.Bit.recursive.value);

    public enum Bit {
        /** No option set */
        none ( (short) 0 ),

        /** Traverse through directories, i.e. perform visit, copy, remove etc actions recursively throughout the directory structure. */
        recursive ( (short) ( 1 << 0 ) ),

        /** Traverse through symbolic linked directories if traverse_options::recursive is set, i.e. directories with property fmode_t::link set. */
        follow_symlinks ( (short) ( 1 << 1 ) ),

        /** Visit the content's parent directory at entry. Both, dir_entry and dir_exit can be set, only one or none. */
        dir_entry ( (short) ( 1 << 2 ) ),

        /** Visit the content's parent directory at exit. Both, dir_entry and dir_exit can be set, only one or none. */
        dir_exit ( (short) ( 1 << 3 ) ),

        /** Enable verbosity mode, potentially used by a path_visitor implementation like remove(). */
        verbose ( (short) ( 1 << 15 ) );

        Bit(final short v) { value = v; }
        public final short value;
    }
    public short mask;

    public TraverseOptions(final short v) {
        mask = v;
    }
    public TraverseOptions() {
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
        return (other instanceof TraverseOptions) &&
               this.mask == ((TraverseOptions)other).mask;
    }
}
