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
         * Generic file type and mode bits as used in file_stats
         */
        enum class fmode_bits : uint32_t {
            /** No mode bit set */
            NONE         =        0,
            /** Entity is a file. */
            FILE         = 1U <<  0,
            /** Entity is a directory. */
            DIR          = 1U <<  1,
            /** Entity is a symbolic link. */
            LINK         = 1U <<  2,
            /** Entity gives no access to user. */
            NO_ACCESS    = 1U << 30,
            /** Entity does not exist. */
            NOT_EXISTING = 1U << 31
        };
        constexpr uint32_t number(const fmode_bits rhs) noexcept {
            return static_cast<uint32_t>(rhs);
        }
        constexpr fmode_bits operator ^(const fmode_bits lhs, const fmode_bits rhs) noexcept {
            return static_cast<fmode_bits> ( number(lhs) ^ number(rhs) );
        }
        constexpr fmode_bits operator |(const fmode_bits lhs, const fmode_bits rhs) noexcept {
            return static_cast<fmode_bits> ( number(lhs) | number(rhs) );
        }
        constexpr fmode_bits& operator |=(fmode_bits& lhs, const fmode_bits rhs) noexcept {
            lhs = static_cast<fmode_bits> ( number(lhs) | number(rhs) );
            return lhs;
        }
        constexpr fmode_bits operator &(const fmode_bits lhs, const fmode_bits rhs) noexcept {
            return static_cast<fmode_bits> ( number(lhs) & number(rhs) );
        }
        constexpr bool operator ==(const fmode_bits lhs, const fmode_bits rhs) noexcept {
            return number(lhs) == number(rhs);
        }
        constexpr bool operator !=(const fmode_bits lhs, const fmode_bits rhs) noexcept {
            return !( lhs == rhs );
        }
        constexpr bool has_fmode_bit(const fmode_bits mask, const fmode_bits bit) noexcept {
            return fmode_bits::NONE != ( mask & bit );
        }
        std::string to_string(const fmode_bits mask) noexcept;

        /**
         * Representing a directory element, optionally split into parent_dir() and item(),
         * where item() shall not be empty.
         */
        class dir_item {
            private:
                std::string parent_dir_;
                std::string element_;

            public:
                dir_item(std::string parent_dir__, std::string element__) noexcept
                : parent_dir_(parent_dir__), element_(element__) {}

                dir_item(std::string element__) noexcept
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
         * Platform agnostic C++ representation of POSIX ::lstat()
         * for a given pathname.
         */
        class file_stats {
            public:
                typedef uint32_t uid_t;
                typedef uint32_t gid_t;

            private:
                dir_item item_;

                fmode_bits mode_;
                uid_t uid_;
                gid_t gid_;
                size_t size_;
                fraction_timespec atime_;
                fraction_timespec mtime_;
                fraction_timespec ctime_;

                int errno_res_;

            public:
                /**
                 * Instantiates a file_stats for the given `path`
                 * using either `::lstat()` (default) or `::stat()`.
                 *
                 * The dir_item will be constructed without parent_dir
                 * @param path the path to produce stats for
                 * @param use_lstat if true using `::lstat()` (default) allowing symbolic links to be detected, otherwise using `::stat()`.
                 */
                file_stats(const std::string& path, const bool use_lstat=true) noexcept;

                /**
                 * Instantiates a file_stats for the given dir_item
                 * using either `::lstat()` (default) or `::stat()`.
                 *
                 * @param item the dir_item to produce stats for
                 * @param use_lstat if true using `::lstat()` (default) allowing symbolic links to be detected, otherwise using `::stat()`.
                 */
                file_stats(const dir_item& item, const bool use_lstat=true) noexcept;

                /** Returns the dir_item. */
                const dir_item& item() const noexcept { return item_; }

                /** Returns a unix path representation from item(). */
                std::string path() const noexcept { return item_.path(); }

                /** Returns the fmode_bits, file type and mode. */
                fmode_bits mode() const noexcept { return mode_; }

                /** Returns the user id, owning the element. */
                uid_t uid() const noexcept { return uid_; }

                /** Returns the group id, owning the element. */
                gid_t gid() const noexcept { return gid_; }

                /** Returns the size in bytes of this element if its a fmode_bits::FILE, otherwise zero. */
                size_t size() const noexcept { return size_; }

                /** Returns the last access time of this element since Unix Epoch. */
                const fraction_timespec& atime() const noexcept { return atime_; }
                /** Returns the last modification time of this element since Unix Epoch. */
                const fraction_timespec& mtime() const noexcept { return mtime_; }
                /** Returns the last status change time of this element since Unix Epoch. */
                const fraction_timespec& ctime() const noexcept { return ctime_; }

                /** Returns the `errno` value occurred to produce this instance, or zero for no error. */
                int errno_res() const noexcept { return errno_res_; }

                /** Returns true if no error occurred */
                bool ok()  const noexcept { return 0 == errno_res_; }

                /** Returns true if given path is fmode_bits::FILE  */
                bool is_file() const noexcept { return has_fmode_bit( mode_, fmode_bits::FILE ); }
                bool is_dir() const noexcept { return has_fmode_bit( mode_, fmode_bits::DIR ); }
                bool is_link() const noexcept { return has_fmode_bit( mode_, fmode_bits::LINK ); }

                bool has_access() const noexcept { return !has_fmode_bit( mode_, fmode_bits::NO_ACCESS ); }
                bool exists() const noexcept { return !has_fmode_bit( mode_, fmode_bits::NOT_EXISTING ); }

                /**
                 * Returns a comprehensive string representation of this element
                 * @param use_space if true, using space instead for 'T' separator and drop trailing UTC `Z` for readability, otherwise be compliant with ISO 8601 (default)
                 */
                std::string to_string(const bool use_space=false) const noexcept;
        };

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
         * All elements of type fmode_bits::FILE, fmode_bits::DIR, fmode_bits::LINK and fmode_bits::NO_ACCESS
         * will be visited by the `visitor`
         * and processing ends if it returns `false`.
         *
         * @param path the starting path
         * @param visitor path_visitor function `bool visitor(const file_stats& item_stats)`.
         * @return true if all visitor invocation returned true and no error occurred, otherwise false
         */
        bool visit(const std::string& path, const path_visitor& visitor) noexcept;

        /**
         * Visit all elements of path in a recursive manner, visiting content first then its parent directory.
         *
         * Any element without access or error otherwise will be skipped.
         *
         * All elements of type fmode_bits::FILE, fmode_bits::DIR, fmode_bits::LINK and fmode_bits::NO_ACCESS
         * will be visited by the `visitor`
         * and processing ends if it returns `false`.
         *
         * @param item_stats pre-fetched file_stats for a given dir_item, used for efficiency
         * @param visitor path_visitor function `bool visitor(const file_stats& item_stats)`.
         * @return true if all visitor invocation returned true and no error occurred, otherwise false
         */
        bool visit(const file_stats& item_stats, const path_visitor& visitor) noexcept;

        /**
         * Remove the given path. If path represents a director, `recursive` must be set to true.
         * @param path path to remove
         * @param recursive indicates to remove directories
         * @return true only if the file or the directory with content has been deleted, otherwise false
         */
        bool remove(const std::string& path, const bool recursive, const bool verbose=false) noexcept;

        /**@}*/

    } /* namespace fs */

} /* namespace jau */

#endif /* JAU_FILE_UTIL_HPP_ */
