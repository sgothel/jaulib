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
 * Generic flag bit class for umount() `flags`
 *
 * See umount(2) for a detailed description.
 *
 * @see FileUtil#umount(long, int)
 * @see FileUtil#umount(String, int)
 */
public class UnmountFlags {

    public static interface Bit {
        String name();
        int value();
    }
    public static enum Bit0 implements Bit {
        none ( 0 );

        Bit0(final int v) { _value = v; }
        private int _value;

        @Override
        public int value() { return _value; }
    }
    protected Bit[] bit_values() { return Bit0.values(); }

    private int mask;

    public int value() { return mask; }

    protected UnmountFlags(final int v) {
        mask = v;
    }
    public UnmountFlags() {
        mask = 0;
    }

    public boolean isSet(final Bit bit) { return bit.value() == ( mask & bit.value() ); }

    public UnmountFlags set(final Bit bit) { mask = mask | bit.value(); return this; }

    @Override
    public String toString() {
        int count = 0;
        final StringBuilder out = new StringBuilder();
        for (final Bit dt : bit_values()) {
            if( 0 != dt.value() && isSet(dt) ) {
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
        return (other instanceof UnmountFlags) &&
               this.mask == ((UnmountFlags)other).mask;
    }
}