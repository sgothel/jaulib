/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021-2023 Gothel Software e.K.
 *
 * ByteStream, ByteInStream_SecMemory and ByteStream_istream are derived from Botan under same license:
 * - Copyright (c) 1999-2007 Jack Lloyd
 * - Copyright (c) 2005 Matthew Gregan
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

// #include <botan_all.h>

#include <cstddef>
#include <cstring>
#include <limits>
#include <jau/cpuid.hpp>
#include <jau/debug.hpp>

#include <jau/io/byte_stream.hpp>
#include <jau/io/io_util.hpp>

extern "C" {
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
}

#ifdef USE_LIBCURL
    #include <curl/curl.h>
#endif

#include <thread>
#include <pthread.h>

#ifndef O_BINARY
#define O_BINARY    0
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK  0
#endif

using namespace jau::io;
using namespace jau::fractions_i64_literals;
using namespace jau::int_literals;

#if defined(__FreeBSD__)
    typedef off_t off64_t;
    #define __posix_openat64 ::openat
    #define __posix_lseek64  ::lseek
#else
    #define __posix_openat64 ::openat64
    #define __posix_lseek64  ::lseek64
#endif

#ifdef USE_LIBCURL
    const size_t jau::io::BEST_URLSTREAM_RINGBUFFER_SIZE = 2_uz * (size_t)CURL_MAX_WRITE_SIZE;
#else
    const size_t jau::io::BEST_URLSTREAM_RINGBUFFER_SIZE = 2_uz * 16384_uz;
#endif

inline constexpr static void copy_mem(void* out, const void* in, size_t n) noexcept {
    if(in != nullptr && out != nullptr && n > 0) {
        std::memcpy(out, in, sizeof(uint8_t)*n);
    }
}

[[nodiscard]] bool ByteStream::read(uint8_t& out) noexcept {
    return 1 == read(&out, 1);
}

[[nodiscard]] bool ByteStream::peek(uint8_t& out) noexcept {
    return 1 == peek(&out, 1, 0);
}

size_t ByteStream::discardRead(size_t n) noexcept {
    if( !good() ) {
        return 0;
    }
    uint8_t buf[1024] = { 0 };
    size_t discarded = 0;

    while(n)
    {
        const size_t got = read(buf, std::min(n, sizeof(buf)));
        if( 0 == got ) {
            break;
        }
        discarded += got;
        n -= got;
    }
    return discarded;
}

[[nodiscard]] uint64_t ByteStream_SecMemory::seek(uint64_t newPos) noexcept {
    PRAGMA_DISABLE_WARNING_PUSH
    PRAGMA_DISABLE_WARNING_TYPE_RANGE_LIMIT
    if( fail() ) {
        return ByteStream::npos;
    } else if( newPos > m_source.size() || newPos > std::numeric_limits<size_t>::max() ) {
        return ByteStream::npos;
    }
    PRAGMA_DISABLE_WARNING_POP
    clearStateFlags(iostate_t::eofbit);
    m_offset = static_cast<size_t>(newPos);
    if( m_mark > newPos ) {
        m_mark = npos;
    }
    if( m_source.size() == m_offset ) {
        addstate_impl( iostate_t::eofbit );
    }
    return m_offset;
}

[[nodiscard]] size_t ByteStream_SecMemory::discard(size_t N) noexcept {
    if( !good() ) {
        return 0;
    }
    size_t n = std::min(N, m_source.size() - m_offset);
    m_offset += n;
    if( m_source.size() == m_offset ) {
        addstate_impl( iostate_t::eofbit );
    }
    return n;
}

[[nodiscard]] bool ByteStream_SecMemory::setMark(uint64_t) noexcept {
    m_mark = m_offset;
    return true;
}

[[nodiscard]] bool ByteStream_SecMemory::seekMark() noexcept {
    if( npos == m_mark ) {
        return false;
    }
    return m_mark == seek(m_mark);
}

size_t ByteStream_SecMemory::read(void* out, size_t length) noexcept {
    if( 0 == length || !good() ) {
        return 0;
    }
    const size_t got = std::min<size_t>(m_source.size() - m_offset, length);
    copy_mem(out, m_source.data() + m_offset, got);
    m_offset += got;
    if( m_source.size() == m_offset ) {
        addstate_impl( iostate_t::eofbit );
    }
    return got;
}

bool ByteStream_SecMemory::available(size_t n) noexcept {
    return m_source.size() - m_offset >= n;
}

