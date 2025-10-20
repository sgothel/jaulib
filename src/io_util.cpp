/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021-2023 Gothel Software e.K.
 * Copyright (c) 2021 ZAFENA AB
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

#include <memory>
#include <unordered_map>

// #include <botan_all.h>

#include <jau/debug.hpp>
#include <jau/io/io_util.hpp>
#include <jau/io/byte_stream.hpp>
#include <jau/string_util.hpp>

#ifdef USE_LIBCURL
    #include <curl/curl.h>
#endif

#include <thread>
#include <pthread.h>

using namespace jau::io;
using namespace jau::fractions_i64_literals;


uint64_t jau::io::read_file(const std::string& input_file,
                            secure_vector<uint8_t>& buffer,
                            const StreamConsumerFunc& consumer_fn) noexcept
{
    if(input_file == "-") {
        ByteStream_File in(0, jau::io::iomode_t::read); // stdin
        return read_stream(in, buffer, consumer_fn);
    } else {
        ByteStream_File in(input_file, jau::io::iomode_t::read);
        return read_stream(in, buffer, consumer_fn);
    }
}

uint64_t jau::io::read_stream(ByteStream& in,
                              secure_vector<uint8_t>& buffer,
                              const StreamConsumerFunc& consumer_fn) noexcept {
    uint64_t total = 0;
    bool has_more;
    do {
        if( in.available(1) ) { // at least one byte to stream, also considers eof
            buffer.resize(buffer.capacity());
            const uint64_t got = in.read(buffer.data(), buffer.capacity());

            buffer.resize(got);
            total += got;
            has_more = 1 <= got && !in.fail() && ( !in.hasContentSize() || total < in.contentSize() );
            try {
                if( !consumer_fn(buffer, !has_more) ) {
                    break; // end streaming
                }
            } catch (std::exception &e) {
                ERR_PRINT("jau::io::read_stream: Caught exception: %s", e.what());
                break; // end streaming
            }
        } else {
            has_more = false;
            buffer.resize(0);
            consumer_fn(buffer, true); // forced final, zero size
        }
    } while( has_more );
    return total;
}

static uint64_t _read_buffer(ByteStream& in,
                             secure_vector<uint8_t>& buffer) noexcept {
    if( in.available(1) ) { // at least one byte to stream, also considers eof
        buffer.resize(buffer.capacity());
        const uint64_t got = in.read(buffer.data(), buffer.capacity());
        buffer.resize(got);
        return got;
    }
    return 0;
}

uint64_t jau::io::read_stream(ByteStream& in,
                              secure_vector<uint8_t>& buffer1, secure_vector<uint8_t>& buffer2,
                              const StreamConsumerFunc& consumer_fn) noexcept {
    secure_vector<uint8_t>* buffers[] = { &buffer1, &buffer2 };
    bool eof[] = { false, false };

    bool eof_read = false;
    uint64_t total_send = 0;
    uint64_t total_read = 0;
    int idx = 0;
    // fill 1st buffer upfront
    {
        uint64_t got = _read_buffer(in, *buffers[idx]);
        total_read += got;
        eof_read = 0 == got || in.fail() || ( in.hasContentSize() && total_read >= in.contentSize() );
        eof[idx] = eof_read;
        ++idx;
    }

    // - buffer_idx was filled
    // - buffer_idx++
    //
    // - while !eof_send do
    //   - read buffer_idx if not eof_read,
    //     - set eof[buffer_idx+1]=true if zero bytes
    //   - buffer_idx++
    //   - sent buffer_idx
    //
    bool eof_send = false;
    while( !eof_send ) {
        int bidx_next = ( idx + 1 ) % 2;
        if( !eof_read ) {
            uint64_t got = _read_buffer(in, *buffers[idx]);
            total_read += got;
            eof_read = 0 == got || in.fail() || ( in.hasContentSize() && total_read >= in.contentSize() );
            eof[idx] = eof_read;
            if( 0 == got ) {
                // read-ahead eof propagation if read zero bytes,
                // hence next consumer_fn() will send last bytes with is_final=true
                eof[bidx_next] = true;
            }
        }
        idx = bidx_next;

        secure_vector<uint8_t>* buffer = buffers[idx];
        eof_send = eof[idx];
        total_send += buffer->size();
        try {
            if( !consumer_fn(*buffer, eof_send) ) {
                return total_send; // end streaming
            }
        } catch (std::exception &e) {
            ERR_PRINT("jau::io::read_stream: Caught exception: %s", e.what());
            return total_send; // end streaming
        }
    }
    return total_send;
}

std::vector<std::string_view> jau::io::uri_tk::supported_protocols() noexcept {
    std::vector<std::string_view> res;
#ifdef USE_LIBCURL
    const curl_version_info_data* cvid = curl_version_info(CURLVERSION_NOW);
    if( nullptr == cvid || nullptr == cvid->protocols ) {
        return res;
    }
    for(int i=0; nullptr != cvid->protocols[i]; ++i) {
        res.emplace_back( cvid->protocols[i] );
    }
#endif // USE_LIBCURL
    return res;
}

