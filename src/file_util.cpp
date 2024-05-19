/*
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

#include <jau/debug.hpp>
#include <jau/file_util.hpp>
#include <jau/base_codec.hpp>
#include <jau/os/os_support.hpp>
#include <jau/secmem.hpp>

#include <cstdint>
#include <cstdlib>
#include <cinttypes>
#include <limits>
#include <cstring>
#include <cstdio>
#include <random>

extern "C" {
    #include <unistd.h>
    #include <dirent.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <sys/mount.h>
    #if defined(__linux__)
        #include <sys/sendfile.h>
        #include <linux/loop.h>
    #endif
}

#ifndef O_BINARY
#define O_BINARY    0
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK  0
#endif

inline constexpr const int _open_dir_flags = O_RDONLY|O_BINARY|O_NOCTTY|O_DIRECTORY;

using namespace jau;
using namespace jau::fs;

#if defined(__linux__) && defined(__GLIBC__)
    #define _USE_STATX_ 1
    #define _USE_SENDFILE_ 1
#elif defined(__linux__)
    #define _USE_SENDFILE_ 1
#endif

#if defined(__FreeBSD__)
    typedef struct ::stat struct_stat64;
    typedef off_t off64_t;
    #define __posix_fstatat64 ::fstatat
    #define __posix_openat64  ::openat
#else
    typedef struct ::stat64 struct_stat64;
    #define __posix_fstatat64 ::fstatat64
    #define __posix_openat64  ::openat64
#endif

std::string jau::fs::get_cwd() noexcept {
    const size_t bsz = PATH_MAX; // including EOS
    std::string str;
    str.reserve(bsz);  // incl. EOS
    str.resize(bsz-1); // excl. EOS

    char* res = ::getcwd(&str[0], bsz);
    if( res == &str[0] ) {
        str.resize(::strnlen(res, bsz));
        str.shrink_to_fit();
        return str;
    } else {
        return std::string();
    }
}

bool jau::fs::chdir(const std::string& path) noexcept {
    return 0 == ::chdir(path.c_str());
}

std::string jau::fs::absolute(const std::string_view& relpath) noexcept {
    const size_t bsz = PATH_MAX; // including EOS
    std::string str;
    str.reserve(bsz);  // incl. EOS
    str.resize(bsz-1); // excl. EOS

    char *res = ::realpath(&relpath[0], &str[0]);
    if( res == &str[0] ) {
        str.resize(::strnlen(res, bsz));
        str.shrink_to_fit();
        return str;
    } else {
        return std::string();
    }
}

static const char c_slash('/');
static const char c_backslash('\\');
static const std::string s_slash("/");
static const std::string s_slash_dot_slash("/./");
static const std::string s_slash_dot("/.");
static const std::string s_dot_slash("./");
static const std::string s_dot(".");
static const std::string s_slash_dotdot_slash("/../");
static const std::string s_slash_dotdot("/..");
static const std::string s_dotdot("..");

std::string jau::fs::dirname(const std::string_view& path) noexcept {
    if( 0 == path.size() ) {
        return s_dot;
    }
    size_t end_pos;
    if( c_slash == path[path.size()-1] ) {
        if( 1 == path.size() ) { // maintain a single '/'
            return std::string(path);
        }
        end_pos = path.size()-2;
    } else {
        end_pos = path.size()-1;
    }
    size_t idx = path.find_last_of(c_slash, end_pos);
    if( idx == std::string_view::npos ) {
        return s_dot;
    } else {
        // ensure `/lala` -> '/', i.e. don't cut off single '/'
        return std::string( path.substr(0, std::max<size_t>(1, idx)) );
    }
}

std::string jau::fs::basename(const std::string_view& path) noexcept {
    if( 0 == path.size() ) {
        return s_dot;
    }
    size_t end_pos;
    if( c_slash == path[path.size()-1] ) {
        if( 1 == path.size() ) { // maintain a single '/'
            return std::string(path);
        }
        end_pos = path.size()-2;
    } else {
        end_pos = path.size()-1;
    }
    size_t idx = path.find_last_of(c_slash, end_pos);
    if( idx == std::string_view::npos ) {
        return std::string( path.substr(0, end_pos+1));
    } else {
        return std::string( path.substr(idx+1, end_pos-idx) );
    }
}

bool jau::fs::isAbsolute(const std::string_view& path) noexcept {
    return path.size() > 0 &&
           ( c_slash == path[0] || ( jau::os::is_windows() && c_backslash == path[0] ) );
}

std::unique_ptr<dir_item::backed_string_view> dir_item::reduce(const std::string_view& path_) noexcept {
    constexpr const bool _debug = false;

    if constexpr ( _debug ) {
        jau::fprintf_td(stderr, "X.0: path '%s'\n", std::string(path_).c_str());
    }

    std::unique_ptr<dir_item::backed_string_view> path2 = std::make_unique<dir_item::backed_string_view>(path_);

    if( s_dot == path_ || s_slash == path_ ) {
        return path2;
    }

    // remove initial './'
    while( path2->view.starts_with(s_dot_slash) ) {
        path2->view = path2->view.substr(2, path2->view.size()-2);
    }

    // remove trailing slash if not ending with '/./' or '/../'
    if( c_slash == path2->view[path2->view.size()-1] &&
        ( path2->view.size() < 3 || std::string_view::npos == path2->view.find(s_slash_dot_slash, path2->view.size()-3) ) &&
        ( path2->view.size() < 4 || std::string_view::npos == path2->view.find(s_slash_dotdot_slash, path2->view.size()-4) )
      )
    {
        path2->view = path2->view.substr(0, path2->view.size()-1);
    }

    // append final '/' to complete '/../' or '/./' sequence
    if( ( path2->view.size() >= 3 && std::string_view::npos != path2->view.find(s_slash_dotdot, path2->view.size()-3) ) ||
        ( path2->view.size() >= 2 && std::string_view::npos != path2->view.find(s_slash_dot, path2->view.size()-2) ) ) {
        path2->backup();
        path2->view = path2->backing.append(s_slash);
    }

    if constexpr ( _debug ) {
        jau::fprintf_td(stderr, "X.1: path2 '%s'\n", path2->to_string(true).c_str());
    }

    // resolve '/./'
    size_t spos=0;
    size_t idx;
    do {
        idx = path2->view.find(s_slash_dot_slash, spos);
        if constexpr ( _debug ) {
            jau::fprintf_td(stderr, "X.2.1: path2: spos %zu, idx %zu, '%s'\n", spos, idx, path2->to_string(true).c_str());
        }
        if( std::string_view::npos == idx ) {
            break;
        }
        std::string_view pre = path2->view.substr(0, idx);
        if( 0 == pre.size() ) {
            // case '/./bbb' -> '/bbb'
            path2->view = path2->view.substr(idx+2);
            spos = 0;
        } else {
            // case '/zzz/aaa/./bbb' -> '/zzz/aaa/bbb'
            const std::string post( path2->view.substr(idx+2) );
            path2->backup_and_append( pre, post );
            spos = pre.size();
        }
        if constexpr ( _debug ) {
            jau::fprintf_td(stderr, "X.2.2: path2: spos %zu, '%s'\n", spos, path2->to_string(true).c_str());
        }
    } while( spos <= path2->view.size()-3 );
    if constexpr ( _debug ) {
        jau::fprintf_td(stderr, "X.2.X: path2: '%s'\n", path2->to_string(true).c_str());
    }

    // resolve '/../'
    spos=0;
    do {
        idx = path2->view.find(s_slash_dotdot_slash, spos);
        if constexpr ( _debug ) {
            jau::fprintf_td(stderr, "X.3.1: path2: spos %zu, idx %zu, '%s'\n", spos, idx, path2->to_string(true).c_str());
        }
        if( std::string_view::npos == idx ) {
            break;
        }
        if( 0 == idx ) {
            // case '/../bbb' -> Error, End
            WARN_PRINT("dir_item::resolve: '..' resolution error: '%s' -> '%s'", std::string(path_).c_str(), path2->to_string().c_str());
            return path2;
        }
        std::string_view pre = path2->view.substr(0, idx);
        if( 2 == idx && s_dotdot == pre ) {
            // case '../../bbb' -> '../../bbb' unchanged
            spos = idx+4;
        } else if( 3 <= idx && s_slash_dotdot == path2->view.substr(idx-3, 3) ) {
            // case '../../../bbb' -> '../../../bbb' unchanged
            spos = idx+4;
        } else {
            std::string pre_str = jau::fs::dirname( pre );
            if( s_slash == pre_str ) {
                // case '/aaa/../bbb' -> '/bbb'
                path2->view = path2->view.substr(idx+3);
                spos = 0;
            } else if( s_dot == pre_str ) {
                // case 'aaa/../bbb' -> 'bbb'
                path2->view = path2->view.substr(idx+4);
                spos = 0;
            } else {
                // case '/zzz/aaa/../bbb' -> '/zzz/bbb'
                const std::string post( path2->view.substr(idx+3) );
                path2->backup_and_append( pre_str, post );
                spos = pre_str.size();
            }
        }
        if constexpr ( _debug ) {
            jau::fprintf_td(stderr, "X.3.2: path2: spos %zu, '%s'\n", spos, path2->to_string(true).c_str());
        }
    } while( spos <= path2->view.size()-4 );
    if constexpr ( _debug ) {
        jau::fprintf_td(stderr, "X.3.X: path2: '%s'\n", path2->to_string(true).c_str());
    }

    // remove trailing slash in path2
    if( c_slash == path2->view[path2->view.size()-1] ) {
        path2->view = path2->view.substr(0, path2->view.size()-1);
    }
    if constexpr ( _debug ) {
        jau::fprintf_td(stderr, "X.X: path2: '%s'\n", path2->to_string(true).c_str());
    }
    return path2;
}

dir_item::dir_item(std::unique_ptr<backed_string_view> cleanpath) noexcept
: dirname_(jau::fs::dirname(cleanpath->view)), basename_(jau::fs::basename(cleanpath->view)), empty_( cleanpath->view.empty() ) {
    if( s_slash == dirname_ && s_slash == basename_ ) { // remove duplicate '/' in basename
        basename_ = s_dot;
    }
}

dir_item::dir_item(std::string dirname__, std::string basename__) noexcept
: dirname_(std::move(dirname__)), basename_(std::move(basename__)), empty_(dirname_.empty() && basename_.empty()) {
}

dir_item::dir_item() noexcept
: dirname_(s_dot), basename_(s_dot), empty_(true) {}

dir_item::dir_item(const std::string_view& path_) noexcept
: dir_item( reduce(path_) )
{ }


std::string dir_item::path() const noexcept {
    if( s_dot ==  dirname_ ) {
        return basename_;
    }
    if( s_dot ==  basename_ ) {
        return dirname_;
    }
    if( s_slash == dirname_ ) {
        return dirname_ + basename_;
    }
    return dirname_ + s_slash + basename_;
}

std::string dir_item::to_string() const noexcept {
    return "['"+dirname()+"', '"+basename()+"']";
}


template<typename T>
static void append_bitstr(std::string& out, T mask, T bit, const std::string& bitstr, bool& comma) {
    if( is_set( mask, bit )) {
        if( comma ) { out.append(", "); }
        out.append(bitstr); comma = true;
    }
}

#define APPEND_BITSTR(U,V,M) append_bitstr(out, M, U::V, #V, comma);

#define FMODEBITS_ENUM(X,M) \
    X(fmode_t,sock,M) \
    X(fmode_t,blk,M) \
    X(fmode_t,chr,M) \
    X(fmode_t,fifo,M) \
    X(fmode_t,dir,M) \
    X(fmode_t,file,M) \
    X(fmode_t,link,M) \
    X(fmode_t,no_access,M) \
    X(fmode_t,not_existing,M)

static void append_bitstr(std::string& out, fmode_t mask, fmode_t bit, const std::string& bitstr) {
    if( is_set( mask, bit )) {
        out.append(bitstr);
    } else {
        out.append("-");
    }
}

std::string jau::fs::to_string(const fmode_t mask, const bool show_rwx) noexcept {
    std::string out;
    bool comma = false;
    FMODEBITS_ENUM(APPEND_BITSTR,mask)
    if( fmode_t::none != ( mask & fmode_t::protection_mask ) ) {
        out.append(", ");
        if( show_rwx ) {
            if( fmode_t::none != ( mask & fmode_t::ugs_set ) ) {
                append_bitstr(out, mask, fmode_t::set_uid, "u");
                append_bitstr(out, mask, fmode_t::set_gid, "g");
                append_bitstr(out, mask, fmode_t::sticky,  "s");
            }
            const std::string r("r");
            const std::string w("w");
            const std::string x("x");
            append_bitstr(out, mask, fmode_t::read_usr,  r);
            append_bitstr(out, mask, fmode_t::write_usr, w);
            append_bitstr(out, mask, fmode_t::exec_usr,  x);
            append_bitstr(out, mask, fmode_t::read_grp,  r);
            append_bitstr(out, mask, fmode_t::write_grp, w);
            append_bitstr(out, mask, fmode_t::exec_grp,  x);
            append_bitstr(out, mask, fmode_t::read_oth,  r);
            append_bitstr(out, mask, fmode_t::write_oth, w);
            append_bitstr(out, mask, fmode_t::exec_oth,  x);
        } else {
            char buf[8];
            int len = snprintf(buf, sizeof(buf), "0%o", (unsigned int)(mask & fmode_t::protection_mask));
            out.append(std::string(buf, len));
        }
    }
    return out;
}

std::string jau::fs::to_named_fd(const int fd) noexcept {
    if( 0 > fd ) {
        return "";
    }
    std::string res("/dev/fd/");
    res.append(std::to_string(fd));
    return res;
}

int jau::fs::from_named_fd(const std::string& named_fd) noexcept {
    int scan_value = -1;
    if( 1 == sscanf(named_fd.c_str(), "/dev/fd/%d", &scan_value) ) {
        // GNU/Linux, FreeBSD, ... ?
        return scan_value;
    } else if( 1 == sscanf(named_fd.c_str(), "/proc/self/fd/%d", &scan_value) ) {
        // GNU/Linux only?
        return scan_value;
    }
    return -1;
}

#define FILESTATS_FIELD_ENUM(X,M) \
    X(file_stats::field_t,type,M) \
    X(file_stats::field_t,mode,M) \
    X(file_stats::field_t,nlink,M) \
    X(file_stats::field_t,uid,M) \
    X(file_stats::field_t,gid,M) \
    X(file_stats::field_t,atime,M) \
    X(file_stats::field_t,mtime,M) \
    X(file_stats::field_t,ctime,M) \
    X(file_stats::field_t,ino,M) \
    X(file_stats::field_t,size,M) \
    X(file_stats::field_t,blocks,M) \
    X(file_stats::field_t,btime,M)

std::string jau::fs::to_string(const file_stats::field_t mask) noexcept {
    std::string out("[");
    bool comma = false;
    FILESTATS_FIELD_ENUM(APPEND_BITSTR,mask)
    out.append("]");
    return out;
}

file_stats::file_stats() noexcept
: has_fields_(field_t::none), item_(), link_target_path_(), link_target_(), mode_(fmode_t::not_existing), fd_(-1),
  uid_(0), gid_(0), size_(0), btime_(), atime_(), ctime_(), mtime_(),
  errno_res_(0)
{}

#if _USE_STATX_
    static constexpr bool jau_has_stat(const uint32_t mask, const uint32_t bit) { return bit == ( mask & bit ); }
#endif

file_stats::file_stats(const ctor_cookie& cc, int dirfd, const dir_item& item, const bool dirfd_is_item_dirname) noexcept
: has_fields_(field_t::none), item_(), link_target_path_(), link_target_(), mode_(fmode_t::none), fd_(-1),
  uid_(0), gid_(0), size_(0), btime_(), atime_(), ctime_(), mtime_(), errno_res_(0)
{
    constexpr const bool _debug = false;
    (void)cc;
    const std::string full_path( item.empty() ? "" : item.path() );
    if( item.empty() && AT_FDCWD != dirfd ) {
        if( 0 <= dirfd ) {
            has_fields_ |= field_t::fd;
            fd_ = dirfd;
            item_ = dir_item(jau::fs::to_named_fd(fd_));
        } else {
            ERR_PRINT("rec_level %d, dirfd %d < 0, %s, dirfd_is_item_dirname %d, AT_EMPTY_PATH",
                    (int)cc.rec_level, dirfd, item.to_string().c_str(), dirfd_is_item_dirname);
            return;
        }
    } else {
        item_ = item;
        int scan_value = jau::fs::from_named_fd(full_path);
        if( 0 <= scan_value ) {
            has_fields_ |= field_t::fd;
            dirfd = scan_value; // intentional overwrite
            fd_ = dirfd;
        } else if( full_path.starts_with("/dev/fd/pipe:") ) {
            // Last resort and should-be unreachable,
            // since above `jau::fs::from_named_fd()` shall hit!
            //
            // fifo/pipe object used in GNU/Linux (at least),
            // which can't be stat'ed / accessed
            has_fields_ |= field_t::type;
            mode_ |= fmode_t::fifo;
            if constexpr ( _debug ) {
                jau::fprintf_td(stderr, "file_stats(%d): FIFO: '%s', errno %d (%s)\n", (int)cc.rec_level, to_string().c_str(), errno, ::strerror(errno));
            }
            return;
        }
    }
    const std::string dirfd_path = has( field_t::fd ) ? "" : ( dirfd_is_item_dirname ? item_.basename() : full_path );

#if _USE_STATX_
    struct ::statx s;
    jau::zero_bytes_sec(&s, sizeof(s));
    int stat_res = ::statx(dirfd, dirfd_path.c_str(),
                           ( AT_NO_AUTOMOUNT | AT_SYMLINK_NOFOLLOW | ( has( field_t::fd ) ? AT_EMPTY_PATH : 0 ) ),
                           ( STATX_BASIC_STATS | STATX_BTIME ), &s);
    if( 0 != stat_res ) {
        if constexpr ( _debug ) {
            jau::fprintf_td(stderr, "file_stats(%d): Test ERROR: '%s', %d, errno %d (%s)\n", (int)cc.rec_level, full_path.c_str(), stat_res, errno, ::strerror(errno));
        }
        switch( errno ) {
            case EACCES:
                mode_ |= fmode_t::no_access;
                break;
            case ENOENT:
                mode_ |= fmode_t::not_existing;
                break;
            default:
                break;
        }
        if( has_access() && exists() ) {
            errno_res_ = errno;
        }
    } else {
        if( jau_has_stat( s.stx_mask, STATX_TYPE ) ) {
            has_fields_ |= field_t::type;
        }
        if( has( field_t::type ) ) {
            if( S_ISLNK( s.stx_mode ) ) {
                mode_ |= fmode_t::link;
            }
            if( S_ISREG( s.stx_mode ) ) {
                mode_ |= fmode_t::file;
            } else if( S_ISDIR( s.stx_mode ) ) {
                mode_ |= fmode_t::dir;
            } else if( S_ISFIFO( s.stx_mode ) ) {
                mode_ |= fmode_t::fifo;
            } else if( S_ISCHR( s.stx_mode ) ) {
                mode_ |= fmode_t::chr;
            } else if( S_ISSOCK( s.stx_mode ) ) {
                mode_ |= fmode_t::sock;
            } else if( S_ISBLK( s.stx_mode ) ) {
                mode_ |= fmode_t::blk;
            }
        }
        if( jau_has_stat( s.stx_mask, STATX_MODE ) ) {
            has_fields_ |= field_t::mode;
            // Append POSIX protection bits
            mode_ |= static_cast<fmode_t>( s.stx_mode & ( S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID | S_ISVTX ) );
        }
        if( jau_has_stat( s.stx_mask, STATX_NLINK ) ) {
            has_fields_ |= field_t::nlink;
        }
        if( jau_has_stat( s.stx_mask, STATX_UID ) ) {
            has_fields_ |= field_t::uid;
            uid_ = s.stx_uid;
        }
        if( jau_has_stat( s.stx_mask, STATX_GID ) ) {
            has_fields_ |= field_t::gid;
            gid_ = s.stx_gid;
        }
        if( jau_has_stat( s.stx_mask, STATX_ATIME ) || 0 != s.stx_atime.tv_sec || 0 != s.stx_atime.tv_nsec ) { // if STATX_ATIME is not reported but has its value (duh?)
            has_fields_ |= field_t::atime;
            atime_ = jau::fraction_timespec( s.stx_atime.tv_sec, s.stx_atime.tv_nsec );
        }
        if( jau_has_stat( s.stx_mask, STATX_MTIME ) ) {
            has_fields_ |= field_t::mtime;
            mtime_ = jau::fraction_timespec( s.stx_mtime.tv_sec, s.stx_mtime.tv_nsec );
        }
        if( jau_has_stat( s.stx_mask, STATX_CTIME ) ) {
            has_fields_ |= field_t::ctime;
            ctime_ = jau::fraction_timespec( s.stx_ctime.tv_sec, s.stx_ctime.tv_nsec );
        }
        if( jau_has_stat( s.stx_mask, STATX_INO ) ) {
            has_fields_ |= field_t::ino;
        }
        if( jau_has_stat( s.stx_mask, STATX_SIZE ) ) {
            if( !is_link() && is_file() ) {
                has_fields_ |= field_t::size;
                size_ = s.stx_size;
            }
        }
        if( jau_has_stat( s.stx_mask, STATX_BLOCKS ) ) {
            has_fields_ |= field_t::blocks;
        }
        if( jau_has_stat( s.stx_mask, STATX_BTIME ) ) {
            has_fields_ |= field_t::btime;
            btime_ = jau::fraction_timespec( s.stx_btime.tv_sec, s.stx_btime.tv_nsec );
        }
        if( is_link() ) {
            // follow symbolic link recursively until !exists(), is_file() or is_dir()
            std::string link_path;
            {
                const size_t path_link_max_len = 0 < s.stx_size ? s.stx_size + 1 : PATH_MAX;
                std::vector<char> buffer;
                buffer.reserve(path_link_max_len);
                buffer.resize(path_link_max_len);
                const ssize_t path_link_len = ::readlinkat(dirfd, dirfd_path.c_str(), buffer.data(), path_link_max_len);
                if( 0 > path_link_len ) {
                    errno_res_ = errno;
                    link_target_ = std::make_shared<file_stats>();
                    goto errorout;
                }
                // Note: if( path_link_len == path_link_max_len ) then buffer may have been truncated
                link_path = std::string(buffer.data(), path_link_len);
            }
            link_target_path_ = std::make_shared<std::string>(link_path);
            if( 0 == cc.rec_level ) {
                // Initial symbolic followed: Test recursive loop-error
                jau::zero_bytes_sec(&s, sizeof(s));
                stat_res = ::statx(dirfd, dirfd_path.c_str(), AT_NO_AUTOMOUNT | ( has( field_t::fd ) ? AT_EMPTY_PATH : 0 ), STATX_BASIC_STATS, &s);
                if( 0 != stat_res ) {
                    if constexpr ( _debug ) {
                        jau::fprintf_td(stderr, "file_stats(%d): Test link ERROR: '%s', %d, errno %d (%s)\n", (int)cc.rec_level, full_path.c_str(), stat_res, errno, ::strerror(errno));
                    }
                    switch( errno ) {
                        case EACCES:
                            mode_ |= fmode_t::no_access;
                            break;
                        case ELOOP:
                            // Too many symbolic links encountered while traversing the pathname
                            [[fallthrough]];
                        case ENOENT:
                            // A component of pathname does not exist
                            [[fallthrough]];
                        default:
                            // Anything else ..
                            mode_ |= fmode_t::not_existing;
                            break;
                    }
                    goto errorout;
                }
            }
            if( 0 < link_path.size() && c_slash == link_path[0] ) {
                link_target_ = std::make_shared<file_stats>(ctor_cookie(cc.rec_level+1), dirfd, dir_item( link_path ), false /* dirfd_is_item_dirname */); // absolute link_path
            } else {
                link_target_ = std::make_shared<file_stats>(ctor_cookie(cc.rec_level+1), dirfd, dir_item( jau::fs::dirname(full_path), link_path ), dirfd_is_item_dirname );
            }
            if( link_target_->has_fd() ) {
                has_fields_ |= field_t::fd;
                fd_ = link_target_->fd();
            }
            if( link_target_->is_socket() ) {
                mode_ |= fmode_t::sock;
            } else if( link_target_->is_block() ) {
                mode_ |= fmode_t::blk;
            } else if( link_target_->is_char() ) {
                mode_ |= fmode_t::chr;
            } else if( link_target_->is_fifo() ) {
                mode_ |= fmode_t::fifo;
            } else if( link_target_->is_dir() ) {
                mode_ |= fmode_t::dir;
            } else if( link_target_->is_file() ) {
                mode_ |= fmode_t::file;
                if( link_target_->has( field_t::size ) ) {
                    has_fields_ |= field_t::size;
                    size_ = link_target_->size();
                }
            } else if( !link_target_->exists() ) {
                mode_ |= fmode_t::not_existing;
            } else if( !link_target_->has_access() ) {
                mode_ |= fmode_t::no_access;
            }
        }
        if constexpr ( _debug ) {
            jau::fprintf_td(stderr, "file_stats(%d): '%s', %d, errno %d (%s)\n", (int)cc.rec_level, to_string().c_str(), stat_res, errno, ::strerror(errno));
        }
    }
