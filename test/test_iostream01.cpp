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

#include <iostream>
#include <cassert>
#include <cinttypes>
#include <cstring>

#include <fstream>
#include <iostream>

#include <thread>
#include <pthread.h>

#define CATCH_CONFIG_RUNNER
// #define CATCH_CONFIG_MAIN
#include <catch2/catch_amalgamated.hpp>
#include <jau/test/catch2_ext.hpp>

#include <jau/file_util.hpp>
#include <jau/io_util.hpp>

#include <jau/debug.hpp>

extern "C" {
    #include <unistd.h>
}

using namespace jau::fractions_i64_literals;

class TestIOStream01 {
    public:
        const std::string url_input_root = "http://localhost:8080/";
        const std::string basename_10kiB = "data-10kiB.bin";

        TestIOStream01() {
            // produce fresh demo data

            jau::fs::remove(basename_10kiB, false /* recursive */);
            {
                std::string one_line = "Hello World, this is a test and I like it. Exactly 100 characters long. 0123456780 abcdefghjklmnop..";
                std::ofstream ofs(basename_10kiB, std::ios::out | std::ios::binary);

                REQUIRE( ofs.good() == true );
                REQUIRE( ofs.is_open() == true );

                for(int i=0; i < 1024*10; i+=one_line.size()) { // 10kiB
                    ofs.write(reinterpret_cast<char*>(one_line.data()), one_line.size());
                }
            }
            std::system("killall mini_httpd");
            std::system("killall mini_httpd");
            std::system("/usr/sbin/mini_httpd -p 8080");
        }

        ~TestIOStream01() {
            std::system("killall mini_httpd");
            std::system("killall mini_httpd");
        }

        void test01() {
            const jau::fs::file_stats in_stats(basename_10kiB);
            const size_t file_size = in_stats.size();
            const std::string url_input = url_input_root + basename_10kiB;

            std::ofstream outfile("test02_01_out.bin", std::ios::out | std::ios::binary);
            REQUIRE( outfile.good() );
            REQUIRE( outfile.is_open() );

            jau::io::secure_vector<uint8_t> buffer(4096);
            size_t consumed_calls = 0;
            uint64_t consumed_total_bytes = 0;
            jau::io::StreamConsumerFunc consume = [&](jau::io::secure_vector<uint8_t>& data, bool is_final) noexcept -> bool {
                consumed_calls++;
                consumed_total_bytes += data.size();
                outfile.write(reinterpret_cast<char*>(data.data()), data.size());
                jau::PLAIN_PRINT(true, "test02io01 #%zu: consumed size %zu, total %" PRIu64 ", capacity %zu, final %d",
                        consumed_calls, data.size(), consumed_total_bytes, data.capacity(), is_final );
                return true;
            };
            uint64_t http_total_bytes = jau::io::read_url_stream(url_input, buffer, consume);
            const uint64_t out_bytes_total = outfile.tellp();
            jau::PLAIN_PRINT(true, "test02io01 Done: total %" PRIu64 ", capacity %zu", consumed_total_bytes, buffer.capacity());

            REQUIRE( file_size == http_total_bytes );
            REQUIRE( consumed_total_bytes == http_total_bytes );
            REQUIRE( consumed_total_bytes == out_bytes_total );
        }

        void test02() {
            const jau::fs::file_stats in_stats(basename_10kiB);
            const size_t file_size = in_stats.size();
            const std::string url_input = url_input_root + basename_10kiB;

            std::ofstream outfile("test02_02_out.bin", std::ios::out | std::ios::binary);
            REQUIRE( outfile.good() );
            REQUIRE( outfile.is_open() );

            constexpr const size_t buffer_size = 4096;
            jau::io::ByteRingbuffer rb(0x00, jau::io::BEST_URLSTREAM_RINGBUFFER_SIZE);
            jau::relaxed_atomic_bool url_has_content_length;
            jau::relaxed_atomic_uint64 url_content_length;
            jau::relaxed_atomic_uint64 url_total_read;
            jau::io::relaxed_atomic_async_io_result_t result;

            std::thread http_thread = jau::io::read_url_stream(url_input, rb, url_has_content_length, url_content_length, url_total_read, result);

            jau::io::secure_vector<uint8_t> buffer(buffer_size);
            size_t consumed_loops = 0;
            uint64_t consumed_total_bytes = 0;

            while( jau::io::async_io_result_t::NONE == result || !rb.isEmpty() ) {
                consumed_loops++;
                // const size_t consumed_bytes = content_length >= 0 ? std::min(buffer_size, content_length - consumed_total_bytes) : rb.getSize();
                const size_t consumed_bytes = rb.getBlocking(buffer.data(), buffer_size, 1, 0_s);
                consumed_total_bytes += consumed_bytes;
                jau::PLAIN_PRINT(true, "test02io02.0 #%zu: consumed[this %zu, total %" PRIu64 ", result %d, rb %s",
                        consumed_loops, consumed_bytes, consumed_total_bytes, result.load(), rb.toString().c_str() );
                outfile.write(reinterpret_cast<char*>(buffer.data()), consumed_bytes);
            }
            const uint64_t out_bytes_total = outfile.tellp();
            jau::PLAIN_PRINT(true, "test02io02.X Done: total %" PRIu64 ", result %d, rb %s",
                    consumed_total_bytes, result.load(), rb.toString().c_str() );

            http_thread.join();

            REQUIRE( url_has_content_length == true );
            REQUIRE( url_content_length == file_size );
            REQUIRE( url_content_length == consumed_total_bytes );
            REQUIRE( url_content_length == url_total_read );
            REQUIRE( url_content_length == out_bytes_total );
            REQUIRE( jau::io::async_io_result_t::SUCCESS == result );
        }
};

METHOD_AS_TEST_CASE( TestIOStream01::test01, "TestIOStream01 - 01");
METHOD_AS_TEST_CASE( TestIOStream01::test02, "TestIOStream01 - 02");