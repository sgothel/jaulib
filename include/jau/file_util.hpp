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

#ifndef JAU_FILE_UTIL_HPP_
#define JAU_FILE_UTIL_HPP_

#include <sys/mount.h>
#include <jau/fraction_type.hpp>
#include <jau/functional.hpp>
#include <jau/enum_util.hpp>
#include <memory>
#include <string>

extern "C" {
    // #include <sys/stat.h>
    // for ::mode_t posix protection bits
    #include <sys/types.h>
}

namespace jau::fs {

    using namespace jau::enums;

    /** @defgroup FileUtils File Utilities
     *  File types and functionality.
     *
     *  @{
     */

    /**
     * Return the current working directory or empty on failure.
     */
    std::string get_cwd() noexcept;

    /** Change working directory */
    bool chdir(const std::string& path) noexcept;

    /**
     * Returns the absolute path of given `relpath` if existing,
     * otherwise an empty string.
     * @param relpath a path, might be relative
     */
    std::string absolute(const std::string_view& relpath) noexcept;

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
    std::string dirname(const std::string_view& path) noexcept;

    /**
     * Return stripped leading directory components from given path separated by `/`.
     *
     * If only the root path `/` is given, return `/`.
     *
     * @param path given path
     * @return last non-slash component or `.`
     */
    std::string basename(const std::string_view& path) noexcept;

    /** Returns true if first character is `/` or - in case of Windows - `\\`. */
    bool isAbsolute(const std::string_view& path) noexcept;

    /**
     * Representing a directory item split into dirname() and basename().
     */
    class dir_item {
        private:
            std::string dirname_;
            std::string basename_;
            bool empty_;

            struct backed_string_view {
                std::string backing;
                std::string_view view;

                backed_string_view() noexcept
                : backing(), view(backing) {}

                backed_string_view(const std::string& backing_, const std::string_view& view_) noexcept
                : backing(backing_),
                    view(backing_.size() > 0 ? ((std::string_view)backing).substr(view_.data() - backing_.data(), view_.size()) : view_) {}

                backed_string_view(const std::string_view& view_) noexcept
                : backing(), view(view_) {}

#if 0
                backed_string_view(const backed_string_view& o) noexcept
                : backing(o.backing),
                    view( o.is_backed() ? ((std::string_view)backing).substr(o.view.data() - o.backing.data(), o.view.size()) : o.view)
                {}
#else
                /** Reason: Inefficient, removing the whole purpose of this class reducing std::string duplication. */
                backed_string_view(const backed_string_view& o) noexcept = delete;
#endif

#if 0
                backed_string_view(backed_string_view&& o) noexcept
                : backing( std::move(o.backing) ),
                    view( std::move(o.view) )
                {
                    fprintf(stderr, "backed_string_view move_ctor %s\n", to_string(true).c_str());
                }
#else
                /** Reason: clang - for some reason - does not move a std::string, but copies it */
                backed_string_view(backed_string_view&& o) noexcept = delete;
#endif

                bool is_backed() const noexcept { return backing.size() > 0; }

                void backup() noexcept {
                    backing = std::string(view);
                    view = backing;
                }
                void backup(const std::string& orig) noexcept {
                    backing = orig;
                    view = backing;
                }
                void backup(const std::string_view& orig) noexcept {
                    backing = std::string(orig);
                    view = backing;
                }
                void backup_and_append(const std::string& orig, const std::string& appendix) noexcept {
                    backing = orig;
                    backing.append(appendix);
                    view = backing;
                }
                void backup_and_append(const std::string_view& orig, const std::string& appendix) noexcept {
                    backing = std::string(orig);
                    backing.append(appendix);
                    view = backing;
                }

                std::string to_string(const bool destailed = false) const noexcept {
                    if (destailed) {
                        return "[backing '" + backing + "', view '" + std::string(view) + "']";
                    }
                    return std::string(view);
                }
            };
            static std::unique_ptr<backed_string_view> reduce(const std::string_view& path_) noexcept;

            dir_item(std::unique_ptr<backed_string_view> cleanpath) noexcept;

        public:
            /** Empty item w/ `.` set for both, dirname and basename. empty() will return true; */
            dir_item() noexcept;

            /**
             * Create a dir_item where path is split into dirname and basename after `.` and `..` has been reduced.
             *
             * empty() will return true if given path_ is empty
             *
             * @param path_ the raw path
             */
            dir_item(const std::string_view& path_) noexcept;

            /**
             * Create a dir_item with already cleaned dirname and basename
             * without any further processing nor validation.
             *
             * empty() will return true if both, given dirname_ and basename_ is empty
             *
             * @param dirname__
             * @param basename__
             * @see reduce()
             * @see jau::fs::dirname()
             * @see jau::fs::basename()
             */
            dir_item(std::string dirname__, std::string basename__) noexcept;

            /** Returns the dirname, shall not be empty and denotes `.` for current working director. */
            const std::string& dirname() const noexcept { return dirname_; }

            /** Return the basename, shall not be empty nor contain a dirname. */
            const std::string& basename() const noexcept { return basename_; }

            /**
             * Returns a full unix path representation combining dirname() and basename().
             */
            std::string path() const noexcept;

