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

#include <cstdint>
#include <cstdlib>
#include <cinttypes>
#include <limits>

#include <jau/debug.hpp>
#include <jau/file_util.hpp>

extern "C" {
    #include <sys/stat.h>
#ifdef __linux__
    #include <sys/sendfile.h>
    #include <linux/loop.h>
#endif
    #include <sys/types.h>
    #include <sys/mount.h>
    #include <dirent.h>
    #include <fcntl.h>
    #include <unistd.h>
}

#ifndef O_BINARY
#define O_BINARY    0
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK  0
#endif

using namespace jau;
using namespace jau::fs;

#ifdef __linux__
    static constexpr const bool _use_statx = true;
    static constexpr const bool _use_sendfile = true;
#else
    static constexpr const bool _use_statx = false;
    static constexpr const bool _use_sendfile = false;
#endif

std::string jau::fs::get_cwd() noexcept {
    char path[PATH_MAX];
    char* res = ::getcwd(path, PATH_MAX);
    if( res == path ) {
        return std::string(path);
    } else {
        return std::string();
    }
}

static const std::string _slash("/");
static const std::string _slash_dot_slash("/./");
static const std::string _slash_dot("/.");
static const std::string _dot_slash("./");
static const std::string _dot(".");
static const std::string _slash_dotdot_slash("/../");
static const std::string _slash_dotdot("/..");
static const std::string _dotdot("..");

std::string jau::fs::dirname(const std::string_view& path) noexcept {
    if( 0 == path.size() ) {
        return _dot;
    }
    size_t end_pos;
    if( '/' == path[path.size()-1] ) {
        if( 1 == path.size() ) { // maintain a single '/'
            return std::string(path);
        }
        end_pos = path.size()-2;
    } else {
        end_pos = path.size()-1;
    }
    size_t idx = path.find_last_of('/', end_pos);
    if( idx == std::string_view::npos ) {
        return _dot;
    } else {
        // ensure `/lala` -> '/', i.e. don't cut off single '/'
        return std::string( path.substr(0, std::max<size_t>(1, idx)) );
    }
}

std::string jau::fs::basename(const std::string_view& path) noexcept {
    if( 0 == path.size() ) {
        return _dot;
    }
    size_t end_pos;
    if( '/' == path[path.size()-1] ) {
        if( 1 == path.size() ) { // maintain a single '/'
            return std::string(path);
        }
        end_pos = path.size()-2;
    } else {
        end_pos = path.size()-1;
    }
    size_t idx = path.find_last_of('/', end_pos);
    if( idx == std::string_view::npos ) {
        return std::string( path.substr(0, end_pos+1));
    } else {
        return std::string( path.substr(idx+1, end_pos-idx) );
    }
}

dir_item::backed_string_view dir_item::reduce(const std::string_view& path_) noexcept {
    constexpr const bool _debug = false;

    if constexpr ( _debug ) {
        jau::fprintf_td(stderr, "X.0: path '%s'\n", std::string(path_).c_str());
    }

    if( _dot == path_ || _slash == path_ ) {
        return backed_string_view( path_ );
    }

    std::string_view path2 = path_;

    // remove initial './'
    if( 0 == path2.find(_dot_slash) ) {
        path2 = path2.substr(2, path2.size()-2);
    }

    // remove trailing slash if not ending with '/./' or '/../'
    if( '/' == path2[path2.size()-1] &&
        ( path2.size() < 3 || std::string_view::npos == path2.find(_slash_dot_slash, path2.size()-3) ) &&
        ( path2.size() < 4 || std::string_view::npos == path2.find(_slash_dotdot_slash, path2.size()-4) )
      )
    {
        path2 = path2.substr(0, path2.size()-1);
    }

    std::string path2_backing;

    // append final '/' to complete '/../' or '/./' sequence
    if( ( path2.size() >= 3 && std::string_view::npos != path2.find(_slash_dotdot, path2.size()-3) ) ||
        ( path2.size() >= 2 && std::string_view::npos != path2.find(_slash_dot, path2.size()-2) ) ) {
        path2_backing = std::string(path2);
        path2 = path2_backing.append(_slash);
    }

    if constexpr ( _debug ) {
        jau::fprintf_td(stderr, "X.1: path2 '%s'\n", std::string(path2).c_str());
    }

    // resolve '/./'
    size_t idx;
    do {
        idx = path2.find(_slash_dot_slash);
        if( std::string_view::npos != idx ) {
            std::string_view pre = path2.substr(0, idx);
            if( 0 == pre.size() ) {
                // case '/./bbb' -> '/bbb'
                path2 = path2.substr(idx+2);
            } else if( _dot == pre ) {
                // case '././bbb' -> 'bbb'
                path2 = path2.substr(idx+3);
            } else {
                // case '/zzz/aaa/./bbb' -> '/zzz/aaa/bbb'
                std::string pre_str = std::string(pre);
                pre_str.append( path2.substr(idx+2) );
                path2_backing = std::move(pre_str);
                path2 = path2_backing;
            }
            if constexpr ( _debug ) {
                jau::fprintf_td(stderr, "X.2.2: path2: '%s'\n", std::string(path2).c_str());
            }
        }
    } while( idx != std::string_view::npos );
    if constexpr ( _debug ) {
        jau::fprintf_td(stderr, "X.2.X: path2: '%s'\n", std::string(path2).c_str());
    }

    // resolve '/../'
    size_t spos=0;
    do {
        idx = path2.find(_slash_dotdot_slash, spos);
        if( std::string_view::npos != idx ) {
            if( 0 == idx ) {
                // case '/../bbb' -> Error, End
                ERR_PRINT2("dir_item::resolve: '..' resolution error: '%s' -> '%s'", std::string(path_).c_str(), std::string(path2).c_str());
                return backed_string_view(std::move(path2_backing), path2);
            }
            std::string_view pre = path2.substr(0, idx);
            if( _dotdot == pre ) {
                // case '../../bbb' -> '../../bbb' unchanged
                spos = idx+4;
            } else if( _dot == pre ) {
                // case './../bbb' -> '../bbb'
                path2 = path2.substr(idx+1);
            } else {
                std::string pre_str = jau::fs::dirname( pre );
                if( _slash == pre_str ) {
                    // case '/aaa/../bbb' -> '/bbb'
                    path2 = path2.substr(idx+3);
                } else {
                    // case '/zzz/aaa/../bbb' -> '/zzz/bbb'
                    pre_str.append( path2.substr(idx+3) );
                    path2_backing = std::move(pre_str);
                    path2 = path2_backing;
                }
            }
            if constexpr ( _debug ) {
                jau::fprintf_td(stderr, "X.3.2: path2: '%s'\n", std::string(path2).c_str());
            }
        }
    } while( idx != std::string_view::npos );
    if constexpr ( _debug ) {
        jau::fprintf_td(stderr, "X.3.X: path2: '%s'\n", std::string(path2).c_str());
    }

    // remove trailing slash in path2
    if( '/' == path2[path2.size()-1] ) {
        path2 = path2.substr(0, path2.size()-1);
    }
    if constexpr ( _debug ) {
        jau::fprintf_td(stderr, "X.X: path2: '%s'\n", std::string(path2).c_str());
    }
    return backed_string_view(std::move(path2_backing), path2);
}

