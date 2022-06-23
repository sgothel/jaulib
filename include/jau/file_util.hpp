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

#include <cstring>
#include <string>
#include <memory>
#include <cstdint>
#include <cstdio>

#include <jau/fraction_type.hpp>
#include <jau/function_def.hpp>

namespace jau {

    namespace fs {

        /** @defgroup FileUtils File Utilities
         *  File types and functionality.
         *
         *  @{
         */

        /**
         * Return the current working directory or empty on failure.
         */
        std::string get_cwd() noexcept;

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

        /**
         * Representing a directory item split into dirname() and basename().
         */
        class dir_item {
            private:
                std::string dirname_;
                std::string basename_;

                struct backed_string_view {
                    std::string backing;
                    std::string_view view;

                    backed_string_view(std::string&& backing_, const std::string_view& view_ ) noexcept
                    : backing(std::move(backing_)), view(view_) {}

                    backed_string_view(const std::string_view& view_ ) noexcept
                    : backing(), view(view_) {}
                };
                static backed_string_view reduce(const std::string_view& path_) noexcept;

                dir_item(const backed_string_view cleanpath) noexcept;

            public:

                dir_item() noexcept
                : dirname_(), basename_() {}

                /**
                 * Create a dir_item where path is split into dirname and basename after `.` and `..` has been reduced.
                 *
                 * @param path_ the raw path
                 */
                dir_item(const std::string_view& path_) noexcept;

                /** Returns the dirname, shall not be empty and denotes `.` for current working director. */
                const std::string& dirname() const noexcept { return dirname_; }

                /** Return the basename, shall not be empty nor contain a dirname. */
                const std::string& basename() const noexcept { return basename_; }

                /**
                 * Returns a full unix path representation combining dirname() and basename().
                 */
                std::string path() const noexcept;

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
            none         =        0,

            /** Protection bit: POSIX S_ISUID */
            set_uid    = 04000,
            /** Protection bit: POSIX S_ISGID */
            set_gid    = 02000,
            /** Protection bit: POSIX S_ISVTX */
            sticky     = 01000,
            /** Protection bit: POSIX S_ISUID | S_ISGID | S_ISVTX */
            ugs_set    = 07000,

            /** Protection bit: POSIX S_IRUSR */
            read_usr   = 00400,
            /** Protection bit: POSIX S_IWUSR */
            write_usr  = 00200,
            /** Protection bit: POSIX S_IXUSR */
            exec_usr   = 00100,
            /** Protection bit: POSIX S_IRWXU */
            rwx_usr    = 00700,

            /** Protection bit: POSIX S_IRGRP */
            read_grp   = 00040,
            /** Protection bit: POSIX S_IWGRP */
            write_grp  = 00020,
            /** Protection bit: POSIX S_IXGRP */
            exec_grp   = 00010,
            /** Protection bit: POSIX S_IRWXG */
            rwx_grp    = 00070,

            /** Protection bit: POSIX S_IROTH */
            read_oth   = 00004,
            /** Protection bit: POSIX S_IWOTH */
            write_oth  = 00002,
            /** Protection bit: POSIX S_IXOTH */
            exec_oth   = 00001,
            /** Protection bit: POSIX S_IRWXO */
            rwx_oth    = 00007,

            /** Protection bit: POSIX S_IRWXU | S_IRWXG | S_IRWXO or rwx_usr | rwx_grp | rwx_oth */
            rwx_all    = 00777,

            /** Default directory protection bit: Safe default: POSIX S_IRWXU | S_IRGRP | S_IXGRP or rwx_usr | read_grp | exec_grp */
            def_dir_prot    = 00750,

            /** Default file protection bit: Safe default: POSIX S_IRUSR | S_IWUSR | S_IRGRP or read_usr | write_usr | read_grp */
            def_file_prot    = 00640,

            /** 12 bit protection bit mask 07777 for rwx_all | set_uid | set_gid | sticky . */
            protection_mask = 0b111111111111,

