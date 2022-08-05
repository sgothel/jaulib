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
package org.jau.io;

/**
 * Mimic std::ios_base::iostate for state functionality, see iostate_func.
 *
 * This `enum class` type fulfills `C++ named requirements: BitmaskType`.
 *
 * @see ByteInStream
 */
public class IOState {
    public enum Bit {
        /** No error occurred nor has EOS being reached. Value is no bit set! */
        none       ( 0 ),

        /** No error occurred nor has EOS being reached. Value is no bit set! */
        goodbit ( 0 ),

        /** Irrecoverable stream error, including loss of integrity of the underlying stream or media. */
        badbit  ( 1 << 0 ),

        /** An input operation reached the end of its stream. */
        eofbit  ( 1 << 1 ),

        /** Input or output operation failed (formatting or extraction error). */
        failbit ( 1 << 2 );

        Bit(final int v) { value = v; }
        public final int value;
    }
    public int mask;

    public IOState(final int v) {
        mask = v;
    }
    public IOState() {
        mask = 0;
    }

    public boolean isSet(final Bit bit) { return bit.value == ( mask & bit.value ); }

    /**
     * Sets the given bit and returns this instance for chaining.
     * @param bit the given Bit value to set
     * @return this instance for chaining.
     */
    public IOState set(final Bit bit) { mask = (short) ( mask | bit.value ); return this; }

    public IOState mask(final int bits) {
        final int r = mask & bits;
        if( r == mask ) { return this; }
        else { return new IOState(r); }
    }

    private static native String to_string(final int mask);

    @Override
    public String toString() {
        return to_string(mask);
    }

    @Override
    public boolean equals(final Object other) {
        if (this == other) {
            return true;
        }
        return (other instanceof IOState) &&
               this.mask == ((IOState)other).mask;
    }
}