            /**
             * Returns true if bot, dirname() and basename() refer to `.`, e.g.. default ctor.
             */
            bool empty() const noexcept { return empty_; }

            bool operator==(const dir_item& rhs) const noexcept {
                return dirname_ == rhs.dirname_ && basename_ == rhs.basename_;
            }

            bool operator!=(const dir_item& rhs) const noexcept {
                return !(*this == rhs);
            }

            /**
             * Returns a comprehensive string representation of this item
             */
            std::string to_string() const noexcept;
    };

    /**
     * Generic file type and POSIX protection mode bits as used in file_stats, touch(), mkdir() etc.
     *
     * The POSIX protection mode bits reside in the lower 16-bits and are bit-wise POSIX compliant
     * while the file type bits reside in the upper 16-bits and are platform agnostic.
     *
     * This `enum class` type fulfills `C++ named requirements: BitmaskType`.
     *
     * @see file_stats
     * @see file_stats::mode()
     */
    enum class fmode_t : uint32_t {
        /** No mode bit set */
        none = 0,

        /** Protection bit: POSIX S_ISUID */
        set_uid = 04000,
        /** Protection bit: POSIX S_ISGID */
        set_gid = 02000,
        /** Protection bit: POSIX S_ISVTX */
        sticky = 01000,
        /** Protection bit: POSIX S_ISUID | S_ISGID | S_ISVTX */
        ugs_set = 07000,

        /** Protection bit: POSIX S_IRUSR */
        read_usr = 00400,
        /** Protection bit: POSIX S_IWUSR */
        write_usr = 00200,
        /** Protection bit: POSIX S_IXUSR */
        exec_usr = 00100,
        /** Protection bit: POSIX S_IRWXU */
        rwx_usr = 00700,

        /** Protection bit: POSIX S_IRGRP */
        read_grp = 00040,
        /** Protection bit: POSIX S_IWGRP */
        write_grp = 00020,
        /** Protection bit: POSIX S_IXGRP */
        exec_grp = 00010,
        /** Protection bit: POSIX S_IRWXG */
        rwx_grp = 00070,

        /** Protection bit: POSIX S_IROTH */
        read_oth = 00004,
        /** Protection bit: POSIX S_IWOTH */
        write_oth = 00002,
        /** Protection bit: POSIX S_IXOTH */
        exec_oth = 00001,
        /** Protection bit: POSIX S_IRWXO */
        rwx_oth = 00007,

        /** Protection bit: POSIX S_IRWXU | S_IRWXG | S_IRWXO or rwx_usr | rwx_grp | rwx_oth */
        rwx_all = 00777,

        /** Default directory protection bit: Safe default: POSIX S_IRWXU | S_IRGRP | S_IXGRP or rwx_usr | read_grp | exec_grp */
        def_dir_prot = 00750,

        /** Default file protection bit: Safe default: POSIX S_IRUSR | S_IWUSR | S_IRGRP or read_usr | write_usr | read_grp */
        def_file_prot = 00640,

        /** 12 bit protection bit mask 07777 for rwx_all | set_uid | set_gid | sticky . */
        protection_mask = 0b00000000000000000000111111111111,

        /** Type: Entity is a socket, might be in combination with link. */
        sock = 0b00000000000000000001000000000000,
        /** Type: Entity is a block device, might be in combination with link. */
        blk = 0b00000000000000000010000000000000,
        /** Type: Entity is a character device, might be in combination with link. */
        chr = 0b00000000000000000100000000000000,
        /** Type: Entity is a fifo/pipe, might be in combination with link. */
        fifo = 0b00000000000000001000000000000000,
        /** Type: Entity is a directory, might be in combination with link. */
        dir = 0b00000000000000010000000000000000,
        /** Type: Entity is a file, might be in combination with link. */
        file = 0b00000000000000100000000000000000,
        /** Type: Entity is a symbolic link, might be in combination with file or dir, fifo, chr, blk or sock. */
        link = 0b00000000000001000000000000000000,
        /** Type: Entity gives no access to user, exclusive bit. */
        no_access = 0b00100000000000000000000000000000,
        /** Type: Entity does not exist, exclusive bit. */
        not_existing = 0b01000000000000000000000000000000,
        /** Type mask for sock | blk | chr | fifo | dir | file | link | no_access | not_existing. */
        type_mask = 0b01100000000001111111000000000000,
    };
    JAU_MAKE_BITFIELD_ENUM_IMPL(fmode_t, sock, blk, chr, fifo, dir, file, link, no_access, not_existing);

    /**
     * Return the string representation of fmode_t
     * @param mask the fmode_t to convert
     * @param show_rwx if true, return verbose POSIX protection bit string representation using `rwx` for user, group and others. Otherwise simply show the octal representation (default)
     * @return the string representation.
     */
    std::string to_string(const fmode_t mask, const bool show_rwx) noexcept;

    /** Returns the POSIX protection bits: rwx_all | set_uid | set_gid | sticky, i.e. fmode_t masked with fmode_t::protection_mask. */
    constexpr ::mode_t posix_protection_bits(const fmode_t mask) noexcept { return static_cast<::mode_t>(mask & fmode_t::protection_mask); }