#else /* _USE_STATX_ */
    struct_stat64 s;
    jau::zero_bytes_sec(&s, sizeof(s));
    int stat_res = __posix_fstatat64(dirfd, dirfd_path.c_str(), &s, AT_SYMLINK_NOFOLLOW | ( has( field_t::fd ) ? AT_EMPTY_PATH : 0 )); // lstat64 compatible
    if( 0 != stat_res ) {
        if constexpr ( _debug ) {
            jau::fprintf_td(stderr, "file_stats(%d): Test ERROR: '%s', %d, errno %d (%s)\n", (int)cc.rec_level, full_path.c_str(), stat_res, errno, ::strerror(errno));
        }
        switch( errno ) {
            case EACCES:
                mode_ |= fmode_t::no_access;
                break;
            case ENOENT:
                mode_ |= fmode_t::not_existing;
                break;
            default:
                break;
        }
        if( has_access() && exists() ) {
            errno_res_ = errno;
        }
    } else {
        has_fields_ = field_t::type  | field_t::mode  | field_t::uid   | field_t::gid |
                      field_t::atime | field_t::ctime | field_t::mtime;

        if( S_ISLNK( s.st_mode ) ) {
            mode_ |= fmode_t::link;
        }
        if( S_ISREG( s.st_mode ) ) {
            mode_ |= fmode_t::file;
            if( !is_link() ) {
                has_fields_ |= field_t::size;
                size_ = s.st_size;
            }
        } else if( S_ISDIR( s.st_mode ) ) {
            mode_ |= fmode_t::dir;
        } else if( S_ISFIFO( s.st_mode ) ) {
            mode_ |= fmode_t::fifo;
        } else if( S_ISCHR( s.st_mode ) ) {
            mode_ |= fmode_t::chr;
        } else if( S_ISSOCK( s.st_mode ) ) {
            mode_ |= fmode_t::sock;
        } else if( S_ISBLK( s.st_mode ) ) {
            mode_ |= fmode_t::blk;
        }

        // Append POSIX protection bits
        mode_ |= static_cast<fmode_t>( s.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID | S_ISVTX ) );

        uid_ = s.st_uid;
        gid_ = s.st_gid;
        atime_ = jau::fraction_timespec( s.st_atim.tv_sec, s.st_atim.tv_nsec );
        ctime_ = jau::fraction_timespec( s.st_ctim.tv_sec, s.st_ctim.tv_nsec );
        mtime_ = jau::fraction_timespec( s.st_mtim.tv_sec, s.st_mtim.tv_nsec );

        if( is_link() ) {
            // follow symbolic link recursively until !exists(), is_file() or is_dir()
            std::string link_path;
            {
                const size_t path_link_max_len = 0 < s.st_size ? s.st_size + 1 : PATH_MAX;
                std::vector<char> buffer;
                buffer.reserve(path_link_max_len);
                buffer.resize(path_link_max_len);
                const ssize_t path_link_len = ::readlinkat(dirfd, dirfd_path.c_str(), buffer.data(), path_link_max_len);
                if( 0 > path_link_len ) {
                    errno_res_ = errno;
                    link_target_ = std::make_shared<file_stats>();
                    goto errorout;
                }
                // Note: if( path_link_len == path_link_max_len ) then buffer may have been truncated
                link_path = std::string(buffer.data(), path_link_len);
            }
            link_target_path_ = std::make_shared<std::string>(link_path);
            if( 0 == cc.rec_level ) {
                // Initial symbolic followed: Test recursive loop-error
                jau::zero_bytes_sec(&s, sizeof(s));
                stat_res = __posix_fstatat64(dirfd, dirfd_path.c_str(), &s, has( field_t::fd ) ? AT_EMPTY_PATH : 0); // stat64 compatible
                if( 0 != stat_res ) {
                    if constexpr ( _debug ) {
                        jau::fprintf_td(stderr, "file_stats(%d): Test link ERROR: '%s', %d, errno %d (%s)\n", (int)cc.rec_level, full_path.c_str(), stat_res, errno, ::strerror(errno));
                    }
                    switch( errno ) {
                        case EACCES:
                            mode_ |= fmode_t::no_access;
                            break;
                        case ELOOP:
                            // Too many symbolic links encountered while traversing the pathname
                            [[fallthrough]];
                        case ENOENT:
                            // A component of pathname does not exist
                            [[fallthrough]];
                        default:
                            // Anything else ..
                            mode_ |= fmode_t::not_existing;
                            break;
                    }
                    goto errorout;
                }
            }
            if( 0 < link_path.size() && c_slash == link_path[0] ) {
                link_target_ = std::make_shared<file_stats>(ctor_cookie(cc.rec_level+1), dirfd, dir_item( link_path ), false /* dirfd_is_item_dirname */); // absolute link_path
            } else {
                link_target_ = std::make_shared<file_stats>(ctor_cookie(cc.rec_level+1), dirfd, dir_item( jau::fs::dirname(full_path), link_path ), dirfd_is_item_dirname );
            }
            if( link_target_->has_fd() ) {
                has_fields_ |= field_t::fd;
                fd_ = link_target_->fd();
            }
            if( link_target_->is_socket() ) {
                mode_ |= fmode_t::sock;
            } else if( link_target_->is_block() ) {
                mode_ |= fmode_t::blk;
            } else if( link_target_->is_char() ) {
                mode_ |= fmode_t::chr;
            } else if( link_target_->is_fifo() ) {
                mode_ |= fmode_t::fifo;
            } else if( link_target_->is_dir() ) {
                mode_ |= fmode_t::dir;
            } else if( link_target_->is_file() ) {
                mode_ |= fmode_t::file;
                has_fields_ |= field_t::size;
                size_ = link_target_->size();
            } else if( !link_target_->exists() ) {
                mode_ |= fmode_t::not_existing;
            } else if( !link_target_->has_access() ) {
                mode_ |= fmode_t::no_access;
            }
        }
        if constexpr ( _debug ) {
            jau::fprintf_td(stderr, "file_stats(%d): '%s', %d, errno %d (%s)\n", (int)cc.rec_level, to_string().c_str(), stat_res, errno, ::strerror(errno));
        }
    }
