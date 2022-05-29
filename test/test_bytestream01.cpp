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

#include <iostream>
#include <cassert>
#include <cinttypes>
#include <cstring>

#include <fstream>
#include <iostream>

#define CATCH_CONFIG_RUNNER
// #define CATCH_CONFIG_MAIN
#include <catch2/catch_amalgamated.hpp>
#include <jau/test/catch2_ext.hpp>

#include <jau/debug.hpp>
#include <jau/file_util.hpp>
#include <jau/byte_stream.hpp>
#include <jau/io_util.hpp>

extern "C" {
    #include <unistd.h>
}

using namespace jau::fractions_i64_literals;

class TestByteStream01 {
    private:
        const size_t IDX_11kiB = 0;
        const size_t IDX_65MiB = 1;
        static std::vector<std::string> fname_payload_lst;
        static std::vector<std::string> fname_payload_copy_lst;
        static std::vector<uint64_t> fname_payload_size_lst;
        const std::string url_input_root = "http://localhost:8080/";

        class data {
            private:
                static void add_test_file(const std::string name, const size_t size_limit) {
                    jau::fs::remove(name, false /* recursive */);
                    jau::fs::remove(name+".enc", false /* recursive */);
                    jau::fs::remove(name+".enc.dec", false /* recursive */);
                    size_t size;
                    {
                        static const std::string one_line = "Hello World, this is a test and I like it. Exactly 100 characters long. 0123456780 abcdefghjklmnop..";
                        std::ofstream ofs(name, std::ios::out | std::ios::binary);

                        REQUIRE( ofs.good() == true );
                        REQUIRE( ofs.is_open() == true );

                        for(size=0; size < size_limit; size+=one_line.size()) {
                            ofs.write(reinterpret_cast<const char*>(one_line.data()), one_line.size());
                        }
                        ofs.write("X", 1); // make it odd
                        size += 1;
                    }
                    fname_payload_lst.push_back(name);
                    fname_payload_copy_lst.push_back(name+".copy");
                    fname_payload_size_lst.push_back( size );
                }
                data() {
                    add_test_file("test_cipher_01_11kiB", 1024*11);
                    add_test_file("test_cipher_02_65MiB", 1024*1024*65);
                }
            public:
                static const data& get() {
                    static data instance;
                    return instance;
                }
        };

    public:
        TestByteStream01() {
            // produce fresh demo data once per whole test class
            const data& d = data::get();
            (void)d;
        }

        ~TestByteStream01() {
            std::system("killall mini_httpd");
            std::system("killall mini_httpd");
        }

        static void httpd_start() {
            std::system("killall mini_httpd");
            std::system("killall mini_httpd");
            std::system("/usr/sbin/mini_httpd -p 8080");
        }

        static bool transfer(jau::io::ByteInStream& input, const std::string& output_fname) {
            const jau::fraction_timespec _t0 = jau::getMonotonicTime();
            jau::PLAIN_PRINT(true, "Transfer Start: %s", input.to_string().c_str());
            {
                const jau::fs::file_stats output_stats(output_fname);
                if( output_stats.exists() ) {
                    if( output_stats.is_file() ) {
                        if( !jau::fs::remove(output_fname, false /* recursive */) ) {
                            ERR_PRINT2("ByteStream copy failed: Failed deletion of existing output file %s", output_fname.c_str());
                            return false;
                        }
                    } else {
                        ERR_PRINT2("ByteStream copy failed: Not overwriting existing output file %s", output_fname.c_str());
                        return false;
                    }
                }
            }
            std::ofstream outfile(output_fname, std::ios::out | std::ios::binary);
            if ( !outfile.good() || !outfile.is_open() ) {
                ERR_PRINT2("ByteStream copy failed: Couldn't open output file %s", output_fname.c_str());
                return false;
            }

            uint64_t out_bytes_payload = 0;
            jau::io::StreamConsumerFunc consume_data = [&](jau::io::secure_vector<uint8_t>& data, bool is_final) -> bool {
                if( !is_final && ( !input.has_content_size() || out_bytes_payload + data.size() < input.content_size() ) ) {
                    outfile.write(reinterpret_cast<char*>(data.data()), data.size());
                    out_bytes_payload += data.size();
                    return true; // continue ..
                } else {
                    outfile.write(reinterpret_cast<char*>(data.data()), data.size());
                    out_bytes_payload += data.size();
                    return false; // EOS
                }
            };
            jau::io::secure_vector<uint8_t> io_buffer;
            io_buffer.reserve(4096);
            const uint64_t in_bytes_total = jau::io::read_stream(input, io_buffer, consume_data);
            input.close();

            if ( 0==in_bytes_total || outfile.fail() ) {
                ERR_PRINT2("ByteStream copy failed: Output file write failed %s", output_fname.c_str());
                return false;
            }

            const jau::fraction_i64 _td = ( jau::getMonotonicTime() - _t0 ).to_fraction_i64();
            jau::io::print_stats("Transfer "+output_fname, out_bytes_payload, _td);
            jau::PLAIN_PRINT(true, "Transfer End: %s", input.to_string().c_str());

            return true;
        }

