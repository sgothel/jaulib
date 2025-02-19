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

#include <cassert>
#include <cinttypes>
#include <cstring>

#include <thread>
#include <pthread.h>

#include <jau/test/catch2_ext.hpp>

#include <jau/file_util.hpp>
#include <jau/io_util.hpp>
#include <jau/byte_stream.hpp>

#include <jau/debug.hpp>

#include "test_httpd.hpp"

extern "C" {
    #include <unistd.h>
}

using namespace jau::fractions_i64_literals;
using namespace jau::int_literals;

class TestIOStream01 {
    public:
        const std::string url_input_root = "http://localhost:8080/";
        const std::string basename_10kiB = "testfile_data_10kiB.bin";

        TestIOStream01() {
            // produce fresh demo data

            jau::fs::remove(basename_10kiB);
            {
                std::string one_line = "Hello World, this is a test and I like it. Exactly 100 characters long. 0123456780 abcdefghjklmnop..";
                jau::io::ByteOutStream_File ofs(basename_10kiB);

                REQUIRE( ofs.good() == true );
                REQUIRE( ofs.is_open() == true );

                for(size_t i=0; i < 1024_uz * 10_uz; i+=one_line.size()) { // 10kiB
                    REQUIRE( one_line.size() == ofs.write(one_line.data(), one_line.size()) );
                }
            }
            if( jau::io::uri_tk::protocol_supported("http:") ) {
                int res = std::system("killall mini_httpd");
                (void)res;
                const std::string cwd = jau::fs::get_cwd();
                const std::string cmd = std::string(mini_httpd_exe)+" -p 8080 -l "+cwd+"/mini_httpd.log";
                jau::PLAIN_PRINT(true, "%s", cmd.c_str());
                res = std::system(cmd.c_str());
                (void)res;
            }
        }

        ~TestIOStream01() {
            if( jau::io::uri_tk::protocol_supported("http:") ) {
                int res = std::system("killall mini_httpd");
                (void)res;
            }
        }