static bool _is_scheme_valid(const std::string_view& scheme) noexcept {
    if ( scheme.empty() ) {
        return false;
    }
    auto pos = std::find_if_not(scheme.begin(), scheme.end(), [&](char c) { // NOLINT(modernize-use-ranges)
        return std::isalnum(c) || c == '+' || c == '.' || c == '-';
    });
    return pos == scheme.end();
}
std::string_view jau::io::uri_tk::get_scheme(const std::string_view& uri) noexcept {
    std::size_t pos = uri.find(':');
    if (pos == std::string_view::npos) {
        return uri.substr(0, 0);
    }
    std::string_view scheme = uri.substr(0, pos);
    if( !_is_scheme_valid( scheme ) ) {
        return uri.substr(0, 0);
    }
    return scheme;
}

bool jau::io::uri_tk::protocol_supported(const std::string_view& uri) noexcept {
    const std::string_view scheme = get_scheme(uri);
    if( scheme.empty() ) {
        return false;
    }
    const std::vector<std::string_view> protos = supported_protocols();
    auto it = std::find(protos.cbegin(), protos.cend(), scheme); // NOLINT(modernize-use-ranges)
    return protos.cend() != it;
}

bool jau::io::uri_tk::is_local_file_protocol(const std::string_view& uri) noexcept {
    return uri.starts_with("file://");
}

bool jau::io::uri_tk::is_httpx_protocol(const std::string_view& uri) noexcept {
    const std::string_view scheme = get_scheme(uri);
    if( scheme.empty() ) {
        return false;
    }
    return "https" == scheme || "http" == scheme;
}

#ifdef USE_LIBCURL

struct curl_glue1_t {
    CURL *curl_handle;
    bool has_content_length;
    uint64_t content_length;
    int32_t status_code;
    uint64_t total_read;
    secure_vector<uint8_t>& buffer;
    StreamConsumerFunc consumer_fn;
};

static size_t consume_header_curl1(char *buffer, size_t size, size_t nmemb, void *userdata) noexcept {
    curl_glue1_t * cg = (curl_glue1_t*)userdata;

    {
        long v;
        const CURLcode r = curl_easy_getinfo(cg->curl_handle, CURLINFO_RESPONSE_CODE, &v);
        if( CURLE_OK == r ) {
            cg->status_code = static_cast<int32_t>(v);
            if( 400 <= v ) {
                IRQ_PRINT("response_code: %ld", v);
                return 0;
            } else {
                DBG_PRINT("consume_header_curl1.0 response_code: %ld", v);
            }
        }
    }
    if( !cg->has_content_length ) {
        curl_off_t v = 0;
        const CURLcode r = curl_easy_getinfo(cg->curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &v);
        if( CURLE_OK == r ) {
            if( 0 > v ) { // curl returns -1 if the size if not known
                cg->content_length = 0;
                cg->has_content_length = false;
            } else {
                cg->content_length = v;
                cg->has_content_length = true;
            }
        }
    }
    const size_t realsize = size * nmemb;

    if( false ) {
        DBG_PRINT("consume_header_curl1.X realsize %zu, total %" PRIu64 " / ( content_len has %d, size %" PRIu64 " )",
               realsize, cg->total_read, cg->has_content_length, cg->content_length );
        std::string blob(buffer, realsize);
        jau::PLAIN_PRINT(true, "%s", blob.c_str());
    }

    return realsize;
}

static size_t consume_data_curl1(char *ptr, size_t size, size_t nmemb, void *userdata) noexcept {
    curl_glue1_t * cg = (curl_glue1_t*)userdata;

    if( !cg->has_content_length ) {
        curl_off_t v = 0;
        CURLcode r = curl_easy_getinfo(cg->curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &v);
        if( CURLE_OK == r ) {
            if( 0 > v ) { // curl returns -1 if the size if not known
                cg->content_length = 0;
                cg->has_content_length = false;
            } else {
                cg->content_length = v;
                cg->has_content_length = true;
            }
        }
    }
    const size_t realsize = size * nmemb;
    DBG_PRINT("consume_data_curl1.0 realsize %zu", realsize);
    cg->buffer.resize(realsize);
    memcpy(cg->buffer.data(), ptr, realsize);

    cg->total_read += realsize;
    const bool is_final = 0 == realsize ||
                          cg->has_content_length ? cg->total_read >= cg->content_length : false;

    DBG_PRINT("consume_data_curl1.X realsize %zu, total %" PRIu64 " / ( content_len has %d, size %" PRIu64 " ), is_final %d",
           realsize, cg->total_read, cg->has_content_length, cg->content_length, is_final );

    try {
        if( !cg->consumer_fn(cg->buffer, is_final) ) {
            return 0; // end streaming
        }
    } catch (std::exception &e) {
        ERR_PRINT("jau::io::read_url_stream: Caught exception: %s", e.what());
        return 0; // end streaming
    }

    return realsize;
}

#endif // USE_LIBCURL