        void test01_copy_file_ok() {
            {
                const size_t file_idx = IDX_11kiB;
                jau::io::ByteInStream_File data_stream(fname_payload_lst[file_idx], true /* use_binary */);

                bool res = transfer(data_stream, fname_payload_copy_lst[file_idx]);
                REQUIRE( true == res );

                jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                REQUIRE( true == out_stats.exists() );
                REQUIRE( true == out_stats.is_file() );
                REQUIRE( data_stream.content_size() == out_stats.size() );
                REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
            }
            {
                const size_t file_idx = IDX_65MiB;
                jau::io::ByteInStream_File data_stream(fname_payload_lst[file_idx], true /* use_binary */);

                bool res = transfer(data_stream, fname_payload_copy_lst[file_idx]);
                REQUIRE( true == res );

                jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                REQUIRE( true == out_stats.exists() );
                REQUIRE( true == out_stats.is_file() );
                REQUIRE( data_stream.content_size() == out_stats.size() );
                REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
            }
        }

        void test11_copy_http_ok() {
            httpd_start();

            {
                const size_t file_idx = IDX_11kiB;

                const std::string uri_original = url_input_root + fname_payload_lst[file_idx];

                jau::io::ByteInStream_URL data_stream(uri_original, 3_s);

                bool res = transfer(data_stream, fname_payload_copy_lst[file_idx]);
                REQUIRE( true == res );

                jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                REQUIRE( true == out_stats.exists() );
                REQUIRE( true == out_stats.is_file() );
                REQUIRE( data_stream.content_size() == out_stats.size() );
                REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
            }
            {
                const size_t file_idx = IDX_65MiB;

                const std::string uri_original = url_input_root + fname_payload_lst[file_idx];

                jau::io::ByteInStream_URL data_stream(uri_original, 3_s);

                bool res = transfer(data_stream, fname_payload_copy_lst[file_idx]);
                REQUIRE( true == res );

                jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                REQUIRE( true == out_stats.exists() );
                REQUIRE( true == out_stats.is_file() );
                REQUIRE( data_stream.content_size() == out_stats.size() );
                REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
            }
        }

        // throttled, no content size
        static void feed_source_00(jau::io::ByteInStream_Feed * data_feed) {
            uint64_t xfer_total = 0;
            jau::io::ByteInStream_File enc_stream(data_feed->id(), true /* use_binary */);
            while( !enc_stream.end_of_data() ) {
                uint8_t buffer[1024]; // 1k
                size_t count = enc_stream.read(buffer, sizeof(buffer));
                if( 0 < count ) {
                    xfer_total += count;
                    data_feed->write(buffer, count);
                    jau::sleep_for( 16_ms );
                }
            }
            // probably set after transfering due to above sleep, which also ends when total size has been reached.
            data_feed->set_eof( jau::io::async_io_result_t::SUCCESS );
        }

