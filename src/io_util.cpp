/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
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

#include <fstream>
#include <iostream>
#include <chrono>

// #include <botan_all.h>

#include <jau/debug.hpp>
#include <jau/io_util.hpp>
#include <jau/byte_stream.hpp>

#ifdef USE_LIBCURL
    #include <curl/curl.h>
#endif

#include <thread>
#include <pthread.h>

using namespace jau::io;
using namespace jau::fractions_i64_literals;


uint64_t jau::io::read_file(const std::string& input_file,
                            secure_vector<uint8_t>& buffer,
                            StreamConsumerFunc consumer_fn) noexcept
{
    if(input_file == "-") {
        ByteInStream_istream in(std::cin);
        return read_stream(in, buffer, consumer_fn);
    } else {
        ByteInStream_File in(input_file);
        return read_stream(in, buffer, consumer_fn);
    }
}

uint64_t jau::io::read_stream(ByteInStream& in,
                              secure_vector<uint8_t>& buffer,
                              StreamConsumerFunc consumer_fn) noexcept {
    uint64_t total = 0;
    bool has_more = !in.end_of_data();
    while( has_more ) {
        if( in.check_available(1) ) { // at least one byte to stream ..
            buffer.resize(buffer.capacity());
            const uint64_t got = in.read(buffer.data(), buffer.capacity());

            buffer.resize(got);
            total += got;
            has_more = 1 <= got && !in.end_of_data() && ( !in.has_content_size() || total < in.content_size() );
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
    }
    return total;
}

static uint64_t _read_buffer(ByteInStream& in,
                             secure_vector<uint8_t>& buffer) noexcept {
    if( in.check_available(1) ) { // at least one byte to stream ..
        buffer.resize(buffer.capacity());
        const uint64_t got = in.read(buffer.data(), buffer.capacity());
        buffer.resize(got);
        return got;
    }
    return 0;
}

uint64_t jau::io::read_stream(ByteInStream& in,
                              secure_vector<uint8_t>& buffer1, secure_vector<uint8_t>& buffer2,
                              StreamConsumerFunc consumer_fn) noexcept {
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
        eof_read = 0 == got || in.end_of_data() || ( in.has_content_size() && total_read >= in.content_size() );
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
            eof_read = 0 == got || in.end_of_data() || ( in.has_content_size() && total_read >= in.content_size() );
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
        res.push_back( std::string_view(cvid->protocols[i]) );
    }
#endif // USE_LIBCURL
    return res;
}

static bool _is_scheme_valid(const std::string_view& scheme) noexcept {
    if ( scheme.empty() ) {
        return false;
    }
    auto pos = std::find_if_not(scheme.begin(), scheme.end(), [&](char c){
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
    auto it = std::find(protos.cbegin(), protos.cend(), scheme);
    return protos.cend() != it;
}

bool jau::io::uri_tk::is_local_file_protocol(const std::string_view& uri) noexcept {
    return 0 == uri.find("file://");
}

#ifdef USE_LIBCURL

struct curl_glue1_t {
    CURL *curl_handle;
    bool has_content_length;
    uint64_t content_length;
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
                                  StreamConsumerFunc consumer_fn) noexcept {
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
    if( nullptr == curl_handle ) {
        ERR_PRINT("Error setting up url %s, null curl handle", url.c_str());
        return 0;
    }

    curl_glue1_t cg = { curl_handle, false, 0, 0, buffer, consumer_fn };

    res = curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorbuffer.data());
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url.c_str(), (int)res, curl_easy_strerror(res));
        goto errout;
    }

    /* set URL to get here */
    res = curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url.c_str(), (int)res, errorbuffer.data());
        goto errout;
    }

    /* Switch on full protocol/debug output while testing */
    res = curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url.c_str(), (int)res, errorbuffer.data());
        goto errout;
    }

    /* disable progress meter, set to 0L to enable it */
    res = curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url.c_str(), (int)res, errorbuffer.data());
        goto errout;
    }

    /* Suppress proxy CONNECT response headers from user callbacks */
    res = curl_easy_setopt(curl_handle, CURLOPT_SUPPRESS_CONNECT_HEADERS, 1L);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url.c_str(), (int)res, errorbuffer.data());
        goto errout;
    }

    /* Don't pass headers to the data stream. */
    res = curl_easy_setopt(curl_handle, CURLOPT_HEADER, 0L);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                url.c_str(), (int)res, errorbuffer.data());
        goto errout;
    }

    /* send header data to this function  */
    res = curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, consume_header_curl1);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                url.c_str(), (int)res, errorbuffer.data());
        goto errout;
    }

    /* set userdata for consume_header_curl2 */
    res = curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void*)&cg);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                url.c_str(), (int)res, errorbuffer.data());
        goto errout;
    }

    /* send all data to this function  */
    res = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, consume_data_curl1);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url.c_str(), (int)res, errorbuffer.data());
        goto errout;
    }

    /* write the page body to this file handle */
    res = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&cg);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url.c_str(), (int)res, errorbuffer.data());
        goto errout;
    }

    /* performs the tast, blocking! */
    res = curl_easy_perform(curl_handle);
    if( CURLE_OK != res ) {
        IRQ_PRINT("processing url %s, error %d %d",
                  url.c_str(), (int)res, errorbuffer.data());
        goto errout;
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);
    return cg.total_read;

