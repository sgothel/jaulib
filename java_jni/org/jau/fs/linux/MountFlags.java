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
package org.jau.fs.linux;

import org.jau.fs.FileUtil;

/**
 * Flag bit values for mount() `flags` under GNU/Linux.
 *
 * See mount(2) for a detailed description.
 *
 * @see FileUtil#mount_image(String, String, String, long, String)
 * @see FileUtil#mount(String, String, String, long, String)
 */
public class MountFlags extends org.jau.fs.MountFlags {

    public static enum Bit implements org.jau.fs.MountFlags.Bit {
        none ( 0 ),
        MS_RDONLY ( 1 ),
        MS_NOSUID ( 2 ),
        MS_NODEV ( 4 ),
        MS_NOEXEC ( 8 ),
        MS_SYNCHRONOUS ( 16 ),
        MS_REMOUNT ( 32 ),
        MS_MANDLOCK ( 64 ),
        MS_DIRSYNC ( 128 ),
        MS_NOATIME ( 1024 ),
        MS_NODIRATIME ( 2048 ),
        MS_BIND ( 4096 ),
        MS_MOVE ( 8192 ),
        MS_REC ( 16384 ),
        MS_SILENT ( 32768 ),
        MS_POSIXACL ( 1 << 16 ),
        MS_UNBINDABLE ( 1 << 17 ),
        MS_PRIVATE ( 1 << 18 ),
        MS_SLAVE ( 1 << 19 ),
        MS_SHARED ( 1 << 20 ),
        MS_RELATIME ( 1 << 21 ),
        MS_KERNMOUNT ( 1 << 22 ),
        MS_I_VERSION (  1 << 23 ),
        MS_STRICTATIME ( 1 << 24 ),
        MS_LAZYTIME ( 1 << 25 ),
        MS_ACTIVE ( 1 << 30 ),
        MS_NOUSER ( 1 << 31 );

        Bit(final long v) { _value = v; }
        private final long _value;

        @Override
        public long value() { return _value; }
    }

    @Override
    protected Bit[] bit_values() {
        return Bit.values();
    }

    public MountFlags(final long v) {
        super(v);
    }

    public MountFlags() {
        super(0);
    }
}