        // throttled, with content size
        static void feed_source_01(jau::io::ByteInStream_Feed * data_feed) {
            jau::fs::file_stats fs_feed(data_feed->id());
            const uint64_t file_size = fs_feed.size();
            data_feed->set_content_size( file_size );

            uint64_t xfer_total = 0;
            jau::io::ByteInStream_File enc_stream(data_feed->id(), true /* use_binary */);
            while( !enc_stream.end_of_data() && xfer_total < file_size ) {
                uint8_t buffer[1024]; // 1k
                size_t count = enc_stream.read(buffer, sizeof(buffer));
                if( 0 < count ) {
                    xfer_total += count;
                    data_feed->write(buffer, count);
                    jau::sleep_for( 16_ms );
                }
            }
            // probably set after transfering due to above sleep, which also ends when total size has been reached.
            data_feed->set_eof( jau::io::async_io_result_t::SUCCESS );
        }

        // full speed, with content size
        static void feed_source_10(jau::io::ByteInStream_Feed * data_feed) {
            jau::fs::file_stats fs_feed(data_feed->id());
            const uint64_t file_size = fs_feed.size();
            data_feed->set_content_size( file_size );

            uint64_t xfer_total = 0;
            jau::io::ByteInStream_File enc_stream(data_feed->id(), true /* use_binary */);
            while( !enc_stream.end_of_data() && xfer_total < file_size ) {
                uint8_t buffer[1024]; // 1k
                size_t count = enc_stream.read(buffer, sizeof(buffer));
                if( 0 < count ) {
                    xfer_total += count;
                    data_feed->write(buffer, count);
                }
            }
            data_feed->set_eof( xfer_total == file_size ? jau::io::async_io_result_t::SUCCESS : jau::io::async_io_result_t::FAILED );
        }

        void test21_copy_fed_ok() {
            {
                const size_t file_idx = IDX_11kiB;
                {
                    // full speed, with content size
                    jau::io::ByteInStream_Feed data_feed(fname_payload_lst[file_idx], 3_s);
                    std::thread feeder_thread= std::thread(&feed_source_10, &data_feed);

                    bool res = transfer(data_feed, fname_payload_copy_lst[file_idx]);
                    if( feeder_thread.joinable() ) {
                        feeder_thread.join();
                    }
                    REQUIRE( true == res );

                    jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.content_size() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                }
                {
                    // throttled, with content size
                    jau::io::ByteInStream_Feed data_feed(fname_payload_lst[file_idx], 3_s);
                    std::thread feeder_thread= std::thread(&feed_source_01, &data_feed);

                    bool res = transfer(data_feed, fname_payload_copy_lst[file_idx]);
                    if( feeder_thread.joinable() ) {
                        feeder_thread.join();
                    }
                    REQUIRE( true == res );

                    jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.content_size() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                }
                {
                    // throttled, no content size, will timeout after 500_ms
                    jau::io::ByteInStream_Feed data_feed(fname_payload_lst[file_idx], 500_ms);
                    std::thread feeder_thread= std::thread(&feed_source_00, &data_feed);

                    bool res = transfer(data_feed, fname_payload_copy_lst[file_idx]);
                    if( feeder_thread.joinable() ) {
                        feeder_thread.join();
                    }
                    REQUIRE( true == res );

                    jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.content_size() == 0 );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                }
            }
            {
                const size_t file_idx = IDX_65MiB;
                {
                    // full speed, with content size
                    jau::io::ByteInStream_Feed data_feed(fname_payload_lst[file_idx], 3_s);
                    std::thread feeder_thread= std::thread(&feed_source_10, &data_feed);

                    bool res = transfer(data_feed, fname_payload_copy_lst[file_idx]);
                    if( feeder_thread.joinable() ) {
                        feeder_thread.join();
                    }
                    REQUIRE( true == res );

                    jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.content_size() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                }
            }
        }
};

std::vector<std::string> TestByteStream01::fname_payload_lst;
std::vector<std::string> TestByteStream01::fname_payload_copy_lst;
std::vector<uint64_t> TestByteStream01::fname_payload_size_lst;

// METHOD_AS_TEST_CASE( TestByteStream01::test01_copy_file_ok,    "TestByteStream01 test01_copy_file_ok");

// METHOD_AS_TEST_CASE( TestByteStream01::test11_copy_http_ok,    "TestByteStream01 test11_copy_http_ok");

METHOD_AS_TEST_CASE( TestByteStream01::test21_copy_fed_ok,     "TestByteStream01 test21_copy_fed_ok");