    /**
     * Returns platform dependent named file descriptor of given file descriptor, if supported.
     *
     * Implementation returns (`%d` stands for integer):
     * - `/dev/fd/%d` (GNU/Linux, FreeBSD, ..)
     *
     * Following standard POSIX mappings exist
     * - fd 0, `/dev/fd/0`, `/dev/stdin`
     * - fd 1, `/dev/fd/1`, `/dev/stdout`
     * - fd 2, `/dev/fd/2`, `/dev/stderr`
     * - fd [0-99], `/dev/fd/[0-99]`
     *
     * Currently implementation always returns above pattern,
     * not handling the target OS differences.
     *
     * @param fd file descriptor.
     * @return the named file descriptor or an empty string if fd < 0 or not supported by OS.
     *
     * @see jau::fs::from_named_fd()
     * @see jau::fs::file_stats:has_fd()
     */
    std::string to_named_fd(const int fd) noexcept;

    /**
     * Returns the file descriptor from the given named file descriptor.
     *
     * Detected named file descriptors are (`%d` stands for integer)
     * - `/dev/fd/%d` (GNU/Linux, FreeBSD, ..)
     * - `/proc/self/fd/%d` (GNU/Linux)
     *
     * @param named_fd the named file descriptor
     * @return file descriptor or -1 if invalid or not supported by OS.
     *
     * @see jau::fs::to_named_fd()
     * @see jau::fs::file_stats:has_fd()
     */
    int from_named_fd(const std::string& named_fd) noexcept;

    /**
     * Platform agnostic representation of POSIX ::lstat() and ::stat()
     * for a given pathname.
     *
     * Implementation follows the symbolic link, i.e. first opens
     * the given pathname with ::lstat() and if identifying as a symbolic link
     * opens it via ::stat() to retrieve the actual properties like size, time and ownership.
     *
     * Implementation supports named file descriptor, see is_fd().
     *
     * On `GNU/Linux` implementation uses ::statx().
     */
    class file_stats {
        public:
            /**
             * Field identifier which bit-mask indicates field_t fields
             */
            enum class field_t : uint32_t {
                /** No mode bit set */
                none = 0,
                /** File type mode bits */
                type = 0b0000000000000001,
                /** POSIX file protection mode bits */
                mode = 0b0000000000000010,
                nlink = 0b0000000000000100,
                uid = 0b0000000000001000,
                gid = 0b0000000000010000,
                atime = 0b0000000000100000,
                mtime = 0b0000000001000000,
                ctime = 0b0000000010000000,
                ino = 0b0000000100000000,
                size = 0b0000001000000000,
                blocks = 0b0000010000000000,
                btime = 0b0000100000000000,
                fd = 0b0001000000000000
            };

            typedef uint32_t uid_t;
            typedef uint32_t gid_t;

        private:
            field_t has_fields_;

            dir_item item_;

            std::shared_ptr<std::string> link_target_path_;  // stored link-target path this symbolic-link points to if is_link(), otherwise nullptr.
            std::shared_ptr<file_stats> link_target_;        // link-target this symbolic-link points to if is_link(), otherwise nullptr.

            fmode_t mode_;
            int fd_;
            uid_t uid_;
            gid_t gid_;
            uint64_t size_;
            fraction_timespec btime_;  // Birth or creation time
            fraction_timespec atime_;  // Last access
            fraction_timespec ctime_;  // Last meta-status change
            fraction_timespec mtime_;  // Last modification

            int errno_res_;

            /** Private class only for private make_shared(). */
            class ctor_cookie {
                friend file_stats;
                uint16_t rec_level;
                ctor_cookie(const uint16_t recursion_level_) { rec_level = recursion_level_; }
            };

        public:
            /** Instantiate an empty file_stats with fmode_t::not_existing set. */
            file_stats() noexcept;

            /** Private ctor for private make_shared<file_stats>() intended for friends. */
            file_stats(const ctor_cookie& cc, int dirfd, const dir_item& item, const bool dirfd_is_item_dirname) noexcept;

            /**
             * Instantiates a file_stats for the given `path`.
             *
             * The dir_item will be constructed without parent_dir
             *
             * @param path the path to produce stats for
             */
            file_stats(const std::string& path) noexcept;

            /**
             * Instantiates a file_stats for the given `path`.
             *
             * The dir_item will be constructed without parent_dir
             *
             * @param dirfd file descriptor of given dir_item item's directory, dir_item::dirname(), or AT_FDCWD for the current working directory of the calling process
             * @param path the path to produce stats for
             */
            file_stats(const int dirfd, const std::string& path) noexcept;

            /**
             * Instantiates a file_stats for the given dir_item.
             *
             * @param item the dir_item to produce stats for
             */
            file_stats(const dir_item& item) noexcept;

            /**
             * Instantiates a file_stats for the given dir_item.
             *
             * @param dirfd file descriptor of given dir_item item's directory, dir_item::dirname(), or AT_FDCWD for the current working directory of the calling process
             * @param item the dir_item to produce stats for
             * @param dirfd_is_item_dirname if true, dir_item::basename() is relative to dirfd (default), otherwise full dir_item::path() is relative to dirfd.
             */
            file_stats(const int dirfd, const dir_item& item, const bool dirfd_is_item_dirname = true) noexcept;

