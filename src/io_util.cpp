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

#include <curl/curl.h>

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
        ByteInStream_File in(input_file, true /* use_binary */);
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
        }
    }
    return total;
}

struct curl_glue1_t {
    CURL *curl_handle;
    bool has_content_length;
    uint64_t content_length;
    uint64_t total_read;
    secure_vector<uint8_t>& buffer;
    StreamConsumerFunc consumer_fn;
};

static size_t consume_curl1(void *ptr, size_t size, size_t nmemb, void *stream) noexcept {
    curl_glue1_t * cg = (curl_glue1_t*)stream;

    if( !cg->has_content_length ) {
        curl_off_t v = 0;
        CURLcode r = curl_easy_getinfo(cg->curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &v);
        if( !r ) {
            cg->content_length = v;
            cg->has_content_length = true;
        }
    }
    const size_t realsize = size * nmemb;
    DBG_PRINT("consume_curl1.0 realsize %zu", realsize);
    cg->buffer.resize(realsize);
    memcpy(cg->buffer.data(), ptr, realsize);

    cg->total_read += realsize;
    const bool is_final = 0 == realsize ||
                          cg->has_content_length ? cg->total_read >= cg->content_length : false;

    DBG_PRINT("consume_curl1.X realsize %zu, total %" PRIu64 " / ( content_len %" PRIu64 " ), is_final %d",
           realsize, cg->total_read, cg->content_length, is_final );

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

uint64_t jau::io::read_url_stream(const std::string& url,
                                  secure_vector<uint8_t>& buffer,
                                  StreamConsumerFunc consumer_fn) noexcept {
    std::vector<char> errorbuffer;
    errorbuffer.reserve(CURL_ERROR_SIZE);
    CURLcode res;

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

    /* send all data to this function  */
    res = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, consume_curl1);
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
        ERR_PRINT("Error processing url %s, error %d %d",
                  url.c_str(), (int)res, errorbuffer.data());
        goto errout;
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);
    return cg.total_read;

errout:
    curl_easy_cleanup(curl_handle);
    return 0;
}

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

static size_t consume_curl2(void *ptr, size_t size, size_t nmemb, void *stream) noexcept {
    curl_glue2_t * cg = (curl_glue2_t*)stream;

    if( async_io_result_t::NONE!= cg->result ) {
        // user abort!
        DBG_PRINT("consume_curl2 ABORT by User: total %" PRIi64 ", result %d, rb %s",
                cg->total_read.load(), cg->result.load(), cg->buffer.toString().c_str() );
        return 0;
    }

    if( !cg->has_content_length ) {
        curl_off_t v = 0;
        const CURLcode r = curl_easy_getinfo(cg->curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &v);
        if( CURLE_OK == r ) {
            cg->content_length = v;
            cg->has_content_length = true;
        }
    }
    const size_t realsize = size * nmemb;
    DBG_PRINT("consume_curl2.0 realsize %zu, rb %s", realsize, cg->buffer.toString().c_str() );
    cg->buffer.putBlocking(reinterpret_cast<uint8_t*>(ptr),
                           reinterpret_cast<uint8_t*>(ptr)+realsize, 0_s);

    cg->total_read.fetch_add(realsize);
    const bool is_final = 0 == realsize ||
                          cg->has_content_length ? cg->total_read >= cg->content_length : false;
    if( is_final ) {
        cg->result = async_io_result_t::SUCCESS;
    }

    DBG_PRINT("consume_curl2.X realsize %zu, total %" PRIu64 " / ( content_len %" PRIu64 " ), is_final %d, result %d, rb %s",
           realsize, cg->total_read.load(), cg->result.load(), cg->content_length.load(), is_final, cg->buffer.toString().c_str() );

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

    /* send all data to this function  */
    res = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, consume_curl2);
    if( CURLE_OK != res ) {
        ERR_PRINT("Error setting up url %s, error %d %d",
                  url, (int)res, errorbuffer.data());
        goto errout;
    }

    /* write the page body to this file handle */
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
            ERR_PRINT("Error processing url %s, error %d %d",
                      url, (int)res, errorbuffer.data());
        } else {
            // User aborted
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

std::thread jau::io::read_url_stream(const std::string& url,
                                     ByteRingbuffer& buffer,
                                     jau::relaxed_atomic_bool& has_content_length,
                                     jau::relaxed_atomic_uint64& content_length,
                                     jau::relaxed_atomic_uint64& total_read,
                                     relaxed_atomic_async_io_result_t& result) noexcept {
    /* init user referenced values */
    has_content_length = false;
    content_length = 0;
    total_read = 0;
    result = io::async_io_result_t::NONE;

    if( buffer.capacity() < BEST_URLSTREAM_RINGBUFFER_SIZE ) {
        buffer.recapacity( BEST_URLSTREAM_RINGBUFFER_SIZE );
    }

    std::unique_ptr<curl_glue2_t> cg ( std::make_unique<curl_glue2_t>(nullptr, has_content_length, content_length, total_read, buffer, result ) );

    return std::thread(&::read_url_stream_thread, url.c_str(), std::move(cg)); // @suppress("Invalid arguments")
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

