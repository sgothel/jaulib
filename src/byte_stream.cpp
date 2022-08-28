/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 *
 * ByteInStream, ByteInStream_SecMemory and ByteInStream_istream are derived from Botan under same license:
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

#include <chrono>

// #include <botan_all.h>

#include <jau/debug.hpp>
#include <jau/byte_stream.hpp>

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

#if defined(__FreeBSD__)
    typedef off_t off64_t;
    #define __posix_openat64 ::openat
    #define __posix_lseek64  ::lseek
#else
    #define __posix_openat64 ::openat64
    #define __posix_lseek64  ::lseek64
#endif

#ifdef USE_LIBCURL
    const size_t jau::io::BEST_URLSTREAM_RINGBUFFER_SIZE = 2*CURL_MAX_WRITE_SIZE;
#else
    const size_t jau::io::BEST_URLSTREAM_RINGBUFFER_SIZE = 2*16384;
#endif

inline constexpr void copy_mem(void* out, const void* in, size_t n) noexcept {
    if(in != nullptr && out != nullptr && n > 0) {
        std::memmove(out, in, sizeof(uint8_t)*n);
    }
}

inline char* cast_uint8_ptr_to_char(uint8_t* b) noexcept {
    return reinterpret_cast<char*>(b);
}

inline const uint8_t* cast_char_ptr_to_uint8(const char* s) noexcept {
   return reinterpret_cast<const uint8_t*>(s);
}

bool ByteInStream::read(uint8_t& out) noexcept {
    return 1 == read(&out, 1);
}

bool ByteInStream::peek(uint8_t& out) noexcept {
    return 1 == peek(&out, 1, 0);
}

size_t ByteInStream::discard(size_t n) noexcept {
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

size_t ByteInStream_SecMemory::read(void* out, size_t length) noexcept {
    if( 0 == length || end_of_data() ) {
        return 0;
    }
    const size_t got = std::min<size_t>(m_source.size() - m_offset, length);
    copy_mem(out, m_source.data() + m_offset, got);
    m_offset += got;
    if( m_source.size() == m_offset ) {
        setstate_impl( iostate::eofbit );
    }
    return got;
}

bool ByteInStream_SecMemory::available(size_t n) noexcept {
    return m_source.size() - m_offset >= n;
}

size_t ByteInStream_SecMemory::peek(void* out, size_t length, size_t peek_offset) noexcept {
    const size_t bytes_left = m_source.size() - m_offset;
    if(peek_offset >= bytes_left) {
        return 0;
    }
    const size_t got = std::min(bytes_left - peek_offset, length);
    copy_mem(out, &m_source[m_offset + peek_offset], got);
    return got;
}

ByteInStream_SecMemory::ByteInStream_SecMemory(const std::string& in)
: m_source(cast_char_ptr_to_uint8(in.data()),
           cast_char_ptr_to_uint8(in.data()) + in.length()),
  m_offset(0)
{ }

void ByteInStream_SecMemory::close() noexcept {
    m_source.clear();
    m_offset = 0;
    setstate_impl( iostate::eofbit );
}

std::string ByteInStream_SecMemory::to_string() const noexcept {
    return "ByteInStream_SecMemory[content size "+jau::to_decstring(m_source.size())+
                            ", consumed "+jau::to_decstring(m_offset)+
                            ", available "+jau::to_decstring(m_source.size()-m_offset)+
                            ", iostate["+jau::io::to_string(rdstate())+
                            "]]";
}

template<typename T>
static void append_bitstr(std::string& out, T mask, T bit, const std::string& bitstr, bool& comma) {
    if( bit == ( mask & bit ) ) {
        if( comma ) { out.append(", "); }
        out.append(bitstr); comma = true;
    }
}

#define APPEND_BITSTR(U,V,W,M) append_bitstr(out, M, U::V, #W, comma);

#define IOSTATE_ENUM(X,M) \
    X(iostate,badbit,bad,M) \
    X(iostate,eofbit,eof,M) \
    X(iostate,failbit,fail,M)

std::string jau::io::to_string(const iostate mask) noexcept {
    if( iostate::goodbit == mask ) {
        return "good";
    }
    std::string out;
    bool comma = false;
    IOSTATE_ENUM(APPEND_BITSTR,mask)
    return out;
}

size_t ByteInStream_File::read(void* out, size_t length) noexcept {
    if( 0 == length || end_of_data() ) {
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
            setstate_impl( iostate::failbit );
            DBG_PRINT("ByteInStream_File::read: Error occurred in %s, errno %d %s", to_string().c_str(), errno, strerror(errno));
            return 0;
        }
        total += static_cast<size_t>(len);
        if( 0 == len || ( m_has_content_length && m_bytes_consumed + total >= m_content_size ) ) {
            setstate_impl( iostate::eofbit ); // Note: std::istream also sets iostate::failbit on eof, we don't.
            break;
        }
    }
    m_bytes_consumed += total;
    return total;
}