dir_item::dir_item(const backed_string_view cleanpath) noexcept
: dirname_(jau::fs::dirname(cleanpath.view)), basename_(jau::fs::basename(cleanpath.view)) {
    if( _slash == dirname_ && _slash == basename_ ) { // remove duplicate '/' in basename
        basename_ = _dot;
    }
}

dir_item::dir_item(const std::string_view& path_) noexcept
: dir_item( reduce(path_) )
{ }


std::string dir_item::path() const noexcept {
    if( _dot ==  dirname_ ) {
        return basename_;
    }
    if( _dot ==  basename_ ) {
        return dirname_;
    }
    if( _slash == dirname_ ) {
        return dirname_ + basename_;
    }
    return dirname_ + _slash + basename_;
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
    return out;
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
: has_fields_(field_t::none), item_(), link_target_path_(), link_target_(), mode_(fmode_t::not_existing),
  uid_(0), gid_(0), size_(0), btime_(), atime_(), ctime_(), mtime_(),
  errno_res_(0)
{}

static constexpr bool jau_has_stat(const uint32_t mask, const uint32_t bit) { return bit == ( mask & bit ); }

file_stats::file_stats(const ctor_cookie& cc, const dir_item& item) noexcept
: has_fields_(field_t::none), item_(item), link_target_path_(), link_target_(), mode_(fmode_t::none),
  uid_(0), gid_(0), size_(0), btime_(), atime_(), ctime_(), mtime_(),
  errno_res_(0)
{
    (void)cc;
    const std::string path( item_.path() );

if constexpr ( _use_statx ) {
    struct ::statx s;
    ::bzero(&s, sizeof(s));
    int stat_res = ::statx(AT_FDCWD, path.c_str(),
                           ( AT_NO_AUTOMOUNT | AT_SYMLINK_NOFOLLOW ),
                           ( STATX_BASIC_STATS | STATX_BTIME ), &s);
    if( 0 != stat_res ) {
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
            if( is_file() && !is_link() ) {
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
                const ssize_t path_link_max_len = 0 < s.stx_size ? s.stx_size + 1 : PATH_MAX;
                char* buffer = (char*) ::malloc(path_link_max_len);
                ssize_t path_link_len = 0;;
                if( nullptr == buffer ) {
                    errno_res_ = errno;
                    link_target_ = std::make_shared<file_stats>();
                    goto errorout;
                }
                path_link_len = ::readlink(path.c_str(), buffer, path_link_max_len);
                if( 0 > path_link_len ) {
                    errno_res_ = errno;
                    ::free(buffer);
                    link_target_ = std::make_shared<file_stats>();
                    goto errorout;
                }
                // Note: if( path_link_len == path_link_max_len ) then buffer may have been truncated
                link_path = std::string(buffer, path_link_len);
            }
            link_target_path_ = std::make_shared<std::string>(link_path);
            if( 0 == cc.rec_level ) {
                // Initial symbolic followed: Test recursive loop-error
                constexpr const bool _debug = false;
                ::bzero(&s, sizeof(s));
                stat_res = ::statx(AT_FDCWD, path.c_str(), AT_NO_AUTOMOUNT, STATX_BASIC_STATS, &s);
                if( 0 != stat_res ) {
                    if constexpr ( _debug ) {
                        jau::fprintf_td(stderr, "file_stats(%d): Test link ERROR: '%s', %d, errno %d (%s)\n", (int)cc.rec_level, path.c_str(), stat_res, errno, ::strerror(errno));
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
                if constexpr ( _debug ) {
                    jau::fprintf_td(stderr, "file_stats(%d): Test link OK: '%s'\n", (int)cc.rec_level, path.c_str());
                }
            }
            if( 0 < link_path.size() && '/' == link_path[0] ) {
                link_target_ = std::make_shared<file_stats>(ctor_cookie(cc.rec_level+1), dir_item( link_path )); // absolute link_path
            } else {
                link_target_ = std::make_shared<file_stats>(ctor_cookie(cc.rec_level+1), dir_item( jau::fs::dirname(path) + _slash + link_path ));
            }
            if( link_target_->is_file() ) {
                mode_ |= fmode_t::file;
                has_fields_ |= field_t::size;
                size_ = link_target_->size();
            } else if( link_target_->is_dir() ) {
                mode_ |= fmode_t::dir;
            } else if( !link_target_->exists() ) {
                mode_ |= fmode_t::not_existing;
            } else if( !link_target_->has_access() ) {
                mode_ |= fmode_t::no_access;
            }
        }
    }
} else { /* constexpr !_use_statx */
    struct ::stat64 s;
    ::bzero(&s, sizeof(s));
    int stat_res = ::lstat64(path.c_str(), &s);
    if( 0 != stat_res ) {
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
            }
        } else if( S_ISDIR( s.st_mode ) ) {
            mode_ |= fmode_t::dir;
        }

        // Append POSIX protection bits
        mode_ |= static_cast<fmode_t>( s.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID | S_ISVTX ) );

        uid_ = s.st_uid;
        gid_ = s.st_gid;
        size_ = is_file() ? s.st_size : 0;
        atime_ = jau::fraction_timespec( s.st_atim.tv_sec, s.st_atim.tv_nsec );
        ctime_ = jau::fraction_timespec( s.st_ctim.tv_sec, s.st_ctim.tv_nsec );
        mtime_ = jau::fraction_timespec( s.st_mtim.tv_sec, s.st_mtim.tv_nsec );

        if( is_link() ) {
            // follow symbolic link recursively until !exists(), is_file() or is_dir()
            std::string link_path;
            {
                const ssize_t path_link_max_len = 0 < s.st_size ? s.st_size + 1 : PATH_MAX;
                char* buffer = (char*) ::malloc(path_link_max_len);
                ssize_t path_link_len = 0;;
                if( nullptr == buffer ) {
                    errno_res_ = errno;
                    link_target_ = std::make_shared<file_stats>();
                    goto errorout;
                }
                path_link_len = ::readlink(path.c_str(), buffer, path_link_max_len);
                if( 0 > path_link_len ) {
                    errno_res_ = errno;
                    ::free(buffer);
                    link_target_ = std::make_shared<file_stats>();
                    goto errorout;
                }
                // Note: if( path_link_len == path_link_max_len ) then buffer may have been truncated
                link_path = std::string(buffer, path_link_len);
            }
            link_target_path_ = std::make_shared<std::string>(link_path);
            if( 0 == cc.rec_level ) {
                // Initial symbolic followed: Test recursive loop-error
                ::bzero(&s, sizeof(s));
                stat_res = ::stat64(path.c_str(), &s);
                if( 0 != stat_res ) {
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
            if( 0 < link_path.size() && '/' == link_path[0] ) {
                link_target_ = std::make_shared<file_stats>(ctor_cookie(cc.rec_level+1), dir_item( link_path )); // absolute link_path
            } else {
                link_target_ = std::make_shared<file_stats>(ctor_cookie(cc.rec_level+1), dir_item( jau::fs::dirname(path) + _slash + link_path ));
            }
            if( link_target_->is_file() ) {
                mode_ |= fmode_t::file;
                has_fields_ |= field_t::size;
                size_ = link_target_->size();
            } else if( link_target_->is_dir() ) {
                mode_ |= fmode_t::dir;
            } else if( !link_target_->exists() ) {
                mode_ |= fmode_t::not_existing;
            } else if( !link_target_->has_access() ) {
                mode_ |= fmode_t::no_access;
            }
        }
    }
} /* constexpr !_use_statx */

    errorout: ;
}

file_stats::file_stats(const dir_item& item) noexcept
: file_stats(ctor_cookie(0), item)
{}

file_stats::file_stats(const std::string& _path) noexcept
: file_stats(ctor_cookie(0), dir_item(_path))
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

std::string file_stats::to_string(const bool use_space) const noexcept {
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
        if( has( field_t::uid ) ) {
            res.append( ", uid " ).append( std::to_string(uid_) );
        }
        if( has( field_t::gid ) ) {
            res.append( ", gid " ).append( std::to_string(gid_) );
        }
        if( has( field_t::size ) ) {
            res.append( ", size " ).append( jau::to_decstring( size_ ) );
        }
        if( has( field_t::btime ) ) {
            res.append( ", btime " ).append( btime_.to_iso8601_string(use_space) );
        }
        if( has( field_t::atime ) ) {
            res.append( ", atime " ).append( atime_.to_iso8601_string(use_space) );
        }
        if( has( field_t::ctime ) ) {
            res.append( ", ctime " ).append( ctime_.to_iso8601_string(use_space) );
        }
        if( has( field_t::mtime ) ) {
            res.append( ", mtime " ).append( mtime_.to_iso8601_string(use_space) );
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
            if( verbose ) {
                jau::fprintf_td(stderr, "mkdir failed: %s, errno %d (%s)\n", stats.to_string().c_str(), errno, strerror(errno));
            }
            return false;
        } else {
            return true;
        }
    } else {
        if( verbose ) {
            jau::fprintf_td(stderr, "mkdir failed: %s, exists but is no dir\n", stats.to_string().c_str());
        }
        return false;
    }
}