#endif /* _USE_STATX_ */

    errorout: ;
}

file_stats::file_stats(const dir_item& item) noexcept
: file_stats(ctor_cookie(0), AT_FDCWD, item, false /* dirfd_is_item_dirname */)
{}

file_stats::file_stats(const int dirfd, const dir_item& item, const bool dirfd_is_item_dirname) noexcept
: file_stats(ctor_cookie(0), dirfd, item, dirfd_is_item_dirname)
{}

file_stats::file_stats(const std::string& _path) noexcept
: file_stats(ctor_cookie(0), AT_FDCWD, dir_item(_path), false /* dirfd_is_item_dirname */)
{}

file_stats::file_stats(const int dirfd, const std::string& _path) noexcept
: file_stats(ctor_cookie(0), dirfd, dir_item(_path), false /* dirfd_is_item_dirname */)
{}

file_stats::file_stats(const int fd) noexcept
: file_stats(ctor_cookie(0), fd, dir_item(), false /* dirfd_is_item_dirname */)
{}

const file_stats* file_stats::final_target(size_t* link_count) const noexcept {
    size_t count = 0;
    const file_stats* fs0 = this;
    const file_stats* fs1 = fs0->link_target().get();
    while( nullptr != fs1 ) {
        ++count;
        fs0 = fs1;
        fs1 = fs0->link_target().get();
    }
    if( nullptr != link_count ) {
        *link_count = count;
    }
    return fs0;
}

bool file_stats::has(const field_t fields) const noexcept {
    return fields == ( has_fields_ & fields );
}

bool file_stats::operator ==(const file_stats& rhs) const noexcept {
    if( this == &rhs ) {
        return true;
    }
    return item_ == rhs.item_ &&
           has_fields_ == rhs.has_fields_ &&
           mode_ == rhs.mode_ &&
           uid_ == rhs.uid_ && gid_ == rhs.gid_ &&
           errno_res_ == rhs.errno_res_ &&
           size_ == rhs.size_ &&
           btime_ == rhs.btime_ &&
           atime_ == rhs.atime_ &&
           ctime_ == rhs.ctime_ &&
           mtime_ == rhs.mtime_ &&
           ( !is_link() ||
             ( link_target_path_ == rhs.link_target_path_&&
               link_target_ == rhs.link_target_
             )
           );
}

std::string file_stats::to_string() const noexcept {
    std::string stored_path, link_detail;
    {
        if( nullptr != link_target_path_ ) {
            stored_path = " [-> "+*link_target_path_+"]";
        }
        size_t link_count;
        const file_stats* final_target_ = final_target(&link_count);
        if( 0 < link_count ) {
            link_detail = " -(" + std::to_string(link_count) + ")-> '" + final_target_->path() + "'";
        }
    }
    std::string res( "file_stats[");
    res.append(jau::fs::to_string(mode_))
       .append(", '"+item_.path()+"'"+stored_path+link_detail );
    if( 0 == errno_res_ ) {
        if( has( field_t::fd ) ) {
            res.append( ", fd " ).append( std::to_string(fd_) );
        }
        if( has( field_t::uid ) ) {
            res.append( ", uid " ).append( std::to_string(uid_) );
        }
        if( has( field_t::gid ) ) {
            res.append( ", gid " ).append( std::to_string(gid_) );
        }
        if( has( field_t::size ) ) {
            res.append( ", size " ).append( jau::to_decstring( size_ ) );
        } else {
            res.append( ", size n/a" );
        }
        if( has( field_t::btime ) ) {
            res.append( ", btime " ).append( btime_.to_iso8601_string() );
        }
        if( has( field_t::atime ) ) {
            res.append( ", atime " ).append( atime_.to_iso8601_string() );
        }
        if( has( field_t::ctime ) ) {
            res.append( ", ctime " ).append( ctime_.to_iso8601_string() );
        }
        if( has( field_t::mtime ) ) {
            res.append( ", mtime " ).append( mtime_.to_iso8601_string() );
        }
        // res.append( ", fields ").append( jau::fs::to_string( has_fields_ ) );
    } else {
        res.append( ", errno " ).append( std::to_string(errno_res_) ).append( ", " ).append( std::string(::strerror(errno_res_)) );
    }
    res.append("]");
    return res;
}