size_t ByteInStream_File::peek(void* out, size_t length, size_t offset) noexcept {
    if( 0 == length || end_of_data() || offset > std::numeric_limits<off64_t>::max() ||
        ( m_has_content_length && m_content_size - m_bytes_consumed < offset + 1 /* min number of bytes to read */ ) ) {
        return 0;
    }
    size_t got = 0;

    off64_t abs_pos = 0;
    if( 0 < offset ) {
        abs_pos = __posix_lseek64(m_fd, static_cast<off64_t>(offset), SEEK_CUR);
        if( 0 > abs_pos ) {
            setstate_impl( iostate::failbit );
            DBG_PRINT("ByteInStream_File::peek: Error occurred (offset1 %zd) in %s, errno %d %s",
                    offset, to_string().c_str(), errno, strerror(errno));
            return 0;
        }
    }
    if( abs_pos == static_cast<off64_t>(offset) ) {
        ssize_t len = 0;
        while ( ( len = ::read(m_fd, out, length) ) < 0 ) {
            if ( errno == EAGAIN || errno == EINTR ) {
                // cont temp unavail or interruption
                continue;
            }
            // Check errno == ETIMEDOUT ??
            setstate_impl( iostate::failbit );
            DBG_PRINT("ByteInStream_File::peak: Error occurred (read) in %s, errno %d %s", to_string().c_str(), errno, strerror(errno));
            return 0;
        }
        got = len; // potentially zero bytes, i.e. eof
    }
    if( __posix_lseek64(m_fd, static_cast<off64_t>(m_bytes_consumed), SEEK_SET) < 0 ) {
        // even though we were able to fetch the desired data above, let's fail if position reset fails
        setstate_impl( iostate::failbit );
        DBG_PRINT("ByteInStream_File::peek: Error occurred (offset2 %zd) in %s, errno %d %s",
                offset, to_string().c_str(), errno, strerror(errno));
        return 0;
    }
    return got;
}

bool ByteInStream_File::available(size_t n) noexcept {
    return is_open() && good() && ( !m_has_content_length || m_content_size - m_bytes_consumed >= (uint64_t)n );
};

ByteInStream_File::ByteInStream_File(const int fd) noexcept
: ByteInStream(),
  stats(fd), m_fd(-1), m_has_content_length(false), m_content_size(0), m_bytes_consumed(0)
{
    if( !stats.exists() || !stats.has_access() ) {
        setstate_impl( iostate::failbit ); // Note: conforming with std::ifstream open
        DBG_PRINT("ByteInStream_File::ctor: Error, not an existing or accessible file in %s, %s", stats.to_string().c_str(), to_string().c_str());
    } else {
        m_has_content_length = stats.has( jau::fs::file_stats::field_t::size );
        m_content_size = m_has_content_length ? stats.size() : 0;
        m_fd = ::dup(fd);
        if ( 0 > m_fd ) {
            setstate_impl( iostate::failbit ); // Note: conforming with std::ifstream open
            DBG_PRINT("ByteInStream_File::ctor: Error occurred in %s, %s", stats.to_string().c_str(), to_string().c_str());
        }
    }
}