size_t ByteStream_SecMemory::peek(void* out, size_t length, uint64_t peek_offset) noexcept {
    PRAGMA_DISABLE_WARNING_PUSH
    PRAGMA_DISABLE_WARNING_TYPE_RANGE_LIMIT
    const size_t bytes_left = m_source.size() - m_offset;
    if( 0 == length || !good() ||
        ( peek_offset > std::numeric_limits<size_t>::max() ) ||
        ( bytes_left < peek_offset + 1 /* min number of bytes to read */ ) ) {
        return 0;
    }
    PRAGMA_DISABLE_WARNING_POP
    const size_t po_sz = static_cast<size_t>(peek_offset);
    const size_t peek_len = std::min(bytes_left - po_sz, length);
    copy_mem(out, &m_source[m_offset + po_sz], peek_len);
    return peek_len;
}

[[nodiscard]] size_t ByteStream_SecMemory::write(const void* out, size_t length) noexcept {
    if( 0 == length || fail() ) {
        return 0;
    }
    const size_t got = std::min<size_t>(m_source.size() - m_offset, length);
    copy_mem(m_source.data() + m_offset, out, got);
    m_offset += got;
    return got;
}

ByteStream_SecMemory::ByteStream_SecMemory(const std::string& in)
: ByteStream(iomode_t::read),
  m_source(cast_char_ptr_to_uint8(in.data()),
           cast_char_ptr_to_uint8(in.data()) + in.length()),
  m_offset(0), m_mark(npos)
{ }

void ByteStream_SecMemory::close() noexcept {
    m_source.clear();
    m_offset = 0;
    addstate_impl( iostate_t::eofbit );
}

std::string ByteStream_SecMemory::toString() const noexcept {
    return "ByteInStream_SecMemory[content size "+jau::to_decstring(m_source.size())+
                            ", consumed "+jau::to_decstring(m_offset)+
                            ", available "+jau::to_decstring(m_source.size()-m_offset)+
                            ", iomode["+jau::io::to_string(m_iomode)+
                            ", iostate["+jau::io::to_string(rdstate())+
                            "]]";
}

[[nodiscard]] uint64_t ByteStream_File::seek(uint64_t newPos) noexcept {
    if( fail() ||
        !m_has_content_length ||
        ( !canWrite() && newPos > m_content_size ) ||
        newPos > std::numeric_limits<off64_t>::max() )
    {
        return ByteStream::npos;
    }
    clearStateFlags(iostate_t::eofbit);
    if( newPos != m_offset ) {
        off64_t abs_pos = __posix_lseek64(m_fd, static_cast<off64_t>(newPos), SEEK_SET);
        if( 0 > abs_pos ) {
            addstate_impl( iostate_t::failbit );
            ERR_PRINT("Failed to seek to position %" PRIu64 " of existing file %s, errno %d %s",
                    newPos, toString().c_str(), errno, strerror(errno));
            return ByteStream::npos;
        }
        m_offset = abs_pos;
        if( m_mark > m_offset ) {
            m_mark = npos;
        }
        if( m_content_size == m_offset ) {
            addstate_impl( iostate_t::eofbit );
        }
    }
    return m_offset;
}

[[nodiscard]] size_t ByteStream_File::discard(size_t N) noexcept {
    if( m_has_content_length ) {
        if( !good() ) {
            return 0;
        }
        const size_t n = std::min(N, m_content_size - m_offset);
        const size_t p0 = m_offset;
        return seek(p0 + n) - p0;
    } else {
        return discardRead(N);
    }
}

[[nodiscard]] bool ByteStream_File::setMark(uint64_t) noexcept {
    m_mark = m_offset;
    return true;
}

[[nodiscard]] bool ByteStream_File::seekMark() noexcept {
    if( npos == m_mark ) {
        return false;
    }
    return m_mark == seek(m_mark);
}

[[nodiscard]] size_t ByteStream_File::read(void* out, size_t length) noexcept {
    if( 0 == length || !good() ) {
        return 0;
    }
    uint8_t* out_u8 = static_cast<uint8_t*>(out);
    size_t total = 0;
    while( total < length ) {
        ssize_t len;
        while ( ( len = ::read(m_fd, out_u8+total, length-total) ) < 0 ) {
            if ( errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR ) {
                // cont temp unavail or interruption
                // unlikely for regular files and we open w/o O_NONBLOCK
                // - EAGAIN (file) and EWOULDBLOCK (socket) if blocking
                // - EINTR (signal)
                continue;
            }
            // Check errno == ETIMEDOUT ??
            addstate_impl( iostate_t::failbit );
            DBG_PRINT("ByteInStream_File::read: Error occurred in %s, errno %d %s", toString().c_str(), errno, strerror(errno));
            return 0;
        }
        total += static_cast<size_t>(len);
        if( 0 == len || ( m_has_content_length && m_offset + total >= m_content_size ) ) {
            addstate_impl( iostate_t::eofbit ); // Note: std::istream also sets iostate::failbit on eof, we don't.
            break;
        }
    }
    m_offset += total;
    return total;
}