bool jau::fs::mkdir(const std::string& path, const fmode_t mode, const bool verbose) noexcept {
    file_stats stats(path);

    if( stats.is_dir() ) {
        if( verbose ) {
            jau::fprintf_td(stderr, "mkdir: dir already exists: %s\n", stats.to_string().c_str());
        }
        return true;
    } else if( !stats.exists() ) {
        const int dir_err = ::mkdir(path.c_str(), posix_protection_bits(mode));
        if ( 0 != dir_err ) {
            ERR_PRINT("%s, failure", stats.to_string().c_str());
            return false;
        } else {
            return true;
        }
    } else {
        ERR_PRINT("%s, exists but is no dir", stats.to_string().c_str());
        return false;
    }
}

bool jau::fs::touch(const std::string& path, const jau::fraction_timespec& atime, const jau::fraction_timespec& mtime, const fmode_t mode) noexcept {
    int fd = ::open(path.c_str(), O_WRONLY|O_CREAT|O_NOCTTY|O_NONBLOCK, posix_protection_bits(mode));
    if( 0 > fd ) {
        ERR_PRINT("Couldn't open/create file '%s'", path.c_str());
        return false;
    }
    struct timespec ts2[2] = { atime.to_timespec(), mtime.to_timespec() };
    bool res;
    if( 0 != ::futimens(fd, ts2) ) {
        ERR_PRINT("Couldn't update time of file '%s'", path.c_str());
        res = false;
    } else {
        res = true;
    }
    ::close(fd);
    return res;
}

bool jau::fs::touch(const std::string& path, const fmode_t mode) noexcept {
    int fd = ::open(path.c_str(), O_WRONLY|O_CREAT|O_NOCTTY|O_NONBLOCK, posix_protection_bits(mode));
    if( 0 > fd ) {
        ERR_PRINT("Couldn't open/create file '%s'", path.c_str());
        return false;
    }
    bool res;
    if( 0 != ::futimens(fd, nullptr /* current time */) ) {
        ERR_PRINT("Couldn't update time of file '%s'", path.c_str());
        res = false;
    } else {
        res = true;
    }
    ::close(fd);
    return res;
}

bool jau::fs::get_dir_content(const std::string& path, const consume_dir_item& digest) noexcept {
    DIR *dir;
    struct dirent *ent;

    if( ( dir = ::opendir( path.c_str() ) ) != nullptr ) {
        while ( ( ent = ::readdir( dir ) ) != nullptr ) {
            std::string fname( ent->d_name );
            if( s_dot != fname && s_dotdot != fname ) { // avoid '.' and '..'
                digest( dir_item( path, fname ) );
            }
        }
        ::closedir (dir);
        return true;
    } else {
        return false;
    }
}

bool jau::fs::get_dir_content(const int dirfd, const std::string& path, const consume_dir_item& digest) noexcept {
    DIR *dir;
    struct dirent *ent;
    int dirfd2 = ::dup(dirfd);
    if( 0 > dirfd2 ) {
        ERR_PRINT("Couldn't duplicate given dirfd %d for path '%s'", dirfd, path.c_str());
        return false;
    }
    if( ( dir = ::fdopendir( dirfd2 ) ) != nullptr ) {
        while ( ( ent = ::readdir( dir ) ) != nullptr ) {
            std::string fname( ent->d_name );
            if( s_dot != fname && s_dotdot != fname ) { // avoid '.' and '..'
                digest( dir_item( path, fname ) );
            }
        }
        ::closedir (dir);
        return true;
    } else {
        return false;
    }
}

#define TRAVERSEEVENT_ENUM(X,M) \
    X(traverse_event,symlink,M) \
    X(traverse_event,file,M) \
    X(traverse_event,dir_check_entry,M) \
    X(traverse_event,dir_entry,M) \
    X(traverse_event,dir_exit,M) \
    X(traverse_event,dir_symlink,M)

std::string jau::fs::to_string(const traverse_event mask) noexcept {
    std::string out("[");
    bool comma = false;
    TRAVERSEEVENT_ENUM(APPEND_BITSTR,mask)
    out.append("]");
    return out;
}


#define TRAVERSEOPTIONS_ENUM(X,M) \
    X(traverse_options,recursive,M) \
    X(traverse_options,follow_symlinks,M) \
    X(traverse_options,lexicographical_order,M) \
    X(traverse_options,dir_check_entry,M) \
    X(traverse_options,dir_entry,M) \
    X(traverse_options,dir_exit,M)

std::string jau::fs::to_string(const traverse_options mask) noexcept {
    std::string out("[");
    bool comma = false;
    TRAVERSEOPTIONS_ENUM(APPEND_BITSTR,mask)
    out.append("]");
    return out;
}

static bool _dir_item_basename_compare(const dir_item& a, const dir_item& b) {
    return a.basename() < b.basename();
}

static bool _visit(const file_stats& item_stats, const traverse_options topts, const path_visitor& visitor, std::vector<int>& dirfds) noexcept {
    const size_t depth = dirfds.size();
    if( item_stats.is_dir() ) {
        if( item_stats.is_link() && !is_set(topts, traverse_options::follow_symlinks) ) {
            return visitor( traverse_event::dir_symlink, item_stats, depth );
        }
        if( !is_set(topts, traverse_options::recursive) ) {
            return visitor( traverse_event::dir_non_recursive, item_stats, depth );
        }
        if( dirfds.size() < 1 ) {
            ERR_PRINT("dirfd stack error: count %zu] @ %s", dirfds.size(), item_stats.to_string().c_str());
            return false;
        }
        const int parent_dirfd = dirfds.back();
        const int this_dirfd = __posix_openat64(parent_dirfd, item_stats.item().basename().c_str(), _open_dir_flags);
        if ( 0 > this_dirfd ) {
            ERR_PRINT("entered path dir couldn't be opened, source %s", item_stats.to_string().c_str());
            return false;
        }
        dirfds.push_back(this_dirfd);

        if( is_set(topts, traverse_options::dir_check_entry) ) {
            if( !visitor( traverse_event::dir_check_entry, item_stats, depth ) ) {
                ::close(this_dirfd);
                dirfds.pop_back();
                return true; // keep traversing in parent, but skip this directory
            }
        }
        if( is_set(topts, traverse_options::dir_entry) ) {
            if( !visitor( traverse_event::dir_entry, item_stats, depth ) ) {
                ::close(this_dirfd);
                dirfds.pop_back();
                return false;
            }
        }
        std::vector<dir_item> content;
        const consume_dir_item cs = jau::bind_capref<void, std::vector<dir_item>, const dir_item&>(&content,
                ( void(*)(std::vector<dir_item>*, const dir_item&) ) /* help template type deduction of function-ptr */
                    ( [](std::vector<dir_item>* receiver, const dir_item& item) -> void { receiver->push_back( item ); } )
            );
        if( get_dir_content(this_dirfd, item_stats.path(), cs) && content.size() > 0 ) {
            if( is_set(topts, traverse_options::lexicographical_order) ) {
                std::sort(content.begin(), content.end(), _dir_item_basename_compare);
            }
            for (const dir_item& element : content) {
                const file_stats element_stats( this_dirfd, element, true /* dirfd_is_item_dirname */ );
                if( element_stats.is_dir() ) { // an OK dir
                    if( element_stats.is_link() && !is_set(topts, traverse_options::follow_symlinks) ) {
                        if( !visitor( traverse_event::dir_symlink, element_stats, depth ) ) {
                            ::close(this_dirfd);
                            dirfds.pop_back();
                            return false;
                        }
                    } else if( !_visit(element_stats, topts, visitor, dirfds) ) { // recursive
                        ::close(this_dirfd);
                        dirfds.pop_back();
                        return false;
                    }
                } else if( !visitor( ( element_stats.is_file() ? traverse_event::file : traverse_event::none ) |
                                     ( element_stats.is_link() ? traverse_event::symlink : traverse_event::none),
                                     element_stats, depth ) )
                {
                    ::close(this_dirfd);
                    dirfds.pop_back();
                    return false;
                }
            }
        }
        if( dirfds.size() < 2 ) {
            ERR_PRINT("dirfd stack error: count %zu] @ %s", dirfds.size(), item_stats.to_string().c_str());
            return false;
        }
        bool res = true;
        if( is_set(topts, traverse_options::dir_exit) ) {
            res = visitor( traverse_event::dir_exit, item_stats, depth ); // keep traversing in parent
        }
        ::close(this_dirfd);
        dirfds.pop_back();
        return res;
    } // endif item_stats.is_dir()
    else if( item_stats.is_file() || !item_stats.ok() ) { // file or error-alike
        return visitor( ( item_stats.is_file() ? traverse_event::file : traverse_event::none ) |
                        ( item_stats.is_link() ? traverse_event::symlink : traverse_event::none),
                        item_stats, depth);
    }
    return true;
}

bool jau::fs::visit(const file_stats& item_stats, const traverse_options topts, const path_visitor& visitor, std::vector<int>* dirfds) noexcept {
    const bool user_dirfds = nullptr != dirfds;
    if( !user_dirfds ) {
        try {
            dirfds = new std::vector<int>();
        } catch (const std::bad_alloc &e) {
            ABORT("Error: bad_alloc: dirfds allocation failed");
            return false; // unreachable
        }
    }
    if( 0 != dirfds->size() ) {
        ERR_PRINT("dirfd stack error: count %zu @ %s", dirfds->size(), item_stats.to_string().c_str());
        return false;
    }
    // initial parent directory dirfd of initial item_stats (a directory)
    const int dirfd = __posix_openat64(AT_FDCWD, item_stats.item().dirname().c_str(), _open_dir_flags);
    if ( 0 > dirfd ) {
        ERR_PRINT("path dirname couldn't be opened, source %s", item_stats.to_string().c_str());
        return false;
    }
    dirfds->push_back(dirfd);

    bool res = _visit(item_stats, topts, visitor, *dirfds);

    if( dirfds->size() != 1 && res ) {
        ERR_PRINT("dirfd stack error: count %zu", dirfds->size());
        res = false;
    }
    while( !dirfds->empty() ) {
        ::close(dirfds->back());
        dirfds->pop_back();
    }
    if( !user_dirfds ) {
        delete dirfds;
    }
    return res;
}

bool jau::fs::visit(const std::string& path, const traverse_options topts, const path_visitor& visitor, std::vector<int>* dirfds) noexcept {
    return jau::fs::visit(file_stats(path), topts, visitor, dirfds);
}