            /**
             * Instantiates a file_stats for the given `fd` file descriptor.
             *
             * @param fd file descriptor of an opened file
             */
            file_stats(const int fd) noexcept;

            /**
             * Returns the dir_item.
             *
             * In case this instance is created by following a symbolic link instance,
             * it represents the resolved path relative to the used symbolic link's dirname.
             *
             * @see is_link()
             * @see path()
             */
            const dir_item& item() const noexcept { return item_; }

            /**
             * Returns the unix path representation.
             *
             * In case this instance is created by following a symbolic link instance,
             * it represents the resolved path relative to the used symbolic link's dirname.
             *
             * @see is_link()
             * @see item()
             */
            std::string path() const noexcept { return item_.path(); }

            /**
             * Returns the stored link-target path this symbolic-link points to if instance is a symbolic-link, otherwise nullptr.
             *
             * @see is_link()
             * @see link_target()
             * @see final_target()
             */
            const std::shared_ptr<std::string>& link_target_path() const noexcept { return link_target_path_; }

            /**
             * Returns the link-target this symbolic-link points to if instance is a symbolic-link, otherwise nullptr.
             *
             * nullptr is also returned for an erroneous symbolic-links, i.e. non-existing link-targets or recursive loop-errors.
             *
             * @see is_link()
             * @see link_target_path()
             * @see final_target()
             */
            const std::shared_ptr<file_stats>& link_target() const noexcept { return link_target_; }

            /**
             * Returns the final target element, either a pointer to this instance if not a symbolic-link
             * or the final link-target a symbolic-link (chain) points to.
             *
             * @param link_count optional size_t pointer to store the number of symbolic links leading to the final target, excluding the final instance. 0 indicates no symbolic-link;
             *
             * @see is_link()
             * @see link_target_path()
             * @see link_target()
             */
            const file_stats* final_target(size_t* link_count = nullptr) const noexcept;

            /** Returns true if the given field_t fields were retrieved, otherwise false. */
            bool has(const field_t fields) const noexcept;

            /** Returns the retrieved field_t fields. */
            constexpr field_t fields() const noexcept { return has_fields_; }

            /** Returns the fmode_t, file type and mode. */
            fmode_t mode() const noexcept { return mode_; }

            /** Returns the POSIX protection bit portion of fmode_t, i.e. mode() & fmode_t::protection_mask. */
            fmode_t prot_mode() const noexcept { return mode_ & fmode_t::protection_mask; }

            /** Returns the type bit portion of fmode_t, i.e. mode() & fmode_t::type_mask. */
            fmode_t type_mode() const noexcept { return mode_ & fmode_t::type_mask; }

            /**
             * Returns the file descriptor if has_fd(), otherwise -1 for no file descriptor.
             *
             * @see has_fd()
             */
            int fd() const noexcept { return fd_; }

            /** Returns the user id, owning the element. */
            uid_t uid() const noexcept { return uid_; }

            /** Returns the group id, owning the element. */
            gid_t gid() const noexcept { return gid_; }

            /**
             * Returns the size in bytes of this element if is_file(), otherwise zero.
             *
             * If the element also is_link(), the linked target size is returned.
             */
            uint64_t size() const noexcept { return size_; }

            /** Returns the birth time of this element since Unix Epoch, i.e. its creation time. */
            const fraction_timespec& btime() const noexcept { return btime_; }
            /** Returns the last access time of this element since Unix Epoch. */
            const fraction_timespec& atime() const noexcept { return atime_; }
            /** Returns the last status change time of this element since Unix Epoch. */
            const fraction_timespec& ctime() const noexcept { return ctime_; }
            /** Returns the last modification time of this element since Unix Epoch. */
            const fraction_timespec& mtime() const noexcept { return mtime_; }

            /** Returns the `errno` value occurred to produce this instance, or zero for no error. */
            constexpr int errno_res() const noexcept { return errno_res_; }

            /** Returns true if no error occurred */
            constexpr bool ok() const noexcept { return 0 == errno_res_; }

            /**
             * Returns true if entity has a file descriptor.
             *
             * @see fd()
             * @see jau::fs::from_named_fd()
             * @see jau::fs::to_named_fd()
             */
            constexpr bool has_fd() const noexcept { return 0 <= fd_; }

            /** Returns true if entity is a socket, might be in combination with is_link().  */
            constexpr bool is_socket() const noexcept { return is_set(mode_, fmode_t::sock); }

            /** Returns true if entity is a block device, might be in combination with is_link().  */
            constexpr bool is_block() const noexcept { return is_set(mode_, fmode_t::blk); }

            /** Returns true if entity is a character device, might be in combination with is_link().  */
            constexpr bool is_char() const noexcept { return is_set(mode_, fmode_t::chr); }

            /** Returns true if entity is a fifo/pipe, might be in combination with is_link().  */
            constexpr bool is_fifo() const noexcept { return is_set(mode_, fmode_t::fifo); }

