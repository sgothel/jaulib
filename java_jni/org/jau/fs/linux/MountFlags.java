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
        rdonly ( 1 ),
        nosuid ( 2 ),
        nodev ( 4 ),
        noexec ( 8 ),
        synchronous ( 16 ),
        remount ( 32 ),
        mandlock ( 64 ),
        dirsync ( 128 ),
        noatime ( 1024 ),
        nodiratime ( 2048 ),
        bind ( 4096 ),
        move ( 8192 ),
        rec ( 16384 ),
        silent ( 32768 ),
        posixacl ( 1 << 16 ),
        unbindable ( 1 << 17 ),
        private_ ( 1 << 18 ),
        slave ( 1 << 19 ),
        shared ( 1 << 20 ),
        relatime ( 1 << 21 ),
        kernmount ( 1 << 22 ),
        i_version (  1 << 23 ),
        strictatime ( 1 << 24 ),
        lazytime ( 1 << 25 ),
        active ( 1 << 30 ),
        nouser ( 1 << 31 );

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