bool jau::fs::remove(const std::string& path, const traverse_options topts) noexcept {
    file_stats path_stats(path);
    if( is_set(topts, traverse_options::verbose) ) {
        jau::fprintf_td(stderr, "remove: '%s' -> %s\n", path.c_str(), path_stats.to_string().c_str());
    }
    if( !path_stats.exists() ) {
        if( is_set(topts, traverse_options::verbose) ) {
            jau::fprintf_td(stderr, "remove: failed: path doesn't exist: %s\n", path_stats.to_string().c_str());
        }
        return false;
    }
    if( path_stats.has_fd() ) {
        if( is_set(topts, traverse_options::verbose) ) {
            jau::fprintf_td(stderr, "remove: failed: path is fd: %s\n", path_stats.to_string().c_str());
        }
        return false;
    }
    if( path_stats.is_file() ||
        ( path_stats.is_dir() && path_stats.is_link() && !is_set(topts, traverse_options::follow_symlinks) )
      )
    {
        int res = ::unlink( path_stats.path().c_str() );
        if( 0 != res ) {
            ERR_PRINT("remove failed: %s, res %d", path_stats.to_string().c_str(), res);
            return false;
        }
        if( is_set(topts, traverse_options::verbose) ) {
            jau::fprintf_td(stderr, "removed: %s\n", path_stats.to_string().c_str());
        }
        return true;
    }
    if( !path_stats.is_dir() ) {
        ERR_PRINT("remove: Error: path is neither file nor dir: %s\n", path_stats.to_string().c_str());
        return false;
    }
    // directory ...
    if( !is_set(topts, traverse_options::recursive) ) {
        if( is_set(topts, traverse_options::verbose) ) {
            jau::fprintf_td(stderr, "remove: Error: path is dir but !recursive, %s\n", path_stats.to_string().c_str());
        }
        return false;
    }
    struct remove_context_t {
        traverse_options topts;
        std::vector<int> dirfds;
    };
    remove_context_t ctx = { topts | jau::fs::traverse_options::dir_exit, std::vector<int>() };

    const path_visitor pv = jau::bind_capref<bool, remove_context_t, traverse_event, const file_stats&, size_t>(&ctx,
            ( bool(*)(remove_context_t*, traverse_event, const file_stats&, size_t) ) /* help template type deduction of function-ptr */
                ( [](remove_context_t* ctx_ptr, traverse_event tevt, const file_stats& element_stats, size_t depth) -> bool {
                    (void)tevt;
                    (void)depth;

                    if( !element_stats.has_access() ) {
                        if( is_set(ctx_ptr->topts, traverse_options::verbose) ) {
                            jau::fprintf_td(stderr, "remove: Error: remove failed: no access, %s\n", element_stats.to_string().c_str());
                        }
                        return false;
                    }
                    const int dirfd = ctx_ptr->dirfds.back();
                    const std::string& basename_ = element_stats.item().basename();
                    if( is_set(tevt, traverse_event::dir_entry) ) {
                        // NOP
                    } else if( is_set(tevt, traverse_event::dir_exit) ) {
                        const int dirfd2 = *( ctx_ptr->dirfds.end() - 2 );
                        const int res = ::unlinkat( dirfd2, basename_.c_str(), AT_REMOVEDIR );
                        if( 0 != res ) {
                            ERR_PRINT("remove failed: %s, res %d", element_stats.to_string().c_str(), res);
                            return false;
                        }
                        if( is_set(ctx_ptr->topts, traverse_options::verbose) ) {
                            jau::fprintf_td(stderr, "remove: %s removed\n", element_stats.to_string().c_str());
                        }
                    } else if( is_set(tevt, traverse_event::file) || is_set(tevt, traverse_event::symlink) || is_set(tevt, traverse_event::dir_symlink) ) {
                        const int res = ::unlinkat( dirfd, basename_.c_str(), 0 );
                        if( 0 != res ) {
                            ERR_PRINT("remove failed: %s, res %d", element_stats.to_string().c_str(), res);
                            return false;
                        }
                        if( is_set(ctx_ptr->topts, traverse_options::verbose) ) {
                            jau::fprintf_td(stderr, "removed: %s\n", element_stats.to_string().c_str());
                        }
                    }
                    return true;
                  } ) );
    return jau::fs::visit(path_stats, ctx.topts, pv, &ctx.dirfds);
}

bool jau::fs::compare(const std::string& source1, const std::string& source2, const bool verbose) noexcept {
    const file_stats s1(source1);
    const file_stats s2(source2);
    return compare(s1, s2, verbose);
}

bool jau::fs::compare(const file_stats& source1, const file_stats& source2, const bool verbose) noexcept {
    if( !source1.is_file() ) {
        ERR_PRINT("source1_stats is not a file: %s", source1.to_string().c_str());
        return false;
    }
    if( !source2.is_file() ) {
        ERR_PRINT("source2_stats is not a file: %s", source2.to_string().c_str());
        return false;
    }

    if( source1.size() != source2.size() ) {
        if( verbose ) {
            jau::fprintf_td(stderr, "compare: Source files size mismatch, %s != %s\n",
                    source1.to_string().c_str(), source2.to_string().c_str());
        }
        return false;
    }

    int src1=-1, src2=-1;
    int src_flags = O_RDONLY|O_BINARY|O_NOCTTY;
    uint64_t offset = 0;

    bool res = false;
    src1 = __posix_openat64(AT_FDCWD, source1.path().c_str(), src_flags);
    if ( 0 > src1 ) {
        ERR_PRINT("Failed to open source1 %s, errno %d, %s", source1.to_string().c_str());
        goto errout;
    }
    src2 = __posix_openat64(AT_FDCWD, source2.path().c_str(), src_flags);
    if ( 0 > src2 ) {
        ERR_PRINT("Failed to open source2 %s, errno %d, %s", source2.to_string().c_str());
        goto errout;
    }
    while ( offset < source1.size()) {
        ssize_t rc1, rc2=0;
        char buffer1[BUFSIZ];
        char buffer2[BUFSIZ];

        if( ( rc1 = ::read(src1, buffer1, sizeof(buffer1)) ) > 0 ) {
            ssize_t bytes_to_write = rc1;
            size_t buffer_offset = 0;
            while( 0 <= rc2 && 0 < bytes_to_write ) { // src2-read required src1 size in chunks, allowing potential multiple read-ops to match src1 size
                while( ( rc2 = ::read(src2, buffer2+buffer_offset, bytes_to_write) ) < 0 ) {
                    if ( errno == EAGAIN || errno == EINTR ) {
                        // cont temp unavail or interruption
                        continue;
                    }
                    break; // error, exist inner (break) and outter loop (rc2<0)
                }
                buffer_offset += rc2;
                bytes_to_write -= rc2;
                offset += (uint64_t)rc2;
            }
        } else if ( 0 > rc1 && ( errno == EAGAIN || errno == EINTR ) ) {
            // cont temp unavail or interruption
            continue;
        }
        if ( 0 > rc1 || 0 > rc2 ) {
            if ( 0 > rc1 ) {
                ERR_PRINT("Failed to read source1 bytes @ %s / %s, %s",
                        jau::to_decstring(offset).c_str(), jau::to_decstring(source1.size()).c_str(),
                        source1.to_string().c_str());
            } else if ( 0 > rc2 ) {
                ERR_PRINT("Failed to read source2 bytes @ %s / %s, %s",
                        jau::to_decstring(offset).c_str(), jau::to_decstring(source2.size()).c_str(),
                        source2.to_string().c_str());
            }
            goto errout;
        }
        if( 0 != ::memcmp(buffer1, buffer2, rc1) ) {
            if( verbose ) {
                jau::fprintf_td(stderr, "compare: Difference within %s bytes @ %s / %s, %s != %s\n",
                        jau::to_decstring(rc1).c_str(), jau::to_decstring(offset-rc1).c_str(), jau::to_decstring(source1.size()).c_str(),
                        source1.to_string().c_str(), source2.to_string().c_str());
            }
            goto errout;
        }
        if ( 0 == rc1 ) {
            break;
        }
    }
    if( offset < source1.size() ) {
        ERR_PRINT("Incomplete transfer %s / %s, %s != %s\n",
                jau::to_decstring(offset).c_str(), jau::to_decstring(source1.size()).c_str(),
                source1.to_string().c_str(), source2.to_string().c_str());
        goto errout;
    }
    res = true;
errout:
    if( 0 <= src1 ) {
        ::close(src1);
    }
    if( 0 <= src2 ) {
        ::close(src2);
    }
    return res;
}

#define COPYOPTIONS_BIT_ENUM(X,M) \
    X(copy_options,recursive,M) \
    X(copy_options,follow_symlinks,M) \
    X(copy_options,into_existing_dir,M) \
    X(copy_options,ignore_symlink_errors,M) \
    X(copy_options,overwrite,M) \
    X(copy_options,preserve_all,M) \
    X(copy_options,sync,M)

std::string jau::fs::to_string(const copy_options mask) noexcept {
    std::string out("[");
    bool comma = false;
    COPYOPTIONS_BIT_ENUM(APPEND_BITSTR,mask)
    out.append("]");
    return out;
}

struct copy_context_t {
    copy_options copts;
    int skip_dst_dir_mkdir;
    std::vector<int> src_dirfds;
    std::vector<int> dst_dirfds;
};