errout:
    curl_easy_cleanup(curl_handle);
#else // USE_LIBCURL
    (void) url;
    (void) buffer;
    (void) consumer_fn;
#endif // USE_LIBCURL
    return 0;
}

#ifdef USE_LIBCURL

struct curl_glue2_t {
    curl_glue2_t(CURL *_curl_handle,
                 jau::relaxed_atomic_bool& _has_content_length,
                 jau::relaxed_atomic_uint64& _content_length,
                 jau::relaxed_atomic_uint64& _total_read,
                 ByteRingbuffer& _buffer,
                 relaxed_atomic_async_io_result_t& _result)
    : curl_handle(_curl_handle),
      has_content_length(_has_content_length),
      content_length(_content_length),
      total_read(_total_read),
      buffer(_buffer),
      result(_result)
    {}

    CURL *curl_handle;
    jau::relaxed_atomic_bool& has_content_length;
    jau::relaxed_atomic_uint64& content_length;
    jau::relaxed_atomic_uint64& total_read;
    ByteRingbuffer& buffer;
    relaxed_atomic_async_io_result_t& result;
};

static size_t consume_header_curl2(char *buffer, size_t size, size_t nmemb, void *userdata) noexcept {
    curl_glue2_t * cg = (curl_glue2_t*)userdata;

    if( async_io_result_t::NONE!= cg->result ) {
        // user abort!
        DBG_PRINT("consume_header_curl2 ABORT by User: total %" PRIi64 ", result %d, rb %s",
                cg->total_read.load(), cg->result.load(), cg->buffer.toString().c_str() );
        cg->buffer.interruptReader();
        return 0;
    }

    {
        long v;
        const CURLcode r = curl_easy_getinfo(cg->curl_handle, CURLINFO_RESPONSE_CODE, &v);
        if( CURLE_OK == r ) {
            if( 400 <= v ) {
                IRQ_PRINT("response_code: %ld", v);
                cg->result = async_io_result_t::FAILED;
                cg->buffer.interruptReader();
                return 0;
            } else {
                DBG_PRINT("consume_header_curl2.0 response_code: %ld", v);
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
        DBG_PRINT("consume_header_curl2.X realsize %zu, total %" PRIu64 " / ( content_len has %d, size %" PRIu64 " ), result %d, rb %s",
               realsize, cg->total_read.load(), cg->has_content_length.load(), cg->content_length.load(), cg->result.load(), cg->buffer.toString().c_str() );
        std::string blob(buffer, realsize);
        jau::PLAIN_PRINT(true, "%s", blob.c_str());
    }

    return realsize;
}

static size_t consume_data_curl2(char *ptr, size_t size, size_t nmemb, void *userdata) noexcept {
    curl_glue2_t * cg = (curl_glue2_t*)userdata;

    if( async_io_result_t::NONE!= cg->result ) {
        // user abort!
        DBG_PRINT("consume_data_curl2 ABORT by User: total %" PRIi64 ", result %d, rb %s",
                cg->total_read.load(), cg->result.load(), cg->buffer.toString().c_str() );
        cg->buffer.interruptReader();
        return 0;
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
    DBG_PRINT("consume_data_curl2.0 realsize %zu, rb %s", realsize, cg->buffer.toString().c_str() );
    cg->buffer.putBlocking(reinterpret_cast<uint8_t*>(ptr),
                           reinterpret_cast<uint8_t*>(ptr)+realsize, 0_s);

    cg->total_read.fetch_add(realsize);
    const bool is_final = 0 == realsize ||
                          cg->has_content_length ? cg->total_read >= cg->content_length : false;
    if( is_final ) {
        cg->result = async_io_result_t::SUCCESS;
    }

    DBG_PRINT("consume_data_curl2.X realsize %zu, total %" PRIu64 " / ( content_len has %d, size %" PRIu64 " ), is_final %d, result %d, rb %s",
           realsize, cg->total_read.load(), cg->has_content_length.load(), cg->content_length.load(), is_final, cg->result.load(), cg->buffer.toString().c_str() );

    return realsize;
}

static void read_url_stream_thread(const char *url, std::unique_ptr<curl_glue2_t> && cg) noexcept {
    std::vector<char> errorbuffer;
    errorbuffer.reserve(CURL_ERROR_SIZE);
    CURLcode res;

    /* init the curl session */
    CURL *curl_handle = curl_easy_init();
    if( nullptr == curl_handle ) {
        ERR_PRINT("Error setting up url %s, null curl handle", url);
        goto errout;
    }
    cg->curl_handle = curl_handle;

    res = curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorbuffer.data());
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url, (int)res, curl_easy_strerror(res));
        goto errout;
    }

    /* set URL to get here */
    res = curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url, (int)res, errorbuffer.data());
        goto errout;
    }

    /* Switch on full protocol/debug output while testing */
    res = curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url, (int)res, errorbuffer.data());
        goto errout;
    }

    /* disable progress meter, set to 0L to enable it */
    res = curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url, (int)res, errorbuffer.data());
        goto errout;
    }

    /* Suppress proxy CONNECT response headers from user callbacks */
    res = curl_easy_setopt(curl_handle, CURLOPT_SUPPRESS_CONNECT_HEADERS, 1L);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url, (int)res, errorbuffer.data());
        goto errout;
    }

    /* Don't pass headers to the data stream. */
    res = curl_easy_setopt(curl_handle, CURLOPT_HEADER, 0L);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url, (int)res, errorbuffer.data());
        goto errout;
    }

    /* send header data to this function  */
    res = curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, consume_header_curl2);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url, (int)res, errorbuffer.data());
        goto errout;
    }

    /* set userdata for consume_header_curl2 */
    res = curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void*)cg.get());
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url, (int)res, errorbuffer.data());
        goto errout;
    }

    /* send received data to this function  */
    res = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, consume_data_curl2);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url, (int)res, errorbuffer.data());
        goto errout;
    }

    /* set userdata for consume_data_curl2 */
    res = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)cg.get());
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url, (int)res, errorbuffer.data());
        goto errout;
    }

    /* performs the tast, blocking! */
    res = curl_easy_perform(curl_handle);
    if( CURLE_OK != res ) {
        if( async_io_result_t::NONE == cg->result ) {
            // Error during normal processing
            IRQ_PRINT("Error processing url %s, error %d %d",
                      url, (int)res, errorbuffer.data());
        } else {
            // User aborted or response code error detected via consume_header_curl2
            DBG_PRINT("Processing aborted url %s, error %d %d",
                      url, (int)res, errorbuffer.data());
        }
        goto errout;
    }

    /* cleanup curl stuff */
    cg->result = async_io_result_t::SUCCESS;
    goto cleanup;