size_t ByteStream_File::peek(void* out, size_t length, uint64_t peek_offset) noexcept {
    const uint64_t bytes_left = ( m_has_content_length ? m_content_size : std::numeric_limits<off64_t>::max() ) - m_offset;
    if( 0 == length || !good() ||
        ( peek_offset > std::numeric_limits<off64_t>::max() ) ||
        ( bytes_left < peek_offset + 1 /* min number of bytes to read */ ) ) {
        return 0;
    }
    size_t got = 0;

    off64_t abs_pos = 0;
    if( 0 < peek_offset ) {
        abs_pos = __posix_lseek64(m_fd, static_cast<off64_t>(peek_offset), SEEK_CUR);
        if( 0 > abs_pos ) {
            addstate_impl( iostate_t::failbit );
            DBG_PRINT("ByteInStream_File::peek: Error occurred (offset1 %zd) in %s, errno %d %s",
                    peek_offset, toString().c_str(), errno, strerror(errno));
            return 0;
        }
    }
    if( abs_pos == static_cast<off64_t>(peek_offset) ) {
        ssize_t len = 0;
        while ( ( len = ::read(m_fd, out, length) ) < 0 ) {
            if ( errno == EAGAIN || errno == EINTR ) {
                // cont temp unavail or interruption
                continue;
            }
            // Check errno == ETIMEDOUT ??
            addstate_impl( iostate_t::failbit );
            DBG_PRINT("ByteInStream_File::peak: Error occurred (read) in %s, errno %d %s", toString().c_str(), errno, strerror(errno));
            return 0;
        }
        got = len; // potentially zero bytes, i.e. eof
    }
    if( __posix_lseek64(m_fd, static_cast<off64_t>(m_offset), SEEK_SET) < 0 ) {
        // even though we were able to fetch the desired data above, let's fail if position reset fails
        addstate_impl( iostate_t::failbit );
        DBG_PRINT("ByteInStream_File::peek: Error occurred (offset2 %zd) in %s, errno %d %s",
                peek_offset, toString().c_str(), errno, strerror(errno));
        return 0;
    }
    return got;
}

bool ByteStream_File::available(size_t n) noexcept {
    return isOpen() && good() && ( !m_has_content_length || m_content_size - m_offset >= (uint64_t)n );
}

size_t ByteStream_File::write(const void* out, size_t length) noexcept {
    if( 0 == length || fail() ) {
        return 0;
    }
    const uint8_t* out_u8 = static_cast<const uint8_t*>(out);
    size_t total = 0;
    while( total < length ) {
        ssize_t len;
        while ( ( len = ::write(m_fd, out_u8+total, length-total) ) < 0 ) {
            if ( errno == EAGAIN || errno == EINTR ) {
                // cont temp unavail or interruption
                // unlikely for regular files and we open w/o O_NONBLOCK
                // - EAGAIN (file) and EWOULDBLOCK (socket) if blocking
                // - EINTR (signal)
                continue;
            }
            // Check errno == ETIMEDOUT ??
            addstate_impl( iostate_t::failbit );
            DBG_PRINT("ByteOutStream_File::write: Error occurred in %s, errno %d %s", toString().c_str(), errno, strerror(errno));
            return 0;
        }
        total += static_cast<size_t>(len);
        if( 0 == len ) {
            addstate_impl( iostate_t::failbit);
            break;
        }
    }
    m_offset += total;
    if( m_has_content_length && m_offset > m_content_size ) {
        m_content_size = m_offset;
        addstate_impl( iostate_t::eofbit );
    }
    return total;
}