            /** Type: Entity is a directory, might be in combination with link. */
            dir          = 0b000010000000000000000,
            /** Type: Entity is a file, might be in combination with link. */
            file         = 0b000100000000000000000,
            /** Type: Entity is a symbolic link, might be in combination with file or dir. */
            link         = 0b001000000000000000000,
            /** Type: Entity gives no access to user, exclusive bit. */
            no_access    = 0b010000000000000000000,
            /** Type: Entity does not exist, exclusive bit. */
            not_existing = 0b100000000000000000000,
            /** Type mask for dir | file | link | no_access | not_existing. */
            type_mask    = 0b111110000000000000000,
        };
        constexpr uint32_t number(const fmode_t rhs) noexcept {
            return static_cast<uint32_t>(rhs);
        }
        constexpr fmode_t operator ~(const fmode_t rhs) noexcept {
            return static_cast<fmode_t> ( ~number(rhs) );
        }
        constexpr fmode_t operator ^(const fmode_t lhs, const fmode_t rhs) noexcept {
            return static_cast<fmode_t> ( number(lhs) ^ number(rhs) );
        }
        constexpr fmode_t operator |(const fmode_t lhs, const fmode_t rhs) noexcept {
            return static_cast<fmode_t> ( number(lhs) | number(rhs) );
        }
        constexpr fmode_t operator &(const fmode_t lhs, const fmode_t rhs) noexcept {
            return static_cast<fmode_t> ( number(lhs) & number(rhs) );
        }
        constexpr fmode_t& operator |=(fmode_t& lhs, const fmode_t rhs) noexcept {
            lhs = static_cast<fmode_t> ( number(lhs) | number(rhs) );
            return lhs;
        }
        constexpr fmode_t& operator &=(fmode_t& lhs, const fmode_t rhs) noexcept {
            lhs = static_cast<fmode_t> ( number(lhs) & number(rhs) );
            return lhs;
        }
        constexpr fmode_t& operator ^=(fmode_t& lhs, const fmode_t rhs) noexcept {
            lhs = static_cast<fmode_t> ( number(lhs) ^ number(rhs) );
            return lhs;
        }
        constexpr bool operator ==(const fmode_t lhs, const fmode_t rhs) noexcept {
            return number(lhs) == number(rhs);
        }
        constexpr bool operator !=(const fmode_t lhs, const fmode_t rhs) noexcept {
            return !( lhs == rhs );
        }
        constexpr bool is_set(const fmode_t mask, const fmode_t bits) noexcept {
            return bits == ( mask & bits );
        }
        /**
         * Return the string representation of fmode_t
         * @param mask the fmode_t to convert
         * @param show_rwx if true, return verbose POSIX protection bit string representation using `rwx` for user, group and others. Otherwise simply show the octal representation (default)
         * @return the string representation.
         */
        std::string to_string(const fmode_t mask, const bool show_rwx=false) noexcept;

        /** Returns the POSIX protection bits: rwx_all | set_uid | set_gid | sticky, i.e. fmode_t masked with fmode_t::protection_mask. */
        constexpr ::mode_t posix_protection_bits(const fmode_t mask) noexcept { return static_cast<::mode_t>(mask & fmode_t::protection_mask); }

        /**
         * Platform agnostic C++ representation of POSIX ::lstat() and ::stat()
         * for a given pathname.
         *
         * Implementation follows the symbolic link, i.e. first opens
         * the given pathname with ::lstat() and if identifying as a symbolic link
         * opens it via ::stat() to retrieve the actual properties like size, time and ownership.
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
                    none           =                  0,
                    /** File type mode bits */
                    type           = 0b0000000000000001,
                    /** POSIX file protection mode bits */
                    mode           = 0b0000000000000010,
                    nlink          = 0b0000000000000100,
                    uid            = 0b0000000000001000,
                    gid            = 0b0000000000010000,
                    atime          = 0b0000000000100000,
                    mtime          = 0b0000000001000000,
                    ctime          = 0b0000000010000000,
                    ino            = 0b0000000100000000,
                    size           = 0b0000001000000000,
                    blocks         = 0b0000010000000000,
                    btime          = 0b0000100000000000
                };

                typedef uint32_t uid_t;
                typedef uint32_t gid_t;

            private:
                field_t has_fields_;

                dir_item item_;

                std::shared_ptr<std::string> link_target_path_; // stored link-target path this symbolic-link points to if is_link(), otherwise nullptr.
                std::shared_ptr<file_stats> link_target_; // link-target this symbolic-link points to if is_link(), otherwise nullptr.