static bool copy_file(const int src_dirfd, const file_stats& src_stats,
                      const int dst_dirfd, const std::string& dst_basename, const copy_options copts) noexcept {
    file_stats dst_stats(dst_dirfd, dst_basename);

    // overwrite: remove pre-existing file, if copy_options::overwrite set
    if( dst_stats.is_file() ) {
        if( !is_set(copts, copy_options::overwrite) ) {
            if( is_set(copts, copy_options::verbose) ) {
                jau::fprintf_td(stderr, "copy: Error: dest_path exists but copy_options::overwrite not set: source %s, dest '%s', copts %s\n",
                        src_stats.to_string().c_str(), dst_stats.to_string().c_str(), to_string( copts ).c_str());
            }
            return false;
        }
        const int res = ::unlinkat(dst_dirfd, dst_basename.c_str(), 0);
        if( 0 != res ) {
            ERR_PRINT("remove existing dest_path for symbolic-link failed: source %s, dest '%s'",
                    src_stats.to_string().c_str(), dst_stats.to_string().c_str());
            return false;
        }
    }

    // copy as symbolic link
    if( src_stats.is_link() && !is_set(copts, copy_options::follow_symlinks) ) {
        const std::shared_ptr<std::string>& link_target_path = src_stats.link_target_path();
        if( nullptr == link_target_path || 0 == link_target_path->size() ) {
            ERR_PRINT("Symbolic link-path is empty %s", src_stats.to_string().c_str());
            return false;
        }
        // symlink
        const int res = ::symlinkat(link_target_path->c_str(), dst_dirfd, dst_basename.c_str());
        if( 0 > res ) {
            if( EPERM == errno && is_set(copts, copy_options::ignore_symlink_errors ) ) {
                if( is_set(copts, copy_options::verbose) ) {
                    jau::fprintf_td(stderr, "copy: Ignored: Failed to create symink %s -> %s, %s, errno %d, %s\n",
                            dst_basename.c_str(), link_target_path->c_str(), src_stats.to_string().c_str(), errno, ::strerror(errno));
                }
                return true;
            }
            ERR_PRINT("Creating symlink failed %s -> %s, %s", dst_basename.c_str(), link_target_path->c_str(), src_stats.to_string().c_str());
            return false;
        }
        if( is_set(copts, copy_options::preserve_all) ) {
            // preserve time
            struct timespec ts2[2] = { src_stats.atime().to_timespec(), src_stats.mtime().to_timespec() };
            if( 0 != ::utimensat(dst_dirfd, dst_basename.c_str(), ts2, AT_SYMLINK_NOFOLLOW) ) {
                ERR_PRINT("Couldn't preserve time of symlink, source %s, dest '%s'", src_stats.to_string().c_str(), dst_basename.c_str());
                return false;
            }
            // preserve ownership
            const uid_t caller_uid = ::geteuid();
            const ::uid_t source_uid = 0 == caller_uid ? src_stats.uid() : -1;
            if( 0 != ::fchownat(dst_dirfd, dst_basename.c_str(), source_uid, src_stats.gid(), AT_SYMLINK_NOFOLLOW) ) {
                if( errno != EPERM && errno != EINVAL ) {
                    ERR_PRINT("Couldn't preserve ownership of symlink, source %s, dest '%s'", src_stats.to_string().c_str(), dst_basename.c_str());
                    return false;
                }
                // OK to fail due to permissions
                if( is_set(copts, copy_options::verbose) ) {
                    jau::fprintf_td(stderr, "copy: Warn: Couldn't preserve ownership of symlink, source %s, dest '%s', errno %d (%s)\n",
                            src_stats.to_string().c_str(), dst_basename.c_str(), errno, ::strerror(errno));
                }
            }
        }
        return true;
    }
    // copy actual file bytes
    const file_stats* target_stats = src_stats.final_target(); // follows symlinks up to definite item
    const fmode_t dest_mode = target_stats->prot_mode();
    const fmode_t omitted_permissions = dest_mode & ( fmode_t::rwx_grp | fmode_t::rwx_oth );

    const uid_t caller_uid = ::geteuid();
    int src=-1, dst=-1;
    int src_flags = O_RDONLY|O_BINARY|O_NOCTTY;
    uint64_t offset = 0;

    bool res = false;
#if defined(__linux__)
    if( caller_uid == target_stats->uid() ) {
        src_flags |= O_NOATIME;
    } // else we are not allowed to not use O_NOATIME
#endif /* defined(__linux__) */
    src = __posix_openat64(src_dirfd, src_stats.item().basename().c_str(), src_flags);
    if ( 0 > src ) {
        if( src_stats.is_link() ) {
            res = is_set(copts, copy_options::ignore_symlink_errors);
        }
        if( !res ) {
            ERR_PRINT("Failed to open source %s", src_stats.to_string().c_str());
        } else if( is_set(copts, copy_options::verbose) ) {
            jau::fprintf_td(stderr, "copy: Ignored: Failed to open source %s, errno %d, %s\n", src_stats.to_string().c_str(), errno, ::strerror(errno));
        }
        goto errout;
    }
    dst = __posix_openat64 (dst_dirfd, dst_basename.c_str(), O_CREAT|O_EXCL|O_WRONLY|O_BINARY|O_NOCTTY, jau::fs::posix_protection_bits( dest_mode & ~omitted_permissions ) );
    if ( 0 > dst ) {
        ERR_PRINT("Failed to open target_path '%s'", dst_basename.c_str());
        goto errout;
    }
    while ( offset < src_stats.size()) {
        ssize_t rc1, rc2=0;
#ifdef _USE_SENDFILE_
        off64_t offset_i = (off64_t)offset; // we drop 1 bit of value-range as off64_t is int64_t
        const uint64_t count = std::max<uint64_t>(std::numeric_limits<ssize_t>::max(), src_stats.size() - offset);
        if( ( rc1 = ::sendfile64(dst, src, &offset_i, (size_t)count) ) >= 0 ) {
            offset = (uint64_t)offset_i;
        }
#else /* _USE_SENDFILE_ */
        char buffer[BUFSIZ];
        if( ( rc1 = ::read(src, buffer, sizeof(buffer)) ) > 0 ) {
            ssize_t bytes_to_write = rc1;
            size_t buffer_offset = 0;
            while( 0 <= rc2 && 0 < bytes_to_write ) { // write the read chunk, allowing potential multiple write-ops
                while( ( rc2 = ::write(dst, buffer+buffer_offset, bytes_to_write) ) < 0 ) {
                    if ( errno == EAGAIN || errno == EINTR ) {
                        // cont temp unavail or interruption
                        continue;
                    }
                    break; // error, exist inner (break) and outter loop (rc2<0)
                }
                buffer_offset += rc2;
                bytes_to_write -= rc2;
                offset += rc2;
            }
        } else if ( 0 > rc1 && ( errno == EAGAIN || errno == EINTR ) ) {
            // cont temp unavail or interruption
            continue;
        }
#endif/* _USE_SENDFILE_ */
        if ( 0 > rc1 || 0 > rc2 ) {
#ifdef _USE_SENDFILE_
            ERR_PRINT("Failed to copy bytes @ %s / %s, %s -> '%s'",
            jau::to_decstring(offset).c_str(), jau::to_decstring(src_stats.size()).c_str(),
            src_stats.to_string().c_str(), dst_basename.c_str());
#else /* _USE_SENDFILE_ */
            if ( 0 > rc1 ) {
                ERR_PRINT("Failed to read bytes @ %s / %s, %s",
                        jau::to_decstring(offset).c_str(), jau::to_decstring(src_stats.size()).c_str(),
                        src_stats.to_string().c_str());
            } else if ( 0 > rc2 ) {
                ERR_PRINT("Failed to write bytes @ %s / %s, %s",
                        jau::to_decstring(offset).c_str(), jau::to_decstring(src_stats.size()).c_str(),
                        dst_basename.c_str());
            }
#endif/* _USE_SENDFILE_ */
            goto errout;
        }
        if ( 0 == rc1 ) {
            break;
        }
    }
    if( offset < src_stats.size() ) {
        ERR_PRINT("Incomplete transfer %s / %s, %s -> '%s'",
                jau::to_decstring(offset).c_str(), jau::to_decstring(src_stats.size()).c_str(),
                src_stats.to_string().c_str(), dst_basename.c_str());
        goto errout;
    }
    res = true;
    if( omitted_permissions != fmode_t::none ) {
        // restore omitted permissions
        if( 0 != ::fchmod(dst, jau::fs::posix_protection_bits( dest_mode  )) ) {
            ERR_PRINT("Couldn't restore omitted permissions, source %s, dest '%s'",
                    src_stats.to_string().c_str(), dst_basename.c_str());
            res = false;
        }
    }
    if( is_set(copts, copy_options::preserve_all) ) {
        // preserve time
        struct timespec ts2[2] = { target_stats->atime().to_timespec(), target_stats->mtime().to_timespec() };
        if( 0 != ::futimens(dst, ts2) ) {
            ERR_PRINT("Couldn't preserve time of file, source %s, dest '%s'",
                    src_stats.to_string().c_str(), dst_basename.c_str());
            res = false;
        }
        // preserve ownership
        ::uid_t source_uid = 0 == caller_uid ? target_stats->uid() : -1;
        if( 0 != ::fchown(dst, source_uid, target_stats->gid()) ) {
            if( errno != EPERM && errno != EINVAL ) {
                ERR_PRINT("Couldn't preserve ownership of file, uid(caller %" PRIu32 ", chown %" PRIu32 "), source %s, dest '%s'",
                        caller_uid, source_uid, src_stats.to_string().c_str(), dst_basename.c_str());
                res = false;
            } else {
                // OK to fail due to permissions
                if( is_set(copts, copy_options::verbose) ) {
                    jau::fprintf_td(stderr, "copy: Ignored: Preserve ownership of file failed, uid(caller %" PRIu32 ", chown %" PRIu32 "), source %s, dest '%s', errno %d (%s)\n",
                            caller_uid, source_uid, src_stats.to_string().c_str(), dst_stats.to_string().c_str(), errno, ::strerror(errno));
                }
            }
        }
    }
    if( is_set(copts, copy_options::sync) ) {
        if( 0 != ::fsync(dst) ) {
            ERR_PRINT("Couldn't synchronize destination file, source %s, dest '%s'",
                    src_stats.to_string().c_str(), dst_basename.c_str());
            res = false;
        }
    }
errout:
    if( 0 <= src ) {
        ::close(src);
    }
    if( 0 <= dst ) {
        ::close(dst);
    }
    return res;
}

static bool copy_push_mkdir(const file_stats& dst_stats, copy_context_t& ctx) noexcept
{
    // atomically, using unpredictable '.'+rand temp dir (if target dir non-existent)
    // and drops read-permissions for user and all of group and others after fetching its dirfd.
    bool new_dir = false;
    std::string basename_;
    const int dest_dirfd = ctx.dst_dirfds.back();
    if( dst_stats.is_dir() ) {
        if( is_set(ctx.copts, copy_options::verbose) ) {
            jau::fprintf_td(stderr, "copy: mkdir directory already exist: %s\n", dst_stats.to_string().c_str());
        }
        basename_.append( dst_stats.item().basename() );
    } else if( !dst_stats.exists() ) {
        new_dir = true;
        constexpr const int32_t val_min = 888;
        constexpr const int32_t val_max = std::numeric_limits<int32_t>::max(); // 6 digits base 38 > INT_MAX
        uint64_t mkdir_cntr = 0;
        std::mt19937_64 prng;
        std::uniform_int_distribution<int32_t> prng_dist(val_min, val_max);
        bool mkdir_ok = false;
        do {
            ++mkdir_cntr;
            const int32_t val_d = prng_dist(prng);
            basename_.clear();
            basename_.append(".").append( jau::codec::base::encode(val_d, jau::codec::base::ascii38_alphabet(), 6) ); // base 38, 6 digits
            if( 0 == ::mkdirat(dest_dirfd, basename_.c_str(), jau::fs::posix_protection_bits(fmode_t::rwx_usr)) ) {
                mkdir_ok = true;
            } else if (errno != EINTR && errno != EEXIST) {
                ERR_PRINT("mkdir failed: %s, temp '%s'", dst_stats.to_string().c_str(), basename_.c_str());
                return false;
            } // else continue on EINTR or EEXIST
        } while( !mkdir_ok && mkdir_cntr < val_max );
        if( !mkdir_ok ) {
            ERR_PRINT("mkdir failed: %s", dst_stats.to_string().c_str());
            return false;
        }
    } else {
        ERR_PRINT("mkdir failed: %s, exists but is no dir", dst_stats.to_string().c_str());
        return false;
    }
    // open dirfd
    const int new_dirfd = __posix_openat64(dest_dirfd, basename_.c_str(), _open_dir_flags);
    if ( 0 > new_dirfd ) {
        if( new_dir ) {
            ERR_PRINT("Couldn't open new dir %s, temp '%s'", dst_stats.to_string().c_str(), basename_.c_str());
        } else {
            ERR_PRINT("Couldn't open new dir %s", dst_stats.to_string().c_str());
        }
        if( new_dir ) {
            ::unlinkat(dest_dirfd, basename_.c_str(), AT_REMOVEDIR);
        }
        return false;
    }
    // drop read permissions to render destination directory transaction safe until copy is done
    if( 0 != ::fchmod(new_dirfd, jau::fs::posix_protection_bits(fmode_t::write_usr | fmode_t::exec_usr)) ) {
        if( new_dir ) {
            ::unlinkat(dest_dirfd, basename_.c_str(), AT_REMOVEDIR);
            ERR_PRINT("zero permissions on dest %s, temp '%s'", dst_stats.to_string().c_str(), basename_.c_str());
        } else {
            ERR_PRINT("zero permissions on dest %s", dst_stats.to_string().c_str());
        }
        ::close(new_dirfd);
        return false;
    }
    if( new_dir ) {
#if defined(__linux__) && defined(__GLIBC__)
        const int rename_flags = 0; // Not supported on all fs: RENAME_NOREPLACE
        const int rename_res = ::renameat2(dest_dirfd, basename_.c_str(), dest_dirfd, dst_stats.item().basename().c_str(), rename_flags);
#else /* defined(__linux__) && defined(__GLIBC__) */
        const int rename_res = ::renameat(dest_dirfd, basename_.c_str(), dest_dirfd, dst_stats.item().basename().c_str());
#endif /* defined(__linux__) && defined(__GLIBC__) */
        if( 0 != rename_res ) {
            ERR_PRINT("rename temp to dest, temp '%s', dest %s", basename_.c_str(), dst_stats.to_string().c_str());
            ::unlinkat(dest_dirfd, basename_.c_str(), AT_REMOVEDIR);
            ::close(new_dirfd);
            return false;
        }
    }
    ctx.dst_dirfds.push_back(new_dirfd);
    return true;
}

static bool copy_dir_preserve(const file_stats& src_stats, const int dst_dirfd, const std::string& dst_basename, const copy_options copts) noexcept {
    const file_stats* target_stats = src_stats.is_link() ? src_stats.link_target().get() : &src_stats;

    // restore permissions
    const fmode_t dest_mode = target_stats->prot_mode();
    if( 0 != ::fchmod(dst_dirfd, jau::fs::posix_protection_bits( dest_mode  )) ) {
        ERR_PRINT("restore permissions, source %s, dest '%s'", src_stats.to_string().c_str(), dst_basename.c_str());
        return false;
    }

    if( is_set(copts, copy_options::preserve_all) ) {
        // preserve time
        struct timespec ts2[2] = { target_stats->atime().to_timespec(), target_stats->mtime().to_timespec() };
        if( 0 != ::futimens(dst_dirfd, ts2) ) {
            ERR_PRINT("preserve time of file failed, source %s, dest '%s'", src_stats.to_string().c_str(), dst_basename.c_str());
            return false;
        }
        // preserve ownership
        const uid_t caller_uid = ::geteuid();
        const ::uid_t source_uid = 0 == caller_uid ? target_stats->uid() : -1;
        if( 0 != ::fchown(dst_dirfd, source_uid, target_stats->gid()) ) {
            if( errno != EPERM && errno != EINVAL ) {
                ERR_PRINT("dir_preserve ownership of file failed, uid(caller %" PRIu32 ", chown %" PRIu32 "), source %s, dest '%s'",
                        caller_uid, source_uid, src_stats.to_string().c_str(), dst_basename.c_str());
                return false;
            }
            // OK to fail due to permissions
            if( is_set(copts, copy_options::verbose) ) {
                jau::fprintf_td(stderr, "copy: Ignored: dir_preserve ownership of file failed, uid(caller %" PRIu32 ", chown %" PRIu32 "), source %s, dest '%s', errno %d (%s)\n",
                        caller_uid, source_uid, src_stats.to_string().c_str(), dst_basename.c_str(), errno, ::strerror(errno));
            }
        }
    }
    if( is_set(copts, copy_options::sync) ) {
        if( 0 != ::fsync(dst_dirfd) ) {
            ERR_PRINT("Couldn't synchronize destination file '%s'", dst_basename.c_str());
            return false;
        }
    }
    return true;
}