static bool _jau_file_size(const int fd, const jau::io::fs::file_stats& stats, const off64_t cur_pos, uint64_t& len) noexcept {
    if( stats.has( jau::io::fs::file_stats::field_t::size ) ) {
        len = stats.size();
        return true;
    }
    if( !stats.is_file() ) {
        return false; // possible pipe or stdin etc
    }
    // should have been covered by stats, but try traditionally harder
    const off64_t end = __posix_lseek64(fd, 0, SEEK_END);
    if( 0 > end ) {
        return false;
    }
    const off64_t cur_pos2 = __posix_lseek64(fd, cur_pos, SEEK_SET);
    if( cur_pos2 != cur_pos ) {
        DBG_PRINT("ByteInStream_File::file_size: Error rewinding to current position failed, orig-pos %" PRIi64 " -> %" PRIi64 ", errno %d %s.",
                  cur_pos, cur_pos2,
                  errno, strerror(errno));
        return false;
    }
    len = uint64_t(end) + 1_u64;
    return true;
}

ByteStream_File::ByteStream_File(const int fd, iomode_t mode) noexcept
: ByteStream(mode),
  m_stats(fd), m_fd(-1), m_has_content_length(false), m_content_size(0),
  m_offset(0), m_mark(npos)
{
    if( !m_stats.exists() || !m_stats.has_access() ) {
        addstate_impl( iostate_t::failbit ); // Note: conforming with std::ifstream open
        DBG_PRINT("ByteInStream_File::ctor: Error, not an existing or accessible file in %s, %s", m_stats.toString().c_str(), toString().c_str());
        return;
    }
    m_fd = ::dup(fd);
    if ( 0 > m_fd ) {
        addstate_impl( iostate_t::failbit ); // Note: conforming with std::ifstream open
        DBG_PRINT("ByteInStream_File::ctor: Error occurred in %s, %s", m_stats.toString().c_str(), toString().c_str());
        return;
    }
    const off64_t cur_pos = __posix_lseek64(m_fd, 0, SEEK_CUR);
    if( 0 > cur_pos ) {
        addstate_impl( iostate_t::failbit );
        ERR_PRINT("Failed to read position of existing file %s, errno %d %s",
                toString().c_str(), errno, strerror(errno));
        return;
    }
    m_offset = cur_pos;
    if( _jau_file_size(m_fd, m_stats, cur_pos, m_content_size) ) {
        m_has_content_length = true;
    }
}

ByteStream_File::ByteStream_File(const int dirfd, const std::string& path, iomode_t iomode, const jau::io::fs::fmode_t fmode) noexcept
: ByteStream(iomode),
  m_stats(), m_fd(-1), m_has_content_length(false), m_content_size(0),
  m_offset(0), m_mark(npos)
{
    if( jau::io::uri_tk::is_local_file_protocol(path) ) {
        // cut of leading `file://`
        std::string path2 = path.substr(7);
        m_stats = jau::io::fs::file_stats(dirfd, path2);
    } else {
        m_stats = jau::io::fs::file_stats(dirfd, path);
    }
    if( !canRead() && !canWrite() ) {
        addstate_impl( iostate_t::failbit ); // Note: conforming with std::ifstream open
        DBG_PRINT("ByteStream_File::ctor: Error, iomode_t invalid: %s, %s", to_string(m_iomode).c_str(), toString().c_str());
        return;
    }
    if( ( m_stats.exists() && !m_stats.is_file() && !m_stats.has_fd() ) || !m_stats.has_access() ) {
        addstate_impl( iostate_t::failbit ); // Note: conforming with std::ofstream open (?)
        DBG_PRINT("ByteStream_File::ctor: Error, an existing non[file, fd] or not accessible element in %s, %s", m_stats.toString().c_str(), toString().c_str());
        return;
    }
    if( !canWrite() && !m_stats.exists() ) {
        addstate_impl( iostate_t::failbit ); // Note: conforming with std::ifstream open
        DBG_PRINT("ByteStream_File::ctor: Error, can't open non-existing read-only file in %s, %s", m_stats.toString().c_str(), toString().c_str());
        return;
    }
    bool truncated = false;
    bool just_opened = false;
    if( m_stats.has_fd() ) {
        m_fd = ::dup( m_stats.fd() );
    } else {
        // O_NONBLOCK, is useless on files and counter to this class logic
        int flags = O_BINARY|O_NOCTTY;
        if( canRead() && !canWrite() ) {
            flags |= O_RDONLY;
        } else if( !canRead() && canWrite() ) {
            flags |= O_WRONLY;
        } else {
            flags |= O_RDWR;
        }
        if( !m_stats.exists() ) {
            flags |= O_CREAT;
        } else if( canWrite() && is_set(m_iomode, iomode_t::trunc) ) {
            flags |= O_TRUNC;
            truncated = true;
        }
        m_fd = __posix_openat64(dirfd, m_stats.path().c_str(), flags, jau::io::fs::posix_protection_bits( fmode ));
        just_opened = true;
    }
    if ( 0 > m_fd ) {
        addstate_impl( iostate_t::failbit ); // Note: conforming with std::ifstream open
        DBG_PRINT("ByteInStream_File::ctor: Error while opening %s, %s", m_stats.toString().c_str(), toString().c_str());
        return;
    }
    if( truncated ) {
        // m_content_size = 0, m_offset = 0
        return;
    }
    const off64_t cur_pos = just_opened || !m_stats.is_file() ? 0 : __posix_lseek64(m_fd, 0, SEEK_CUR);
    if( 0 > cur_pos ) {
        addstate_impl( iostate_t::failbit );
        ERR_PRINT("Failed to read position of existing file %s, errno %d %s",
                toString().c_str(), errno, strerror(errno));
        return;
    }
    m_offset = cur_pos;
    if( _jau_file_size(m_fd, m_stats, cur_pos, m_content_size) ) {
        m_has_content_length = true;
    }
    if( m_stats.is_file() && is_set(m_iomode, iomode_t::atend) ) {
        const off64_t end = __posix_lseek64(m_fd, 0, SEEK_END);
        if( 0 > end ) {
            addstate_impl( iostate_t::failbit );
            ERR_PRINT("Failed to position existing file to end %s, errno %d %s",
                    toString().c_str(), errno, strerror(errno));
            return;
        }
        m_offset = end;
    }
}

