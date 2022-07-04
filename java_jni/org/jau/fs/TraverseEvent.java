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
 * Filesystem traverse event used to call path_visitor for path elements from visit().
 *
 * This `enum class` type fulfills `C++ named requirements: BitmaskType`.
 *
 * @see FileUtil.PathVisitor
 * @see FileUtil#visit(FileStats, TraverseOptions, org.jau.fs.FileUtil.PathVisitor)
 */
public enum TraverseEvent {
    /** No value, neither file, symlink nor dir_entry or dir_exit. Implying an error state in file_stat, e.g. !file_stats::has_access(). */
    none ( (short) 0 ),

    /**
     * Visiting a symbolic-link, either to a file or a non-existing entity. Not followed symbolic-links to a directory is expressed via dir_symlink.
     *
     * In case of a symbolic-link to an existing file, file is also set, i.e. file_symlink.
     */
    symlink ( (short) (1 << 0) ),

    /** Visiting a file, may be in conjunction with symlink, i.e. file_symlink */
    file ( (short) (1 << 1) ),

    /** Visiting a symlink to a file, i.e. symlink | file  */
    file_symlink ( (short) ( 1 << 0 /* symmlink */ | 1 << 1 /* file */ ) ),

    /**
     * Visiting a directory on entry, see traverse_options::dir_entry.
     *
     * If a directory is visited non-recursive, i.e. traverse_options::recursive not set,
     * dir_entry and dir_exit are set, see dir_non_recursive.
     *
     * If a directory is a symbolic link which is not followed, i.e. traverse_options::follow_symlinks not set,
     * dir_symlink is used instead.
     */
    dir_entry ( (short) (1 << 2) ),

    /**
     * Visiting a directory on exit, see traverse_options::dir_exit.
     *
     * If a directory is visited non-recursive, i.e. traverse_options::recursive not set,
     * dir_entry and dir_exit are set, see dir_non_recursive.
     *
     * If a directory is a symbolic link which is not followed, i.e. traverse_options::follow_symlinks not set,
     * dir_symlink is used instead.
     */
    dir_exit ( (short) (1 << 3) ),

    /**
     * Visiting a symbolic-link to a directory which is not followed, i.e. traverse_options::follow_symlinks not set.
     */
    dir_symlink ( (short) (1 << 4) ),

    /**
     * Visiting a directory non-recursive, i.e. traverse_options::recursive not set.
     *
     * Value is a bit-mask of dir_entry | dir_exit
     */
    dir_non_recursive ( (short) ( 1 << 2 /* dir_entry */ | 1 << 3 /* dir_exit */ ) );

    TraverseEvent(final short v) { value = v; }
    public final short value;
}