bool jau::fs::copy(const std::string& source_path, const std::string& target_path, const copy_options copts) noexcept {
    traverse_options topts = traverse_options::dir_entry | traverse_options::dir_exit;
    if( is_set(copts, copy_options::recursive) ) {
        topts |= traverse_options::recursive;
    }
    if( is_set(copts, copy_options::follow_symlinks) ) {
        topts |= traverse_options::follow_symlinks;
    }
    if( is_set(copts, copy_options::verbose) ) {
        topts |= traverse_options::verbose;
    }
    file_stats source_stats(source_path);
    file_stats target_stats(target_path);

    if( source_stats.is_file() ) {
        //
        // single file copy
        //
        if( target_stats.exists() ) {
            if( target_stats.is_file() ) {
                if( !is_set(copts, copy_options::overwrite)) {
                    if( is_set(copts, copy_options::verbose) ) {
                        jau::fprintf_td(stderr, "copy: Error: source_path is file, target_path existing file w/o overwrite, source %s, target %s\n",
                                source_stats.to_string().c_str(), target_stats.to_string().c_str());
                    }
                    return false;
                }
            } // else file2file to directory
        } // else file2file to explicit new file
        const int src_dirfd = __posix_openat64(AT_FDCWD, source_stats.item().dirname().c_str(), _open_dir_flags);
        if ( 0 > src_dirfd ) {
            ERR_PRINT("source_path dir couldn't be opened, source %s", source_stats.to_string().c_str());
            return false;
        }

        std::string dst_basename;
        int dst_dirfd = -1;
        if( target_stats.is_dir() ) {
            // file2file to directory
            dst_dirfd = __posix_openat64(AT_FDCWD, target_stats.path().c_str(), _open_dir_flags);
            if ( 0 > dst_dirfd ) {
                ERR_PRINT("target dir couldn't be opened, target %s", target_stats.to_string().c_str());
                ::close(src_dirfd);
                return false;
            }
            dst_basename = source_stats.item().basename();
        } else {
            // file2file to file
            file_stats target_parent_stats(target_stats.item().dirname());
            if( !target_parent_stats.is_dir() ) {
                if( is_set(copts, copy_options::verbose) ) {
                    jau::fprintf_td(stderr, "copy: Error: target parent is not an existing directory, target %s, target_parent %s\n",
                            target_stats.to_string().c_str(), target_parent_stats.to_string().c_str());
                }
                ::close(src_dirfd);
                return false;
            }
            dst_dirfd = __posix_openat64(AT_FDCWD, target_parent_stats.path().c_str(), _open_dir_flags);
            if ( 0 > dst_dirfd ) {
                ERR_PRINT("target_parent dir couldn't be opened, target %s, target_parent %s",
                        target_stats.to_string().c_str(), target_parent_stats.to_string().c_str());
                ::close(src_dirfd);
                return false;
            }
            dst_basename = target_stats.item().basename();
        }
        if( !copy_file(src_dirfd, source_stats, dst_dirfd, dst_basename, copts) ) {
            return false;
        }
        ::close(src_dirfd);
        ::close(dst_dirfd);
        return true;
    }
    if( !source_stats.is_dir() ) {
        if( is_set(copts, copy_options::verbose) ) {
            jau::fprintf_td(stderr, "copy: Error: source_path is neither file nor dir, source %s, target %s\n",
                    source_stats.to_string().c_str(), target_stats.to_string().c_str());
        }
        return false;
    }

    //
    // directory copy
    //
    copy_context_t ctx { copts, 0, std::vector<int>(), std::vector<int>() };

    if( !is_set(copts, copy_options::recursive) ) {
        if( is_set(copts, copy_options::verbose) ) {
            jau::fprintf_td(stderr, "copy: Error: source_path is dir but !recursive, %s\n", source_stats.to_string().c_str());
        }
        return false;
    }
    if( target_stats.exists() && !target_stats.is_dir() ) {
        if( is_set(copts, copy_options::verbose) ) {
            jau::fprintf_td(stderr, "copy: Error: source_path is dir but target_path exist and is no dir, source %s, target %s\n",
                    source_stats.to_string().c_str(), target_stats.to_string().c_str());
        }
        return false;
    }
    // src_dirfd of 'source_stats.item().dirname().c_str()' will be pushed by visit() itself
    if( target_stats.is_dir() && !is_set(copts, copy_options::into_existing_dir) ) {
        // Case: If dest_path exists as a directory, source_path dir will be copied below the dest_path directory
        //       _if_ copy_options::into_existing_dir is not set. Otherwise its content is copied into the existing dest_path.
        const int dst_dirfd = __posix_openat64(AT_FDCWD, target_stats.path().c_str(), _open_dir_flags);
        if ( 0 > dst_dirfd ) {
            ERR_PRINT("target dir couldn't be opened, target %s", target_stats.to_string().c_str());
            return false;
        }
        ctx.dst_dirfds.push_back(dst_dirfd);
    } else {
        // Case: If dest_path doesn't exist, source_path dir content is copied into the newly created dest_path.
        // - OR - dest_path does exist and copy_options::into_existing_dir is set
        file_stats target_parent_stats(target_stats.item().dirname());
        if( !target_parent_stats.is_dir() ) {
            if( is_set(copts, copy_options::verbose) ) {
                jau::fprintf_td(stderr, "copy: Error: target parent is not an existing directory, target %s, target_parent %s\n",
                        target_stats.to_string().c_str(), target_parent_stats.to_string().c_str());
            }
            return false;
        }
        const int dst_parent_dirfd = __posix_openat64(AT_FDCWD, target_parent_stats.path().c_str(), _open_dir_flags);
        if ( 0 > dst_parent_dirfd ) {
            ERR_PRINT("target dirname couldn't be opened, target %s, target_parent %s",
                    target_stats.to_string().c_str(), target_parent_stats.to_string().c_str());
            return false;
        }
        ctx.dst_dirfds.push_back(dst_parent_dirfd);

        if( target_stats.is_dir() ) {
            // Case: copy_options::into_existing_dir
            const int dst_dirfd = __posix_openat64(AT_FDCWD, target_stats.path().c_str(), _open_dir_flags);
            if ( 0 > dst_dirfd ) {
                ERR_PRINT("target dir couldn't be opened, target %s", target_stats.to_string().c_str());
                return false;
            }
            ctx.dst_dirfds.push_back(dst_dirfd);
        } else {
            if( !copy_push_mkdir(target_stats, ctx) ) {
                return false;
            }
        }
        ctx.skip_dst_dir_mkdir = 1;
    }
    const path_visitor pv = jau::bind_capref<bool, copy_context_t, traverse_event, const file_stats&, size_t>(&ctx,
            ( bool(*)(copy_context_t*, traverse_event, const file_stats&, size_t) ) /* help template type deduction of function-ptr */
                ( [](copy_context_t* ctx_ptr, traverse_event tevt, const file_stats& element_stats, size_t depth) -> bool {
                    (void)depth;
                    if( !element_stats.has_access() ) {
                        if( is_set(ctx_ptr->copts, copy_options::verbose) ) {
                            jau::fprintf_td(stderr, "copy: Error: remove failed: no access, %s\n", element_stats.to_string().c_str());
                        }
                        return false;
                    }
                    if( ctx_ptr->dst_dirfds.size() < 1 ) {
                        ERR_PRINT("dirfd stack error: count[src %zu, dst %zu, dst_skip %d] @ %s",
                                ctx_ptr->src_dirfds.size(), ctx_ptr->dst_dirfds.size(), ctx_ptr->skip_dst_dir_mkdir, element_stats.to_string().c_str());
                        return false;
                    }
                    const int src_dirfd = ctx_ptr->src_dirfds.back();
                    const int dst_dirfd = ctx_ptr->dst_dirfds.back();
                    const std::string& basename_ = element_stats.item().basename();
                    if( is_set(tevt, traverse_event::dir_entry) ) {
                        if( 0 < ctx_ptr->skip_dst_dir_mkdir ) {
                            --ctx_ptr->skip_dst_dir_mkdir;
                        } else {
                            const file_stats target_stats_(dst_dirfd, basename_);
                            if( !copy_push_mkdir(target_stats_, *ctx_ptr) ) {
                                return false;
                            }
                        }
                    } else if( is_set(tevt, traverse_event::dir_exit) ) {
                        if( ctx_ptr->dst_dirfds.size() < 2 ) {
                            ERR_PRINT("dirfd stack error: count[src %zu, dst %zu] @ %s",
                                    ctx_ptr->src_dirfds.size(), ctx_ptr->dst_dirfds.size(), element_stats.to_string().c_str());
                            return false;
                        }
                        if( !copy_dir_preserve( element_stats, dst_dirfd, basename_, ctx_ptr->copts ) ) {
                            return false;
                        }
                        ::close(dst_dirfd);
                        ctx_ptr->dst_dirfds.pop_back();
                    } else if( is_set(tevt, traverse_event::file) || is_set(tevt, traverse_event::symlink) || is_set(tevt, traverse_event::dir_symlink) ) {
                        if( !copy_file(src_dirfd, element_stats, dst_dirfd, basename_, ctx_ptr->copts) ) {
                            return false;
                        }
                    }
                    return true;
                  } ) );
    bool res = jau::fs::visit(source_stats, topts, pv, &ctx.src_dirfds);
    while( !ctx.dst_dirfds.empty() ) {
        ::close(ctx.dst_dirfds.back());
        ctx.dst_dirfds.pop_back();
    }
    return res;
}

bool jau::fs::rename(const std::string& oldpath, const std::string& newpath) noexcept {
    file_stats oldpath_stats(oldpath);
    file_stats newpath_stats(newpath);
    if( !oldpath_stats.is_link() && !oldpath_stats.exists() ) {
        ERR_PRINT("oldpath doesn't exist, oldpath %s, newpath %s\n",
                oldpath_stats.to_string().c_str(), newpath_stats.to_string().c_str());
        return false;
    }
    if( 0 != ::rename(oldpath_stats.path().c_str(), newpath_stats.path().c_str()) ) {
        ERR_PRINT("rename failed, oldpath %s, newpath %s\n",
                oldpath_stats.to_string().c_str(), newpath_stats.to_string().c_str());
        return false;
    }
    return true;
}

void jau::fs::sync() noexcept {
    ::sync();
}

static bool set_effective_uid(::uid_t user_id) {
    if( 0 != ::seteuid(user_id) ) {
        ERR_PRINT("seteuid(%" PRIu32 ") failed", user_id);
        return false;
    }
    return true;
}