ByteStream_File::ByteStream_File(const std::string& path, iomode_t mode, const jau::io::fs::fmode_t fmode) noexcept
: ByteStream_File(AT_FDCWD, path, mode, fmode) {}

void ByteStream_File::close() noexcept {
    if( 0 <= m_fd ) {
        ::close(m_fd);
        m_fd = -1;
        addstate_impl( iostate_t::eofbit );
    }
}

void ByteStream_File::flush() noexcept {
    if( 0 <= m_fd && canWrite() ) {
        ::fdatasync(m_fd);
    }
}

std::string ByteStream_File::toString() const noexcept {
    return "ByteInStream_File[content_length "+( has_content_size() ? jau::to_decstring(m_content_size) : "n/a" )+
                            ", consumed "+jau::to_decstring(m_offset)+
                            ", available "+jau::to_decstring(get_available())+
                            ", fd "+std::to_string(m_fd)+
                            ", iomode["+jau::io::to_string(m_iomode)+
                            ", iostate["+jau::io::to_string(rdstate())+
                            "], "+m_stats.toString()+
                            "]";
}


ByteInStream_URL::ByteInStream_URL(std::string url, const jau::fraction_i64& timeout) noexcept
: ByteStream(iomode_t::read),
  m_url(std::move(url)), m_timeout(timeout), m_buffer(BEST_URLSTREAM_RINGBUFFER_SIZE),
  m_stream_resp( read_url_stream_async(nullptr, m_url, /*httpPostReq=*/nullptr, &m_buffer, AsyncStreamConsumerFunc()) ),
  m_offset(0), m_mark(npos), m_rewindbuf()
{ }

void ByteInStream_URL::close() noexcept {
    DBG_PRINT("ByteInStream_URL: close.0 %s, %s", id().c_str(), to_string_int().c_str());

    if( m_stream_resp->processing() ) {
        m_stream_resp->result = io_result_t::SUCCESS; // signal end of streaming
    }

    m_buffer.close( true /* zeromem */); // also unblocks all r/w ops
    if( m_stream_resp->thread.joinable() ) {
        DBG_PRINT("ByteInStream_URL: close.1 %s, %s", id().c_str(), m_buffer.toString().c_str());
        m_stream_resp->thread.join();
    }
    std::thread none;
    m_stream_resp->thread.swap(none);
    DBG_PRINT("ByteInStream_URL: close.X %s, %s", id().c_str(), to_string_int().c_str());
}