ByteInStream_File::ByteInStream_File(const int dirfd, const std::string& path) noexcept
: ByteInStream(),
  stats(), m_fd(-1), m_has_content_length(false), m_content_size(0), m_bytes_consumed(0)
{
    if( jau::io::uri_tk::is_local_file_protocol(path) ) {
        // cut of leading `file://`
        std::string path2 = path.substr(7);
        stats = jau::fs::file_stats(dirfd, path2);
    } else {
        stats = jau::fs::file_stats(dirfd, path);
    }
    if( !stats.exists() || !stats.has_access() ) {
        setstate_impl( iostate::failbit ); // Note: conforming with std::ifstream open
        DBG_PRINT("ByteInStream_File::ctor: Error, not an existing or accessible file in %s, %s", stats.to_string().c_str(), to_string().c_str());
    } else {
        if( stats.has( jau::fs::file_stats::field_t::size ) ) {
            m_has_content_length = true;
            m_content_size = stats.size();
        } else {
            m_has_content_length = false;
            m_content_size = 0;
        }
        // O_NONBLOCK, is useless on files and counter to this class logic
        const int src_flags = O_RDONLY|O_BINARY|O_NOCTTY;
        if( stats.has_fd() ) {
            m_fd = ::dup( stats.fd() );
        } else {
            m_fd = __posix_openat64(dirfd, stats.path().c_str(), src_flags);
        }
        if ( 0 > m_fd ) {
            setstate_impl( iostate::failbit ); // Note: conforming with std::ifstream open
            DBG_PRINT("ByteInStream_File::ctor: Error while opening %s, %s", stats.to_string().c_str(), to_string().c_str());
        }
    }
}

ByteInStream_File::ByteInStream_File(const std::string& path) noexcept
: ByteInStream_File(AT_FDCWD, path) {}

void ByteInStream_File::close() noexcept {
    if( 0 <= m_fd ) {
        ::close(m_fd);
        m_fd = -1;
        setstate_impl( iostate::eofbit );
    }
}

std::string ByteInStream_File::to_string() const noexcept {
    return "ByteInStream_File[content_length "+( has_content_size() ? jau::to_decstring(m_content_size) : "n/a" )+
                            ", consumed "+jau::to_decstring(m_bytes_consumed)+
                            ", available "+jau::to_decstring(get_available())+
                            ", fd "+std::to_string(m_fd)+
                            ", iostate["+jau::io::to_string(rdstate())+
                            "], "+stats.to_string()+
                            "]";
}


ByteInStream_URL::ByteInStream_URL(const std::string& url, const jau::fraction_i64& timeout) noexcept
: m_url(url), m_timeout(timeout), m_buffer(0x00, BEST_URLSTREAM_RINGBUFFER_SIZE),
  m_has_content_length( false ), m_content_size( 0 ), m_total_xfered( 0 ), m_result( io::async_io_result_t::NONE ),
  m_bytes_consumed(0)

{
    m_url_thread = read_url_stream(m_url, m_buffer, m_has_content_length, m_content_size, m_total_xfered, m_result);
    if( nullptr == m_url_thread ) {
        // url protocol not supported
        m_result = async_io_result_t::FAILED;
    }
}

void ByteInStream_URL::close() noexcept {
    DBG_PRINT("ByteInStream_URL: close.0 %s, %s", id().c_str(), to_string_int().c_str());

    if( async_io_result_t::NONE == m_result ) {
        m_result = async_io_result_t::SUCCESS; // signal end of streaming
    }

    m_buffer.close( true /* zeromem */); // also unblocks all r/w ops
    if( nullptr != m_url_thread && m_url_thread->joinable() ) {
        DBG_PRINT("ByteInStream_URL: close.1 %s, %s", id().c_str(), m_buffer.toString().c_str());
        m_url_thread->join();
    }
    m_url_thread = nullptr;
    DBG_PRINT("ByteInStream_URL: close.X %s, %s", id().c_str(), to_string_int().c_str());
}

bool ByteInStream_URL::available(size_t n) noexcept {
    if( async_io_result_t::NONE != m_result ) {
        // url thread ended, only remaining bytes in buffer available left
        return m_buffer.size() >= n;
    }
    if( m_has_content_length && m_content_size - m_bytes_consumed < n ) {
        return false;
    }
    // I/O still in progress, we have to poll until data is available or timeout
    return m_buffer.waitForElements(n, m_timeout) >= n;
}

