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
 * Native file types and functionality.
 */
public final class FileUtil {
    /**
     * Remove the given path. If path represents a director, `recursive` must be set to true.
     *
     * The given traverse_options `options` are handled as follows:
     * - traverse_options::parent_dir_last will be added by implementation to operate correct
     * - traverse_options::recursive shall shall be set by caller to remove directories
     * - traverse_options::follow_symlinks shall be set by caller to remove symbolic linked directories recursively, which is kind of dangerous.
     *   If not set, only the symbolic link will be removed (default)
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
     * Implementation either uses ::sendfile() if running under `GNU/Linux`,
     * otherwise POSIX ::read() and ::write().
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
     * Attach the filesystem image named in `image_path` to `target_path`.
     *
     * This method requires root permissions.
     *
     * @param image_path path of image source file
     * @param mount_point directory where `image_path` shall be attached to
     * @param fs_type type of filesystem, e.g. `squashfs`, `tmpfs`, `iso9660`, etc.
     * @param mountflags mount flags, e.g. `MS_LAZYTIME | MS_NOATIME | MS_RDONLY` for a read-only lazy-time and no-atime filesystem.
     * @param fs_options special filesystem options
     * @return mount_ctx structure containing mounted status etc
     *
     * @see umount()
     */
    public static native long mount_image(final String image_path, final String mount_point, final String fs_type,
                                          final long mountflags, final String fs_options);

    /**
     * Detach the given mount_ctc `context`
     *
     * This method requires root permissions.
     *
     * @param context mount_ctx previously attached via mount_image()
     * @return true if successful, otherwise false
     *
     * @see mount_image()
     */
    public static native boolean umount(final long context);
}