bool ByteInStream_URL::available(size_t n) noexcept {
    if( !good() || !m_stream_resp->processing() ) {
        // url thread ended, only remaining bytes in buffer available left
        return m_buffer.size() >= n;
    }
    m_stream_resp->header_resp.wait_until_completion(m_timeout);
    if( m_stream_resp->has_content_length && m_stream_resp->content_length - m_offset < n ) {
        return false;
    }
    // I/O still in progress, we have to poll until data is available or timeout
    // set_eof() unblocks ringbuffer via set_end_of_input(true) permanently, hence blocking call on !m_has_content_length is OK.
    bool timeout_occured;
    const size_t avail = m_buffer.waitForElements(n, m_timeout, timeout_occured);
    if( avail < n ) {
        if( timeout_occured ) {
            addstate_impl( iostate_t::timeout );
            if( m_stream_resp->processing() ) {
                m_stream_resp->result = io_result_t::FAILED;
            }
            m_buffer.interruptWriter();
        }
        return false;
    } else {
        return true;
    }
}

bool ByteInStream_URL::isOpen() const noexcept {
    // url thread has not ended or remaining bytes in buffer available left
    return m_stream_resp->processing() || m_buffer.size() > 0;
}

bool ByteInStream_URL::has_content_size() const noexcept {
    m_stream_resp->header_resp.wait_until_completion(m_timeout);
    return m_stream_resp->has_content_length;
}

[[nodiscard]] uint64_t ByteInStream_URL::seek(uint64_t newPos) noexcept {
    m_stream_resp->header_resp.wait_until_completion(m_timeout);
    const uint64_t length = content_size();
    if( fail() ) {
        return ByteStream::npos;
    } else if( m_rewindbuf.covered(m_mark, newPos) ){
        clearStateFlags(iostate_t::eofbit);
        m_offset = newPos;
        return m_offset;
    } else if( !has_content_size() ) {
        return ByteStream::npos;
    } else if( newPos > length ) {
        return ByteStream::npos;
    } else if(newPos >= m_offset) {
        clearStateFlags(iostate_t::eofbit);
        discardRead(newPos - m_offset);
        return m_offset;
    } else {
        DBG_PRINT("ByteInStream_URL::seek newPos %" PRIu64 "< position %" PRIu64 " not implemented", newPos, m_offset);
        return ByteStream::npos;
    }
}

[[nodiscard]] size_t ByteInStream_URL::discard(size_t N) noexcept {
    return discardRead(N);
}

