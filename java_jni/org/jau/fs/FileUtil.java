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

import java.time.Instant;
import java.util.List;

/**
 * Native file types and functionality.
 */
public final class FileUtil {
    /**
     * Return the current working directory or empty on failure.
     */
    public static native String get_cwd();

    /**
     * Return stripped last component from given path separated by `/`, excluding the trailing separator `/`.
     *
     * If no directory separator `/` is contained, return `.`.
     *
     * If only the root path `/` is given, return `/`.
     *
     * @param path given path
     * @return leading directory name w/o slash or `.`
     */
    public static native String dirname(final String path);

    /**
     * Return stripped leading directory components from given path separated by `/`.
     *
     * If only the root path `/` is given, return `/`.
     *
     * @param path given path
     * @return last non-slash component or `.`
     */
    public static native String basename(final String path);

    /**
     * Create directory
     * @param path full path to new directory
     * @param mode fmode_t POSIX protection bits used, defaults to {@link FMode#def_dir}
     * @param verbose defaults to false
     * @return true if successful, otherwise false
     */
    public static boolean mkdir(final String path, final FMode mode) {
        return mkdirImpl(path, mode.mask);
    }
    private static native boolean mkdirImpl(final String path, final int mode);

    /**
     * See {@link #mkdir(String, FMode)} using {@link FMode#def_dir}
     */
    public static boolean mkdir(final String path) {
        return mkdirImpl(path, FMode.def_dir.mask);
    }

    /**
     * Touch the file with given atime and mtime and create file if not existing yet.
     * @param path full path to file
     * @param atime new access time
     * @param mtime new modification time
     * @param mode fmode_t POSIX protection bits used, defaults to {@link FMode#def_file}
     * @param verbose defaults to false
     * @return true if successful, otherwise false
     */
    public static boolean touch(final String path, final Instant atime, final Instant mtime,
                                final FMode mode) {
        return touchImpl(path,
                         atime.getEpochSecond(), atime.getNano(),
                         mtime.getEpochSecond(), mtime.getNano(),
                         mode.mask);
    }
    private static native boolean touchImpl(final String path,
                                            long atime_s, long atime_ns,
                                            long mtime_s, long mtime_ns,
                                            int mode);

    public static final long UTIME_NOW  = ((1l << 30) - 1l);

    /**
     * Touch the file with current time and create file if not existing yet.
     * @param path full path to file
     * @param mode fmode_t POSIX protection bits used, defaults to {@link FMode#def_file}
     * @param verbose defaults to false
     * @return true if successful, otherwise false
     */
    public static boolean touch(final String path, final FMode mode) {
        return touchImpl(path, 0, UTIME_NOW, 0, UTIME_NOW, mode.mask);
    }

    /**
     * Returns a list of directory elements excluding `.` and `..` for the given path, non recursive.
     *
     * @param path path to directory
     * @return list of DirItem if given path exists, is directory and is readable, otherwise null
     */
    public static native List<DirItem> get_dir_content(final String path);

    /**
     * Path visitor for {@link FileUtil#visit(FileStats, TraverseOptions, PathVisitor)}
     */
    public static interface PathVisitor {
        boolean visit(TraverseEvent tevt, final FileStats item_stats);
    }

    /**
     * Visit element(s) of a given path, see traverse_options for detailed settings.
     *
     * All elements of type fmode_t::file, fmode_t::dir and fmode_t::no_access or fmode_t::not_existing
     * will be visited by the given path_visitor `visitor`.
     *
     * Processing ends if the `visitor returns `false`.
     *
     * @param path the starting path
     * @param topts given traverse_options for this operation
     * @param visitor path_visitor function `bool visitor(const file_stats& item_stats)`.
     * @return true if all visitor invocations returned true, otherwise false
     */
    public static boolean visit(final String path, final TraverseOptions topts, final PathVisitor visitor) {
        return visit(new FileStats(path), topts, visitor);
    }