            /** Returns true if entity is a directory, might be in combination with is_link().  */
            constexpr bool is_dir() const noexcept { return is_set(mode_, fmode_t::dir); }

            /** Returns true if entity is a file, might be in combination with is_link().  */
            constexpr bool is_file() const noexcept { return is_set(mode_, fmode_t::file); }

            /** Returns true if entity is a symbolic link, might be in combination with is_file(), is_dir(), is_fifo(), is_char(), is_block(), is_socket(). */
            constexpr bool is_link() const noexcept { return is_set(mode_, fmode_t::link); }

            /** Returns true if entity gives no access to user, exclusive bit. */
            constexpr bool has_access() const noexcept { return !is_set(mode_, fmode_t::no_access); }

            /** Returns true if entity does not exist, exclusive bit. */
            constexpr bool exists() const noexcept { return !is_set(mode_, fmode_t::not_existing); }

            bool operator==(const file_stats& rhs) const noexcept;

            bool operator!=(const file_stats& rhs) const noexcept {
                return !(*this == rhs);
            }

            /**
             * Returns a comprehensive string representation of this element
             */
            std::string to_string() const noexcept;
    };
    JAU_MAKE_BITFIELD_ENUM_IMPL2(file_stats::field_t, field_t, type, mode, nlink, uid, gid, atime, mtime, ctime, ino, size, blocks, btime);

    /**
     * Create directory
     * @param path full path to new directory
     * @param mode fmode_t POSIX protection bits used, defaults to jau::fs::fmode_t::def_dir_prot
     * @param verbose defaults to false
     * @return true if successful, otherwise false
     */
    bool mkdir(const std::string& path, const fmode_t mode = jau::fs::fmode_t::def_dir_prot, const bool verbose = false) noexcept;

    /**
     * Touch the file with given atime and mtime and create file if not existing yet.
     * @param path full path to file
     * @param atime new access time
     * @param mtime new modification time
     * @param mode fmode_t POSIX protection bits used, defaults to jau::fs::fmode_t::def_file_prot
     * @return true if successful, otherwise false
     */
    bool touch(const std::string& path, const jau::fraction_timespec& atime, const jau::fraction_timespec& mtime,
               const fmode_t mode = jau::fs::fmode_t::def_file_prot) noexcept;

    /**
     * Touch the file with current time and create file if not existing yet.
     * @param path full path to file
     * @param mode fmode_t POSIX protection bits used, defaults to jau::fs::fmode_t::def_file_prot
     * @return true if successful, otherwise false
     */
    bool touch(const std::string& path, const fmode_t mode = jau::fs::fmode_t::def_file_prot) noexcept;

    /**
     * `void consume_dir_item(const dir_item& item)`
     */
    typedef jau::function<void(const dir_item&)> consume_dir_item;

    /**
     * Returns a list of directory elements excluding `.` and `..` for the given path, non recursive.
     *
     * The custom consume_dir_item `digest` may also be used to filter the element, besides storing it.
     *
     * @param path path to directory
     * @param digest consume_dir_item function to receive each directory item, e.g. `void consume_dir_item(const dir_item& item)`
     * @return true if given path exists, is directory and is readable, otherwise false
     */
    bool get_dir_content(const std::string& path, const consume_dir_item& digest) noexcept;

    /**
     * Returns a list of directory elements excluding `.` and `..` for the given path, non recursive.
     *
     * The custom consume_dir_item `digest` may also be used to filter the element, besides storing it.
     *
     * @param dirfd file descriptor to given `path` left untouched as a copy is being used to retrieve the directory content.
     * @param path path to directory
     * @param digest consume_dir_item function to receive each directory item, e.g. `void consume_dir_item(const dir_item& item)`
     * @return true if given path exists, is directory and is readable, otherwise false
     */
    bool get_dir_content(const int dirfd, const std::string& path, const consume_dir_item& digest) noexcept;

    /**
     * Filesystem traverse event used to call path_visitor for path elements from visit().
     *
     * This `enum class` type fulfills `C++ named requirements: BitmaskType`.
     *
     * @see path_visitor
     * @see visit()
     */
    enum class traverse_event : uint16_t {
        /** No value, neither file, symlink nor dir_entry or dir_exit. Implying an error state in file_stat, e.g. !file_stats::has_access(). */
        none = 0,

        /**
         * Visiting a symbolic-link, either to a file or a non-existing entity. Not followed symbolic-links to a directory is expressed via dir_symlink.
         *
         * In case of a symbolic-link to an existing file, file is also set, i.e. file_symlink.
         */
        symlink = 1 << 0,

        /** Visiting a file, may be in conjunction with symlink, i.e. file_symlink. */
        file = 1 << 1,

        /** Visiting a symlink to a file, i.e. symlink | file  */
        file_symlink = symlink | file,

        /**
         * Visiting a symbolic-link to a directory which is not followed, i.e. traverse_options::follow_symlinks not set.
         */
        dir_symlink = 1 << 2,

        /**
         * Visiting a directory on entry, see traverse_options::dir_check_entry.
         *
         * This allows the path_visitor to deny traversal into the directory by returning false,
         * otherwise continuing traversal.
         */
        dir_check_entry = 1 << 7,