[[nodiscard]] bool ByteInStream_URL::setMark(uint64_t readLimit) noexcept {
    if( m_rewindbuf.setMark(m_mark, m_offset, readLimit) ) {
        m_mark = m_offset;
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] bool ByteInStream_URL::seekMark() noexcept {
    if( npos == m_mark ) {
        return false;
    }
    return m_mark == seek(m_mark);
}

size_t ByteInStream_URL::read(void* out, size_t length) noexcept {
    m_stream_resp->header_resp.wait_until_completion(m_timeout);
    if( 0 == length || !good() ) {
        return 0;
    }
    return m_rewindbuf.read(m_mark, m_offset, newData, out, length);
}

size_t ByteInStream_URL::peek(void* out, size_t length, size_t peek_offset) noexcept {
    (void)out;
    (void)length;
    (void)peek_offset;
    DBG_PRINT("ByteInStream_URL::peek not implemented");
    return 0;
}

iostate_t ByteInStream_URL::rdstate() const noexcept {
    if ( ( !m_stream_resp->processing() && m_buffer.isEmpty() ) ||
         ( m_stream_resp->has_content_length && m_offset >= m_stream_resp->content_length ) )
    {
        addstate_impl( iostate_t::eofbit );
    }
    if( m_stream_resp->failed() ) {
        addstate_impl( iostate_t::failbit );
    }
    return rdstate_impl();
}

std::string ByteInStream_URL::to_string_int() const noexcept {
    return m_url+", Url[content_length "+( has_content_size() ? jau::to_decstring(m_stream_resp->content_length.load()) : "n/a" )+
                       ", xfered "+jau::to_decstring(m_stream_resp->total_read.load())+
                       ", result "+std::to_string((int8_t)m_stream_resp->result.load())+
           "], consumed "+jau::to_decstring(m_offset)+
           ", available "+jau::to_decstring(get_available())+
           ", iomode["+jau::io::to_string(m_iomode)+
           ", iostate["+jau::io::to_string(rdstate())+
           "], rewind[mark "+std::to_string(m_mark)+", "+m_rewindbuf.toString()+
           "], "+m_buffer.toString();
}
std::string ByteInStream_URL::toString() const noexcept {
    return "ByteInStream_URL["+to_string_int()+"]";
}

std::unique_ptr<ByteStream> jau::io::to_ByteInStream(const std::string& path_or_uri, jau::fraction_i64 timeout) noexcept {
    if( !jau::io::uri_tk::is_local_file_protocol(path_or_uri) &&
         jau::io::uri_tk::protocol_supported(path_or_uri) )
    {
        std::unique_ptr<ByteStream> res = std::make_unique<ByteInStream_URL>(path_or_uri, timeout);
        if( nullptr != res && !res->fail() ) {
            return res;
        }
    }
    std::unique_ptr<ByteStream> res = std::make_unique<ByteStream_File>(path_or_uri, iomode_t::read);
    if( nullptr != res && !res->fail() ) {
        return res;
    }
    return nullptr;
}

ByteInStream_Feed::ByteInStream_Feed(std::string id_name, const jau::fraction_i64& timeout) noexcept
: ByteStream(iomode_t::read),
  m_id(std::move(id_name)), m_timeout(timeout), m_buffer(BEST_URLSTREAM_RINGBUFFER_SIZE),
  m_has_content_length( false ), m_content_size( 0 ), m_total_xfered( 0 ), m_result( io::io_result_t::NONE ),
  m_offset(0)
{ }

void ByteInStream_Feed::close() noexcept {
    DBG_PRINT("ByteInStream_Feed: close.0 %s, %s", id().c_str(), to_string_int().c_str());

    if( io_result_t::NONE == m_result ) {
        m_result = io_result_t::SUCCESS; // signal end of streaming
    }
    m_buffer.close( true /* zeromem */); // also unblocks all r/w ops
    DBG_PRINT("ByteInStream_Feed: close.X %s, %s", id().c_str(), to_string_int().c_str());
}

bool ByteInStream_Feed::available(size_t n) noexcept {
    if( !good() || io_result_t::NONE != m_result ) {
        // feeder completed, only remaining bytes in buffer available left
        return m_buffer.size() >= n;
    }
    if( m_has_content_length && m_content_size - m_offset < n ) {
        return false;
    }
    // I/O still in progress, we have to poll until data is available or timeout
    // set_eof() unblocks ringbuffer via set_end_of_input(true) permanently, hence blocking call on !m_has_content_length is OK.
    bool timeout_occured;
    const size_t avail = m_buffer.waitForElements(n, m_timeout, timeout_occured);
    if( avail < n ) {
        if( timeout_occured ) {
            addstate_impl( iostate_t::timeout );
            if( io_result_t::NONE == m_result ) {
                m_result = io_result_t::FAILED;
            }
            m_buffer.interruptWriter();
        }
        return false;
    } else {
        return true;
    }
}

bool ByteInStream_Feed::isOpen() const noexcept {
    // feeder has not ended or remaining bytes in buffer available left
    return io_result_t::NONE == m_result || m_buffer.size() > 0;
}

[[nodiscard]] uint64_t ByteInStream_Feed::seek(uint64_t newPos) noexcept {
    const uint64_t length = m_content_size;
    if( fail() ) {
        return ByteStream::npos;
    } else if( m_rewindbuf.covered(m_mark, newPos) ){
        clearStateFlags(iostate_t::eofbit);
        m_offset = newPos;
        return m_offset;
    } else if( !has_content_size() ) {
        return ByteStream::npos;
    } else if( newPos > length ) {
        return ByteStream::npos;
    } else if(newPos >= m_offset) {
        clearStateFlags(iostate_t::eofbit);
        discardRead(newPos - m_offset);
        return m_offset;
    } else {
        DBG_PRINT("ByteInStream_Feed::seek newPos %" PRIu64 "< position %" PRIu64 " not implemented", newPos, m_offset);
        return ByteStream::npos;
    }
}

[[nodiscard]] size_t ByteInStream_Feed::discard(size_t N) noexcept {
    return discardRead(N);
}

[[nodiscard]] bool ByteInStream_Feed::setMark(uint64_t readLimit) noexcept {
    if( m_rewindbuf.setMark(m_mark, m_offset, readLimit) ) {
        m_mark = m_offset;
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] bool ByteInStream_Feed::seekMark() noexcept {
    if( npos == m_mark ) {
        return false;
    }
    return m_mark == seek(m_mark);
}

size_t ByteInStream_Feed::read(void* out, size_t length) noexcept {
    if( 0 == length || !good() ) {
        return 0;
    }
    return m_rewindbuf.read(m_mark, m_offset, newData, out, length);
}

size_t ByteInStream_Feed::peek(void* out, size_t length, size_t peek_offset) noexcept {
    (void)out;
    (void)length;
    (void)peek_offset;
    DBG_PRINT("ByteInStream_Feed::peek not implemented");
    return 0;
}

iostate_t ByteInStream_Feed::rdstate() const noexcept {
    if ( ( io_result_t::NONE != m_result && m_buffer.isEmpty() ) ||
         ( m_has_content_length && m_offset >= m_content_size ) )
    {
        addstate_impl( iostate_t::eofbit );
    }
    if( io_result_t::FAILED == m_result ) {
        addstate_impl( iostate_t::failbit );
    }
    return rdstate_impl();
}

size_t ByteInStream_Feed::write(const void* in, size_t length, const jau::fraction_i64& timeout) noexcept {
    if( 0 < length && ( good() && io_result_t::NONE == m_result ) ) { // feeder still running
        bool timeout_occured;
        const uint8_t *in8 = reinterpret_cast<const uint8_t*>(in);
        if( m_buffer.putBlocking(in8, in8+length, timeout, timeout_occured) ) {
            m_total_xfered.fetch_add(length);
            return length;
        } else {
            if( timeout_occured ) {
                addstate_impl( iostate_t::timeout );
                m_buffer.interruptWriter();
            } else {
                addstate_impl( iostate_t::failbit );
            }
            if( io_result_t::NONE == m_result ) {
                m_result = io_result_t::FAILED;
            }
            return 0;
        }
    } else {
        return 0;
    }
}

void ByteInStream_Feed::set_eof(const io_result_t result) noexcept {
    m_result = result;
    m_buffer.set_end_of_input(true); // still considering last data, also irqs blocking ringbuffer reader
}

std::string ByteInStream_Feed::to_string_int() const noexcept {
    return m_id+", ext[content_length "+( has_content_size() ? jau::to_decstring(m_content_size.load()) : "n/a" )+
                   ", xfered "+jau::to_decstring(m_total_xfered.load())+
                   ", result "+std::to_string((int8_t)m_result.load())+
           "], consumed "+std::to_string(m_offset)+
           ", available "+std::to_string(get_available())+
           ", iomode["+jau::io::to_string(m_iomode)+
           ", iostate["+jau::io::to_string(rdstate())+
           "], "+m_buffer.toString();
}

std::string ByteInStream_Feed::toString() const noexcept {
    return "ByteInStream_Feed["+to_string_int()+"]";
}

void ByteStream_Recorder::close() noexcept {
    clear_recording();
    m_parent.close();
    DBG_PRINT("ByteInStream_Recorder: close.X %s", id().c_str());
}

void ByteStream_Recorder::start_recording() noexcept {
    m_buffer.resize(0);
    m_rec_offset = m_offset;
    m_is_recording = true;
}

void ByteStream_Recorder::stop_recording() noexcept {
    m_is_recording = false;
}

void ByteStream_Recorder::clear_recording() noexcept {
    m_is_recording = false;
    m_buffer.clear();
    m_rec_offset = 0;
}

size_t ByteStream_Recorder::read(void* out, size_t length) noexcept {
    const size_t consumed_bytes = m_parent.read(out, length);
    m_offset += consumed_bytes;
    if( is_recording() && consumed_bytes > 0 ) {
        uint8_t* out_u8 = static_cast<uint8_t*>(out);
        m_buffer.insert(m_buffer.end(), out_u8, out_u8+consumed_bytes);
    }
    return consumed_bytes;
}

size_t ByteStream_Recorder::write(const void* out, size_t length) noexcept {
    const size_t consumed_bytes = m_parent.write(out, length);
    m_offset += consumed_bytes;
    if( is_recording() && consumed_bytes > 0 ) {
        const uint8_t* out_u8 = reinterpret_cast<const uint8_t*>(out);
        m_buffer.insert(m_buffer.end(), out_u8, out_u8+consumed_bytes);
    }
    return consumed_bytes;
}

std::string ByteStream_Recorder::toString() const noexcept {
    return "ByteInStream_Recorder[parent "+m_parent.id()+", recording[on "+std::to_string(m_is_recording)+
                            " offset "+jau::to_decstring(m_rec_offset)+
                            "], consumed "+jau::to_decstring(m_offset)+
                            ", iomode["+jau::io::to_string(m_iomode)+
                            ", iostate["+jau::io::to_string(rdstate())+"]]";
}

bool ByteStream::write(const uint8_t& in) noexcept {
    return 1 == write(&in, 1);
}