    /**
     * Visit element(s) of a given path, see traverse_options for detailed settings.
     *
     * All elements of type fmode_t::file, fmode_t::dir and fmode_t::no_access or fmode_t::not_existing
     * will be visited by the given path_visitor `visitor`.
     *
     * Processing ends if the `visitor returns `false`.
     *
     * @param item_stats pre-fetched file_stats for a given dir_item, used for efficiency
     * @param topts given traverse_options for this operation
     * @param visitor path_visitor function `bool visitor(const file_stats& item_stats)`.
     * @return true if all visitor invocations returned true, otherwise false
     */
    public static boolean visit(final FileStats item_stats, final TraverseOptions topts, final PathVisitor visitor) {
        if( item_stats.is_dir() ) {
            if( item_stats.is_link() && !topts.isSet(TraverseOptions.Bit.follow_symlinks) ) {
                return visitor.visit( TraverseEvent.dir_symlink, item_stats );
            }
            if( !topts.isSet(TraverseOptions.Bit.recursive) ) {
                return visitor.visit( TraverseEvent.dir_non_recursive, item_stats );
            }
            if( topts.isSet(TraverseOptions.Bit.dir_entry) ) {
                if( !visitor.visit( TraverseEvent.dir_entry, item_stats ) ) {
                    return false;
                }
            }
            final List<DirItem> content = get_dir_content(item_stats.path());
            if( null != content && content.size() > 0 ) {
                for (final DirItem element : content) {
                    final FileStats element_stats = new FileStats( element.path() );
                    if( element_stats.is_dir() ) { // an OK dir
                        if( element_stats.is_link() && !topts.isSet(TraverseOptions.Bit.follow_symlinks) ) {
                            if( !visitor.visit( TraverseEvent.dir_symlink, element_stats ) ) {
                                return false;
                            }
                        } else if( !visit(element_stats, topts, visitor) ) { // recursive
                            return false;
                        }
                    } else if( !visitor.visit( element_stats.is_file() && element_stats.is_link() ? TraverseEvent.file_symlink :
                                               ( element_stats.is_file() ? TraverseEvent.file :
                                                 ( element_stats.is_link() ? TraverseEvent.symlink : TraverseEvent.none ) ),
                                               element_stats ) )
                    {
                        return false;
                    }
                }
            }
        }
        if( item_stats.is_dir() && topts.isSet(TraverseOptions.Bit.dir_exit) ) {
            return visitor.visit( TraverseEvent.dir_exit, item_stats );
        } else if( item_stats.is_file() || !item_stats.ok() ) { // file or error-alike
            return visitor.visit( item_stats.is_file() && item_stats.is_link() ? TraverseEvent.file_symlink :
                                  ( item_stats.is_file() ? TraverseEvent.file :
                                    ( item_stats.is_link() ? TraverseEvent.symlink : TraverseEvent.none ) ),
                                  item_stats );
        } else {
            return true;
        }
    }

    /**
     * Remove the given path. If path represents a director, `recursive` must be set to true.
     *
     * The given traverse_options `options` are handled as follows:
     * - traverse_options::parent_dir_last will be added by implementation to operate correct
     * - traverse_options::recursive shall shall be set by caller to remove directories
     * - traverse_options::follow_symlinks shall be set by caller to remove symbolic linked directories recursively, which is kind of dangerous.
     *   If not set, only the symbolic link will be removed (default)
     *
     * Implementation is most data-race-free (DRF), utilizes following safeguards
     * - utilizing parent directory file descriptor and `openat()` and `unlinkat()` operations against concurrent mutation
     *
     * @param path path to remove
     * @param topts given traverse_options for this operation, defaults to traverse_options::none
     * @return true only if the file or the directory with content has been deleted, otherwise false
     */
    public static boolean remove(final String path, final TraverseOptions topts) {
        return remove_impl(path, topts.mask);
    }
    private static native boolean remove_impl(final String path, final short topts);