uint64_t jau::io::read_url_stream(const std::string& url,
                                  secure_vector<uint8_t>& buffer,
                                  const StreamConsumerFunc& consumer_fn) noexcept {
#ifdef USE_LIBCURL
    std::vector<char> errorbuffer;
    errorbuffer.reserve(CURL_ERROR_SIZE);
    CURLcode res;

    if( !uri_tk::protocol_supported(url) ) {
        const std::string_view scheme = uri_tk::get_scheme(url);
        DBG_PRINT("Protocol of given uri-scheme '%s' not supported. Supported protocols [%s].",
                std::string(scheme).c_str(), jau::to_string(uri_tk::supported_protocols(), ",").c_str());
        return 0;
    }

    /* init the curl session */
    CURL *curl_handle = curl_easy_init();
    DBG_PRINT("CURL: Create own handle %p", curl_handle);
    if( nullptr == curl_handle ) {
        ERR_PRINT("Error setting up url %s, null curl handle", url.c_str());
        return 0;
    }

    curl_glue1_t cg = { .curl_handle=curl_handle, .has_content_length=false, .content_length=0,
                        .status_code=0, .total_read=0, .buffer=buffer, .consumer_fn=consumer_fn };

    res = curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorbuffer.data());
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url.c_str(), (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* set URL to get here */
    res = curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url.c_str(), (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* Switch on full protocol/debug output while testing */
    res = curl_easy_setopt(curl_handle, CURLOPT_VERBOSE,
                           jau::environment::getBooleanProperty("jau_io_net_verbose", false) ? 1L : 0L );
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url.c_str(), (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    if( !jau::environment::getBooleanProperty("jau_io_net_ssl_verifypeer", true) ) {
        res = curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        if( CURLE_OK != res ) {
            ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                      url.c_str(), (int)res, curl_easy_strerror(res), errorbuffer.data());
            goto errout;
        }
    }

    /* disable progress meter, set to 0L to enable it */
    res = curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url.c_str(), (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* Suppress proxy CONNECT response headers from user callbacks */
    res = curl_easy_setopt(curl_handle, CURLOPT_SUPPRESS_CONNECT_HEADERS, 1L);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url.c_str(), (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* Don't pass headers to the data stream. */
    res = curl_easy_setopt(curl_handle, CURLOPT_HEADER, 0L);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url.c_str(), (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* send header data to this function  */
    res = curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, consume_header_curl1);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url.c_str(), (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* set userdata for consume_header_curl2 */
    res = curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void*)&cg);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url.c_str(), (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* send all data to this function  */
    res = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, consume_data_curl1);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url.c_str(), (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* write the page body to this file handle */
    res = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&cg);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url.c_str(), (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* performs the tast, blocking! */
    res = curl_easy_perform(curl_handle);
    if( CURLE_OK != res ) {
        IRQ_PRINT("Error processing url %s, error %d '%s' '%s'",
                  url.c_str(), (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* cleanup curl stuff */
    DBG_PRINT("CURL: Freeing own handle %p", curl_handle);
    curl_easy_cleanup(curl_handle);
    return cg.total_read;

errout:
    DBG_PRINT("CURL: Freeing own handle %p", curl_handle);
    curl_easy_cleanup(curl_handle);
#else // USE_LIBCURL
    (void) url;
    (void) buffer;
    (void) consumer_fn;
#endif // USE_LIBCURL
    return 0;
}

void jau::io::url_header_resp::notify_complete(const int32_t response_code) noexcept {
    {
        std::unique_lock<std::mutex> lockWrite(m_sync);
        m_completed = true;
        m_response_code = response_code;
    }
    m_cv.notify_all(); // have mutex unlocked before notify_all to avoid pessimistic re-block of notified wait() thread.
}

bool jau::io::url_header_resp::wait_until_completion(const jau::fraction_i64& timeout) noexcept {
    std::unique_lock<std::mutex> lock(m_sync);
    const fraction_timespec timeout_time = getMonotonicTime() + fraction_timespec(timeout);
    while( !m_completed ) {
        if( fractions_i64::zero == timeout ) {
            m_cv.wait(lock);
        } else {
            std::cv_status s = wait_until(m_cv, lock, timeout_time );
            if( std::cv_status::timeout == s && !m_completed ) {
                return false;
            }
        }
    }
    return m_completed;
}

#ifdef USE_LIBCURL

struct curl_glue2_sync_t {
    curl_glue2_sync_t(void *_curl_handle,
                 http::PostRequestPtr _post_request,
                 ByteRingbuffer *_buffer,
                 SyncStreamResponseRef _response,
                 SyncStreamConsumerFunc _consumer_fn)
    : curl_handle(reinterpret_cast<CURL*>(_curl_handle)),
      post_request(std::move(_post_request)),
      buffer(_buffer),
      response_code(0),
      response(std::move(_response)),
      consumer_fn(std::move(_consumer_fn))
    {}

    CURL *curl_handle;
    http::PostRequestPtr post_request;
    ByteRingbuffer *buffer;
    int32_t response_code;
    SyncStreamResponseRef response;
    SyncStreamConsumerFunc consumer_fn;

    void interrupt_all() noexcept {
        if( buffer ) {
            buffer->interruptReader();
        }
        response->header_resp.notify_complete(response_code);
    }
    void set_end_of_input() noexcept {
        if( buffer ) {
            buffer->set_end_of_input(true);
        }
        response->header_resp.notify_complete(response_code);
    }
};

struct curl_glue2_async_t {
    curl_glue2_async_t(void *_curl_handle,
                 http::PostRequestPtr _post_request,
                 ByteRingbuffer *_buffer,
                 AsyncStreamResponseRef _response,
                 AsyncStreamConsumerFunc _consumer_fn)
    : curl_handle(reinterpret_cast<CURL*>(_curl_handle)),
      post_request(std::move(_post_request)),
      buffer(_buffer),
      response_code(0),
      response(std::move(_response)),
      consumer_fn(std::move(_consumer_fn))
    {}

    CURL *curl_handle;
    http::PostRequestPtr post_request;
    ByteRingbuffer *buffer;
    int32_t response_code;
    AsyncStreamResponseRef response;
    AsyncStreamConsumerFunc consumer_fn;

    void interrupt_all() noexcept {
        if( buffer ) {
            buffer->interruptReader();
        }
        response->header_resp.notify_complete(response_code);
    }
    void set_end_of_input() noexcept {
        if( buffer ) {
            buffer->set_end_of_input(true);
        }
        response->header_resp.notify_complete(response_code);
    }
};

static size_t consume_header_curl2_sync(char *buffer, size_t size, size_t nmemb, void *userdata) noexcept {
    curl_glue2_sync_t * cg = (curl_glue2_sync_t*)userdata;
    SyncStreamResponse& response = *cg->response;

    if( io_result_t::NONE != response.result ) {
        // user abort!
        DBG_PRINT("consume_header_curl2_sync ABORT by User: total %" PRIi64 ", result %d, rb %s",
                response.total_read, response.result.load() );
        cg->set_end_of_input();
        return 0;
    }

    {
        long v;
        const CURLcode r = curl_easy_getinfo(cg->curl_handle, CURLINFO_RESPONSE_CODE, &v);
        if( CURLE_OK == r ) {
            cg->response_code = static_cast<int32_t>(v);
            if( 400 <= v ) {
                IRQ_PRINT("response_code: %ld", v);
                response.result = io_result_t::FAILED;
                cg->set_end_of_input();
                return 0;
            } else {
                DBG_PRINT("consume_header_curl2.0 response_code: %ld", v);
            }
        }
    }
    if( !response.has_content_length ) {
        curl_off_t v = 0;
        const CURLcode r = curl_easy_getinfo(cg->curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &v);
        if( CURLE_OK == r ) {
            if( 0 <= v ) { // curl returns -1 if the size is not known
                response.content_length = v;
                response.has_content_length = true;
            }
        }
    }
    const size_t realsize = size * nmemb;

    if( 2 == realsize && 0x0d == buffer[0] && 0x0a == buffer[1] ) {
        response.header_resp.notify_complete(cg->response_code);
        DBG_PRINT("consume_header_curl2.0 header_completed");
    }

    if( false ) {
        DBG_PRINT("consume_header_curl2.X realsize %zu, total %" PRIu64 " / ( content_len has %d, size %" PRIu64 " ), result %d",
               realsize, response.total_read, response.has_content_length, response.content_length, response.result.load() );
        std::string blob(buffer, realsize);
        jau::PLAIN_PRINT(true, "%s", jau::toHexString((uint8_t*)buffer, realsize, jau::lb_endian_t::little).c_str());
        jau::PLAIN_PRINT(true, "%s", blob.c_str());
    }

    return realsize;
}

static size_t consume_header_curl2_async(char *buffer, size_t size, size_t nmemb, void *userdata) noexcept {
    curl_glue2_async_t * cg = (curl_glue2_async_t*)userdata;
    AsyncStreamResponse& response = *cg->response;

    if( io_result_t::NONE != response.result ) {
        // user abort!
        const std::string s = cg->buffer ? cg->buffer->toString() : "null";
        DBG_PRINT("consume_header_curl2 ABORT by User: total %" PRIi64 ", result %d, rb %s",
                response.total_read.load(), response.result.load(), s.c_str() );
        cg->set_end_of_input();
        return 0;
    }

    {
        long v;
        const CURLcode r = curl_easy_getinfo(cg->curl_handle, CURLINFO_RESPONSE_CODE, &v);
        if( CURLE_OK == r ) {
            cg->response_code = static_cast<int32_t>(v);
            if( 400 <= v ) {
                IRQ_PRINT("response_code: %ld", v);
                response.result = io_result_t::FAILED;
                cg->set_end_of_input();
                return 0;
            } else {
                DBG_PRINT("consume_header_curl2.0 response_code: %ld", v);
            }
        }
    }
    if( !response.has_content_length ) {
        curl_off_t v = 0;
        const CURLcode r = curl_easy_getinfo(cg->curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &v);
        if( CURLE_OK == r ) {
            if( 0 <= v ) { // curl returns -1 if the size is not known
                response.content_length = v;
                response.has_content_length = true;
            }
        }
    }
    const size_t realsize = size * nmemb;

    if( 2 == realsize && 0x0d == buffer[0] && 0x0a == buffer[1] ) {
        response.header_resp.notify_complete(cg->response_code);
        DBG_PRINT("consume_header_curl2.0 header_completed");
    }

    if( false ) {
        const std::string s = cg->buffer ? cg->buffer->toString() : "null";
        DBG_PRINT("consume_header_curl2.X realsize %zu, total %" PRIu64 " / ( content_len has %d, size %" PRIu64 " ), result %d, rb %s",
               realsize, response.total_read.load(), response.has_content_length.load(), response.content_length.load(), response.result.load(), s.c_str() );
        std::string blob(buffer, realsize);
        jau::PLAIN_PRINT(true, "%s", jau::toHexString((uint8_t*)buffer, realsize, jau::lb_endian_t::little).c_str());
        jau::PLAIN_PRINT(true, "%s", blob.c_str());
    }

    return realsize;
}

static size_t consume_data_curl2_sync(char *ptr, size_t size, size_t nmemb, void *userdata) noexcept {
    curl_glue2_sync_t * cg = (curl_glue2_sync_t*)userdata;
    SyncStreamResponse& response = *cg->response;

    if( io_result_t::NONE != response.result ) {
        // user abort!
        // user abort!
        const std::string s = cg->buffer ? cg->buffer->toString() : "null";
        DBG_PRINT("consume_data_curl2 ABORT by User: total %" PRIi64 ", result %d, rb %s",
                response.total_read, response.result.load(), s.c_str() );
        cg->set_end_of_input();
        return 0;
    }

    if( !response.has_content_length ) {
        curl_off_t v = 0;
        const CURLcode r = curl_easy_getinfo(cg->curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &v);
        if( CURLE_OK == r ) {
            if( 0 <= v ) { // curl returns -1 if the size if not known
                response.content_length = v;
                response.has_content_length = true;
            }
        }
    }

    // Ensure header completion is being sent
    if( !response.header_resp.completed() ) {
        response.header_resp.notify_complete();
    }

    const size_t realsize = size * nmemb;
    if( jau::environment::get().debug ) {
        const std::string s = cg->buffer ? cg->buffer->toString() : "null";
        DBG_PRINT("consume_data_curl2.0 realsize %zu, rb %s", realsize, s.c_str() );
    }
    if( cg->buffer ) {
        bool timeout_occured;
        if( !cg->buffer->putBlocking(reinterpret_cast<uint8_t*>(ptr),
                                     reinterpret_cast<uint8_t*>(ptr)+realsize, 0_s, timeout_occured) ) {
            DBG_PRINT("consume_data_curl2 Failed put: total %" PRIi64 ", result %d, timeout %d, rb %s",
                    response.total_read, response.result.load(), timeout_occured, cg->buffer->toString().c_str() );
            if( timeout_occured ) {
                cg->set_end_of_input();
            }
            return 0;
        }
    }

    response.total_read += realsize;
    const bool is_final = 0 == realsize ||
                          response.has_content_length ? response.total_read >= response.content_length : false;
    if( is_final ) {
        response.result = io_result_t::SUCCESS;
        cg->set_end_of_input();
    }
    if( cg->consumer_fn ) {
        try {
            if( !cg->consumer_fn(*cg->response, reinterpret_cast<uint8_t*>(ptr), realsize, is_final) ) {
                return 0; // end streaming
            }
        } catch (std::exception &e) {
            ERR_PRINT("jau::io::read_url_stream: Caught exception: %s", e.what());
            return 0; // end streaming
        }
    }

    if( jau::environment::get().debug ) {
        const std::string s = cg->buffer ? cg->buffer->toString() : "null";
        DBG_PRINT("consume_data_curl2.X realsize %zu, total %" PRIu64 " / ( content_len has %d, size %" PRIu64 " ), is_final %d, result %d, rb %s",
               realsize, response.total_read, response.has_content_length, response.content_length, is_final, response.result.load(), s.c_str() );
    }

    return realsize;
}

static size_t consume_data_curl2_async(char *ptr, size_t size, size_t nmemb, void *userdata) noexcept {
    curl_glue2_async_t * cg = (curl_glue2_async_t*)userdata;
    AsyncStreamResponse& response = *cg->response;

    if( io_result_t::NONE != response.result ) {
        // user abort!
        const std::string s = cg->buffer ? cg->buffer->toString() : "null";
        DBG_PRINT("consume_data_curl2 ABORT by User: total %" PRIi64 ", result %d, rb %s",
                response.total_read.load(), response.result.load(), s.c_str() );
        cg->set_end_of_input();
        return 0;
    }

    if( !response.has_content_length ) {
        curl_off_t v = 0;
        const CURLcode r = curl_easy_getinfo(cg->curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &v);
        if( CURLE_OK == r ) {
            if( 0 <= v ) { // curl returns -1 if the size if not known
                response.content_length = v;
                response.has_content_length = true;
            }
        }
    }

    // Ensure header completion is being sent
    if( !response.header_resp.completed() ) {
        response.header_resp.notify_complete();
    }

    const size_t realsize = size * nmemb;
    if( jau::environment::get().debug ) {
        const std::string s = cg->buffer ? cg->buffer->toString() : "null";
        DBG_PRINT("consume_data_curl2.0 realsize %zu, rb %s", realsize, s.c_str() );
    }
    if( cg->buffer ) {
        bool timeout_occured;
        if( !cg->buffer->putBlocking(reinterpret_cast<uint8_t*>(ptr),
                                     reinterpret_cast<uint8_t*>(ptr)+realsize, 0_s, timeout_occured) ) {
            DBG_PRINT("consume_data_curl2 Failed put: total %" PRIi64 ", result %d, timeout %d, rb %s",
                    response.total_read.load(), response.result.load(), timeout_occured, cg->buffer->toString().c_str() );
            if( timeout_occured ) {
                cg->set_end_of_input();
            }
            return 0;
        }
    }

    response.total_read.fetch_add(realsize);
    const bool is_final = 0 == realsize ||
                          response.has_content_length ? response.total_read >= response.content_length : false;
    if( is_final ) {
        response.result = io_result_t::SUCCESS;
        cg->set_end_of_input();
    }
    if( cg->consumer_fn ) {
        try {
            if( !cg->consumer_fn(*cg->response, reinterpret_cast<uint8_t*>(ptr), realsize, is_final) ) {
                return 0; // end streaming
            }
        } catch (std::exception &e) {
            ERR_PRINT("jau::io::read_url_stream: Caught exception: %s", e.what());
            return 0; // end streaming
        }
    }

    if( jau::environment::get().debug ) {
        const std::string s = cg->buffer ? cg->buffer->toString() : "null";
        DBG_PRINT("consume_data_curl2.X realsize %zu, total %" PRIu64 " / ( content_len has %d, size %" PRIu64 " ), is_final %d, result %d, rb %s",
               realsize, response.total_read.load(), response.has_content_length.load(), response.content_length.load(), is_final, response.result.load(), s.c_str() );
    }

    return realsize;
}

static bool read_url_stream_impl(CURL *curl_handle, std::vector<char>& errorbuffer,
                                 const char *url, http::PostRequest *post_request,
                                 relaxed_atomic_io_result_t& result,
                                 curl_write_callback header_cb, curl_write_callback write_cb, void* ctx_data) noexcept
{
    struct curl_slist *header_slist = nullptr;
    CURLcode res;

    res = curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorbuffer.data());
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s'",
                  url, (int)res, curl_easy_strerror(res));
        goto errout;
    }

    /* set URL to get here */
    res = curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url, (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    if( nullptr != post_request ) {
        http::PostRequest &post = *post_request;
        // slist1 = curl_slist_append(slist1, "Content-Type: application/json");
        // slist1 = curl_slist_append(slist1, "Accept: application/json");
        if( post.header.size() > 0 ) {
            for (const std::pair<const std::string, std::string>& n : post.header) {
                std::string v = n.first+": "+n.second;
                header_slist = curl_slist_append(header_slist, v.c_str());
            }
            curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, header_slist);
            if( CURLE_OK != res ) {
                ERR_PRINT("Error setting up POST header, error %d '%s' '%s'",
                          (int)res, curl_easy_strerror(res), errorbuffer.data());
                goto errout;
            }
        }

        res = curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, post.body.data());
        if( CURLE_OK != res ) {
            ERR_PRINT("Error setting up POST fields, error %d '%s' '%s'",
                      (int)res, curl_easy_strerror(res), errorbuffer.data());
            goto errout;
        }
    }
    /* Switch on full protocol/debug output while testing */
    res = curl_easy_setopt(curl_handle, CURLOPT_VERBOSE,
                           jau::environment::getBooleanProperty("jau_io_net_verbose", false) ? 1L : 0L );
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url, (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    if( !jau::environment::getBooleanProperty("jau_io_net_ssl_verifypeer", true) ) {
        res = curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        if( CURLE_OK != res ) {
            ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                      url, (int)res, curl_easy_strerror(res), errorbuffer.data());
            goto errout;
        }
    }

    /* disable progress meter, set to 0L to enable it */
    res = curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url, (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* Suppress proxy CONNECT response headers from user callbacks */
    res = curl_easy_setopt(curl_handle, CURLOPT_SUPPRESS_CONNECT_HEADERS, 1L);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url, (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* Don't pass headers to the data stream. */
    res = curl_easy_setopt(curl_handle, CURLOPT_HEADER, 0L);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url, (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* send header data to this function  */
    res = curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, header_cb);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url, (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* set userdata for consume_header_curl2 */
    res = curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, ctx_data);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url, (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* send received data to this function  */
    res = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_cb);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url, (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* set userdata for consume_data_curl2 */
    res = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, ctx_data);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d '%s' '%s'",
                  url, (int)res, curl_easy_strerror(res), errorbuffer.data());
        goto errout;
    }

    /* performs the tast, blocking! */
    res = curl_easy_perform(curl_handle);
    if( CURLE_OK != res ) {
        if( io_result_t::NONE == result ) {
            // Error during normal processing
            IRQ_PRINT("Error processing url %s, error %d '%s' '%s'",
                      url, (int)res, curl_easy_strerror(res), errorbuffer.data());
        } else {
            // User aborted or response code error detected via consume_header_curl2
            DBG_PRINT("Processing aborted url %s, error %d '%s' '%s'",
                      url, (int)res, curl_easy_strerror(res), errorbuffer.data());
        }
        goto errout;
    }

    if( nullptr != header_slist ) {
        curl_slist_free_all(header_slist);
    }
    return true;

errout:
    if( nullptr != header_slist ) {
        curl_slist_free_all(header_slist);
    }
    return false;
}

static void read_url_stream_sync(const char *url, curl_glue2_sync_t& cg) noexcept {
    std::vector<char> errorbuffer;
    errorbuffer.reserve(CURL_ERROR_SIZE);

    bool owns_curl_handle = false;
    CURL *curl_handle;
    if( nullptr == cg.curl_handle ) {
        /* init the curl session */
        owns_curl_handle = true;
        curl_handle = curl_easy_init();
        if( nullptr == curl_handle ) {
            ERR_PRINT("Error setting up url %s, null curl handle", url);
            goto errout;
        }
        cg.curl_handle = curl_handle;
        DBG_PRINT("CURL: Created own handle %p", curl_handle);
    } else {
        curl_handle = cg.curl_handle;
        DBG_PRINT("CURL: Reusing own handle %p", curl_handle);
    }

    if( !read_url_stream_impl(curl_handle, errorbuffer,
                              url, cg.post_request.get(), cg.response->result,
                              consume_header_curl2_sync, consume_data_curl2_sync, (void*)&cg) ) {
        goto errout;
    }

    // Ensure header completion is being sent
    if( !cg.response->header_resp.completed() ) {
        cg.response->header_resp.notify_complete();
    }
    if( cg.response->result != io_result_t::SUCCESS ) {
        cg.response->result = io_result_t::SUCCESS;
        if( cg.consumer_fn ) {
            try {
                cg.consumer_fn(*cg.response, nullptr, 0, true);
            } catch (std::exception &e) {
                ERR_PRINT("jau::io::read_url_stream: Caught exception: %s", e.what());
                goto errout;
            }
        }
    }
    goto cleanup;

errout:
    cg.response->result = io_result_t::FAILED;
    cg.set_end_of_input();

cleanup:
    if( owns_curl_handle && nullptr != curl_handle ) {
        DBG_PRINT("CURL: Freeing own handle %p", curl_handle);
        curl_easy_cleanup(curl_handle);
        cg.curl_handle = nullptr;
    }
}

static void read_url_stream_async(const char *url, std::unique_ptr<curl_glue2_async_t> && cg) noexcept {
    std::vector<char> errorbuffer;
    errorbuffer.reserve(CURL_ERROR_SIZE);

    bool owns_curl_handle = false;
    CURL *curl_handle;
    if( nullptr == cg->curl_handle ) {
        /* init the curl session */
        owns_curl_handle = true;
        curl_handle = curl_easy_init();
        if( nullptr == curl_handle ) {
            ERR_PRINT("Error setting up url %s, null curl handle", url);
            goto errout;
        }
        cg->curl_handle = curl_handle;
        DBG_PRINT("CURL: Created own handle %p", curl_handle);
    } else {
        curl_handle = cg->curl_handle;
        DBG_PRINT("CURL: Reusing own handle %p", curl_handle);
    }

    if( !read_url_stream_impl(curl_handle, errorbuffer,
                              url, cg->post_request.get(), cg->response->result,
                              consume_header_curl2_async, consume_data_curl2_async, (void*)cg.get()) ) {
        goto errout;
    }

    // Ensure header completion is being sent
    if( !cg->response->header_resp.completed() ) {
        cg->response->header_resp.notify_complete();
    }
    if( cg->response->result != io_result_t::SUCCESS ) {
        cg->response->result = io_result_t::SUCCESS;
        if( cg->consumer_fn ) {
            try {
                cg->consumer_fn(*cg->response, nullptr, 0, true);
            } catch (std::exception &e) {
                ERR_PRINT("jau::io::read_url_stream: Caught exception: %s", e.what());
                goto errout;
            }
        }
    }
    goto cleanup;

errout:
    cg->response->result = io_result_t::FAILED;
    cg->set_end_of_input();

cleanup:
    if( owns_curl_handle && nullptr != curl_handle ) {
        DBG_PRINT("CURL: Freeing own handle %p", curl_handle);
        curl_easy_cleanup(curl_handle);
        cg->curl_handle = nullptr;
    }
}

#endif // USE_LIBCURL


net_tk_handle jau::io::create_net_tk_handle() noexcept {
#ifdef USE_LIBCURL
    CURL* h = ::curl_easy_init();
    DBG_PRINT("CURL: Created user handle %p", h);
    return static_cast<jau::io::net_tk_handle>( h );
#else
    return static_cast<jau::io::net_tk_handle>( nullptr );
#endif
}
void jau::io::free_net_tk_handle(net_tk_handle handle) noexcept {
#ifdef USE_LIBCURL
    CURL* h = static_cast<CURL*>(handle);
    if( nullptr != h ) {
        DBG_PRINT("CURL: Freeing user handle %p", h);
        curl_easy_cleanup(h);
    }
#else
    (void)handle;
#endif
}

SyncStreamResponseRef jau::io::read_url_stream_sync(net_tk_handle handle, const std::string& url,
                                                    http::PostRequestPtr httpPostReq, ByteRingbuffer *buffer,
                                                    const SyncStreamConsumerFunc& consumer_fn) noexcept {
    /* init user referenced values */
    SyncStreamResponseRef res = std::make_shared<SyncStreamResponse>(handle);

#ifdef USE_LIBCURL
    if( !uri_tk::protocol_supported(url) ) {
#endif // USE_LIBCURL
        (void)httpPostReq;
        (void)consumer_fn;
        res->result = io::io_result_t::FAILED;
        res->header_resp.notify_complete();
        // buffer.set_end_of_input(true);
        const std::string_view scheme = uri_tk::get_scheme(url);
        DBG_PRINT("Protocol of given uri-scheme '%s' not supported. Supported protocols [%s].",
                std::string(scheme).c_str(), jau::to_string(uri_tk::supported_protocols(), ",").c_str());
        return res;
#ifdef USE_LIBCURL
    }
    curl_glue2_sync_t cg (handle, std::move(httpPostReq), buffer, res, consumer_fn );
    read_url_stream_sync(url.c_str(), cg);
    return res;
#endif // USE_LIBCURL
}

AsyncStreamResponseRef jau::io::read_url_stream_async(net_tk_handle handle, const std::string& url,
                                                      http::PostRequestPtr httpPostReq, ByteRingbuffer *buffer,
                                                      const AsyncStreamConsumerFunc& consumer_fn) noexcept {
    /* init user referenced values */
    AsyncStreamResponseRef res = std::make_shared<AsyncStreamResponse>(handle);

#ifdef USE_LIBCURL
    if( !uri_tk::protocol_supported(url) ) {
#endif // USE_LIBCURL
        (void)httpPostReq;
        (void)buffer;
        (void)consumer_fn;
        res->result = io::io_result_t::FAILED;
        res->header_resp.notify_complete();
        // buffer.set_end_of_input(true);
        const std::string_view scheme = uri_tk::get_scheme(url);
        DBG_PRINT("Protocol of given uri-scheme '%s' not supported. Supported protocols [%s].",
                std::string(scheme).c_str(), jau::to_string(uri_tk::supported_protocols(), ",").c_str());
        return res;
#ifdef USE_LIBCURL
    }

    std::unique_ptr<curl_glue2_async_t> cg ( std::make_unique<curl_glue2_async_t>(handle, std::move(httpPostReq),
                                                                                  buffer, res, consumer_fn ) );

    res->thread = std::thread(&::read_url_stream_async, url.c_str(), std::move(cg)); // @suppress("Invalid arguments")
    return res;
#endif // USE_LIBCURL
}

void jau::io::print_stats(const std::string& prefix, const uint64_t& out_bytes_total, const jau::fraction_i64& td) noexcept {
    jau::PLAIN_PRINT(true, "%s: Duration %s s, %s ms", prefix.c_str(),
            td.toString().c_str(), jau::to_decstring(td.to_ms()).c_str());

    if( out_bytes_total >= 100'000'000 ) {
        jau::PLAIN_PRINT(true, "%s: Size %s MB", prefix.c_str(),
                jau::to_decstring(std::llround((double)out_bytes_total/1'000'000.0)).c_str());
    } else if( out_bytes_total >= 100'000 ) {
        jau::PLAIN_PRINT(true, "%s: Size %s KB", prefix.c_str(),
                jau::to_decstring(std::llround((double)out_bytes_total/1'000.0)).c_str());
    } else {
        jau::PLAIN_PRINT(true, "%s: Size %s B", prefix.c_str(),
                jau::to_decstring(out_bytes_total).c_str());
    }

    const uint64_t _rate_bps = std::llround( (double)out_bytes_total / td.to_double() ); // bytes per second
    const uint64_t _rate_bitps = std::llround( ( (double)out_bytes_total * 8.0 ) / td.to_double() ); // bits per second

    if( _rate_bitps >= 100'000'000 ) {
        jau::PLAIN_PRINT(true, "%s: Bitrate %s Mbit/s, %s MB/s", prefix.c_str(),
                jau::to_decstring(std::llround((double)_rate_bitps/1'000'000.0)).c_str(),
                jau::to_decstring(std::llround((double)_rate_bps/1'000'000.0)).c_str());
    } else if( _rate_bitps >= 100'000 ) {
        jau::PLAIN_PRINT(true, "%s: Bitrate %s kbit/s, %s kB/s", prefix.c_str(),
                jau::to_decstring(std::llround((double)_rate_bitps/1'000.0)).c_str(),
                jau::to_decstring(std::llround((double)_rate_bps/1'000.0)).c_str());
    } else {
        jau::PLAIN_PRINT(true, "%s: Bitrate %s bit/s, %s B/s", prefix.c_str(),
                jau::to_decstring(_rate_bitps).c_str(),
                jau::to_decstring(_rate_bps).c_str());
    }
}