bool ByteInStream_URL::is_open() const noexcept {
    // url thread has not ended or remaining bytes in buffer available left
    return async_io_result_t::NONE == m_result || m_buffer.size() > 0;
}

size_t ByteInStream_URL::read(void* out, size_t length) noexcept {
    if( 0 == length || end_of_data() ) {
        return 0;
    }
    const size_t consumed_bytes = m_buffer.get(static_cast<uint8_t*>(out), length, 1);
    m_bytes_consumed += consumed_bytes;
    // DBG_PRINT("ByteInStream_Feed::read: size %zu/%zu bytes, %s", consumed_bytes, length, to_string_int().c_str() );
    return consumed_bytes;
}

size_t ByteInStream_URL::peek(void* out, size_t length, size_t peek_offset) noexcept {
    (void)out;
    (void)length;
    (void)peek_offset;
    ERR_PRINT("ByteInStream_URL::peek not implemented");
    return 0;
}

iostate ByteInStream_URL::rdstate() const noexcept {
    if ( ( async_io_result_t::NONE != m_result && m_buffer.isEmpty() ) ||
         ( m_has_content_length && m_bytes_consumed >= m_content_size ) )
    {
        setstate_impl( iostate::eofbit );
    }
    if( async_io_result_t::FAILED == m_result ) {
        setstate_impl( iostate::failbit );
    }
    return rdstate_impl();
}

std::string ByteInStream_URL::to_string_int() const noexcept {
    return m_url+", Url[content_length "+( has_content_size() ? jau::to_decstring(m_content_size.load()) : "n/a" )+
                       ", xfered "+jau::to_decstring(m_total_xfered.load())+
                       ", result "+std::to_string((int8_t)m_result.load())+
           "], consumed "+jau::to_decstring(m_bytes_consumed)+
           ", available "+jau::to_decstring(get_available())+
           ", iostate["+jau::io::to_string(rdstate())+
           "], "+m_buffer.toString();
}
std::string ByteInStream_URL::to_string() const noexcept {
    return "ByteInStream_URL["+to_string_int()+"]";
}

std::unique_ptr<ByteInStream> jau::io::to_ByteInStream(const std::string& path_or_uri, jau::fraction_i64 timeout) noexcept {
    if( !jau::io::uri_tk::is_local_file_protocol(path_or_uri) &&
         jau::io::uri_tk::protocol_supported(path_or_uri) )
    {
        std::unique_ptr<ByteInStream> res = std::make_unique<ByteInStream_URL>(path_or_uri, timeout);
        if( nullptr != res && !res->fail() ) {
            return res;
        }
    }
    std::unique_ptr<ByteInStream> res = std::make_unique<ByteInStream_File>(path_or_uri);
    if( nullptr != res && !res->fail() ) {
        return res;
    }
    return nullptr;
}

ByteInStream_Feed::ByteInStream_Feed(const std::string& id_name, const jau::fraction_i64& timeout) noexcept
: m_id(id_name), m_timeout(timeout), m_buffer(0x00, BEST_URLSTREAM_RINGBUFFER_SIZE),
  m_has_content_length( false ), m_content_size( 0 ), m_total_xfered( 0 ), m_result( io::async_io_result_t::NONE ),
  m_bytes_consumed(0)
{ }

void ByteInStream_Feed::close() noexcept {
    DBG_PRINT("ByteInStream_Feed: close.0 %s, %s", id().c_str(), to_string_int().c_str());

    if( async_io_result_t::NONE == m_result ) {
        m_result = async_io_result_t::SUCCESS; // signal end of streaming
    }
    m_buffer.close( true /* zeromem */); // also unblocks all r/w ops
    DBG_PRINT("ByteInStream_Feed: close.X %s, %s", id().c_str(), to_string_int().c_str());
}