errout:
    cg->result = async_io_result_t::FAILED;

cleanup:
    if( nullptr != curl_handle ) {
        curl_easy_cleanup(curl_handle);
    }
    return;
}

#endif // USE_LIBCURL

std::unique_ptr<std::thread> jau::io::read_url_stream(const std::string& url,
                                                      ByteRingbuffer& buffer,
                                                      jau::relaxed_atomic_bool& has_content_length,
                                                      jau::relaxed_atomic_uint64& content_length,
                                                      jau::relaxed_atomic_uint64& total_read,
                                                      relaxed_atomic_async_io_result_t& result) noexcept {
    /* init user referenced values */
    has_content_length = false;
    content_length = 0;
    total_read = 0;

#ifdef USE_LIBCURL
    if( !uri_tk::protocol_supported(url) ) {
#else // USE_LIBCURL
        (void) buffer;
#endif // USE_LIBCURL
        result = io::async_io_result_t::FAILED;
        const std::string_view scheme = uri_tk::get_scheme(url);
        DBG_PRINT("Protocol of given uri-scheme '%s' not supported. Supported protocols [%s].",
                std::string(scheme).c_str(), jau::to_string(uri_tk::supported_protocols(), ",").c_str());
        return nullptr;
#ifdef USE_LIBCURL
    }
    result = io::async_io_result_t::NONE;

    if( buffer.capacity() < BEST_URLSTREAM_RINGBUFFER_SIZE ) {
        buffer.recapacity( BEST_URLSTREAM_RINGBUFFER_SIZE );
    }

    std::unique_ptr<curl_glue2_t> cg ( std::make_unique<curl_glue2_t>(nullptr, has_content_length, content_length, total_read, buffer, result ) );

    return std::make_unique<std::thread>(&::read_url_stream_thread, url.c_str(), std::move(cg)); // @suppress("Invalid arguments")
#endif // USE_LIBCURL
}

