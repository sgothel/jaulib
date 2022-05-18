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
#include <cinttypes>

#include <jau/debug.hpp>
#include <jau/file_util.hpp>

extern "C" {
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <dirent.h>
    #include <fcntl.h>
    #include <unistd.h>
}

using namespace jau;
using namespace jau::fs;

#define CASE2_TO_STRING(U,V) case U::V: return #V;

#define FMODE_BIT_ENUM(X) \
    X(fmode_bits,NONE) \
    X(fmode_bits,FILE) \
    X(fmode_bits,DIR) \
    X(fmode_bits,LINK) \
    X(fmode_bits,NO_ACCESS) \
    X(fmode_bits,NOT_EXISTING)

static std::string _get_fmode_bit_str(const fmode_bits bit) noexcept {
    switch(bit) {
    FMODE_BIT_ENUM(CASE2_TO_STRING)
        default: ; // fall through intended
    }
    return "??? "+jau::to_hexstring(number(bit));
}

std::string jau::fs::to_string(const fmode_bits mask) noexcept {
    std::string out("[");
    bool comma = false;
    if( has_fmode_bit( mask, fmode_bits::FILE )) {
        out.append(_get_fmode_bit_str( fmode_bits::FILE )); comma = true;
    }
    if( has_fmode_bit( mask, fmode_bits::DIR )) {
        if( comma ) { out.append(", "); }
        out.append(_get_fmode_bit_str( fmode_bits::DIR )); comma = true;
    }
    if( has_fmode_bit( mask, fmode_bits::LINK )) {
        if( comma ) { out.append(", "); }
        out.append(_get_fmode_bit_str( fmode_bits::LINK )); comma = true;
    }
    if( has_fmode_bit( mask, fmode_bits::NO_ACCESS )) {
        if( comma ) { out.append(", "); }
        out.append(_get_fmode_bit_str( fmode_bits::NO_ACCESS )); comma = true;
    }
    if( has_fmode_bit( mask, fmode_bits::NOT_EXISTING )) {
        if( comma ) { out.append(", "); }
        out.append(_get_fmode_bit_str( fmode_bits::NOT_EXISTING )); comma = true;
    }
    out.append("]");
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

file_stats::file_stats(const dir_item& item, const bool use_lstat) noexcept
: item_(item), mode_(fmode_bits::NONE),
  uid_(0), gid_(0), size_(0), atime_(), mtime_(), ctime_(),
  errno_res_(0)
{
    struct stat s;
    int stat_res;
    if( use_lstat ) {
        stat_res = ::lstat(item_.path().c_str(), &s);
    } else {
        stat_res = ::stat(item_.path().c_str(), &s);
    }
    if( 0 != stat_res ) {
        switch( errno ) {
            case EACCES:
                mode_ |= fmode_bits::NO_ACCESS;
                break;
            case ENOENT:
                mode_ |= fmode_bits::NOT_EXISTING;
                break;
            default:
                break;
        }
        if( has_access() && exists() ) {
            errno_res_ = errno;
        }
    } else {
        if( S_ISREG( s.st_mode ) ) {
            mode_ |= fmode_bits::FILE;
        }
        if( S_ISDIR( s.st_mode ) ) {
            mode_ |= fmode_bits::DIR;
        }
        if( S_ISLNK( s.st_mode ) ) {
            mode_ |= fmode_bits::LINK;
        }
        uid_ = s.st_uid;
        gid_ = s.st_gid;
        size_ = is_file() ? s.st_size : 0;
        atime_ = jau::fraction_timespec( s.st_atim.tv_sec, s.st_atim.tv_nsec );
        mtime_ = jau::fraction_timespec( s.st_mtim.tv_sec, s.st_mtim.tv_nsec );
        ctime_ = jau::fraction_timespec( s.st_ctim.tv_sec, s.st_ctim.tv_nsec );
    }
}

file_stats::file_stats(const std::string& _path, const bool use_lstat) noexcept
: file_stats(dir_item("", _path), use_lstat)
{}

std::string file_stats::to_string(const bool use_space) const noexcept {
    std::string res( "file_stats['"+item_.path()+"', "+jau::fs::to_string(mode_) );
    if( 0 == errno_res_ ) {
        res.append( ", uid " ).append( std::to_string(uid_) ).append( ", gid " ).append( std::to_string(gid_) )
        .append( ", size " ).append( std::to_string( size_ ) )
        .append( ", atime " ).append( atime_.to_iso8601_string(use_space) )
        .append( ", mtime " ).append( mtime_.to_iso8601_string(use_space) )
        .append( ", ctime " ).append( ctime_.to_iso8601_string(use_space) );
    } else {
        res.append( ", errno " ).append( std::to_string(errno_res_) ).append( ", " ).append( std::string(::strerror(errno_res_)) );
    }
    res.append("]");
    return res;
}

bool jau::fs::mkdir(const std::string& path, const bool verbose) noexcept {
    file_stats fstats(path);

    if( !fstats.ok() ) {
        if( verbose ) {
            jau::fprintf_td(stderr, "mkdir stat failed: %s\n", fstats.to_string().c_str());
        }
        return false;
    }
    if( fstats.is_dir() ) {
        if( verbose ) {
            jau::fprintf_td(stderr, "mkdir failed: %s, dir already exists\n", fstats.to_string().c_str());
        }
        return true;
    } else if( !fstats.exists() ) {
        const int dir_err = ::mkdir(path.c_str(), S_IRWXU | S_IRWXG ); // excluding others
        if ( 0 != dir_err ) {
            if( verbose ) {
                jau::fprintf_td(stderr, "mkdir failed: %s, errno %d (%s)\n", fstats.to_string().c_str(), errno, strerror(errno));
            }
            return false;
        } else {
            return true;
        }
    } else {
        if( verbose ) {
            jau::fprintf_td(stderr, "mkdir failed: %s, exists but is no dir\n", fstats.to_string().c_str());
        }
        return false;
    }
}

bool jau::fs::touch(const std::string& path, const jau::fraction_timespec& atime, const jau::fraction_timespec& mtime, const bool verbose) noexcept {
    const ::mode_t mode = 0666;
    int fd = ::open(path.c_str(), O_WRONLY|O_CREAT|O_NOCTTY|O_NONBLOCK, mode);
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

bool jau::fs::touch(const std::string& path, const bool verbose) noexcept {
    const ::mode_t mode = 0666;
    int fd = ::open(path.c_str(), O_WRONLY|O_CREAT|O_NOCTTY|O_NONBLOCK, mode);
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

bool jau::fs::visit(const file_stats& item_stats, const path_visitor& visitor) noexcept {
    if( item_stats.is_dir() ) {
        std::vector<dir_item> content;
        const consume_dir_item cs = jau::bindCaptureRefFunc<void, std::vector<dir_item>, const dir_item&>(&content,
                ( void(*)(std::vector<dir_item>*, const dir_item&) ) /* help template type deduction of function-ptr */
                    ( [](std::vector<dir_item>* receiver, const dir_item& item) -> void { receiver->push_back( item ); } )
            );
        if( get_dir_content(item_stats.path(), cs) && content.size() > 0 ) {
            for (const dir_item& element : content) {
                const file_stats element_stats( element );
                if( !element_stats.ok() || !element_stats.exists() ) {
                    continue;
                } else if( element_stats.is_dir() ) {
                    if( !jau::fs::visit(element_stats, visitor) ) {
                        return false;
                    }
                } else if( element_stats.is_file() || element_stats.is_link() || !element_stats.has_access() ) {
                    if( !visitor( element_stats ) ) {
                        return false;
                    }
                }
            }
        }
    }
    return visitor( item_stats );
}

bool jau::fs::visit(const std::string& path, const path_visitor& visitor) noexcept {
    return jau::fs::visit(file_stats(path), visitor);
}

bool jau::fs::remove(const std::string& path, const bool recursive, const bool verbose) noexcept {
    file_stats path_stats(path);
    if( path_stats.is_dir() && !recursive ) {
        if( verbose ) {
            jau::fprintf_td(stderr, "remove: Error: path is dir but !recursive, %s\n", path_stats.to_string().c_str());
        }
        return false;
    }
    const path_visitor pv = jau::bindCaptureRefFunc<bool, const bool, const file_stats&>(&verbose,
            ( bool(*)(const bool*, const file_stats&) ) /* help template type deduction of function-ptr */
                ( [](const bool* _verbose, const file_stats& element_stats) -> bool {
                    if( !element_stats.has_access() ) {
                        if( *_verbose ) {
                            jau::fprintf_td(stderr, "remove: Error: remove failed: no access, %s\n", element_stats.to_string().c_str());
                        }
                        return false;
                    }
                    const int res = ::remove( element_stats.path().c_str() );
                    if( 0 != res ) {
                        if( *_verbose ) {
                            jau::fprintf_td(stderr, "remove: Error: remove failed: %s, res %d, errno %d (%s)\n",
                                    element_stats.to_string().c_str(), res, errno, strerror(errno));
                        }
                        return false;
                    } else {
                        return true;
                    }
                  } ) );
    return jau::fs::visit(path_stats, pv);
}
