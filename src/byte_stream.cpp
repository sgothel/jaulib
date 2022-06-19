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

#include <fstream>
#include <iostream>
#include <chrono>

// #include <botan_all.h>

#include <jau/debug.hpp>
#include <jau/file_util.hpp>
#include <jau/byte_stream.hpp>

#ifdef USE_LIBCURL
    #include <curl/curl.h>
#endif

#include <thread>
#include <pthread.h>

using namespace jau::io;
using namespace jau::fractions_i64_literals;

#ifdef USE_LIBCURL
    const size_t jau::io::BEST_URLSTREAM_RINGBUFFER_SIZE = 2*CURL_MAX_WRITE_SIZE;
#else
    const size_t jau::io::BEST_URLSTREAM_RINGBUFFER_SIZE = 2*16384;
#endif

inline constexpr void copy_mem(uint8_t* out, const uint8_t* in, size_t n) noexcept {
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

#ifndef BOTAN_VERSION_MAJOR

size_t ByteInStream::read_byte(uint8_t& out) noexcept {
    return read(&out, 1);
}

size_t ByteInStream::peek_byte(uint8_t& out) const noexcept {
    return peek(&out, 1, 0);
}

size_t ByteInStream::discard_next(size_t n) noexcept {
    uint8_t buf[64] = { 0 };
    size_t discarded = 0;

    while(n)
    {
        const size_t got = this->read(buf, std::min(n, sizeof(buf)));
        discarded += got;
        n -= got;

        if(got == 0)
            break;
    }

    return discarded;
}

#endif /* BOTAN_VERSION_MAJOR */

size_t ByteInStream_SecMemory::read(uint8_t out[], size_t length) NOEXCEPT_BOTAN {
    if( 0 == length || end_of_data() ) {
        return 0;
    }
    const size_t got = std::min<size_t>(m_source.size() - m_offset, length);
    copy_mem(out, m_source.data() + m_offset, got);
    m_offset += got;
    return got;
}

bool ByteInStream_SecMemory::check_available(size_t n) NOEXCEPT_BOTAN {
    return m_source.size() - m_offset >= n;
}

size_t ByteInStream_SecMemory::peek(uint8_t out[], size_t length, size_t peek_offset) const NOEXCEPT_BOTAN {
    const size_t bytes_left = m_source.size() - m_offset;
    if(peek_offset >= bytes_left) {
        return 0;
    }
    const size_t got = std::min(bytes_left - peek_offset, length);
    copy_mem(out, &m_source[m_offset + peek_offset], got);
    return got;
}

bool ByteInStream_SecMemory::end_of_data() const NOEXCEPT_BOTAN {
    return m_source.size() == m_offset;
}

ByteInStream_SecMemory::ByteInStream_SecMemory(const std::string& in)
: m_source(cast_char_ptr_to_uint8(in.data()),
           cast_char_ptr_to_uint8(in.data()) + in.length()),
  m_offset(0)
{ }

void ByteInStream_SecMemory::close() noexcept {
    m_source.clear();
    m_offset = 0;
}

std::string ByteInStream_SecMemory::to_string() const noexcept {
    return "ByteInStream_SecMemory[content size "+jau::to_decstring(m_source.size())+
                            ", consumed "+jau::to_decstring(m_offset)+
                            ", available "+jau::to_decstring(m_source.size()-m_offset)+
                            ", eod "+std::to_string( end_of_data() )+
                            ", error "+std::to_string( error() )+
                            "]";
}

size_t ByteInStream_istream::read(uint8_t out[], size_t length) NOEXCEPT_BOTAN {
    if( 0 == length || end_of_data() ) {
        return 0;
    }
    m_source.read(cast_uint8_ptr_to_char(out), length);
    if( error() ) {
        DBG_PRINT("ByteInStream_istream::read: Error occurred in %s", to_string().c_str());
        return 0;
    }
    const size_t got = static_cast<size_t>(m_source.gcount());
    m_bytes_consumed += got;
    return got;
}

bool ByteInStream_istream::check_available(size_t n) NOEXCEPT_BOTAN {
    // stream size is dynamic, hence can't store size until end
    const std::streampos orig_pos = m_source.tellg();
    m_source.seekg(0, std::ios::end);
    uint64_t avail = static_cast<uint64_t>(m_source.tellg() - orig_pos);
    m_source.seekg(orig_pos);
    return avail >= n;
}

size_t ByteInStream_istream::peek(uint8_t out[], size_t length, size_t offset) const NOEXCEPT_BOTAN {
    if( 0 == length || end_of_data() ) {
        return 0;
    }
    size_t got = 0;

    if( offset ) {
        secure_vector<uint8_t> buf(offset);
        m_source.read(cast_uint8_ptr_to_char(buf.data()), buf.size());
        if( error() ) {
            DBG_PRINT("ByteInStream_istream::peek: Error occurred (offset) in %s", to_string().c_str());
            return 0;
        }
        got = static_cast<size_t>(m_source.gcount());
    }

    if(got == offset) {
        m_source.read(cast_uint8_ptr_to_char(out), length);
        if( error() ) {
            DBG_PRINT("ByteInStream_istream::peek: Error occurred (read) in %s", to_string().c_str());
            return 0;
        }
        got = static_cast<size_t>(m_source.gcount());
    }

    if( m_source.eof() ) {
        m_source.clear();
    }
    m_source.seekg(m_bytes_consumed, std::ios::beg);

    return got;
}

bool ByteInStream_istream::end_of_data() const NOEXCEPT_BOTAN {
    return !m_source.good();
}

std::string ByteInStream_istream::id() const NOEXCEPT_BOTAN {
    return m_identifier;
}

ByteInStream_istream::ByteInStream_istream(std::istream& in, const std::string& name) noexcept
: m_identifier(name), m_source(in),
  m_bytes_consumed(0)
{ }

void ByteInStream_istream::close() noexcept {
    // nop
}

std::string ByteInStream_istream::to_string() const noexcept {
    return "ByteInStream_Stream["+m_identifier+
                            ", consumed "+jau::to_decstring(m_bytes_consumed)+
                            ", eod "+std::to_string( end_of_data() )+
                            ", error "+std::to_string( error() )+
                            "]";
}

size_t ByteInStream_File::read(uint8_t out[], size_t length) NOEXCEPT_BOTAN {
    if( 0 == length || end_of_data() ) {
        return 0;
    }
    m_source->read(cast_uint8_ptr_to_char(out), length);
    if( error() ) {
        DBG_PRINT("ByteInStream_File::read: Error occurred in %s", to_string().c_str());
        return 0;
    }
    const size_t got = static_cast<size_t>(m_source->gcount());
    m_bytes_consumed += got;
    return got;
}

size_t ByteInStream_File::peek(uint8_t out[], size_t length, size_t offset) const NOEXCEPT_BOTAN {
    if( 0 == length || end_of_data() ) {
        return 0;
    }
    size_t got = 0;

    if(offset) {
        secure_vector<uint8_t> buf(offset);
        m_source->read(cast_uint8_ptr_to_char(buf.data()), buf.size());
        if( error() ) {
            DBG_PRINT("ByteInStream_File::peek: Error occurred (offset) in %s", to_string().c_str());
            return 0;
        }
        got = static_cast<size_t>(m_source->gcount());
    }

    if(got == offset) {
        m_source->read(cast_uint8_ptr_to_char(out), length);
        if( error() ) {
            DBG_PRINT("ByteInStream_File::peek: Error occurred (read) in %s", to_string().c_str());
            return 0;
        }
        got = static_cast<size_t>(m_source->gcount());
    }

    if(m_source->eof()) {
        m_source->clear();
    }
    m_source->seekg(m_bytes_consumed, std::ios::beg);

    return got;
}

bool ByteInStream_File::check_available(size_t n) NOEXCEPT_BOTAN {
    return nullptr != m_source && m_content_size - m_bytes_consumed >= (uint64_t)n;
};

bool ByteInStream_File::end_of_data() const NOEXCEPT_BOTAN {
    return nullptr == m_source || !m_source->good() || m_bytes_consumed >= m_content_size;
}

std::string ByteInStream_File::id() const NOEXCEPT_BOTAN {
    return m_identifier;
}

ByteInStream_File::ByteInStream_File(const std::string& path, bool use_binary) noexcept
: m_identifier(path),
  m_source(),
  m_content_size(0), m_bytes_consumed(0)
{
    std::unique_ptr<jau::fs::file_stats> stats;
    if( jau::io::uri::is_local_file_protocol(path) ) {
        // cut of leading `file://`
        std::string path2 = path.substr(7);
        stats = std::make_unique<jau::fs::file_stats>(path2);
    } else {
        stats = std::make_unique<jau::fs::file_stats>(path);
    }
    if( !stats->exists() || !stats->has_access() ) {
        DBG_PRINT("ByteInStream_File::ctor: Error, not an existing or accessible file in %s, %s", stats->to_string(true).c_str(), to_string().c_str());
    } else if( !stats->is_file() ) {
        DBG_PRINT("ByteInStream_File::ctor: Error, not a file in %s, %s", stats->to_string(true).c_str(), to_string().c_str());
    } else {
        m_content_size = stats->size();
        m_source = std::make_unique<std::ifstream>(stats->path(), use_binary ? std::ios::binary : std::ios::in);
        if( error() ) {
            DBG_PRINT("ByteInStream_File::ctor: Error occurred in %s, %s", stats->to_string(true).c_str(), to_string().c_str());
            m_source = nullptr;
        }
    }
}

void ByteInStream_File::close() noexcept {
    if( nullptr != m_source ) {
        m_source->close();
    }
}

std::string ByteInStream_File::to_string() const noexcept {
    return "ByteInStream_File["+m_identifier+", content_length "+jau::to_decstring(m_content_size)+
                            ", consumed "+jau::to_decstring(m_bytes_consumed)+
                            ", available "+jau::to_decstring(m_content_size - m_bytes_consumed)+
                            ", eod "+std::to_string( end_of_data() )+
                            ", error "+std::to_string( error() )+
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

    m_buffer.drop(m_buffer.size()); // unblock putBlocking(..)
    if( nullptr != m_url_thread && m_url_thread->joinable() ) {
        DBG_PRINT("ByteInStream_URL: close.1 %s, %s", id().c_str(), m_buffer.toString().c_str());
        m_url_thread->join();
    }
    m_url_thread = nullptr;
    m_buffer.clear( true /* zeromem */);
    DBG_PRINT("ByteInStream_URL: close.X %s, %s", id().c_str(), to_string_int().c_str());
}

bool ByteInStream_URL::check_available(size_t n) NOEXCEPT_BOTAN {
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

size_t ByteInStream_URL::read(uint8_t out[], size_t length) NOEXCEPT_BOTAN {
    if( 0 == length || end_of_data() ) {
        return 0;
    }
    const size_t consumed_bytes = m_buffer.get(out, length, 1);
    m_bytes_consumed += consumed_bytes;
    // DBG_PRINT("ByteInStream_Feed::read: size %zu/%zu bytes, %s", consumed_bytes, length, to_string_int().c_str() );
    return consumed_bytes;
}

size_t ByteInStream_URL::peek(uint8_t out[], size_t length, size_t peek_offset) const NOEXCEPT_BOTAN {
    (void)out;
    (void)length;
    (void)peek_offset;
    ERR_PRINT("ByteInStream_URL::peek not implemented");
    return 0;
}

bool ByteInStream_URL::end_of_data() const NOEXCEPT_BOTAN {
    return ( async_io_result_t::NONE != m_result && m_buffer.isEmpty() ) ||
           ( m_has_content_length && m_bytes_consumed >= m_content_size );
}

std::string ByteInStream_URL::to_string_int() const noexcept {
    return m_url+", Url[content_length has "+std::to_string(m_has_content_length.load())+
                       ", size "+jau::to_decstring(m_content_size.load())+
                       ", xfered "+jau::to_decstring(m_total_xfered.load())+
                       ", result "+std::to_string((int8_t)m_result.load())+
           "], consumed "+jau::to_decstring(m_bytes_consumed)+
           ", available "+jau::to_decstring(get_available())+
           ", eod "+std::to_string( end_of_data() )+
           ", error "+std::to_string( error() )+
           ", "+m_buffer.toString();
}
std::string ByteInStream_URL::to_string() const noexcept {
    return "ByteInStream_URL["+to_string_int()+"]";
}

std::unique_ptr<ByteInStream> jau::io::to_ByteInStream(const std::string& path_or_uri, jau::fraction_i64 timeout) noexcept {
    if( !jau::io::uri::is_local_file_protocol(path_or_uri) &&
         jau::io::uri::protocol_supported(path_or_uri) )
    {
        std::unique_ptr<ByteInStream> res = std::make_unique<ByteInStream_URL>(path_or_uri, timeout);
        if( nullptr != res && !res->error() ) {
            return res;
        }
    }
    std::unique_ptr<ByteInStream> res = std::make_unique<ByteInStream_File>(path_or_uri);
    if( nullptr != res && !res->error() ) {
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
    m_buffer.drop(m_buffer.size()); // unblock putBlocking(..)

    m_buffer.clear( true /* zeromem */);
    DBG_PRINT("ByteInStream_Feed: close.X %s, %s", id().c_str(), to_string_int().c_str());
}

bool ByteInStream_Feed::check_available(size_t n) NOEXCEPT_BOTAN {
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

size_t ByteInStream_Feed::read(uint8_t out[], size_t length) NOEXCEPT_BOTAN {
    if( 0 == length || end_of_data() ) {
        return 0;
    }
    const size_t consumed_bytes = m_buffer.get(out, length, 1);
    m_bytes_consumed += consumed_bytes;
    // DBG_PRINT("ByteInStream_Feed::read: size %zu/%zu bytes, %s", consumed_bytes, length, to_string_int().c_str() );
    return consumed_bytes;
}

size_t ByteInStream_Feed::peek(uint8_t out[], size_t length, size_t peek_offset) const NOEXCEPT_BOTAN {
    (void)out;
    (void)length;
    (void)peek_offset;
    ERR_PRINT("ByteInStream_Feed::peek not implemented");
    return 0;
}

bool ByteInStream_Feed::end_of_data() const NOEXCEPT_BOTAN {
    return ( async_io_result_t::NONE != m_result && m_buffer.isEmpty() ) ||
           ( m_has_content_length && m_bytes_consumed >= m_content_size );
}

void ByteInStream_Feed::write(uint8_t in[], size_t length) noexcept {
    if( 0 < length ) {
        m_buffer.putBlocking(in, in+length, m_timeout);
        m_total_xfered.fetch_add(length);
    }
}

std::string ByteInStream_Feed::to_string_int() const noexcept {
    return m_id+", ext[content_length has "+std::to_string(m_has_content_length.load())+
                   ", size "+jau::to_decstring(m_content_size.load())+
                   ", xfered "+jau::to_decstring(m_total_xfered.load())+
                   ", result "+std::to_string((int8_t)m_result.load())+
           "], consumed "+std::to_string(m_bytes_consumed)+
           ", available "+std::to_string(get_available())+
           ", eod "+std::to_string( end_of_data() )+
           ", error "+std::to_string( error() )+
           ", "+m_buffer.toString();
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

size_t ByteInStream_Recorder::read(uint8_t out[], size_t length) NOEXCEPT_BOTAN {
    const size_t consumed_bytes = m_parent.read(out, length);
    m_bytes_consumed += consumed_bytes;
    if( is_recording() ) {
        m_buffer.insert(m_buffer.end(), out, out+consumed_bytes);
    }
    return consumed_bytes;
}

std::string ByteInStream_Recorder::to_string() const noexcept {
    return "ByteInStream_Recorder[parent "+m_parent.id()+", recording[on "+std::to_string(m_is_recording)+
                                                   " offset "+jau::to_decstring(m_rec_offset)+
                            "], consumed "+jau::to_decstring(m_bytes_consumed)+
                            ", eod "+std::to_string(end_of_data())+
                            ", error "+std::to_string(error())+"]";
}