        /**
         * Visiting a directory on entry, see traverse_options::dir_entry.
         *
         * If a directory is visited non-recursive, i.e. traverse_options::recursive not set,
         * dir_entry and dir_exit are set, see dir_non_recursive.
         *
         * If a directory is a symbolic link which is not followed, i.e. traverse_options::follow_symlinks not set,
         * dir_symlink is used instead.
         */
        dir_entry = 1 << 8,

        /**
         * Visiting a directory on exit, see traverse_options::dir_exit.
         *
         * If a directory is visited non-recursive, i.e. traverse_options::recursive not set,
         * dir_entry and dir_exit are set, see dir_non_recursive.
         *
         * If a directory is a symbolic link which is not followed, i.e. traverse_options::follow_symlinks not set,
         * dir_symlink is used instead.
         */
        dir_exit = 1 << 9,

        /**
         * Visiting a directory non-recursive, i.e. traverse_options::recursive not set.
         *
         * Value is a bit-mask of dir_entry | dir_exit
         */
        dir_non_recursive = dir_entry | dir_exit
    };
    JAU_MAKE_BITFIELD_ENUM_IMPL(traverse_event, symlink, file, dir_check_entry, dir_entry, dir_exit, dir_symlink);

    /**
     * path_visitor jau::FunctionDef definition
     * - `bool visitor(traverse_event tevt, const file_stats& item_stats, size_t depth)`
     *
     * Depth being the recursive directory depth starting with 1 for the initial directory.
     *
     * Returning `false` stops traversal in general but traverse_options::dir_check_entry
     * will only skip traversing the denied directory.
     */
    typedef jau::function<bool(traverse_event, const file_stats&, size_t)> path_visitor;

    /**
     * Filesystem traverse options used to visit() path elements.
     *
     * This `enum class` type fulfills `C++ named requirements: BitmaskType`.
     *
     * @see visit()
     * @see remove()
     */
    enum class traverse_options : uint16_t {
        /** No option set */
        none = 0,

        /** Traverse through directories, i.e. perform visit, copy, remove etc actions recursively throughout the directory structure. */
        recursive = 1U << 0,

        /** Traverse through symbolic linked directories if traverse_options::recursive is set, i.e. directories with property fmode_t::link set. */
        follow_symlinks = 1U << 1,

        /** Traverse through elements in lexicographical order. This might be required when computing an order dependent outcome like a hash value. */
        lexicographical_order = 1U << 2,

        /** Call path_visitor at directory entry, allowing path_visitor to skip traversal of this directory if returning false. */
        dir_check_entry = 1U << 7,

        /** Call path_visitor at directory entry. Both, dir_entry and dir_exit can be set, only one or none. */
        dir_entry = 1U << 8,

        /** Call path_visitor at directory exit. Both, dir_entry and dir_exit can be set, only one or none. */
        dir_exit = 1U << 9,

        /** Enable verbosity mode, potentially used by a path_visitor implementation like remove(). */
        verbose = 1U << 15
    };
    JAU_MAKE_BITFIELD_ENUM_IMPL(traverse_options, recursive, follow_symlinks, lexicographical_order, dir_check_entry, dir_entry, dir_exit);

    /**
     * Visit element(s) of a given path, see traverse_options for detailed settings.
     *
     * All elements of type fmode_t::file, fmode_t::dir and fmode_t::no_access or fmode_t::not_existing
     * will be visited by the given path_visitor `visitor`.
     *
     * Depth passed to path_visitor is the recursive directory depth and starts with 1 for the initial directory.
     *
     * path_visitor returning `false` stops traversal in general but traverse_options::dir_check_entry
     * will only skip traversing the denied directory.
     *
     * @param path the starting path
     * @param topts given traverse_options for this operation
     * @param visitor path_visitor function `bool visitor(const file_stats& item_stats, size_t depth)`.
     * @param dirfds optional empty `dirfd` stack pointer defaults to nullptr.
     *        If user provided, exposes the used `dirfd` stack, which last entry represents the currently visited directory.
     *        The `dirfd` stack starts and ends empty, i.e. all directory file descriptor are closed.
     *        In case of recursive directory traversion, the initial dir_entry visit starts with depth 1 and 2 fds, its parent and current directory.
     * @return true if successful including no path_visitor stopped traversal by returning `false` excluding traverse_options::dir_check_entry.
     */
    bool visit(const std::string& path, const traverse_options topts, const path_visitor& visitor, std::vector<int>* dirfds = nullptr) noexcept;