                fmode_t mode_;
                uid_t uid_;
                gid_t gid_;
                uint64_t size_;
                fraction_timespec btime_; // Birth or creation time
                fraction_timespec atime_; // Last access
                fraction_timespec ctime_; // Last meta-status change
                fraction_timespec mtime_; // Last modification

                int errno_res_;

                /** Private class only for private make_shared(). */
                class ctor_cookie { friend file_stats; uint16_t rec_level; ctor_cookie(const uint16_t recursion_level_) { rec_level=recursion_level_; } };

            public:
                /** Instantiate an empty file_stats with fmode_t::not_existing set. */
                file_stats() noexcept;

                /** Private ctor for private make_shared<file_stats>() intended for friends. */
                file_stats(const ctor_cookie& cc, const dir_item& item) noexcept;

                /**
                 * Instantiates a file_stats for the given `path`.
                 *
                 * The dir_item will be constructed without parent_dir
                 * @param path the path to produce stats for
                 */
                file_stats(const std::string& path) noexcept;

                /**
                 * Instantiates a file_stats for the given dir_item.
                 *
                 * @param item the dir_item to produce stats for
                 */
                file_stats(const dir_item& item) noexcept;

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
                const file_stats* final_target(size_t* link_count=nullptr) const noexcept;

                /** Returns true if the given field_t fields were retrieved, otherwise false. */
                bool has(const field_t fields) const noexcept;

                /** Returns the retrieved field_t fields. */
                constexpr field_t fields() const noexcept { return has_fields_; }

                /** Returns the fmode_t, file type and mode. */
                fmode_t mode() const noexcept { return mode_; }

                /** Returns the POSIX protection bit portion of fmode_t, i.e. mode() & fmode_t::protection_mask. */
                fmode_t prot_mode() const noexcept { return mode_ & fmode_t::protection_mask; }

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
                int errno_res() const noexcept { return errno_res_; }

                /** Returns true if no error occurred */
                bool ok()  const noexcept { return 0 == errno_res_; }

                /** Returns true if entity is a file, might be in combination with is_link().  */
                bool is_file() const noexcept { return is_set( mode_, fmode_t::file ); }

                /** Returns true if entity is a directory, might be in combination with is_link().  */
                bool is_dir() const noexcept { return is_set( mode_, fmode_t::dir ); }

                /** Returns true if entity is a symbolic link, might be in combination with is_file() or is_dir(). */
                bool is_link() const noexcept { return is_set( mode_, fmode_t::link ); }

                /** Returns true if entity gives no access to user, exclusive bit. */
                bool has_access() const noexcept { return !is_set( mode_, fmode_t::no_access ); }

                /** Returns true if entity does not exist, exclusive bit. */
                bool exists() const noexcept { return !is_set( mode_, fmode_t::not_existing ); }

