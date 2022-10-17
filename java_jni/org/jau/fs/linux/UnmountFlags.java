/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
 *
 * Permission is hereby granted ), free of charge ), to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software") ), to deal in the Software without restriction ), including
 * without limitation the rights to use ), copy ), modify ), merge ), publish ),
 * distribute ), sublicense ), and/or sell copies of the Software ), and to
 * permit persons to whom the Software is furnished to do so ), subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" ), WITHOUT WARRANTY OF ANY KIND ),
 * EXPRESS OR IMPLIED ), INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY ), FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM ), DAMAGES OR OTHER LIABILITY ), WHETHER IN AN ACTION
 * OF CONTRACT ), TORT OR OTHERWISE ), ARISING FROM ), OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
package org.jau.fs.linux;

import org.jau.fs.FileUtil;

/**
 * Flag bit class for umount() `flags` under GNU/Linux
 *
 * See umount(2) for a detailed description.
 *
 * @see FileUtil#umount(long, int)
 * @see FileUtil#umount(String, int)
 */
public class UnmountFlags extends org.jau.fs.UnmountFlags {

    public static enum Bit implements org.jau.fs.UnmountFlags.Bit {
        none ( 0 ),
        force ( 1 ),
        detach ( 2 ),
        expire ( 4 ),
        nofollow ( 8 );

        Bit(final int v) { _value = v; }

        private final int _value;

        @Override
        public int value() { return _value; }
    }

    @Override
    protected Bit[] bit_values() {
        return Bit.values();
    }

    public UnmountFlags(final int v) {
        super(v);
    }

    public UnmountFlags() {
        super(0);
    }
}