void jau::io::print_stats(const std::string& prefix, const uint64_t& out_bytes_total, const jau::fraction_i64& td) noexcept {
    jau::PLAIN_PRINT(true, "%s: Duration %s s, %s ms", prefix.c_str(),
            td.to_string().c_str(), jau::to_decstring(td.to_ms()).c_str());

    if( out_bytes_total >= 100'000'000 ) {
        jau::PLAIN_PRINT(true, "%s: Size %s MB", prefix.c_str(),
                jau::to_decstring(std::llround(out_bytes_total/1'000'000.0)).c_str());
    } else if( out_bytes_total >= 100'000 ) {
        jau::PLAIN_PRINT(true, "%s: Size %s KB", prefix.c_str(),
                jau::to_decstring(std::llround(out_bytes_total/1'000.0)).c_str());
    } else {
        jau::PLAIN_PRINT(true, "%s: Size %s B", prefix.c_str(),
                jau::to_decstring(out_bytes_total).c_str());
    }

    const uint64_t _rate_bps = std::llround( out_bytes_total / td.to_double() ); // bytes per second
    const uint64_t _rate_bitps = std::llround( ( out_bytes_total * 8.0 ) / td.to_double() ); // bits per second

    if( _rate_bitps >= 100'000'000 ) {
        jau::PLAIN_PRINT(true, "%s: Bitrate %s Mbit/s, %s MB/s", prefix.c_str(),
                jau::to_decstring(std::llround(_rate_bitps/1'000'000.0)).c_str(),
                jau::to_decstring(std::llround(_rate_bps/1'000'000.0)).c_str());
    } else if( _rate_bitps >= 100'000 ) {
        jau::PLAIN_PRINT(true, "%s: Bitrate %s kbit/s, %s kB/s", prefix.c_str(),
                jau::to_decstring(std::llround(_rate_bitps/1'000.0)).c_str(),
                jau::to_decstring(std::llround(_rate_bps/1'000.0)).c_str());
    } else {
        jau::PLAIN_PRINT(true, "%s: Bitrate %s bit/s, %s B/s", prefix.c_str(),
                jau::to_decstring(_rate_bitps).c_str(),
                jau::to_decstring(_rate_bps).c_str());
    }
}