    /**
     * Visit element(s) of a given path, see traverse_options for detailed settings.
     *
     * All elements of type fmode_t::file, fmode_t::dir and fmode_t::no_access or fmode_t::not_existing
     * will be visited by the given path_visitor `visitor`.
     *
     * Depth passed to path_visitor is the recursive directory depth and starts with 1 for the initial directory.
     *
     * path_visitor returning `false` stops traversal in general but traverse_options::dir_check_entry
     * will only skip traversing the denied directory.
     *
     * @param item_stats pre-fetched file_stats for a given dir_item, used for efficiency
     * @param topts given traverse_options for this operation
     * @param visitor path_visitor function `bool visitor(const file_stats& item_stats, size_t depth)`.
     * @param dirfds optional empty `dirfd` stack pointer defaults to nullptr.
     *        If user provided, exposes the used `dirfd` stack, which last entry represents the currently visited directory.
     *        The `dirfd` stack starts and ends empty, i.e. all directory file descriptor are closed.
     *        In case of recursive directory traversion, the initial dir_entry visit starts with depth 1 and 2 fds, its parent and current directory.
     * @return true if successful including no path_visitor stopped traversal by returning `false` excluding traverse_options::dir_check_entry.
     */
    bool visit(const file_stats& item_stats, const traverse_options topts, const path_visitor& visitor, std::vector<int>* dirfds = nullptr) noexcept;

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
    bool remove(const std::string& path, const traverse_options topts = traverse_options::none) noexcept;

    /**
     * Compare the bytes of both files, denoted by source1 and source2.
     *
     * @param source1 first source file to compare
     * @param source2 second source file to compare
     * @param verbose defaults to false
     * @return true if both elements are files and their bytes are equal, otherwise false.
     */
    bool compare(const file_stats& source1, const file_stats& source2, const bool verbose = false) noexcept;

    /**
     * Compare the bytes of both files, denoted by source1 and source2.
     *
     * @param source1 first source file to compare
     * @param source2 second source file to compare
     * @param verbose defaults to false
     * @return true if both elements are files and their bytes are equal, otherwise false.
     */
    bool compare(const std::string& source1, const std::string& source2, const bool verbose = false) noexcept;

    /**
     * Filesystem copy options used to copy() path elements.
     *
     * By default, the fmode_t POSIX protection mode bits are preserved
     * while using the caller's uid and gid as well as current timestamps. <br />
     * Use copy_options::preserve_all to preserve uid and gid if allowed from the caller and access- and modification-timestamps.
     *
     * This `enum class` type fulfills `C++ named requirements: BitmaskType`.
     *
     * @see copy()
     */
    enum class copy_options : uint16_t {
        /** No option set */
        none = 0,

        /** Traverse through directories, i.e. perform visit, copy, remove etc actions recursively throughout the directory structure. */
        recursive = 1 << 0,

        /** Copy referenced symbolic linked files or directories instead of just the symbolic link with property fmode_t::link set. */
        follow_symlinks = 1 << 1,

        /**
         * Copy source dir content into an already existing destination directory as if destination directory did not exist.
         *
         * Otherwise, if destination directory already exist, the source directory will be copied below the destination directory.
         */
        into_existing_dir = 1 << 2,

        /**
         * Ignore errors from erroneous symlinks, e.g. non-existing link-targets, recursive loop-errors.or unsupported symmlinks on target filesystem.
         *
         * This flag is required to
         * - copy erroneous non-existing symlinks if using follow_symlinks
         * - copy erroneous recursive loop-error symlinks if using follow_symlinks
         * - ignore symlinks if not supported by target filesystem if not using follow_symlinks
         */
        ignore_symlink_errors = 1 << 8,

        /** Overwrite existing destination files. */
        overwrite = 1 << 9,

        /** Preserve uid and gid if allowed and access- and modification-timestamps, i.e. producing a most exact meta-data copy. */
        preserve_all = 1 << 10,

        /** Ensure data and meta-data file synchronization is performed via ::fsync() after asynchronous copy operations of a file's content. */
        sync = 1 << 11,

        /** Enable verbosity mode, show error messages on stderr. */
        verbose = 1 << 15
    };
    JAU_MAKE_BITFIELD_ENUM_IMPL(copy_options, recursive, follow_symlinks, into_existing_dir, ignore_symlink_errors, overwrite, preserve_all, sync);

    /**
     * Copy the given source_path to dest_path using copy_options.
     *
     * The behavior is similar like POSIX `cp` commandline tooling.
     *
     * The following behavior is being followed regarding dest_path:
     * - If source_path is a directory and copy_options::recursive set
     *   - If dest_path doesn't exist, source_path dir content is copied into the newly created dest_path.
     *   - If dest_path exists as a directory, source_path dir will be copied below the dest_path directory
     *     _if_ copy_options::into_existing_dir is not set. Otherwise its content is copied into the existing dest_path.
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
    bool copy(const std::string& source_path, const std::string& dest_path, const copy_options copts = copy_options::none) noexcept;

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
    bool rename(const std::string& oldpath, const std::string& newpath) noexcept;

    /**
     * Synchronizes filesystems, i.e. all pending modifications to filesystem metadata and cached file data will be written to the underlying filesystems.
     */
    void sync() noexcept;

    struct mount_ctx {
            bool mounted;
            std::string target;
            int loop_device_id;

            mount_ctx(std::string target_, const int loop_device_id_)
            : mounted(true), target(std::move(target_)), loop_device_id(loop_device_id_) {}

            mount_ctx()
            : mounted(false), target(), loop_device_id(-1) {}
    };

    /**
     * Generic flag bit values for mount() `flags`.
     *
     * See mount(2) for a detailed description.
     */
    typedef uint64_t mountflags_t;