                /**
                 * Returns a comprehensive string representation of this element
                 * @param use_space if true, using space instead for 'T' separator and drop trailing UTC `Z` for readability, otherwise be compliant with ISO 8601 (default)
                 */
                std::string to_string(const bool use_space=false) const noexcept;
        };
        constexpr uint32_t number(const file_stats::field_t rhs) noexcept {
            return static_cast<uint32_t>(rhs);
        }
        constexpr file_stats::field_t operator ~(const file_stats::field_t rhs) noexcept {
            return static_cast<file_stats::field_t> ( ~number(rhs) );
        }
        constexpr file_stats::field_t operator ^(const file_stats::field_t lhs, const file_stats::field_t rhs) noexcept {
            return static_cast<file_stats::field_t> ( number(lhs) ^ number(rhs) );
        }
        constexpr file_stats::field_t operator |(const file_stats::field_t lhs, const file_stats::field_t rhs) noexcept {
            return static_cast<file_stats::field_t> ( number(lhs) | number(rhs) );
        }
        constexpr file_stats::field_t operator &(const file_stats::field_t lhs, const file_stats::field_t rhs) noexcept {
            return static_cast<file_stats::field_t> ( number(lhs) & number(rhs) );
        }
        constexpr file_stats::field_t& operator |=(file_stats::field_t& lhs, const file_stats::field_t rhs) noexcept {
            lhs = static_cast<file_stats::field_t> ( number(lhs) | number(rhs) );
            return lhs;
        }
        constexpr file_stats::field_t& operator &=(file_stats::field_t& lhs, const file_stats::field_t rhs) noexcept {
            lhs = static_cast<file_stats::field_t> ( number(lhs) & number(rhs) );
            return lhs;
        }
        constexpr file_stats::field_t& operator ^=(file_stats::field_t& lhs, const file_stats::field_t rhs) noexcept {
            lhs = static_cast<file_stats::field_t> ( number(lhs) ^ number(rhs) );
            return lhs;
        }
        constexpr bool operator ==(const file_stats::field_t lhs, const file_stats::field_t rhs) noexcept {
            return number(lhs) == number(rhs);
        }
        constexpr bool operator !=(const file_stats::field_t lhs, const file_stats::field_t rhs) noexcept {
            return !( lhs == rhs );
        }
        constexpr bool is_set(const file_stats::field_t mask, const file_stats::field_t bits) noexcept {
            return bits == ( mask & bits );
        }
        std::string to_string(const file_stats::field_t mask) noexcept;

        /**
         * Create directory
         * @param path full path to new directory
         * @param mode fmode_t POSIX protection bits used, defaults to jau::fs::fmode_t::def_dir_prot
         * @param verbose defaults to false
         * @return true if successful, otherwise false
         */
        bool mkdir(const std::string& path, const fmode_t mode=jau::fs::fmode_t::def_dir_prot, const bool verbose=false) noexcept;

        /**
         * Touch the file with given atime and mtime and create file if not existing yet.
         * @param path full path to file
         * @param atime new access time
         * @param mtime new modification time
         * @param mode fmode_t POSIX protection bits used, defaults to jau::fs::fmode_t::def_file_prot
         * @param verbose defaults to false
         * @return true if successful, otherwise false
         */
        bool touch(const std::string& path, const jau::fraction_timespec& atime, const jau::fraction_timespec& mtime,
                   const fmode_t mode=jau::fs::fmode_t::def_file_prot, const bool verbose=false) noexcept;

        /**
         * Touch the file with current time and create file if not existing yet.
         * @param path full path to file
         * @param mode fmode_t POSIX protection bits used, defaults to jau::fs::fmode_t::def_file_prot
         * @param verbose defaults to false
         * @return true if successful, otherwise false
         */
        bool touch(const std::string& path, const fmode_t mode=jau::fs::fmode_t::def_file_prot, const bool verbose=false) noexcept;

        /**
         * `void consume_dir_item(const dir_item& item)`
         */
        typedef jau::FunctionDef<void, const dir_item&> consume_dir_item;

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
                 * In case of a symbolic-link to an existing file, file is also set.
                 */
                symlink = 1 << 0,

                /** Visiting a file, may be in conjunction with symlink. */
                file = 1 << 1,

                /**
                 * Visiting a directory on entry, see traverse_options::dir_entry.
                 *
                 * If a directory is visited non-recursive, i.e. traverse_options::recursive not set,
                 * dir_entry and dir_exit are set, see dir_non_recursive.
                 *
                 * If a directory is a symbolic link which is not followed, i.e. traverse_options::follow_symlinks not set,
                 * dir_symlink is used instead.
                 */
                dir_entry = 1 << 2,

                /**
                 * Visiting a directory on exit, see traverse_options::dir_exit.
                 *
                 * If a directory is visited non-recursive, i.e. traverse_options::recursive not set,
                 * dir_entry and dir_exit are set, see dir_non_recursive.
                 *
                 * If a directory is a symbolic link which is not followed, i.e. traverse_options::follow_symlinks not set,
                 * dir_symlink is used instead.
                 */
                dir_exit = 1 << 3,

                /**
                 * Visiting a symbolic-link to a directory which is not followed, i.e. traverse_options::follow_symlinks not set.
                 */
                dir_symlink = 1 << 4,

                /**
                 * Visiting a directory non-recursive, i.e. traverse_options::recursive not set.
                 *
                 * Value is a bit-mask of dir_entry | dir_exit
                 */
                dir_non_recursive = dir_entry | dir_exit
        };
        constexpr uint16_t number(const traverse_event rhs) noexcept {
            return static_cast<uint16_t>(rhs);
        }
        constexpr traverse_event operator ~(const traverse_event rhs) noexcept {
            return static_cast<traverse_event> ( ~number(rhs) );
        }
        constexpr traverse_event operator ^(const traverse_event lhs, const traverse_event rhs) noexcept {
            return static_cast<traverse_event> ( number(lhs) ^ number(rhs) );
        }
        constexpr traverse_event operator |(const traverse_event lhs, const traverse_event rhs) noexcept {
            return static_cast<traverse_event> ( number(lhs) | number(rhs) );
        }
        constexpr traverse_event operator &(const traverse_event lhs, const traverse_event rhs) noexcept {
            return static_cast<traverse_event> ( number(lhs) & number(rhs) );
        }
        constexpr traverse_event& operator |=(traverse_event& lhs, const traverse_event rhs) noexcept {
            lhs = static_cast<traverse_event> ( number(lhs) | number(rhs) );
            return lhs;
        }
        constexpr traverse_event& operator &=(traverse_event& lhs, const traverse_event rhs) noexcept {
            lhs = static_cast<traverse_event> ( number(lhs) & number(rhs) );
            return lhs;
        }
        constexpr traverse_event& operator ^=(traverse_event& lhs, const traverse_event rhs) noexcept {
            lhs = static_cast<traverse_event> ( number(lhs) ^ number(rhs) );
            return lhs;
        }
        constexpr bool operator ==(const traverse_event lhs, const traverse_event rhs) noexcept {
            return number(lhs) == number(rhs);
        }
        constexpr bool operator !=(const traverse_event lhs, const traverse_event rhs) noexcept {
            return !( lhs == rhs );
        }
        constexpr bool is_set(const traverse_event mask, const traverse_event bit) noexcept {
            return bit == ( mask & bit );
        }
        std::string to_string(const traverse_event mask) noexcept;

        /**
         * path_visitor jau::FunctionDef definition
         * - `bool visitor(traverse_event tevt, const file_stats& item_stats)`
         */
        typedef jau::FunctionDef<bool, traverse_event, const file_stats&> path_visitor;

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
                recursive = 1 << 0,

                /** Traverse through symbolic linked directories if traverse_options::recursive is set, i.e. directories with property fmode_t::link set. */
                follow_symlinks = 1 << 1,

                /** Visit the content's parent directory at entry. Both, dir_entry and dir_exit can be set, only one or none. */
                dir_entry = 1 << 2,

                /** Visit the content's parent directory at exit. Both, dir_entry and dir_exit can be set, only one or none. */
                dir_exit = 1 << 3,

                /** Enable verbosity mode, potentially used by a path_visitor implementation like remove(). */
                verbose = 1 << 15
        };
        constexpr uint16_t number(const traverse_options rhs) noexcept {
            return static_cast<uint16_t>(rhs);
        }
        constexpr traverse_options operator ~(const traverse_options rhs) noexcept {
            return static_cast<traverse_options> ( ~number(rhs) );
        }
        constexpr traverse_options operator ^(const traverse_options lhs, const traverse_options rhs) noexcept {
            return static_cast<traverse_options> ( number(lhs) ^ number(rhs) );
        }
        constexpr traverse_options operator |(const traverse_options lhs, const traverse_options rhs) noexcept {
            return static_cast<traverse_options> ( number(lhs) | number(rhs) );
        }
        constexpr traverse_options operator &(const traverse_options lhs, const traverse_options rhs) noexcept {
            return static_cast<traverse_options> ( number(lhs) & number(rhs) );
        }
        constexpr traverse_options& operator |=(traverse_options& lhs, const traverse_options rhs) noexcept {
            lhs = static_cast<traverse_options> ( number(lhs) | number(rhs) );
            return lhs;
        }
        constexpr traverse_options& operator &=(traverse_options& lhs, const traverse_options rhs) noexcept {
            lhs = static_cast<traverse_options> ( number(lhs) & number(rhs) );
            return lhs;
        }
        constexpr traverse_options& operator ^=(traverse_options& lhs, const traverse_options rhs) noexcept {
            lhs = static_cast<traverse_options> ( number(lhs) ^ number(rhs) );
            return lhs;
        }
        constexpr bool operator ==(const traverse_options lhs, const traverse_options rhs) noexcept {
            return number(lhs) == number(rhs);
        }
        constexpr bool operator !=(const traverse_options lhs, const traverse_options rhs) noexcept {
            return !( lhs == rhs );
        }
        constexpr bool is_set(const traverse_options mask, const traverse_options bit) noexcept {
            return bit == ( mask & bit );
        }
        std::string to_string(const traverse_options mask) noexcept;

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
        bool visit(const std::string& path, const traverse_options topts, const path_visitor& visitor) noexcept;

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
        bool visit(const file_stats& item_stats, const traverse_options topts, const path_visitor& visitor) noexcept;

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
        bool remove(const std::string& path, const traverse_options topts=traverse_options::none) noexcept;

        /**
         * Compare the bytes of both files, denoted by source1 and source2.
         *
         * @param source1 first source file to compare
         * @param source2 second source file to compare
         * @param verbose defaults to false
         * @return true if both elements are files and their bytes are equal, otherwise false.
         */
        bool compare(const file_stats& source1, const file_stats& source2, const bool verbose=false) noexcept;

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
             * Ignore errors from erroneous symlinks, i.e. non-existing link-targets or recursive loop-errors.
             *
             * This flag is required to copy erroneous symlinks using follow_symlinks, otherwise not.
             */
            ignore_symlink_errors = 1 << 8,

            /** Overwrite existing destination files, always. */
            overwrite = 1 << 9,

            /** Preserve uid and gid if allowed and access- and modification-timestamps, i.e. producing a most exact meta-data copy. */
            preserve_all = 1 << 10,

            /** Ensure data and meta-data file synchronization is performed via ::fsync() after asynchronous copy operations of a file's content. */
            sync = 1 << 11,

            /** Enable verbosity mode, show error messages on stderr. */
            verbose = 1 << 15
        };
        constexpr uint16_t number(const copy_options rhs) noexcept {
            return static_cast<uint16_t>(rhs);
        }
        constexpr copy_options operator ~(const copy_options rhs) noexcept {
            return static_cast<copy_options> ( ~number(rhs) );
        }
        constexpr copy_options operator ^(const copy_options lhs, const copy_options rhs) noexcept {
            return static_cast<copy_options> ( number(lhs) ^ number(rhs) );
        }
        constexpr copy_options operator |(const copy_options lhs, const copy_options rhs) noexcept {
            return static_cast<copy_options> ( number(lhs) | number(rhs) );
        }
        constexpr copy_options operator &(const copy_options lhs, const copy_options rhs) noexcept {
            return static_cast<copy_options> ( number(lhs) & number(rhs) );
        }
        constexpr copy_options& operator |=(copy_options& lhs, const copy_options rhs) noexcept {
            lhs = static_cast<copy_options> ( number(lhs) | number(rhs) );
            return lhs;
        }
        constexpr copy_options& operator &=(copy_options& lhs, const copy_options rhs) noexcept {
            lhs = static_cast<copy_options> ( number(lhs) & number(rhs) );
            return lhs;
        }
        constexpr copy_options& operator ^=(copy_options& lhs, const copy_options rhs) noexcept {
            lhs = static_cast<copy_options> ( number(lhs) ^ number(rhs) );
            return lhs;
        }
        constexpr bool operator ==(const copy_options lhs, const copy_options rhs) noexcept {
            return number(lhs) == number(rhs);
        }
        constexpr bool operator !=(const copy_options lhs, const copy_options rhs) noexcept {
            return !( lhs == rhs );
        }
        constexpr bool is_set(const copy_options mask, const copy_options bit) noexcept {
            return bit == ( mask & bit );
        }
        std::string to_string(const copy_options mask) noexcept;

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
        bool copy(const std::string& source_path, const std::string& dest_path, const copy_options copts = copy_options::none) noexcept;

        struct mount_ctx {
            bool mounted;
            std::string mount_point;
            int loop_device_id;

            mount_ctx(const std::string& mount_point_, const int loop_device_id_)
            : mounted(true), mount_point(mount_point_), loop_device_id(loop_device_id_) {}

            mount_ctx()
            : mounted(false), mount_point(), loop_device_id(-1) {}
        };

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
        mount_ctx mount_image(const std::string& image_path, const std::string& mount_point, const std::string& fs_type,
                              const unsigned long mountflags, const std::string fs_options="");

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
        bool umount(const mount_ctx& context);

        /**@}*/

    } /* namespace fs */

} /* namespace jau */

#endif /* JAU_FILE_UTIL_HPP_ */