jau::fs::mount_ctx jau::fs::mount_image(const std::string& image_path, const std::string& target, const std::string& fs_type,
                                        const mountflags_t flags, const std::string& fs_options)
{
    file_stats image_stats(image_path);
    if( !image_stats.is_file()) {
        ERR_PRINT("image_path not a file: %s", image_stats.to_string().c_str());
        return mount_ctx();
    }
    file_stats target_stats(target);
    if( !target_stats.is_dir()) {
        ERR_PRINT("target not a dir: %s", target_stats.to_string().c_str());
        return mount_ctx();
    }
    const std::string target_path(target_stats.path());
    int backingfile = __posix_openat64(AT_FDCWD, image_stats.path().c_str(), O_RDWR);
    if( 0 > backingfile )  {
        ERR_PRINT("Couldn't open image-file '%s': res %d", image_stats.to_string().c_str(), backingfile);
        return mount_ctx();
    }
#if defined(__linux__)
    const ::uid_t caller_uid = ::geteuid();
    int loop_device_id = -1;

    ::pid_t pid = ::fork();
    if( 0 == pid ) {
        int loop_ctl_fd = -1, loop_device_fd = -1;
        char loopname[4096];
        int mount_res = -1;
        void* fs_options_cstr = nullptr;

        if( 0 != caller_uid ) {
            if( !set_effective_uid(0) ) {
                goto errout_child;
            }
        }
        loop_ctl_fd = __posix_openat64(AT_FDCWD, "/dev/loop-control", O_RDWR);
        if( 0 > loop_ctl_fd ) {
            ERR_PRINT("Couldn't open loop-control: res %d", loop_ctl_fd);
            goto errout_child;
        }

        loop_device_id = (int) ::ioctl(loop_ctl_fd, LOOP_CTL_GET_FREE);
        if( 0 > loop_device_id ) {
            ERR_PRINT("Couldn't get free loop-device: res %d", loop_device_id);
            goto errout_child;
        }
        if( 254 < loop_device_id ) { // _exit() encoding
            ERR_PRINT("loop-device %d out of valid range [0..254]", loop_device_id);
            goto errout_child;
        }
        ::close(loop_ctl_fd);
        loop_ctl_fd = -1;

        snprintf(loopname, sizeof(loopname), "/dev/loop%d", loop_device_id);
        jau::INFO_PRINT("mount: Info: Using loop-device '%s'", loopname);

        loop_device_fd = __posix_openat64(AT_FDCWD, loopname, O_RDWR);
        if( 0 > loop_device_fd ) {
            ERR_PRINT("Couldn't open loop-device '%s': res %d", loopname, loop_device_fd);
            goto errout_child;
        }
        if( 0 > ::ioctl(loop_device_fd, LOOP_SET_FD, backingfile) ) {
            ERR_PRINT("Couldn't attach image-file '%s' to loop-device '%s'", image_stats.to_string().c_str(), loopname);
            goto errout_child;
        }

        if( fs_options.size() > 0 ) {
            fs_options_cstr = (void*) fs_options.data();
        }
        mount_res = ::mount(loopname, target_path.c_str(), fs_type.c_str(), flags, fs_options_cstr);
        if( 0 != mount_res ) {
            ERR_PRINT("source_path %s, target_path %s, fs_type %s, res %d",
                    image_stats.path().c_str(), target_path.c_str(), fs_type.c_str(), mount_res);
            ::ioctl(loop_device_fd, LOOP_CLR_FD, 0);
            goto errout_child;
        }
        ::close(loop_device_fd);
        ::_exit(loop_device_id+1);

errout_child:
        if( 0 <= loop_ctl_fd ) {
            ::close(loop_ctl_fd);
        }
        if( 0 <= loop_device_fd ) {
            ::close(loop_device_fd);
        }
        ::_exit(0);
    } else if( 0 < pid ) {
        int pid_status = 0;
        ::pid_t child_pid = ::waitpid(pid, &pid_status, 0);
        if( 0 > child_pid ) {
            ERR_PRINT("wait(%d) failed: child_pid %d", pid, child_pid);
        } else {
            if( child_pid != pid ) {
                WARN_PRINT("wait(%d) terminated child_pid %d", pid, child_pid);
            }
            if( !WIFEXITED(pid_status) ) {
                WARN_PRINT("wait(%d) terminated abnormally child_pid %d, pid_status %d", pid, child_pid, pid_status);
                goto errout;
            }
            loop_device_id = WEXITSTATUS(pid_status);
            if( 0 >= loop_device_id ) {
                goto errout;
            }
            --loop_device_id;
        }
    } else {
        ERR_PRINT("Couldn't fork() process: res %d", pid);
        goto errout;
    }
    if( 0 <= backingfile ) {
        ::close(backingfile);
    }
    return mount_ctx(target_path, loop_device_id);

errout:
#else
    (void)fs_type;
    (void)flags;
    (void)fs_options;
#endif
    if( 0 <= backingfile ) {
        ::close(backingfile);
    }
    return mount_ctx();
}

mount_ctx jau::fs::mount(const std::string& source, const std::string& target, const std::string& fs_type,
                         const mountflags_t flags, const std::string& fs_options)
{
    if( source.empty() ) {
        ERR_PRINT("source is an empty string ");
        return mount_ctx();
    }
    file_stats source_stats(source);
    file_stats target_stats(target);
    if( !target_stats.is_dir()) {
        ERR_PRINT("target not a dir: %s", target_stats.to_string().c_str());
        return mount_ctx();
    }
    const std::string target_path(target_stats.path());
    const ::uid_t caller_uid = ::geteuid();

    ::pid_t pid = ::fork();
    if( 0 == pid ) {
        void* fs_options_cstr = nullptr;

        if( 0 != caller_uid ) {
            if( !set_effective_uid(0) ) {
                ::_exit( EXIT_FAILURE );
            }
        }
        if( fs_options.size() > 0 ) {
            fs_options_cstr = (void*) fs_options.data();
        }
#if defined(__linux__)
        const int mount_res = ::mount(source_stats.path().c_str(), target_path.c_str(), fs_type.c_str(), flags, fs_options_cstr);
#elif defined(__FreeBSD__)
        /**
         * Non generic due to the 'fstype'_args structure passed via data
         * ufs_args data = { .fsspec=source_stats.path().c_str() };
         * ::mount(fs_type.c_str(), target_path.c_str(), mountflags, data);
         */
        (void)flags;
        (void)fs_options_cstr;
        const int mount_res = -1;
#else
        #if !defined(JAU_OS_TYPE_WASM)
            #warning Add OS support
        #endif
        (void)flags;
        (void)fs_options_cstr;
        const int mount_res = -1;
#endif
        if( 0 != mount_res ) {
            ERR_PRINT("source_path %s, target_path %s, fs_type %s, flags %" PRIu64 ", res %d",
                    source_stats.path().c_str(), target_path.c_str(), fs_type.c_str(), flags, mount_res);
            ::_exit( EXIT_FAILURE );
        } else {
            ::_exit( EXIT_SUCCESS );
        }

    } else if( 0 < pid ) {
        int pid_status = 0;
        ::pid_t child_pid = ::waitpid(pid, &pid_status, 0);
        if( 0 > child_pid ) {
            ERR_PRINT("wait(%d) failed: child_pid %d", pid, child_pid);
        } else {
            if( child_pid != pid ) {
                WARN_PRINT("wait(%d) terminated child_pid %d", pid, child_pid);
            }
            if( !WIFEXITED(pid_status) ) {
                WARN_PRINT("wait(%d) terminated abnormally child_pid %d, pid_status %d", pid, child_pid, pid_status);
            } else if( EXIT_SUCCESS == WEXITSTATUS(pid_status) ) {
                return mount_ctx(target_path, -1);
            } // else mount failed
        }
    } else {
        ERR_PRINT("Couldn't fork() process: res %d", pid);
    }
    return mount_ctx();
}

bool jau::fs::umount(const mount_ctx& context, const umountflags_t flags)
{
    if( !context.mounted) {
        return false;
    }
    file_stats target_stats(context.target);
    if( !target_stats.is_dir()) {
        return false;
    }
    const ::uid_t caller_uid = ::geteuid();

    ::pid_t pid = ::fork();
    if( 0 == pid ) {
        if( 0 != caller_uid ) {
            if( !set_effective_uid(0) ) {
                ::_exit( EXIT_FAILURE );
            }
        }
#if defined(__linux__)
        const int umount_res = ::umount2(target_stats.path().c_str(), flags);
#elif defined(__FreeBSD__)
        const int umount_res = ::unmount(target_stats.path().c_str(), flags);
#else
        #if !defined(JAU_OS_TYPE_WASM)
            #warning Add OS support
        #endif
        const int umount_res = -1;
#endif
        if( 0 != umount_res ) {
            ERR_PRINT("Couldn't umount '%s', flags %d: res %d\n", target_stats.to_string().c_str(), flags, umount_res);
        }
        if( 0 > context.loop_device_id ) {
            // mounted w/o loop-device, done
            ::_exit(0 == umount_res ? EXIT_SUCCESS : EXIT_FAILURE);
        }
#if defined(__linux__)
        int loop_device_fd = -1;
        char loopname[4096];

        snprintf(loopname, sizeof(loopname), "/dev/loop%d", context.loop_device_id);
        jau::INFO_PRINT("umount: Info: Using loop-device '%s'", loopname);

        loop_device_fd = __posix_openat64(AT_FDCWD, loopname, O_RDWR);
        if( 0 > loop_device_fd ) {
            ERR_PRINT("Couldn't open loop-device '%s': res %d", loopname, loop_device_fd);
            goto errout_child;
        }
        if( 0 > ::ioctl(loop_device_fd, LOOP_CLR_FD, 0) ) {
            ERR_PRINT("Couldn't detach loop-device '%s'", loopname);
            goto errout_child;
        }
        ::close(loop_device_fd);
        ::_exit(0 == umount_res ? EXIT_SUCCESS : EXIT_FAILURE);
#endif

        // No loop-device handling for OS
        ::_exit( EXIT_FAILURE );

#if defined(__linux__)
errout_child:
        if( 0 <= loop_device_fd ) {
            ::close(loop_device_fd);
        }
        ::_exit( EXIT_FAILURE );
#endif
    } else if( 0 < pid ) {
        int pid_status = 0;
        ::pid_t child_pid = ::waitpid(pid, &pid_status, 0);
        if( 0 > child_pid ) {
            ERR_PRINT("wait(%d) failed: child_pid %d", pid, child_pid);
        } else {
            if( child_pid != pid ) {
                WARN_PRINT("wait(%d) terminated child_pid %d", pid, child_pid);
            }
            if( !WIFEXITED(pid_status) ) {
                WARN_PRINT("wait(%d) terminated abnormally child_pid %d, pid_status %d", pid, child_pid, pid_status);
            } else if( EXIT_SUCCESS == WEXITSTATUS(pid_status) ) {
                return true;
            } // else umount failed
        }
    } else {
        ERR_PRINT("Couldn't fork() process: res %d", pid);
    }
    return false;
}

bool jau::fs::umount(const std::string& target, const umountflags_t flags)
{
    if( target.empty() ) {
        return false;
    }
    file_stats target_stats(target);
    if( !target_stats.is_dir()) {
        return false;
    }
    const ::uid_t caller_uid = ::geteuid();

    ::pid_t pid = ::fork();
    if( 0 == pid ) {
        if( 0 != caller_uid ) {
            if( !set_effective_uid(0) ) {
                ::_exit( EXIT_FAILURE );
            }
        }
#if defined(__linux__)
        const int umount_res = ::umount2(target_stats.path().c_str(), flags);
#elif defined(__FreeBSD__)
        const int umount_res = ::unmount(target_stats.path().c_str(), flags);
#else
        #if !defined(JAU_OS_TYPE_WASM)
            #warning Add OS support
        #endif
        const int umount_res = -1;
#endif
        if( 0 == umount_res ) {
            ::_exit( EXIT_SUCCESS );
        } else {
            ERR_PRINT("Couldn't umount '%s', flags %d: res %d\n", target_stats.to_string().c_str(), flags, umount_res);
            ::_exit( EXIT_FAILURE );
        }
    } else if( 0 < pid ) {
        int pid_status = 0;
        ::pid_t child_pid = ::waitpid(pid, &pid_status, 0);
        if( 0 > child_pid ) {
            ERR_PRINT("wait(%d) failed: child_pid %d", pid, child_pid);
        } else {
            if( child_pid != pid ) {
                WARN_PRINT("wait(%d) terminated child_pid %d", pid, child_pid);
            }
            if( !WIFEXITED(pid_status) ) {
                WARN_PRINT("wait(%d) terminated abnormally child_pid %d, pid_status %d", pid, child_pid, pid_status);
            } else if( EXIT_SUCCESS == WEXITSTATUS(pid_status) ) {
                return true;
            } // else umount failed
        }
    } else {
        ERR_PRINT("Couldn't fork() process: res %d", pid);
    }
    return false;
}