bool ByteInStream_Feed::available(size_t n) noexcept {
    if( async_io_result_t::NONE != m_result ) {
        // feeder completed, only remaining bytes in buffer available left
        return m_buffer.size() >= n;
    }
    if( m_has_content_length && m_content_size - m_bytes_consumed < n ) {
        return false;
    }
    // I/O still in progress, we have to poll until data is available or timeout
    return m_buffer.waitForElements(n, m_timeout) >= n;
}

bool ByteInStream_Feed::is_open() const noexcept {
    // url thread has not ended or remaining bytes in buffer available left
    return async_io_result_t::NONE == m_result || m_buffer.size() > 0;
}

size_t ByteInStream_Feed::read(void* out, size_t length) noexcept {
    if( 0 == length || end_of_data() ) {
        return 0;
    }
    const size_t got = m_buffer.get(static_cast<uint8_t*>(out), length, 1);
    m_bytes_consumed += got;
    // DBG_PRINT("ByteInStream_Feed::read: size %zu/%zu bytes, %s", consumed_bytes, length, to_string_int().c_str() );
    return got;
}

size_t ByteInStream_Feed::peek(void* out, size_t length, size_t peek_offset) noexcept {
    (void)out;
    (void)length;
    (void)peek_offset;
    ERR_PRINT("ByteInStream_Feed::peek not implemented");
    return 0;
}

iostate ByteInStream_Feed::rdstate() const noexcept {
    if ( ( async_io_result_t::NONE != m_result && m_buffer.isEmpty() ) ||
         ( m_has_content_length && m_bytes_consumed >= m_content_size ) )
    {
        setstate_impl( iostate::eofbit );
    }
    if( async_io_result_t::FAILED == m_result ) {
        setstate_impl( iostate::failbit );
    }
    return rdstate_impl();
}

void ByteInStream_Feed::write(uint8_t in[], size_t length) noexcept {
    if( 0 < length ) {
        m_buffer.putBlocking(in, in+length, m_timeout);
        m_total_xfered.fetch_add(length);
    }
}

void ByteInStream_Feed::set_eof(const async_io_result_t result) noexcept {
    m_result = result;
    interruptReader();
}

std::string ByteInStream_Feed::to_string_int() const noexcept {
    return m_id+", ext[content_length "+( has_content_size() ? jau::to_decstring(m_content_size.load()) : "n/a" )+
                   ", xfered "+jau::to_decstring(m_total_xfered.load())+
                   ", result "+std::to_string((int8_t)m_result.load())+
           "], consumed "+std::to_string(m_bytes_consumed)+
           ", available "+std::to_string(get_available())+
           ", iostate["+jau::io::to_string(rdstate())+
           "], "+m_buffer.toString();
}

std::string ByteInStream_Feed::to_string() const noexcept {
    return "ByteInStream_Feed["+to_string_int()+"]";
}

void ByteInStream_Recorder::close() noexcept {
    clear_recording();
    m_parent.close();
    DBG_PRINT("ByteInStream_Recorder: close.X %s", id().c_str());
}

void ByteInStream_Recorder::start_recording() noexcept {
    m_buffer.resize(0);
    m_rec_offset = m_bytes_consumed;
    m_is_recording = true;
}

void ByteInStream_Recorder::stop_recording() noexcept {
    m_is_recording = false;
}

void ByteInStream_Recorder::clear_recording() noexcept {
    m_is_recording = false;
    m_buffer.clear();
    m_rec_offset = 0;
}

size_t ByteInStream_Recorder::read(void* out, size_t length) noexcept {
    const size_t consumed_bytes = m_parent.read(out, length);
    m_bytes_consumed += consumed_bytes;
    if( is_recording() ) {
        uint8_t* out_u8 = static_cast<uint8_t*>(out);
        m_buffer.insert(m_buffer.end(), out_u8, out_u8+consumed_bytes);
    }
    return consumed_bytes;
}

std::string ByteInStream_Recorder::to_string() const noexcept {
    return "ByteInStream_Recorder[parent "+m_parent.id()+", recording[on "+std::to_string(m_is_recording)+
                            " offset "+jau::to_decstring(m_rec_offset)+
                            "], consumed "+jau::to_decstring(m_bytes_consumed)+
                            ", iostate["+jau::io::to_string(rdstate())+"]]";
}

