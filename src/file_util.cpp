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
#endif
    #include <sys/types.h>
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

std::string dir_item::path() const noexcept {
    if( parent_dir_.size() > 0 ) {
        if( element_.size() > 0 ) {
            return parent_dir_ + "/" + element_;
        } else {
            // item shall not be empty
            return parent_dir_;
        }
    } else {
        return element_;
    }
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
: has_fields_(field_t::none), item_(), link_target_(), mode_(fmode_t::not_existing),
  uid_(0), gid_(0), size_(0), btime_(), atime_(), ctime_(), mtime_(),
  errno_res_(0)
{}

static constexpr bool jau_has_stat(const uint32_t mask, const uint32_t bit) { return bit == ( mask & bit ); }

file_stats::file_stats(const ctor_cookie& cc, const dir_item& item, const std::string_view symlink) noexcept
: has_fields_(field_t::none), item_(item), link_target_(), mode_(fmode_t::none),
  uid_(0), gid_(0), size_(0), btime_(), atime_(), ctime_(), mtime_(),
  errno_res_(0)
{
    (void)cc;
    const bool follow_symlink = symlink.size() > 0;
    const std::string path(follow_symlink ? symlink : item_.path());

if constexpr ( _use_statx ) {
    struct ::statx s;
    ::bzero(&s, sizeof(s));
    int stat_res = ::statx(AT_FDCWD, path.c_str(),
                           ( AT_NO_AUTOMOUNT | ( follow_symlink ? 0 : AT_SYMLINK_NOFOLLOW ) ),
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
        if( is_link() && !follow_symlink ) {
            // follow symbolic link (only once), retrieve info from underlying file
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
            link_target_ = std::make_shared<file_stats>(ctor_cookie(0), dir_item(link_path), path /* symlink */);
            if( link_target_->is_file() ) {
                mode_ |= fmode_t::file;
                if( !link_target_->is_link() ) {
                    has_fields_ |= field_t::size;
                    size_ = link_target_->size();
                }
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
    int stat_res = follow_symlink ? ::stat64(path.c_str(), &s) : ::lstat64(path.c_str(), &s);
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
                      field_t::atime | field_t::ctime | field_t::mtime | field_t::size;

        if( S_ISLNK( s.st_mode ) ) {
            mode_ |= fmode_t::link;
        }
        if( S_ISREG( s.st_mode ) ) {
            mode_ |= fmode_t::file;
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

        if( is_link() && !follow_symlink ) {
            // follow symbolic link (only once), retrieve info from underlying file
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
            link_target_ = std::make_shared<file_stats>(ctor_cookie(0), dir_item(link_path), path /* symlink */);
            if( link_target_->is_file() ) {
                mode_ |= fmode_t::file;
                if( !link_target_->is_link() ) {
                    has_fields_ |= field_t::size;
                    size_ = link_target_->size();
                }
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
: file_stats(ctor_cookie(0), item, std::string_view() /* symlink */)
{}

file_stats::file_stats(const std::string& _path) noexcept
: file_stats(ctor_cookie(0), dir_item(_path), std::string_view() /* symlink */)
{}

bool file_stats::has(const field_t fields) const noexcept {
    return fields == ( has_fields_ & fields );
}

std::string file_stats::to_string(const bool use_space) const noexcept {
    const std::string link_detail = is_link() ? " -> '" + link_target_->path() + "'" : "";
    std::string res( "file_stats[");
    res.append(jau::fs::to_string(mode_))
       .append(", '"+item_.path()+"'"+link_detail );
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

std::string jau::fs::dirname(const std::string_view& path) noexcept {
    if( 0 == path.size() ) {
        return std::string();
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
        return ".";
    } else {
        return std::string( path.substr(0, idx) );
    }
}

std::string jau::fs::basename(const std::string_view& path) noexcept {
    if( 0 == path.size() ) {
        return std::string();
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

std::string jau::fs::get_cwd() noexcept {
    char path[PATH_MAX];
    char* res = ::getcwd(path, PATH_MAX);
    if( res == path ) {
        return std::string(path);
    } else {
        return std::string();
    }
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
            if( "." != fname && ".." != fname ) { // avoid '.' and '..'
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