    /**
     * Flag bit values for mount() `flags` under GNU/Linux.
     *
     * See mount(2) for a detailed description.
     */
    enum class mountflags_linux : mountflags_t {
        none = 0,
        rdonly = 1,
        nosuid = 2,
        nodev = 4,
        noexec = 8,
        synchronous = 16,
        remount = 32,
        mandlock = 64,
        dirsync = 128,
        noatime = 1024,
        nodiratime = 2048,
        bind = 4096,
        move = 8192,
        rec = 16384,
        silent = 32768,
        posixacl = 1 << 16,
        unbindable = 1 << 17,
        private_ = 1 << 18,
        slave = 1 << 19,
        shared = 1 << 20,
        relatime = 1 << 21,
        kernmount = 1 << 22,
        i_version = 1 << 23,
        strictatime = 1 << 24,
        lazytime = 1 << 25,
        active = 1 << 30,
        nouser = 1UL << 31
    };
    JAU_MAKE_BITFIELD_ENUM_IMPL(mountflags_linux, rdonly, nosuid, nodev, noexec, synchronous, remount, mandlock, dirsync, noatime,
                                nodiratime, bind, move, rec, silent, posixacl, unbindable, private_, slave, shared, relatime,
                                kernmount, i_version, strictatime, lazytime, active, nouser);

    /**
     * Attach the filesystem image named in `image_path` to `target`
     * using an intermediate platform specific filesystem image loop-device.
     *
     * This method either requires root permissions <br />
     * or the following capabilities: `cap_sys_admin`,`cap_setuid`, `cap_setgid`.
     *
     * Unmounting shall be done via umount() with mount_ctx argument to ensure
     * all intermediate resources are released.
     *
     * @param image_path path of image source file
     * @param target directory where `image_path` filesystem shall be attached to
     * @param fs_type type of filesystem, e.g. `squashfs`, `tmpfs`, `iso9660`, etc.
     * @param flags filesystem agnostic mount flags, see mountflags_linux.
     * @param fs_options comma separated options for the filesystem `fs_type`, see mount(8) for available options for the used filesystem.
     * @return mount_ctx structure containing mounted status etc
     *
     * @see mountflags_t
     * @see mountflags_linux
     * @see mount()
     * @see umount()
     */
    mount_ctx mount_image(const std::string& image_path, const std::string& target, const std::string& fs_type,
                          const mountflags_t flags, const std::string& fs_options = "");

    /**
     * Attach the filesystem named in `source` to `target`
     * using the given filesystem source directly.
     *
     * This method either requires root permissions <br />
     * or the following capabilities: `cap_sys_admin`,`cap_setuid`, `cap_setgid`.
     *
     * @param source filesystem path for device, directory, file or dummy-string which shall be attached
     * @param target directory where `source` filesystem shall be attached to
     * @param fs_type type of filesystem, e.g. `squashfs`, `tmpfs`, `iso9660`, etc.
     * @param flags filesystem agnostic mount flags, see mountflags_linux.
     * @param fs_options comma separated options for the filesystem `fs_type`, see mount(8) for available options for the used filesystem.
     * @return mount_ctx structure containing mounted status etc
     *
     * @see mountflags_t
     * @see mountflags_linux
     * @see mount_image()
     * @see umount()
     */
    mount_ctx mount(const std::string& source, const std::string& target, const std::string& fs_type,
                    const mountflags_t flags, const std::string& fs_options = "");

    /**
     * Generic flag bit values for umount() `flags`.
     *
     * See umount(2) for a detailed description.
     */
    typedef int umountflags_t;

    /**
     * Flag bit values for umount() `flags` under GNU/Linux.
     *
     * See umount(2) for a detailed description.
     */
    enum class umountflags_linux : umountflags_t {
        force = 1,
        detach = 2,
        expire = 4,
        nofollow = 8
    };
    JAU_MAKE_BITFIELD_ENUM_IMPL(umountflags_linux, force, detach, expire, nofollow);

    /**
     * Detach the given mount_ctx `context`
     *
     * This method either requires root permissions <br />
     * or the following capabilities: `cap_sys_admin`,`cap_setuid`, `cap_setgid`.
     *
     * @param context mount_ctx previously attached via mount_image() or mount()
     * @param flags optional umount options, if supported by the system. See umount_options_linux.
     * @return true if successful, otherwise false
     *
     * @see umountflags_t
     * @see umountflags_linux
     * @see mount()
     * @see mount_image()
     */
    bool umount(const mount_ctx& context, const umountflags_t flags);

    /**
     * Detach the topmost filesystem mounted on `target`
     * optionally using given `umountflags` options if supported.
     *
     * This method either requires root permissions <br />
     * or the following capabilities: `cap_sys_admin`,`cap_setuid`, `cap_setgid`.
     *
     * @param target directory of previously attached filesystem
     * @param flags optional umount options, if supported by the system. See umount_options_linux.
     * @return true if successful, otherwise false
     *
     * @see umountflags_t
     * @see umountflags_linux
     * @see mount()
     * @see mount_image()
     */
    bool umount(const std::string& target, const umountflags_t flags);

    /**@}*/

}  // namespace jau::fs

#endif /* JAU_FILE_UTIL_HPP_ */