bool ByteOutStream::write(const uint8_t& in) noexcept {
    return 1 == write(&in, 1);
}

size_t ByteOutStream_File::write(const void* out, size_t length) noexcept {
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
            setstate_impl( iostate::failbit );
            DBG_PRINT("ByteOutStream_File::write: Error occurred in %s, errno %d %s", to_string().c_str(), errno, strerror(errno));
            return 0;
        }
        total += static_cast<size_t>(len);
        if( 0 == len ) {
            setstate_impl( iostate::failbit);
            break;
        }
    }
    m_bytes_consumed += total;
    return total;
}

ByteOutStream_File::ByteOutStream_File(const int fd) noexcept
: stats(fd), m_fd(-1),
  m_bytes_consumed(0)
{
    if( !stats.exists() || !stats.has_access() ) {
        setstate_impl( iostate::failbit ); // Note: conforming with std::ifstream open
        DBG_PRINT("ByteOutStream_File::ctor: Error, not an existing or accessible file in %s, %s", stats.to_string().c_str(), to_string().c_str());
    } else {
        m_fd = ::dup(fd);
        if ( 0 > m_fd ) {
            setstate_impl( iostate::failbit ); // Note: conforming with std::ifstream open
            DBG_PRINT("ByteOutStream_File::ctor: Error occurred in %s, %s", stats.to_string().c_str(), to_string().c_str());
        }
        // open file-descriptor is appending anyways
    }
}

ByteOutStream_File::ByteOutStream_File(const int dirfd, const std::string& path, const jau::fs::fmode_t mode) noexcept
: stats(), m_fd(-1),
  m_bytes_consumed(0)
{
    if( jau::io::uri_tk::is_local_file_protocol(path) ) {
        // cut of leading `file://`
        std::string path2 = path.substr(7);
        stats = jau::fs::file_stats(dirfd, path2);
    } else {
        stats = jau::fs::file_stats(dirfd, path);
    }
    if( ( stats.exists() && !stats.is_file() && !stats.has_fd() ) || !stats.has_access() ) {
        setstate_impl( iostate::failbit ); // Note: conforming with std::ofstream open (?)
        DBG_PRINT("ByteOutStream_File::ctor: Error, an existing non[file, fd] or not accessible element in %s, %s", stats.to_string().c_str(), to_string().c_str());
    } else {
        // O_NONBLOCK, is useless on files and counter to this class logic
        if( stats.has_fd() ) {
            m_fd = ::dup( stats.fd() );
        } else {
            const int dst_flags = ( stats.exists() ? 0 : O_CREAT|O_EXCL ) | O_WRONLY|O_BINARY|O_NOCTTY;
            m_fd = __posix_openat64(dirfd, stats.path().c_str(), dst_flags, jau::fs::posix_protection_bits( mode ));
        }
        if ( 0 > m_fd ) {
            setstate_impl( iostate::failbit ); // Note: conforming with std::ifstream open
            DBG_PRINT("ByteOutStream_File::ctor: Error while opening %s, %s", stats.to_string().c_str(), to_string().c_str());
        }
        if( stats.is_file() ) {
            off64_t abs_pos = __posix_lseek64(m_fd, 0, SEEK_END);
            if( 0 > abs_pos ) {
                setstate_impl( iostate::failbit );
                ERR_PRINT("Failed to position existing file to end %s, errno %d %s",
                        to_string().c_str(), errno, strerror(errno));
            }
        }
    }
}

ByteOutStream_File::ByteOutStream_File(const std::string& path, const jau::fs::fmode_t mode) noexcept
: ByteOutStream_File(AT_FDCWD, path, mode) {}

void ByteOutStream_File::close() noexcept {
    if( 0 <= m_fd ) {
        ::close(m_fd);
        m_fd = -1;
        setstate_impl( iostate::eofbit );
    }
}

std::string ByteOutStream_File::to_string() const noexcept {
    return "ByteOutStream_File[consumed "+jau::to_decstring(m_bytes_consumed)+
                            ", fd "+std::to_string(m_fd)+
                            ", iostate["+jau::io::to_string(rdstate())+
                            "], "+stats.to_string()+
                            "]";
}