    /**
     * Compare the bytes of both files, denoted by source1 and source2.
     *
     * @param source1 first source file to compare
     * @param source2 second source file to compare
     * @param verbose defaults to false
     * @return true if both elements are files and their bytes are equal, otherwise false.
     */
    public static native boolean compare(final String source1, final String source2, final boolean verbose);

    /**
     * Copy the given source_path to dest_path using copy_options.
     *
     * The behavior is similar like POSIX `cp` commandline tooling.
     *
     * The following behavior is being followed regarding dest_path:
     * - If source_path is a directory and copy_options::recursive set
     *   - If dest_path doesn't exist, source_path dir content is copied into the newly created dest_path.
     *   - If dest_path exists as a directory, source_path dir will be copied below the dest_path directory.
     *   - Everything else is considered an error
     * - If source_path is a file
     *   - If dest_path doesn't exist, source_path file is copied to dest_path as a file.
     *   - If dest_path exists as a directory, source_path file will be copied below the dest_path directory.
     *   - If dest_path exists as a file, copy_options::overwrite must be set to have it overwritten by the source_path file
     *   - Everything else is considered an error
     *
     * Implementation either uses ::sendfile() if running under `GNU/Linux`,
     * otherwise POSIX ::read() and ::write().
     *
     * Implementation is most data-race-free (DRF), utilizes following safeguards on recursive directory copy
     * - utilizing parent directory file descriptor and `openat()` operations against concurrent mutation
     * - for each entered *directory*
     *   - new destination directory is create with '.<random_number>' and user-rwx permissions only
     *   - its file descriptor is being opened
     *   - its user-read permission is dropped, remains user-wx permissions only
     *   - its renamed to destination path
     *   - all copy operations are performed inside
     *   - at exit, its permissions are restored, etc.
     *
     * See copy_options for details.
     *
     * @param source_path
     * @param dest_path
     * @param copts
     * @return true if successful, otherwise false
     */
    public static boolean copy(final String source_path, final String dest_path, final CopyOptions copts) {
        return copy_impl(source_path, dest_path, copts.mask);
    }
    private static native boolean copy_impl(final String source_path, final String dest_path, final short copts);

    /**
     * Rename oldpath to newpath using POSIX `::rename()`, with the following combinations
     * - oldpath and newpath refer to the same file, a successful no-operation.
     * - oldpath file
     *   - newpath not-existing file
     *   - newpath existing file to be atomically replaced
     * - oldpath directory
     *   - newpath not-existing directory
     *   - newpath existing empty directory
     * - oldpath symlink will be renamed
     * - newpath symlink will be overwritten
     *
     * @param oldpath previous path
     * @param newpath new path
     * @return true only if the rename operation was successful, otherwise false
     */
    public static native boolean rename(final String oldpath, final String newpath);

    /**
     * Attach the filesystem image named in `image_path` to `target_path`.
     *
     * This method either requires root permissions <br />
     * or the following capabilities: `cap_sys_admin`,`cap_setuid`, `cap_setgid`.
     *
     * @param image_path path of image source file
     * @param mount_point directory where `image_path` shall be attached to
     * @param fs_type type of filesystem, e.g. `squashfs`, `tmpfs`, `iso9660`, etc.
     * @param mountflags mount flags, e.g. `MS_LAZYTIME | MS_NOATIME | MS_RDONLY` for a read-only lazy-time and no-atime filesystem.
     * @param fs_options special filesystem options
     * @return native mount context if successful, otherwise null
     *
     * @see umount()
     */
    public static native long mount_image(final String image_path, final String mount_point, final String fs_type,
                                          final long mountflags, final String fs_options);

    /**
     * Detach the given mount_ctc `context`
     *
     * This method either requires root permissions <br />
     * or the following capabilities: `cap_sys_admin`,`cap_setuid`, `cap_setgid`.
     *
     * @param context native mount context, previously attached via mount_image()
     * @return true if successful, otherwise false
     *
     * @see mount_image()
     */
    public static native boolean umount(final long context);
}
