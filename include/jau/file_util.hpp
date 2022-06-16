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
         * Representing a directory element, optionally split into parent_dir() and item(),
         * where item() shall not be empty.
         */
        class dir_item {
            private:
                std::string parent_dir_;
                std::string element_;

            public:
                dir_item() noexcept
                : parent_dir_(), element_() {}

                dir_item(const std::string_view& parent_dir__, const std::string_view& element__) noexcept
                : parent_dir_(parent_dir__), element_(element__) {}

                dir_item(const std::string_view& element__) noexcept
                : parent_dir_(), element_(element__) {}

                /** Returns the parent dir, may be empty. */
                const std::string& parent_dir() const noexcept { return parent_dir_; }

                /** Return the directory element, shall not be empty and may contain full path. */
                const std::string& element() const noexcept { return element_; }

                /**
                 * Returns a unix path representation combining parent_dir() and element().
                 */
                std::string path() const noexcept;
        };

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

                std::shared_ptr<file_stats> link_target_; // Link-target a symbolic-link points to if is_link(), otherwise nullptr.
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
                class ctor_cookie { friend file_stats; ctor_cookie(const uint16_t secret) { (void)secret; } };

            public:
                /** Instantiate an empty file_stats with fmode_t::not_existing set. */
                file_stats() noexcept;

                /** Private ctor for private make_shared<file_stats>() intended for friends. */
                file_stats(const ctor_cookie& cc, const dir_item& item, const std::string_view follow_symlink) noexcept;

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

                /** Returns the dir_item. */
                const dir_item& item() const noexcept { return item_; }

                /** Returns a unix path representation. */
                std::string path() const noexcept { return item_.path(); }

                /** Returns the link-target a symbolic-link points to if is_link(), otherwise nullptr. */
                const std::shared_ptr<file_stats>& link_target() const noexcept { return link_target_; }

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
         * Return stripped last component from given path separated by `/`, excluding the trailing separator `/`.
         *
         * If no directory separator `/` is contained, return `.`.
         *
         * @param path given path
         * @return leading directory name w/o slash or `.`
         */
        std::string dirname(const std::string_view& path) noexcept;

        /**
         * Return stripped leading directory components from given path separated by `/`.
         *
         * @param path given path
         * @return last non-slash component or `.`
         */
        std::string basename(const std::string_view& path) noexcept;

        /**
         * Return the current working directory or empty on failure.
         */
        std::string get_cwd() noexcept;

        /**
         * Create directory
         * @param path full path to new directory
         * @param verbose
         * @return true if successful, otherwise false
         */
        bool mkdir(const std::string& path, const bool verbose=false) noexcept;

        /**
         * Touch the file with given atime and mtime and create file if not existing yet.
         * @param path full path to file
         * @param atime new access time
         * @param mtime new modification time
         * @param verbose
         * @return true if successful, otherwise false
         */
        bool touch(const std::string& path, const jau::fraction_timespec& atime, const jau::fraction_timespec& mtime, const bool verbose=false) noexcept;

        /**
         * Touch the file with current time and create file if not existing yet.
         * @param path full path to file
         * @param verbose
         * @return true if successful, otherwise false
         */
        bool touch(const std::string& path, const bool verbose=false) noexcept;

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
         * path_visitor jau::FunctionDef definition
         * - `bool visitor(const file_stats& item_stats)`
         */
        typedef jau::FunctionDef<bool, const file_stats&> path_visitor;

        /**
         * Visit all elements of path in a recursive manner, visiting content first then its parent directory.
         *
         * Any element without access or error otherwise will be skipped.
         *
         * All elements of type fmode_bits::FILE, fmode_bits::DIR and fmode_bits::NO_ACCESS
         * will be visited by the `visitor`
         * and processing ends if it returns `false`.
         *
         * @param path the starting path
         * @param follow_sym_link_dirs pass true to visit directories with property fmode_bits::LINK, otherwise false.
         * @param visitor path_visitor function `bool visitor(const file_stats& item_stats)`.
         * @return true if all visitor invocation returned true and no error occurred, otherwise false
         */
        bool visit(const std::string& path, const bool follow_sym_link_dirs, const path_visitor& visitor) noexcept;

        /**
         * Visit all elements of path in a recursive manner, visiting content first then its parent directory.
         *
         * Any element without access or error otherwise will be skipped.
         *
         * All elements of type fmode_bits::FILE, fmode_bits::DIR and fmode_bits::NO_ACCESS
         * will be visited by the `visitor`
         * and processing ends if it returns `false`.
         *
         * @param item_stats pre-fetched file_stats for a given dir_item, used for efficiency
         * @param follow_sym_link_dirs pass true to visit directories with property fmode_bits::LINK, otherwise false.
         * @param visitor path_visitor function `bool visitor(const file_stats& item_stats)`.
         * @return true if all visitor invocation returned true and no error occurred, otherwise false
         */
        bool visit(const file_stats& item_stats, const bool follow_sym_link_dirs, const path_visitor& visitor) noexcept;

        /**
         * Remove the given path. If path represents a director, `recursive` must be set to true.
         * @param path path to remove
         * @param recursive indicates to remove directories
         * @param follow_sym_link_dirs pass true to remove directories with property fmode_bits::LINK (default), otherwise false.
         * @param verbose pass true for verbose information about deleted files and directories, otherwise false (default)
         * @return true only if the file or the directory with content has been deleted, otherwise false
         */
        bool remove(const std::string& path, const bool recursive, const bool follow_sym_link_dirs=true, const bool verbose=false) noexcept;

        /**@}*/

    } /* namespace fs */

} /* namespace jau */

#endif /* JAU_FILE_UTIL_HPP_ */
