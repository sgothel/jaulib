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
 * Representing a directory item split into {@link #dirname()} and {@link #basename()}.
 */
public class DirItem {
    static private final String _slash = "/";
    static private final String _dot = ".";

    private final String dirname_;
    private final String basename_;

    /* pp */ DirItem(final String dirname, final String basename) {
        dirname_ = dirname;
        basename_ = basename;
    }

    /** Empty item w/ `.` set for both, dirname and basename */
    public DirItem() {
        dirname_ = _dot;
        basename_ = _dot;
    }

    /**
     * Create a dir_item where path is split into dirname and basename after `.` and `..` has been reduced.
     *
     * @param path_ the raw path
     */
    public DirItem(final String path) {
        final String[] s2 = getString2DirItem(path);
        dirname_ = s2[0];
        basename_ = s2[1];
    }
    private static native String[/*3*/] getString2DirItem(final String s);

    /** Returns the dirname, shall not be empty and denotes `.` for current working director. */
    public String dirname() { return dirname_; }

    /** Return the basename, shall not be empty nor contain a dirname. */
    public String basename() { return basename_; }

    /**
     * Returns a full unix path representation combining dirname() and basename().
     */
    public String path() {
        if( _dot.equals( dirname_ ) ) {
            return basename_;
        }
        if( _dot.equals( basename_ ) ) {
            return dirname_;
        }
        if( _slash.equals( dirname_ ) ) {
            return dirname_ + basename_;
        }
        return dirname_ + _slash + basename_;
    }

    @Override
    public String toString() {
        return "['"+dirname()+"', '"+basename()+"']";
    }

    @Override
    public boolean equals(final Object other) {
        if (this == other) {
            return true;
        }
        return (other instanceof DirItem) &&
               this.dirname_.equals( ((DirItem)other).dirname_ ) &&
               this.basename_.equals( ((DirItem)other).basename_ );
    }
}