bool jau::fs::touch(const std::string& path, const jau::fraction_timespec& atime, const jau::fraction_timespec& mtime, const fmode_t mode, const bool verbose) noexcept {
    int fd = ::open(path.c_str(), O_WRONLY|O_CREAT|O_NOCTTY|O_NONBLOCK, posix_protection_bits(mode));
    if( 0 > fd ) {
        if( verbose ) {
            jau::fprintf_td(stderr, "touch failed: Couldn't open/create file '%s', errno %d (%s)\n", path.c_str(), errno, strerror(errno));
        }
        return false;
    }

    struct timespec ts2[2] = { atime.to_timespec(), mtime.to_timespec() };
    bool res;
    if( 0 != ::futimens(fd, ts2) ) {
        if( verbose ) {
            jau::fprintf_td(stderr, "touch failed: Couldn't update time of file '%s', errno %d (%s)\n", path.c_str(), errno, strerror(errno));
        }
        res = false;
    } else {
        res = true;
    }
    ::close(fd);
    return res;
}

bool jau::fs::touch(const std::string& path, const fmode_t mode, const bool verbose) noexcept {
    int fd = ::open(path.c_str(), O_WRONLY|O_CREAT|O_NOCTTY|O_NONBLOCK, posix_protection_bits(mode));
    if( 0 > fd ) {
        if( verbose ) {
            jau::fprintf_td(stderr, "touch failed: Couldn't open/create file '%s', errno %d (%s)\n", path.c_str(), errno, strerror(errno));
        }
        return false;
    }

    bool res;
    if( 0 != ::futimens(fd, NULL /* current time */) ) {
        if( verbose ) {
            jau::fprintf_td(stderr, "touch failed: Couldn't update time of file '%s', errno %d (%s)\n", path.c_str(), errno, strerror(errno));
        }
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
        while ( ( ent = ::readdir( dir ) ) != NULL ) {
            std::string fname( ent->d_name );
            if( _dot != fname && _dotdot != fname ) { // avoid '.' and '..'
                digest( dir_item( path + _slash + fname ) );
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
    X(traverse_options,dir_entry,M) \
    X(traverse_options,dir_exit,M)

std::string jau::fs::to_string(const traverse_options mask) noexcept {
    std::string out("[");
    bool comma = false;
    TRAVERSEOPTIONS_ENUM(APPEND_BITSTR,mask)
    out.append("]");
    return out;
}

bool jau::fs::visit(const file_stats& item_stats, const traverse_options topts, const path_visitor& visitor) noexcept {
    if( item_stats.is_dir() ) {
        if( item_stats.is_link() && !is_set(topts, traverse_options::follow_symlinks) ) {
            return visitor( traverse_event::dir_symlink, item_stats );
        }
        if( !is_set(topts, traverse_options::recursive) ) {
            return visitor( traverse_event::dir_non_recursive, item_stats );
        }
        if( is_set(topts, traverse_options::dir_entry) ) {
            if( !visitor( traverse_event::dir_entry, item_stats ) ) {
                return false;
            }
        }
        std::vector<dir_item> content;
        const consume_dir_item cs = jau::bindCaptureRefFunc<void, std::vector<dir_item>, const dir_item&>(&content,
                ( void(*)(std::vector<dir_item>*, const dir_item&) ) /* help template type deduction of function-ptr */
                    ( [](std::vector<dir_item>* receiver, const dir_item& item) -> void { receiver->push_back( item ); } )
            );
        if( get_dir_content(item_stats.path(), cs) && content.size() > 0 ) {
            for (const dir_item& element : content) {
                const file_stats element_stats( element );
                if( element_stats.is_dir() ) { // an OK dir
                    if( element_stats.is_link() && !is_set(topts, traverse_options::follow_symlinks) ) {
                        if( !visitor( traverse_event::dir_symlink, element_stats ) ) {
                            return false;
                        }
                    } else if( !visit(element_stats, topts, visitor) ) { // recursive
                        return false;
                    }
                } else if( !visitor( ( element_stats.is_file() ? traverse_event::file : traverse_event::none ) |
                                     ( element_stats.is_link() ? traverse_event::symlink : traverse_event::none),
                                     element_stats ) )
                {
                    return false;
                }
            }
        }
    }
    if( item_stats.is_dir() && is_set(topts, traverse_options::dir_exit) ) {
        return visitor( traverse_event::dir_exit, item_stats );
    } else if( item_stats.is_file() || !item_stats.ok() ) { // file or error-alike
        return visitor( ( item_stats.is_file() ? traverse_event::file : traverse_event::none ) |
                        ( item_stats.is_link() ? traverse_event::symlink : traverse_event::none),
                        item_stats );
    } else {
        return true;
    }
}

bool jau::fs::visit(const std::string& path, const traverse_options topts, const path_visitor& visitor) noexcept {
    return jau::fs::visit(file_stats(path), topts, visitor);
}

bool jau::fs::remove(const std::string& path, const traverse_options topts) noexcept {
    file_stats path_stats(path);
    if( path_stats.is_dir() && !is_set(topts, traverse_options::recursive) ) {
        if( is_set(topts, traverse_options::verbose) ) {
            jau::fprintf_td(stderr, "remove: Error: path is dir but !recursive, %s\n", path_stats.to_string().c_str());
        }
        return false;
    }
    const path_visitor pv = jau::bindCaptureRefFunc<bool, const traverse_options, traverse_event, const file_stats&>(&topts,
            ( bool(*)(const traverse_options*, traverse_event, const file_stats&) ) /* help template type deduction of function-ptr */
                ( [](const traverse_options* _options, traverse_event tevt, const file_stats& element_stats) -> bool {
                    (void)tevt;
                    const int res = ::remove( element_stats.path().c_str() );
                    if( 0 != res ) {
                        if( is_set(*_options, traverse_options::verbose) ) {
                            jau::fprintf_td(stderr, "remove: Error: remove failed: %s, res %d, errno %d (%s)\n",
                                    element_stats.to_string().c_str(), res, errno, strerror(errno));
                        }
                        return false;
                    } else {
                        if( is_set(*_options, traverse_options::verbose) ) {
                            jau::fprintf_td(stderr, "remove: %s removed\n", element_stats.to_string().c_str());
                        }
                        return true;
                    }
                  } ) );
    return jau::fs::visit(path_stats,
                ( topts & ~jau::fs::traverse_options::dir_entry ) | jau::fs::traverse_options::dir_exit, pv);
}

#define COPYOPTIONS_BIT_ENUM(X,M) \
    X(copy_options,recursive,M) \
    X(copy_options,follow_symlinks,M) \
    X(copy_options,ignore_symlink_errors,M) \
    X(copy_options,overwrite,M) \
    X(copy_options,preserve_all,M) \
    X(copy_options,sync,M)

#define APPEND_BIT_COPYOPTIONS(U,V) _append_bitstr(out, mask, U::V, #V, comma);

std::string jau::fs::to_string(const copy_options mask) noexcept {
    std::string out("[");
    bool comma = false;
    COPYOPTIONS_BIT_ENUM(APPEND_BITSTR,mask)
    out.append("]");
    return out;
}

bool jau::fs::compare(const file_stats& source1, const file_stats& source2, const bool verbose) noexcept {
    if( !source1.is_file() ) {
        if( verbose ) {
            jau::fprintf_td(stderr, "compare: Error: source1_stats is not a file: %s\n", source1.to_string().c_str());
        }
        return false;
    }
    if( !source2.is_file() ) {
        if( verbose ) {
            jau::fprintf_td(stderr, "compare: Error: source2_stats is not a file: %s\n", source2.to_string().c_str());
        }
        return false;
    }

    if( source1.size() != source2.size() ) {
        if( verbose ) {
            jau::fprintf_td(stderr, "compare: Error: Source files size mismatch, %s != %s\n",
                    source1.to_string().c_str(), source2.to_string().c_str());
        }
        return false;
    }

    int src1=-1, src2=-1;
    int src_flags = O_RDONLY|O_BINARY|O_NOCTTY;
    uint64_t offset = 0;

    bool res = false;
    src1 = ::open64(source1.path().c_str(), src_flags);
    if ( 0 > src1 ) {
        if( verbose ) {
            jau::fprintf_td(stderr, "compare: Error: Failed to open source1 %s, errno %d, %s\n", source1.to_string().c_str(), errno, ::strerror(errno));
        }
        goto errout;
    }
    src2 = ::open64(source2.path().c_str(), src_flags);
    if ( 0 > src2 ) {
        if( verbose ) {
            jau::fprintf_td(stderr, "compare: Error: Failed to open source2 %s, errno %d, %s\n", source2.to_string().c_str(), errno, ::strerror(errno));
        }
        goto errout;
    }
    while ( offset < source1.size()) {
        ssize_t rc1, rc2=0;
        char buffer1[BUFSIZ];
        char buffer2[BUFSIZ];
        if( ( rc1 = ::read(src1, buffer1, sizeof(buffer1)) ) > 0 ) {
            ssize_t bytes_to_write = rc1;
            size_t buffer_offset = 0;
            while( 0 < bytes_to_write ) { // src2-read required src1 size in chunks, allowing potential multiple read-ops to match src1 size
                if( ( rc2 = ::read(src2, buffer2+buffer_offset, bytes_to_write) ) < 0 ) {
                    break;
                }
                buffer_offset += rc2;
                bytes_to_write -= rc2;
                offset += (uint64_t)rc2;
            }
        }
        if ( 0 > rc1 || 0 > rc2 ) {
            if( verbose ) {
                if ( 0 > rc1 ) {
                    jau::fprintf_td(stderr, "compare: Error: Failed to read source1 bytes @ %s / %s, %s, errno %d, %s\n",
                            jau::to_decstring(offset).c_str(), jau::to_decstring(source1.size()).c_str(),
                            source1.to_string().c_str(), errno, ::strerror(errno));
                } else if ( 0 > rc2 ) {
                    jau::fprintf_td(stderr, "compare: Error: Failed to read source2 bytes @ %s / %s, %s, errno %d, %s\n",
                            jau::to_decstring(offset).c_str(), jau::to_decstring(source2.size()).c_str(),
                            source2.to_string().c_str(), errno, ::strerror(errno));
                }
            }
            goto errout;
        }
        if( 0 != ::memcmp(buffer1, buffer2, rc1) ) {
            if( verbose ) {
                jau::fprintf_td(stderr, "compare: Error: Comparison failed of %s bytes @ %s / %s, %s != %s\n",
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
        if( verbose ) {
            jau::fprintf_td(stderr, "compare: Error: Incomplete transfer %s / %s, %s != %s\n",
                    jau::to_decstring(offset).c_str(), jau::to_decstring(source1.size()).c_str(),
                    source1.to_string().c_str(), source2.to_string().c_str(), errno, ::strerror(errno));
        }
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

static bool copy_file(const file_stats& source_stats, const std::string& dest_path, const copy_options copts) noexcept {
    file_stats dest_stats(dest_path);

    // overwrite: remove pre-existing file, if copy_options::overwrite set
    if( dest_stats.is_file() ) {
        if( !is_set(copts, copy_options::overwrite) ) {
            if( is_set(copts, copy_options::verbose) ) {
                jau::fprintf_td(stderr, "copy: Error: dest_path exists but copy_options::overwrite not set: source %s, dest '%s', copts %s\n",
                        source_stats.to_string().c_str(), dest_stats.to_string().c_str(), to_string( copts ).c_str());
            }
            return false;
        }
        const int res = ::remove( dest_path.c_str() );
        if( 0 != res ) {
            if( is_set(copts, copy_options::verbose) ) {
                jau::fprintf_td(stderr, "copy: Error: remove existing dest_path for symbolic-link failed: source %s, dest '%s', errno %d (%s)\n",
                        source_stats.to_string().c_str(), dest_stats.to_string().c_str(), errno, strerror(errno));
            }
            return false;
        }
    }

    // copy as symbolic link
    if( source_stats.is_link() && !is_set(copts, copy_options::follow_symlinks) ) {
        const std::shared_ptr<std::string> link_target_path = source_stats.link_target_path();
        if( nullptr == link_target_path || 0 == link_target_path->size() ) {
            if( is_set(copts, copy_options::verbose) ) {
                jau::fprintf_td(stderr, "copy: Error: Symbolic link-path is empty %s\n", source_stats.to_string().c_str());
            }
            return false;
        }
        // symlink
        const int res = ::symlink(link_target_path->c_str(), dest_path.c_str());
        if( 0 > res ) {
            if( is_set(copts, copy_options::verbose) ) {
                jau::fprintf_td(stderr, "copy: Error: Creating symlink failed %s -> %s, errno %d, %s\n",
                        dest_path.c_str(), link_target_path->c_str(), errno, ::strerror(errno));
            }
            return false;
        }
        if( is_set(copts, copy_options::preserve_all) ) {
            // preserve time
            struct timespec ts2[2] = { source_stats.atime().to_timespec(), source_stats.mtime().to_timespec() };
            if( 0 != ::utimensat(AT_FDCWD, dest_path.c_str(), ts2, AT_SYMLINK_NOFOLLOW) ) {
                if( is_set(copts, copy_options::verbose) ) {
                    jau::fprintf_td(stderr, "copy: Error: Couldn't preserve time of symlink, source %s, dest '%s', errno %d (%s)\n",
                            source_stats.to_string().c_str(), dest_path.c_str(), errno, ::strerror(errno));
                }
                return false;
            }
            // preserve ownership
            const uid_t caller_uid = getuid();
            const ::uid_t source_uid = 0 == caller_uid ? source_stats.uid() : -1;
            if( 0 != ::fchownat(AT_FDCWD, dest_path.c_str(), source_uid, source_stats.gid(), AT_SYMLINK_NOFOLLOW) ) {
                if( errno != EPERM && errno != EINVAL ) { // OK to fail due to permissions
                    if( is_set(copts, copy_options::verbose) ) {
                        jau::fprintf_td(stderr, "copy: Error: Couldn't preserve ownership of symlink, source %s, dest '%s', errno %d (%s)\n",
                                source_stats.to_string().c_str(), dest_path.c_str(), errno, ::strerror(errno));
                    }
                    return false;
                }
            }
        }
        return true;
    }
    // copy actual file bytes
    const file_stats* target_stats = source_stats.final_target(); // follows symlinks up to definite item
    const fmode_t dest_mode = target_stats->prot_mode();
    const fmode_t omitted_permissions = dest_mode & ( fmode_t::rwx_grp | fmode_t::rwx_oth );

    const uid_t caller_uid = getuid();
    int src=-1, dst=-1;
    int src_flags = O_RDONLY|O_BINARY|O_NOCTTY;
    uint64_t offset = 0;

    bool res = false;
    if( caller_uid == target_stats->uid() ) {
        src_flags |= O_NOATIME;
    } // else we are not allowed to not use O_NOATIME
    src = ::open64(source_stats.path().c_str(), src_flags);
    if ( 0 > src ) {
        if( source_stats.is_link() ) {
            res = is_set(copts, copy_options::ignore_symlink_errors);
        }
        if( is_set(copts, copy_options::verbose) ) {
            jau::fprintf_td(stderr, "copy: %s: Failed to open source %s, errno %d, %s\n", res?"Ignored":"Error", source_stats.to_string().c_str(), errno, ::strerror(errno));
        }
        goto errout;
    }
    dst = ::open64(dest_path.c_str(), O_CREAT|O_WRONLY|O_BINARY|O_EXCL|O_CLOEXEC|O_NOCTTY, jau::fs::posix_protection_bits( dest_mode & ~omitted_permissions ) );
    if ( 0 > dst ) {
        if( is_set(copts, copy_options::verbose) ) {
            jau::fprintf_td(stderr, "copy: Error: Failed to open target_path '%s', errno %d, %s\n", dest_path.c_str(), errno, ::strerror(errno));
        }
        goto errout;
    }
    while ( offset < source_stats.size()) {
        ssize_t rc1, rc2=0;
        if constexpr ( _use_sendfile ) {
            off64_t offset_i = (off64_t)offset; // we drop 1 bit of value-range as off64_t is int64_t
            const uint64_t count = std::max<uint64_t>(std::numeric_limits<ssize_t>::max(), source_stats.size() - offset);
            if( ( rc1 = ::sendfile64(dst, src, &offset_i, (size_t)count) ) >= 0 ) {
                offset = (uint64_t)offset_i;
            }
        } else {
            char buffer[BUFSIZ];
            if( ( rc1 = ::read(src, buffer, sizeof(buffer)) ) > 0 ) {
                ssize_t bytes_to_write = rc1;
                size_t buffer_offset = 0;
                while( 0 < bytes_to_write ) { // write the read chunk, allowing potential multiple write-ops
                    if( ( rc2 = ::write(dst, buffer+buffer_offset, bytes_to_write) ) < 0 ) {
                        break;
                    }
                    buffer_offset += rc2;
                    bytes_to_write -= rc2;
                    offset += rc2;
                }
            }
        }
        if ( 0 > rc1 || 0 > rc2 ) {
            if( is_set(copts, copy_options::verbose) ) {
                if constexpr ( _use_sendfile ) {
                    jau::fprintf_td(stderr, "copy: Error: Failed to copy bytes @ %s / %s, %s -> '%s', errno %d, %s\n",
                            jau::to_decstring(offset).c_str(), jau::to_decstring(source_stats.size()).c_str(),
                            source_stats.to_string().c_str(), dest_path.c_str(), errno, ::strerror(errno));
                } else if ( 0 > rc1 ) {
                    jau::fprintf_td(stderr, "copy: Error: Failed to read bytes @ %s / %s, %s, errno %d, %s\n",
                            jau::to_decstring(offset).c_str(), jau::to_decstring(source_stats.size()).c_str(),
                            source_stats.to_string().c_str(), errno, ::strerror(errno));
                } else if ( 0 > rc2 ) {
                    jau::fprintf_td(stderr, "copy: Error: Failed to write bytes @ %s / %s, %s, errno %d, %s\n",
                            jau::to_decstring(offset).c_str(), jau::to_decstring(source_stats.size()).c_str(),
                            dest_path.c_str(), errno, ::strerror(errno));
                }
            }
            goto errout;
        }
        if ( 0 == rc1 ) {
            break;
        }
    }
    if( offset < source_stats.size() ) {
        if( is_set(copts, copy_options::verbose) ) {
            jau::fprintf_td(stderr, "copy: Error: Incomplete transfer %s / %s, %s -> '%s', errno %d, %s\n",
                    jau::to_decstring(offset).c_str(), jau::to_decstring(source_stats.size()).c_str(),
                    source_stats.to_string().c_str(), dest_path.c_str(), errno, ::strerror(errno));
        }
        goto errout;
    }
    res = true;
    if( omitted_permissions != fmode_t::none ) {
        // restore omitted permissions
        if( 0 != ::fchmod(dst, jau::fs::posix_protection_bits( dest_mode  )) ) {
            if( is_set(copts, copy_options::verbose) ) {
                jau::fprintf_td(stderr, "copy: Error: Couldn't restore omitted permissions, source %s, dest '%s', errno %d (%s)\n",
                        source_stats.to_string().c_str(), dest_path.c_str(), errno, ::strerror(errno));
            }
            res = false;
        }
    }
    if( is_set(copts, copy_options::preserve_all) ) {
        // preserve time
        struct timespec ts2[2] = { target_stats->atime().to_timespec(), target_stats->mtime().to_timespec() };
        if( 0 != ::futimens(dst, ts2) ) {
            if( is_set(copts, copy_options::verbose) ) {
                jau::fprintf_td(stderr, "copy: Error: Couldn't preserve time of file, source %s, dest '%s', errno %d (%s)\n",
                        source_stats.to_string().c_str(), dest_path.c_str(), errno, ::strerror(errno));
            }
            res = false;
        }
        // preserve ownership
        ::uid_t source_uid = 0 == caller_uid ? target_stats->uid() : -1;
        if( 0 != ::fchown(dst, source_uid, target_stats->gid()) ) {
            if( errno != EPERM && errno != EINVAL ) { // OK to fail due to permissions
                if( is_set(copts, copy_options::verbose) ) {
                    jau::fprintf_td(stderr, "copy: Error: Couldn't preserve ownership of file, source %s, dest '%s', errno %d (%s)\n",
                            source_stats.to_string().c_str(), dest_path.c_str(), errno, ::strerror(errno));
                }
                res = false;
            }
        }
    }
    if( is_set(copts, copy_options::sync) ) {
        if( 0 != ::fsync(dst) ) {
            if( is_set(copts, copy_options::verbose) ) {
                jau::fprintf_td(stderr, "copy: Error: Couldn't synchronize destination file, source %s, dest '%s', errno %d (%s)\n",
                        source_stats.to_string().c_str(), dest_path.c_str(), errno, ::strerror(errno));
            }
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

static bool copy_mkdir(const file_stats& source_stats, const file_stats& dest_stats, const copy_options copts) noexcept {
    (void)source_stats;
    const std::string dest_path = dest_stats.path();

    if( dest_stats.is_dir() ) {
        if( is_set(copts, copy_options::verbose) ) {
            jau::fprintf_td(stderr, "copy: mkdir directory already exist: %s\n", dest_stats.to_string().c_str());
        }
    } else if( !dest_stats.exists() ) {
        const int dir_err = ::mkdir(dest_path.c_str(), posix_protection_bits(fmode_t::rwx_usr));
        if ( 0 != dir_err ) {
            if( is_set(copts, copy_options::verbose) ) {
                jau::fprintf_td(stderr, "copy: mkdir failed: %s, errno %d (%s)\n", dest_stats.to_string().c_str(), errno, strerror(errno));
            }
            return false;
        }
    } else {
        if( is_set(copts, copy_options::verbose) ) {
            jau::fprintf_td(stderr, "copy: mkdir failed: %s, exists but is no dir\n", dest_stats.to_string().c_str());
        }
        return false;
    }
    return true;
}

static bool copy_dir_preserve(const file_stats& source_stats, const file_stats& dest_stats, const copy_options copts) noexcept {
    const std::string dest_path = dest_stats.path();
    if( !dest_stats.is_dir() ) {
        if( is_set(copts, copy_options::verbose) ) {
            jau::fprintf_td(stderr, "copy: dir_preserve failed: %s, is no dir\n", dest_stats.to_string().c_str());
        }
        return false;
    }
    const file_stats* target_stats = source_stats.is_link() ? source_stats.link_target().get() : &source_stats;

    // restore permissions
    const fmode_t dest_mode = target_stats->prot_mode();
    if( 0 != ::fchmodat(AT_FDCWD, dest_path.c_str(), jau::fs::posix_protection_bits( dest_mode ), 0) ) {
        if( is_set(copts, copy_options::verbose) ) {
            jau::fprintf_td(stderr, "copy: Error: dir_preserve restore permissions, source %s, dest '%s', errno %d (%s)\n",
                    source_stats.to_string().c_str(), dest_path.c_str(), errno, ::strerror(errno));
        }
        return false;
    }

    if( is_set(copts, copy_options::preserve_all) ) {
        // preserve time
        struct timespec ts2[2] = { target_stats->atime().to_timespec(), target_stats->mtime().to_timespec() };
        if( 0 != ::utimensat(AT_FDCWD, dest_path.c_str(), ts2, 0) ) {
            if( is_set(copts, copy_options::verbose) ) {
                jau::fprintf_td(stderr, "copy: Error: dir_preserve time of file failed, source %s, dest '%s', errno %d (%s)\n",
                        source_stats.to_string().c_str(), dest_stats.to_string().c_str(), errno, ::strerror(errno));
            }
            return false;
        }
        // preserve ownership
        const uid_t caller_uid = getuid();
        const ::uid_t source_uid = 0 == caller_uid ? target_stats->uid() : -1;
        if( 0 != ::chown( dest_path.c_str(), source_uid, target_stats->gid() ) ) {
            if( errno != EPERM && errno != EINVAL ) { // OK to fail due to permissions
                if( is_set(copts, copy_options::verbose) ) {
                    jau::fprintf_td(stderr, "copy: Error: dir_preserve ownership of file failed, source %s, dest '%s', errno %d (%s)\n",
                            source_stats.to_string().c_str(), dest_stats.to_string().c_str(), errno, ::strerror(errno));
                }
                return false;
            }
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
    bool target_path_is_root = false;

    if( source_stats.is_dir() ) {
        if( !is_set(copts, copy_options::recursive) ) {
            if( is_set(copts, copy_options::verbose) ) {
                jau::fprintf_td(stderr, "copy: Error: source_path is dir but !recursive, %s\n", source_stats.to_string().c_str());
            }
            return false;
        }
        if( !target_stats.exists() ) {
            target_stats = file_stats(target_stats.path()); // update
            target_path_is_root = true;
        } else if( !target_stats.is_dir() ) {
            if( is_set(copts, copy_options::verbose) ) {
                jau::fprintf_td(stderr, "copy: Error: source_path is dir but target_path not, source %s, target %s\n",
                        source_stats.to_string().c_str(), target_stats.to_string().c_str());
            }
            return false;
        }
    } else if( source_stats.is_file() ) {
        if( target_stats.exists() ) {
            if( target_stats.is_file() ) {
                if( !is_set(copts, copy_options::overwrite)) {
                    if( is_set(copts, copy_options::verbose) ) {
                        jau::fprintf_td(stderr, "copy: Error: source_path is file, target_path existing file w/o overwrite, source %s, target %s\n",
                                source_stats.to_string().c_str(), target_stats.to_string().c_str());
                    }
                    return false;
                }
                target_path_is_root = true;
            }
        } else {
            // else new file2file
            target_path_is_root = true;
        }
    } else {
        if( is_set(copts, copy_options::verbose) ) {
            jau::fprintf_td(stderr, "copy: Error: source_path is neither file nor dir, source %s, target %s\n",
                    source_stats.to_string().c_str(), target_stats.to_string().c_str());
        }
        return false;
    }
    struct copy_context_t {
            file_stats source_stats;
            size_t source_path_len;
            std::string source_basename;
            file_stats target_stats;
            copy_options copts;
            bool target_path_is_root;
    };
    copy_context_t copy_context { source_stats, source_path.size(), basename(source_path), target_stats, copts, target_path_is_root };
    const path_visitor pv = jau::bindCaptureRefFunc<bool, copy_context_t, traverse_event, const file_stats&>(&copy_context,
            ( bool(*)(copy_context_t*, traverse_event, const file_stats&) ) /* help template type deduction of function-ptr */
                ( [](copy_context_t* ctx, traverse_event tevt, const file_stats& element_stats) -> bool {
                    if( !element_stats.has_access() ) {
                        if( is_set(ctx->copts, copy_options::verbose) ) {
                            jau::fprintf_td(stderr, "copy: Error: remove failed: no access, %s\n", element_stats.to_string().c_str());
                        }
                        return false;
                    }
                    std::string element_path = element_stats.path();
                    std::string element_path_trail;
                    if( ctx->source_path_len < element_path.size() ) {
                        element_path_trail = _slash + element_path.substr(ctx->source_path_len+1);
                    }
                    std::string target_path_;
                    if( ctx->target_path_is_root ) {
                        target_path_ = ctx->target_stats.path() + element_path_trail;
                    } else {
                        target_path_ = ctx->target_stats.path() + _slash + ctx->source_basename + element_path_trail;
                    }
                    if( is_set(tevt, traverse_event::dir_entry) ) {
                        const file_stats target_stats_(target_path_);
                        if( !copy_mkdir( element_stats, target_stats_, ctx->copts ) ) {
                            return false;
                        }
                    } else if( is_set(tevt, traverse_event::dir_exit) ) {
                        const file_stats target_stats_(target_path_);
                        if( !copy_dir_preserve( element_stats, target_stats_, ctx->copts ) ) {
                            return false;
                        }
                    } else if( is_set(tevt, traverse_event::file) || is_set(tevt, traverse_event::symlink) || is_set(tevt, traverse_event::dir_symlink) ) {
                        if( !copy_file(element_stats, target_path_, ctx->copts) ) {
                            return false;
                        }
                    }
                    return true;
                  } ) );
    return jau::fs::visit(source_stats, topts, pv);
}

jau::fs::mount_ctx jau::fs::mount_image(const std::string& image_path, const std::string& mount_point, const std::string& fs_type,
                                        const unsigned long mountflags, const std::string fs_options)
{
    file_stats image_stats(image_path);
    if( !image_stats.is_file()) {
        return mount_ctx();
    }
    file_stats target_stats(mount_point);
    if( !target_stats.is_dir()) {
        return mount_ctx();
    }
    int backingfile = ::open64(image_stats.path().c_str(), O_RDWR);
    if( 0 > backingfile )  {
        jau::fprintf_td(stderr, "mount: Error: Couldn't open image-file '%s': res %d, errno %d (%s)\n",
                image_stats.to_string().c_str(), backingfile, errno, ::strerror(errno));
        return mount_ctx();
    }
#ifdef __linux__
    int loop_device_id = -1;
    int loop_ctl_fd = -1, loop_device_fd = -1;
    char loopname[4096];
    void* fs_options_cstr = nullptr;
    int mount_res = -1;

    loop_ctl_fd = ::open64("/dev/loop-control", O_RDWR);
    if( 0 > loop_ctl_fd ) {
        jau::fprintf_td(stderr, "mount: Error: Couldn't open loop-control: res %d, errno %d (%s)\n",
                loop_ctl_fd, errno, ::strerror(errno));
        goto errout;
    }

    loop_device_id = (int) ::ioctl(loop_ctl_fd, LOOP_CTL_GET_FREE);
    if( 0 > loop_device_id ) {
        jau::fprintf_td(stderr, "mount: Error: Couldn't get free loop-device: res %d, errno %d (%s)\n",
                loop_device_id, errno, ::strerror(errno));
        goto errout;
    }
    ::close(loop_ctl_fd);
    loop_ctl_fd = -1;

    snprintf(loopname, sizeof(loopname), "/dev/loop%d", loop_device_id);
    jau::fprintf_td(stderr, "mount: Info: Using loop-device '%s'\n", loopname);

    loop_device_fd = ::open64(loopname, O_RDWR);
    if( 0 > loop_device_fd ) {
        jau::fprintf_td(stderr, "mount: Error: Couldn't open loop-device '%s': res %d, errno %d (%s)\n",
                loopname, loop_device_fd, errno, ::strerror(errno));
        goto errout;
    }
    if( 0 > ::ioctl(loop_device_fd, LOOP_SET_FD, backingfile) ) {
        jau::fprintf_td(stderr, "mount: Error: Couldn't attach image-file '%s' to loop-device '%s': errno %d (%s)\n",
                image_stats.to_string().c_str(), loopname, errno, ::strerror(errno));
        goto errout;
    }
    ::close(loop_device_fd);
    loop_device_fd = -1;
    ::close(backingfile);
    backingfile = -1;

    if( fs_options.size() > 0 ) {
        fs_options_cstr = (void*) fs_options.data();
    }
    mount_res = ::mount(loopname, target_stats.path().c_str(), fs_type.c_str(), mountflags, fs_options_cstr);
    if( 0 != mount_res ) {
        jau::fprintf_td(stderr, "mount: Error: source_path %s, target_path %s, fs_type %s, res %d, errno %d (%s)\n",
                image_path.c_str(), mount_point.c_str(), fs_type.c_str(), mount_res, errno, ::strerror(errno));
        ::ioctl(loop_device_fd, LOOP_CLR_FD, 0);
        goto errout;
    }
    return mount_ctx(mount_point, loop_device_id);

errout:
    if( 0 <= loop_ctl_fd ) {
        ::close(loop_ctl_fd);
    }
    if( 0 <= loop_device_fd ) {
        ::close(loop_device_fd);
    }
#else
    (void)fs_type;
    (void)mountflags;
    (void)fs_options;
#endif
    if( 0 <= backingfile ) {
        ::close(backingfile);
    }
    return mount_ctx();
}

bool jau::fs::umount(const mount_ctx& context)
{
    if( !context.mounted) {
        return false;
    }
    file_stats target_stats(context.mount_point);
    if( !target_stats.is_dir()) {
        return false;
    }
    const int umount_res = ::umount(target_stats.path().c_str());
    if( 0 != umount_res ) {
        jau::fprintf_td(stderr, "umount: Error: Couldn't umount '%s': res %d, errno %d (%s)\n",
                target_stats.to_string().c_str(), umount_res, errno, ::strerror(errno));
    }
    if( 0 > context.loop_device_id ) {
        // mounted w/o loop-device, done
        return 0 == umount_res;
    }
#ifdef __linux__
    int loop_device_fd = -1;
    char loopname[4096];

    snprintf(loopname, sizeof(loopname), "/dev/loop%d", context.loop_device_id);
    jau::fprintf_td(stderr, "umount: Info: Using loop-device '%s'\n", loopname);

    loop_device_fd = ::open64(loopname, O_RDWR);
    if( 0 > loop_device_fd ) {
        jau::fprintf_td(stderr, "umount: Error: Couldn't open loop-device '%s': res %d, errno %d (%s)\n",
                loopname, loop_device_fd, errno, ::strerror(errno));
        goto errout;
    }
    if( 0 > ::ioctl(loop_device_fd, LOOP_CLR_FD, 0) ) {
        jau::fprintf_td(stderr, "umount: Error: Couldn't deattach loop-device '%s': errno %d (%s)\n",
                loopname, errno, ::strerror(errno));
        goto errout;
    }
    ::close(loop_device_fd);
    loop_device_fd = -1;

    return 0 == umount_res;

errout:
    if( 0 <= loop_device_fd ) {
        ::close(loop_device_fd);
    }
#endif
    return false;
}