        void test00_protocols() {
            {
                std::vector<std::string_view> protos = jau::io::uri_tk::supported_protocols();
                jau::PLAIN_PRINT(true, "test00_protocols: Supported protocols: %zu: %s", protos.size(), jau::to_string(protos, ",").c_str());
#ifdef USE_LIBCURL
                REQUIRE( 0 < protos.size() );
#else
                REQUIRE( 0 == protos.size() );
#endif
            }
            const bool http_support_expected = jau::io::uri_tk::protocol_supported("http:");
            const bool file_support_expected = jau::io::uri_tk::protocol_supported("file:");
            {
                const std::string url = url_input_root + basename_10kiB;
                REQUIRE( false == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( http_support_expected == jau::io::uri_tk::protocol_supported(url) );
            }
            {
                const std::string url = "https://localhost:8080/" + basename_10kiB;
                REQUIRE( false == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( http_support_expected == jau::io::uri_tk::protocol_supported(url) );
            }
            {
                const std::string url = "file://" + basename_10kiB;
                REQUIRE( true == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( file_support_expected == jau::io::uri_tk::protocol_supported(url) );
            }
            {
                const std::string url = "lala://localhost:8080/" + basename_10kiB;
                REQUIRE( false == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( false == jau::io::uri_tk::protocol_supported(url) );
            }
            {
                // sync read_url_stream w/ unknown protocol
                const std::string url = "lala://localhost:8080/" + basename_10kiB;
                jau::io::secure_vector<uint8_t> buffer(4096);
                size_t consumed_calls = 0;
                uint64_t consumed_total_bytes = 0;
                jau::io::StreamConsumerFunc consume = [&](jau::io::secure_vector<uint8_t>& data, bool is_final) noexcept -> bool {
                    (void)is_final;
                    consumed_calls++;
                    consumed_total_bytes += data.size();
                    return true;
                };
                uint64_t http_total_bytes = jau::io::read_url_stream(url, buffer, consume);
                REQUIRE( 0 == http_total_bytes );
                REQUIRE( consumed_total_bytes == http_total_bytes );
                REQUIRE( 0 == consumed_calls );
            }
            {
                // sync read_url_stream w/ unknown protocol
                const std::string url = "lala://localhost:8080/" + basename_10kiB;

                jau::io::ByteRingbuffer rb(jau::io::BEST_URLSTREAM_RINGBUFFER_SIZE);
                jau::io::SyncStreamResponseRef res = jau::io::read_url_stream_sync(nullptr, url, nullptr, &rb, nullptr);
                REQUIRE( res->header_resp.completed() == true );
                REQUIRE( res->has_content_length == false );
                REQUIRE( res->content_length == 0 );
                REQUIRE( res->failed() == true );
            }
            {
                // async read_url_stream w/ unknown protocol
                const std::string url = "lala://localhost:8080/" + basename_10kiB;

                jau::io::ByteRingbuffer rb(jau::io::BEST_URLSTREAM_RINGBUFFER_SIZE);
                jau::io::AsyncStreamResponseRef res = jau::io::read_url_stream_async(nullptr, url, nullptr, &rb, nullptr);
                REQUIRE( !res->thread.joinable() );
                REQUIRE( res->header_resp.completed() == true );
                REQUIRE( res->has_content_length == false );
                REQUIRE( res->content_length == 0 );
                REQUIRE( res->failed() == true );
            }
        }

        void test01_sync_ok() {
            if( !jau::io::uri_tk::protocol_supported("http:") ) {
                jau::PLAIN_PRINT(true, "http not supported, abort\n");
                return;
            }
            const jau::fs::file_stats in_stats(basename_10kiB);
            const size_t file_size = in_stats.size();
            const std::string url_input = url_input_root + basename_10kiB;

            jau::io::ByteOutStream_File outfile("testfile01_01_out.bin");
            REQUIRE( outfile.good() );
            REQUIRE( outfile.is_open() );

            jau::io::secure_vector<uint8_t> buffer(4096);
            size_t consumed_calls = 0;
            uint64_t consumed_total_bytes = 0;
            jau::io::StreamConsumerFunc consume = [&](jau::io::secure_vector<uint8_t>& data, bool is_final) noexcept -> bool {
                consumed_calls++;
                if( data.size() != outfile.write(data.data(), data.size()) ) {
                    return false;
                }
                consumed_total_bytes += data.size();
                jau::PLAIN_PRINT(true, "test01_sync_ok #%zu: consumed size %zu, total %" PRIu64 ", capacity %zu, final %d",
                        consumed_calls, data.size(), consumed_total_bytes, data.capacity(), is_final );
                return true;
            };
            uint64_t http_total_bytes = jau::io::read_url_stream(url_input, buffer, consume);
            const uint64_t out_bytes_total = outfile.tellp();
            jau::PLAIN_PRINT(true, "test01_sync_ok Done: total %" PRIu64 ", capacity %zu", consumed_total_bytes, buffer.capacity());

            REQUIRE( file_size == http_total_bytes );
            REQUIRE( consumed_total_bytes == http_total_bytes );
            REQUIRE( consumed_total_bytes == out_bytes_total );
        }

        void test02_sync_404() {
            if( !jau::io::uri_tk::protocol_supported("http:") ) {
                jau::PLAIN_PRINT(true, "http not supported, abort\n");
                return;
            }
            const std::string url_input = url_input_root + "doesnt_exists.txt";

            jau::io::ByteOutStream_File outfile("testfile02_01_out.bin");
            REQUIRE( outfile.good() );
            REQUIRE( outfile.is_open() );

            jau::io::secure_vector<uint8_t> buffer(4096);
            size_t consumed_calls = 0;
            uint64_t consumed_total_bytes = 0;
            jau::io::StreamConsumerFunc consume = [&](jau::io::secure_vector<uint8_t>& data, bool is_final) noexcept -> bool {
                consumed_calls++;
                if( data.size() != outfile.write(data.data(), data.size()) ) {
                    return false;
                }
                consumed_total_bytes += data.size();
                jau::PLAIN_PRINT(true, "test02_sync_404 #%zu: consumed size %zu, total %" PRIu64 ", capacity %zu, final %d",
                        consumed_calls, data.size(), consumed_total_bytes, data.capacity(), is_final );
                return true;
            };
            uint64_t http_total_bytes = jau::io::read_url_stream(url_input, buffer, consume);
            const uint64_t out_bytes_total = outfile.tellp();
            jau::PLAIN_PRINT(true, "test02_sync_404 Done: total %" PRIu64 ", capacity %zu", consumed_total_bytes, buffer.capacity());

            REQUIRE( 0 == http_total_bytes );
            REQUIRE( consumed_total_bytes == http_total_bytes );
            REQUIRE( consumed_total_bytes == out_bytes_total );
        }

        void test11_async_ok() {
            if( !jau::io::uri_tk::protocol_supported("http:") ) {
                jau::PLAIN_PRINT(true, "http not supported, abort\n");
                return;
            }
            const jau::fs::file_stats in_stats(basename_10kiB);
            const size_t file_size = in_stats.size();
            const std::string url_input = url_input_root + basename_10kiB;

            jau::io::ByteOutStream_File outfile("testfile11_01_out.bin");
            REQUIRE( outfile.good() );
            REQUIRE( outfile.is_open() );

            constexpr const size_t buffer_size = 4096;
            jau::io::ByteRingbuffer rb(jau::io::BEST_URLSTREAM_RINGBUFFER_SIZE);

            jau::io::AsyncStreamResponseRef res = jau::io::read_url_stream_async(nullptr, url_input, nullptr, &rb, nullptr);
            REQUIRE( res->failed() == false );

            jau::io::secure_vector<uint8_t> buffer(buffer_size);
            size_t consumed_loops = 0;
            uint64_t consumed_total_bytes = 0;

            while( res->processing() || !rb.isEmpty() ) {
                consumed_loops++;
                // const size_t consumed_bytes = content_length >= 0 ? std::min(buffer_size, content_length - consumed_total_bytes) : rb.getSize();
                const size_t consumed_bytes = rb.getBlocking(buffer.data(), buffer_size, 1, 500_ms);
                consumed_total_bytes += consumed_bytes;
                jau::PLAIN_PRINT(true, "test11_async_ok.0 #%zu: consumed[this %zu, total %" PRIu64 ", result %d, rb %s",
                        consumed_loops, consumed_bytes, consumed_total_bytes, res->result.load(), rb.toString().c_str() );
                if( consumed_bytes != outfile.write(buffer.data(), consumed_bytes) ) {
                    break;
                }
            }
            const uint64_t out_bytes_total = outfile.tellp();
            jau::PLAIN_PRINT(true, "test11_async_ok.X Done: total %" PRIu64 ", result %d, rb %s",
                    consumed_total_bytes, (int)res->result.load(), rb.toString().c_str() );

            res->thread.join();

            REQUIRE( res->header_resp.completed() == true );
            REQUIRE( res->has_content_length == true );
            REQUIRE( res->content_length == file_size );
            REQUIRE( res->content_length == consumed_total_bytes );
            REQUIRE( res->content_length == out_bytes_total );
            REQUIRE( res->success() == true );
        }

        void test12_async_404() {
            if( !jau::io::uri_tk::protocol_supported("http:") ) {
                jau::PLAIN_PRINT(true, "http not supported, abort\n");
                return;
            }
            const std::string url_input = url_input_root + "doesnt_exists.txt";

            jau::io::ByteOutStream_File outfile("testfile12_01_out.bin");
            REQUIRE( outfile.good() );
            REQUIRE( outfile.is_open() );

            constexpr const size_t buffer_size = 4096;
            jau::io::ByteRingbuffer rb(jau::io::BEST_URLSTREAM_RINGBUFFER_SIZE);

            jau::io::AsyncStreamResponseRef res = jau::io::read_url_stream_async(nullptr, url_input, nullptr, &rb, nullptr);
            REQUIRE( res->failed() == false );

            jau::io::secure_vector<uint8_t> buffer(buffer_size);
            size_t consumed_loops = 0;
            uint64_t consumed_total_bytes = 0;

            while( res->processing() || !rb.isEmpty() ) {
                consumed_loops++;
                // const size_t consumed_bytes = content_length >= 0 ? std::min(buffer_size, content_length - consumed_total_bytes) : rb.getSize();
                const size_t consumed_bytes = rb.getBlocking(buffer.data(), buffer_size, 1, 500_ms);
                consumed_total_bytes += consumed_bytes;
                jau::PLAIN_PRINT(true, "test12_async_404.0 #%zu: consumed[this %zu, total %" PRIu64 ", result %d, rb %s",
                        consumed_loops, consumed_bytes, consumed_total_bytes, res->result.load(), rb.toString().c_str() );
                if( consumed_bytes != outfile.write(reinterpret_cast<char*>(buffer.data()), consumed_bytes) ) {
                    break;
                }
            }
            const uint64_t out_bytes_total = outfile.tellp();
            jau::PLAIN_PRINT(true, "test12_async_404.X Done: total %" PRIu64 ", result %d, rb %s",
                    consumed_total_bytes, (int)res->result.load(), rb.toString().c_str() );

            res->thread.join();

            REQUIRE( res->header_resp.completed() == true );
            REQUIRE( res->has_content_length == false );
            REQUIRE( res->content_length == 0 );
            REQUIRE( res->content_length == consumed_total_bytes );
            REQUIRE( res->content_length == out_bytes_total );
            REQUIRE( res->failed() == true );
            REQUIRE( res->header_resp.response_code() == 404 );
        }

};

METHOD_AS_TEST_CASE( TestIOStream01::test00_protocols, "TestIOStream01 - test00_protocols");
METHOD_AS_TEST_CASE( TestIOStream01::test01_sync_ok,   "TestIOStream01 - test01_sync_ok");
METHOD_AS_TEST_CASE( TestIOStream01::test02_sync_404,  "TestIOStream01 - test02_sync_404");
METHOD_AS_TEST_CASE( TestIOStream01::test11_async_ok,  "TestIOStream01 - test11_async_ok");
METHOD_AS_TEST_CASE( TestIOStream01::test12_async_404, "TestIOStream01 - test12_async_404");

