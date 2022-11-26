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

#include <cassert>
#include <cinttypes>
#include <cstring>

#include <jau/test/catch2_ext.hpp>

#include <jau/debug.hpp>
#include <jau/file_util.hpp>
#include <jau/byte_stream.hpp>
#include <jau/io_util.hpp>

#include "test_httpd.hpp"

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
                static bool add_test_file(const std::string name, const size_t size_limit) {
                    jau::fs::remove(name);
                    jau::fs::remove(name+".enc");
                    jau::fs::remove(name+".enc.dec");
                    jau::fs::remove(name+".copy");
                    size_t size;
                    {
                        static const std::string one_line = "Hello World, this is a test and I like it. Exactly 100 characters long. 0123456780 abcdefghjklmnop..";
                        jau::io::ByteOutStream_File ofs(name);

                        REQUIRE( ofs.good() == true );
                        REQUIRE( ofs.is_open() == true );

                        for(size=0; size < size_limit; size+=one_line.size()) {
                            if( one_line.size() != ofs.write(one_line.data(), one_line.size()) ) {
                                ERR_PRINT("Write %zu bytes to test file failed: %s", one_line.size(), ofs.to_string().c_str());
                                return false;
                            }
                        }
                        if( 1 != ofs.write("X", 1) ) { // make it odd
                            ERR_PRINT("Write %zu bytes to test file failed: %s", 1, ofs.to_string().c_str());
                            return false;
                        }
                        size += 1;
                    }
                    fname_payload_lst.push_back(name);
                    fname_payload_copy_lst.push_back(name+".copy");
                    fname_payload_size_lst.push_back( size );
                    return true;
                }
                data() {
                    REQUIRE( true == add_test_file("testfile_blob_01_11kiB.bin", (size_t)(1024*11)) );
                    REQUIRE( true == add_test_file("testfile_blob_02_65MiB.bin", (size_t)(1024*1024*65)) );
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
            if( jau::io::uri_tk::protocol_supported("http:") ) {
                int res = std::system("killall mini_httpd");
                (void)res;
            }
        }

        static void httpd_start() {
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

        static bool transfer(jau::io::ByteInStream& input, const std::string& output_fname, const size_t buffer_size=4096) {
            const jau::fraction_timespec _t0 = jau::getMonotonicTime();
            jau::PLAIN_PRINT(true, "Transfer Start: %s", input.to_string().c_str());
            {
                const jau::fs::file_stats output_stats(output_fname);
                if( output_stats.exists() ) {
                    if( output_stats.is_file() ) {
                        if( !jau::fs::remove(output_fname) ) {
                            ERR_PRINT2("ByteStream copy failed: Failed deletion of existing output file %s", output_fname.c_str());
                            return false;
                        }
                    } else {
                        ERR_PRINT2("ByteStream copy failed: Not overwriting existing output file %s", output_fname.c_str());
                        return false;
                    }
                }
            }
            jau::io::ByteOutStream_File outfile(output_fname);
            if ( !outfile.good() || !outfile.is_open() ) {
                ERR_PRINT2("ByteStream copy failed: Couldn't open output file %s", output_fname.c_str());
                return false;
            }

            uint64_t out_bytes_payload = 0;
            jau::io::StreamConsumerFunc consume_data = [&](jau::io::secure_vector<uint8_t>& data, bool is_final) -> bool {
                if( !is_final && ( !input.has_content_size() || out_bytes_payload + data.size() < input.content_size() ) ) {
                    const size_t written = outfile.write(data.data(), data.size());
                    out_bytes_payload += written;
                    return data.size() == written; // continue ..
                } else {
                    const size_t written = outfile.write(data.data(), data.size());
                    out_bytes_payload += written;
                    return false; // EOS
                }
            };
            jau::io::secure_vector<uint8_t> io_buffer;
            io_buffer.reserve(buffer_size);
            const uint64_t in_bytes_total = jau::io::read_stream(input, io_buffer, consume_data);
            input.close();

            if ( 0==in_bytes_total || input.fail() ) {
                IRQ_PRINT("ByteStream copy failed: Input file read failed in %s, out %s", input.to_string().c_str(), outfile.to_string().c_str());
                return false;
            }
            if ( outfile.fail() ) {
                IRQ_PRINT("ByteStream copy failed: Output file write failed in %s, out %s", input.to_string().c_str(), outfile.to_string().c_str());
                return false;
            }

            const jau::fraction_i64 _td = ( jau::getMonotonicTime() - _t0 ).to_fraction_i64();
            jau::io::print_stats("Transfer "+output_fname, out_bytes_payload, _td);
            jau::PLAIN_PRINT(true, "Transfer End: %s", input.to_string().c_str());

            return true;
        }

        void test00a_protocols_error() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            const bool http_support_expected = jau::io::uri_tk::protocol_supported("http:");
            const bool file_support_expected = jau::io::uri_tk::protocol_supported("file:");
            httpd_start();
            {
                std::vector<std::string_view> protos = jau::io::uri_tk::supported_protocols();
                jau::PLAIN_PRINT(true, "test00_protocols: Supported protocols: %zu: %s", protos.size(), jau::to_string(protos, ",").c_str());
                if( http_support_expected ) { // assume no http -> no curl
                    REQUIRE( 0 < protos.size() );
                } else {
                    REQUIRE( 0 == protos.size() );
                }
            }
            const size_t file_idx = IDX_11kiB;
            {
                const std::string url = "not_exiting_file.txt";
                REQUIRE( false == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( false == jau::io::uri_tk::protocol_supported(url) );

                std::unique_ptr<jau::io::ByteInStream> in = jau::io::to_ByteInStream(url);
                if( nullptr != in ) {
                    jau::PLAIN_PRINT(true, "test00_protocols: not_exiting_file: %s", in->to_string().c_str());
                }
                REQUIRE( nullptr == in );
            }
            {
                const std::string url = "file://not_exiting_file_uri.txt";
                REQUIRE( true == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( file_support_expected == jau::io::uri_tk::protocol_supported(url) );

                std::unique_ptr<jau::io::ByteInStream> in = jau::io::to_ByteInStream(url);
                if( nullptr != in ) {
                    jau::PLAIN_PRINT(true, "test00_protocols: not_exiting_file_uri: %s", in->to_string().c_str());
                }
                REQUIRE( nullptr == in );
            }
            {
                const std::string url = "lala://localhost:8080/" + fname_payload_lst[file_idx];
                REQUIRE( false == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( false == jau::io::uri_tk::protocol_supported(url) );

                std::unique_ptr<jau::io::ByteInStream> in = jau::io::to_ByteInStream(url);
                if( nullptr != in ) {
                    jau::PLAIN_PRINT(true, "test00_protocols: not_exiting_protocol_uri: %s", in->to_string().c_str());
                }
                REQUIRE( nullptr == in );
            }
            {
                const std::string url = url_input_root + "not_exiting_http_uri.txt";
                REQUIRE( false == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( http_support_expected == jau::io::uri_tk::protocol_supported(url) );

                std::unique_ptr<jau::io::ByteInStream> in = jau::io::to_ByteInStream(url);
                if( http_support_expected ) {
                    REQUIRE( nullptr != in );
                    jau::sleep_for( 100_ms ); // time to read 404 response
                    jau::PLAIN_PRINT(true, "test00_protocols: not_exiting_http_uri: %s", in->to_string().c_str());
                    REQUIRE( true == in->end_of_data() );
                    REQUIRE( true == in->fail() );
                    REQUIRE( 0 == in->content_size() );
                } else {
                    REQUIRE( nullptr == in );
                }
            }
        }

        void test00b_protocols_ok() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            const bool http_support_expected = jau::io::uri_tk::protocol_supported("http:");
            const bool file_support_expected = jau::io::uri_tk::protocol_supported("file:");
            httpd_start();
            const size_t file_idx = IDX_11kiB;
            {
                const std::string url = fname_payload_lst[file_idx];
                REQUIRE( false == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( false == jau::io::uri_tk::protocol_supported(url) );

                std::unique_ptr<jau::io::ByteInStream> in = jau::io::to_ByteInStream(url);
                if( nullptr != in ) {
                    jau::PLAIN_PRINT(true, "test00_protocols: local-file-0: %s", in->to_string().c_str());
                }
                REQUIRE( nullptr != in );
                REQUIRE( false == in->fail() );

                bool res = transfer(*in, fname_payload_copy_lst[file_idx]);
                REQUIRE( true == res );

                jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                REQUIRE( true == out_stats.exists() );
                REQUIRE( true == out_stats.is_file() );
                REQUIRE( in->content_size() == out_stats.size() );
                REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                REQUIRE( true == jau::fs::compare(in->id(), out_stats.path(), true /* verbose */) );
            }
            {
                const std::string url = "file://" + fname_payload_lst[file_idx];
                REQUIRE( true == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( file_support_expected == jau::io::uri_tk::protocol_supported(url) );

                std::unique_ptr<jau::io::ByteInStream> in = jau::io::to_ByteInStream(url);
                if( nullptr != in ) {
                    jau::PLAIN_PRINT(true, "test00_protocols: local-file-1: %s", in->to_string().c_str());
                } else {
                    jau::PLAIN_PRINT(true, "test00_protocols: local-file-1: NULL from url '%s'", url.c_str());
                }
                REQUIRE( nullptr != in );
                REQUIRE( false == in->fail() );

                bool res = transfer(*in, fname_payload_copy_lst[file_idx]);
                REQUIRE( true == res );

                jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                REQUIRE( true == out_stats.exists() );
                REQUIRE( true == out_stats.is_file() );
                REQUIRE( in->content_size() == out_stats.size() );
                REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                REQUIRE( true == jau::fs::compare(fname_payload_lst[file_idx], out_stats.path(), true /* verbose */) );
            }
            {
                const std::string url = url_input_root + fname_payload_lst[file_idx];
                REQUIRE( false == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( http_support_expected == jau::io::uri_tk::protocol_supported(url) );

                std::unique_ptr<jau::io::ByteInStream> in = jau::io::to_ByteInStream(url);
                if( nullptr != in ) {
                    jau::PLAIN_PRINT(true, "test00_protocols: http: %s", in->to_string().c_str());
                }
                if( http_support_expected ) {
                    REQUIRE( nullptr != in );
                    REQUIRE( false == in->fail() );

                    bool res = transfer(*in, fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == res );

                    jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( in->content_size() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::fs::compare(fname_payload_lst[file_idx], out_stats.path(), true /* verbose */) );
                } else {
                    REQUIRE( nullptr == in );
                }
            }
        }

        void test01_copy_file_ok_11kiB_buff4k() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            const size_t file_idx = IDX_11kiB;
            jau::io::ByteInStream_File data_stream(fname_payload_lst[file_idx]);

            bool res = transfer(data_stream, fname_payload_copy_lst[file_idx], 4096);
            REQUIRE( true == res );

            jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
            REQUIRE( true == out_stats.exists() );
            REQUIRE( true == out_stats.is_file() );
            REQUIRE( data_stream.content_size() == out_stats.size() );
            REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
            REQUIRE( true == jau::fs::compare(data_stream.id(), out_stats.path(), true /* verbose */) );
        }

        void test02_copy_file_ok_65MiB_buff4k() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            const size_t file_idx = IDX_65MiB;
            jau::io::ByteInStream_File data_stream(fname_payload_lst[file_idx]);

            bool res = transfer(data_stream, fname_payload_copy_lst[file_idx], 4096);
            REQUIRE( true == res );

            jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
            REQUIRE( true == out_stats.exists() );
            REQUIRE( true == out_stats.is_file() );
            REQUIRE( data_stream.content_size() == out_stats.size() );
            REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
            REQUIRE( true == jau::fs::compare(data_stream.id(), out_stats.path(), true /* verbose */) );
        }

        void test04_copy_file_ok_65MiB_buff32k() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            const size_t file_idx = IDX_65MiB;
            jau::io::ByteInStream_File data_stream(fname_payload_lst[file_idx]);

            bool res = transfer(data_stream, fname_payload_copy_lst[file_idx], 32768);
            REQUIRE( true == res );

            jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
            REQUIRE( true == out_stats.exists() );
            REQUIRE( true == out_stats.is_file() );
            REQUIRE( data_stream.content_size() == out_stats.size() );
            REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
            REQUIRE( true == jau::fs::compare(data_stream.id(), out_stats.path(), true /* verbose */) );
        }

        void test11_copy_http_ok_buff32k() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            if( !jau::io::uri_tk::protocol_supported("http:") ) {
                jau::PLAIN_PRINT(true, "http not supported, abort\n");
                return;
            }
            httpd_start();
            {
                const size_t file_idx = IDX_11kiB;

                const std::string uri_original = url_input_root + fname_payload_lst[file_idx];

                jau::io::ByteInStream_URL data_stream(uri_original, 500_ms);

                bool res = transfer(data_stream, fname_payload_copy_lst[file_idx], 32768);
                REQUIRE( true == res );

                jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                REQUIRE( true == out_stats.exists() );
                REQUIRE( true == out_stats.is_file() );
                REQUIRE( data_stream.content_size() == out_stats.size() );
                REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                REQUIRE( true == jau::fs::compare(fname_payload_lst[file_idx], out_stats.path(), true /* verbose */) );
            }
            {
                const size_t file_idx = IDX_65MiB;

                const std::string uri_original = url_input_root + fname_payload_lst[file_idx];

                jau::io::ByteInStream_URL data_stream(uri_original, 500_ms);

                bool res = transfer(data_stream, fname_payload_copy_lst[file_idx], 32768);
                REQUIRE( true == res );

                jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                REQUIRE( true == out_stats.exists() );
                REQUIRE( true == out_stats.is_file() );
                REQUIRE( data_stream.content_size() == out_stats.size() );
                REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                REQUIRE( true == jau::fs::compare(fname_payload_lst[file_idx], out_stats.path(), true /* verbose */) );
            }
        }

        void test12_copy_http_404() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            if( !jau::io::uri_tk::protocol_supported("http:") ) {
                jau::PLAIN_PRINT(true, "http not supported, abort\n");
                return;
            }
            httpd_start();
            {
                const size_t file_idx = IDX_11kiB;

                const std::string uri_original = url_input_root + "doesnt_exists.txt";

                jau::io::ByteInStream_URL data_stream(uri_original, 500_ms);

                bool res = transfer(data_stream, fname_payload_copy_lst[file_idx]);
                REQUIRE( false == res );

                jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                REQUIRE( true == out_stats.exists() );
                REQUIRE( true == out_stats.is_file() );
                REQUIRE( data_stream.fail() == true );
                REQUIRE( data_stream.has_content_size() == false );
                REQUIRE( data_stream.content_size() == 0 );
                REQUIRE( 0 == out_stats.size() );
            }
        }

        // throttled, no content size, interruptReader() via set_eof() will avoid timeout
        static void feed_source_00(jau::io::ByteInStream_Feed * data_feed, const size_t feed_size=1024) {
            uint64_t xfer_total = 0;
            jau::io::ByteInStream_File data_stream(data_feed->id());
            uint8_t buffer[feed_size];
            while( !data_stream.end_of_data() ) {
                size_t count = data_stream.read(buffer, sizeof(buffer));
                if( 0 < count ) {
                    xfer_total += count;
                    data_feed->write(buffer, count);
                    jau::sleep_for( 16_ms );
                }
            }
            // probably set after transfering due to above sleep, which also ends when total size has been reached.
            data_feed->set_eof( jau::io::async_io_result_t::SUCCESS );
            (void)xfer_total; // not used yet ..
        }

        // throttled, with content size
        static void feed_source_01(jau::io::ByteInStream_Feed * data_feed, const size_t feed_size=1024) {
            uint64_t xfer_total = 0;
            jau::io::ByteInStream_File data_stream(data_feed->id());
            const uint64_t file_size = data_stream.content_size();
            data_feed->set_content_size( file_size );
            uint8_t buffer[feed_size];
            while( !data_stream.end_of_data() && xfer_total < file_size ) {
                size_t count = data_stream.read(buffer, sizeof(buffer));
                if( 0 < count ) {
                    xfer_total += count;
                    data_feed->write(buffer, count);
                    jau::sleep_for( 16_ms );
                }
            }
            // probably set after transfering due to above sleep, which also ends when total size has been reached.
            data_feed->set_eof( xfer_total == file_size ? jau::io::async_io_result_t::SUCCESS : jau::io::async_io_result_t::FAILED );
        }

        // full speed, with content size
        static void feed_source_10(jau::io::ByteInStream_Feed * data_feed, const size_t feed_size=1024) {
            uint64_t xfer_total = 0;
            jau::io::ByteInStream_File data_stream(data_feed->id());
            const uint64_t file_size = data_stream.content_size();
            data_feed->set_content_size( data_stream.content_size() );
            uint8_t buffer[feed_size];
            while( !data_stream.end_of_data() && xfer_total < file_size ) {
                size_t count = data_stream.read(buffer, sizeof(buffer));
                if( 0 < count ) {
                    xfer_total += count;
                    data_feed->write(buffer, count);
                }
            }
            data_feed->set_eof( xfer_total == file_size ? jau::io::async_io_result_t::SUCCESS : jau::io::async_io_result_t::FAILED );
        }

        // full speed, no content size, interrupting @ 1024 bytes within our header
        static void feed_source_20(jau::io::ByteInStream_Feed * data_feed, const size_t feed_size=1024) {
            uint64_t xfer_total = 0;
            jau::io::ByteInStream_File data_stream(data_feed->id());
            uint8_t buffer[feed_size];
            while( !data_stream.end_of_data() ) {
                size_t count = data_stream.read(buffer, sizeof(buffer));
                if( 0 < count ) {
                    xfer_total += count;
                    data_feed->write(buffer, count);
                    if( xfer_total >= 1024 ) {
                        data_feed->set_eof( jau::io::async_io_result_t::FAILED ); // calls data_feed->interruptReader();
                        return;
                    }
                }
            }
            // probably set after transfering due to above sleep, which also ends when total size has been reached.
            // data_feed->set_eof( jau::io::async_io_result_t::SUCCESS );
        }

        // full speed, with content size, interrupting 1/4 way
        static void feed_source_21(jau::io::ByteInStream_Feed * data_feed, const size_t feed_size=1024) {
            uint64_t xfer_total = 0;
            jau::io::ByteInStream_File data_stream(data_feed->id());
            const uint64_t file_size = data_stream.content_size();
            data_feed->set_content_size( data_stream.content_size() );
            uint8_t buffer[feed_size];
            while( !data_stream.end_of_data() ) {
                size_t count = data_stream.read(buffer, sizeof(buffer));
                if( 0 < count ) {
                    xfer_total += count;
                    data_feed->write(buffer, count);
                    if( xfer_total >= file_size/4 ) {
                        data_feed->set_eof( jau::io::async_io_result_t::FAILED ); // calls data_feed->interruptReader();
                        return;
                    }
                }
            }
            // probably set after transfering due to above sleep, which also ends when total size has been reached.
            // data_feed->set_eof( jau::io::async_io_result_t::SUCCESS );
        }

        void test20_copy_fed_ok_buff4k_feed1k() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            const size_t buffer_size = 4096;
            const size_t feed_size = 1024;
            {
                const size_t file_idx = IDX_11kiB;
                {
                    // full speed, with content size
                    jau::io::ByteInStream_Feed data_feed(fname_payload_lst[file_idx], 500_ms);
                    std::thread feeder_thread= std::thread(&feed_source_10, &data_feed, feed_size);

                    bool res = transfer(data_feed, fname_payload_copy_lst[file_idx], buffer_size);
                    if( feeder_thread.joinable() ) {
                        feeder_thread.join();
                    }
                    REQUIRE( true == res );

                    jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.content_size() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
                }
                {
                    // throttled, with content size
                    jau::io::ByteInStream_Feed data_feed(fname_payload_lst[file_idx], 500_ms);
                    std::thread feeder_thread= std::thread(&feed_source_01, &data_feed, feed_size);

                    bool res = transfer(data_feed, fname_payload_copy_lst[file_idx], buffer_size);
                    if( feeder_thread.joinable() ) {
                        feeder_thread.join();
                    }
                    REQUIRE( true == res );

                    jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.content_size() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
                }
                {
                    // throttled, no content size, interruptReader() via set_eof() will avoid timeout
                    jau::io::ByteInStream_Feed data_feed(fname_payload_lst[file_idx], 500_ms);
                    std::thread feeder_thread= std::thread(&feed_source_00, &data_feed, feed_size);

                    bool res = transfer(data_feed, fname_payload_copy_lst[file_idx], buffer_size);
                    if( feeder_thread.joinable() ) {
                        feeder_thread.join();
                    }
                    REQUIRE( true == res );

                    jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.content_size() == 0 );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
                }
            }
            {
                const size_t file_idx = IDX_65MiB;
                {
                    // full speed, with content size
                    jau::io::ByteInStream_Feed data_feed(fname_payload_lst[file_idx], 500_ms);
                    std::thread feeder_thread= std::thread(&feed_source_10, &data_feed, feed_size);

                    bool res = transfer(data_feed, fname_payload_copy_lst[file_idx], buffer_size);
                    if( feeder_thread.joinable() ) {
                        feeder_thread.join();
                    }
                    REQUIRE( true == res );

                    jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.content_size() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
                }
            }
        }

        void test21_copy_fed_ok_buff32k() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            const size_t buffer_size = 32768;
            const size_t feed_size = 32768;
            {
                const size_t file_idx = IDX_11kiB;
                {
                    // full speed, with content size
                    jau::io::ByteInStream_Feed data_feed(fname_payload_lst[file_idx], 500_ms);
                    std::thread feeder_thread= std::thread(&feed_source_10, &data_feed, feed_size);

                    bool res = transfer(data_feed, fname_payload_copy_lst[file_idx], buffer_size);
                    if( feeder_thread.joinable() ) {
                        feeder_thread.join();
                    }
                    REQUIRE( true == res );

                    jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.content_size() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
                }
                {
                    // throttled, with content size
                    jau::io::ByteInStream_Feed data_feed(fname_payload_lst[file_idx], 500_ms);
                    std::thread feeder_thread= std::thread(&feed_source_01, &data_feed, feed_size);

                    bool res = transfer(data_feed, fname_payload_copy_lst[file_idx], buffer_size);
                    if( feeder_thread.joinable() ) {
                        feeder_thread.join();
                    }
                    REQUIRE( true == res );

                    jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.content_size() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
                }
                {
                    // throttled, no content size, interruptReader() via set_eof() will avoid timeout
                    jau::io::ByteInStream_Feed data_feed(fname_payload_lst[file_idx], 500_ms);
                    std::thread feeder_thread= std::thread(&feed_source_00, &data_feed, feed_size);

                    bool res = transfer(data_feed, fname_payload_copy_lst[file_idx], buffer_size);
                    if( feeder_thread.joinable() ) {
                        feeder_thread.join();
                    }
                    REQUIRE( true == res );

                    jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.content_size() == 0 );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
                }
            }
        }

        void test22_copy_fed_ok_buff32k() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            const size_t buffer_size = 32768;
            const size_t feed_size = 32768;
            {
                const size_t file_idx = IDX_65MiB;
                {
                    // full speed, with content size
                    jau::io::ByteInStream_Feed data_feed(fname_payload_lst[file_idx], 500_ms);
                    std::thread feeder_thread= std::thread(&feed_source_10, &data_feed, feed_size);

                    bool res = transfer(data_feed, fname_payload_copy_lst[file_idx], buffer_size);
                    if( feeder_thread.joinable() ) {
                        feeder_thread.join();
                    }
                    REQUIRE( true == res );

                    jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.content_size() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
                }
            }
        }

        void test23_copy_fed_irq() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            const size_t buffer_size = 4096;
            const size_t feed_size = 1024;
            {
                const size_t file_idx = IDX_65MiB;
                {
                    // full speed, no content size, interrupting @ 1024 bytes within our header
                    jau::io::ByteInStream_Feed data_feed(fname_payload_lst[file_idx], 500_ms);
                    std::thread feeder_thread= std::thread(&feed_source_20, &data_feed, feed_size);

                    bool res = transfer(data_feed, fname_payload_copy_lst[file_idx], buffer_size);
                    if( feeder_thread.joinable() ) {
                        feeder_thread.join();
                    }
                    REQUIRE( false == res );

                    jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( false == data_feed.has_content_size() );
                    REQUIRE( 0 == data_feed.content_size() );
                    REQUIRE( fname_payload_size_lst[file_idx] > out_stats.size() ); // interrupted...
                    REQUIRE( false == jau::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
                }
                {
                    // full speed, with content size, interrupting 1/4 way
                    jau::io::ByteInStream_Feed data_feed(fname_payload_lst[file_idx], 500_ms);
                    std::thread feeder_thread= std::thread(&feed_source_21, &data_feed, feed_size);

                    bool res = transfer(data_feed, fname_payload_copy_lst[file_idx], buffer_size);
                    if( feeder_thread.joinable() ) {
                        feeder_thread.join();
                    }
                    REQUIRE( false == res );

                    jau::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( true == data_feed.has_content_size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == data_feed.content_size() );
                    REQUIRE( data_feed.content_size() > out_stats.size() ); // interrupted...
                    REQUIRE( false == jau::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
                }
            }
        }

};

std::vector<std::string> TestByteStream01::fname_payload_lst;
std::vector<std::string> TestByteStream01::fname_payload_copy_lst;
std::vector<uint64_t> TestByteStream01::fname_payload_size_lst;

METHOD_AS_TEST_CASE( TestByteStream01::test00a_protocols_error,           "test00a_protocols_error");
METHOD_AS_TEST_CASE( TestByteStream01::test00b_protocols_ok,              "test00b_protocols_ok");

METHOD_AS_TEST_CASE( TestByteStream01::test01_copy_file_ok_11kiB_buff4k,  "test01_copy_file_ok_11kiB_buff4k");
METHOD_AS_TEST_CASE( TestByteStream01::test02_copy_file_ok_65MiB_buff4k,  "test02_copy_file_ok_65MiB_buff4k");
METHOD_AS_TEST_CASE( TestByteStream01::test04_copy_file_ok_65MiB_buff32k, "test04_copy_file_ok_65MiB_buff32k");

METHOD_AS_TEST_CASE( TestByteStream01::test11_copy_http_ok_buff32k,       "test11_copy_http_ok");
METHOD_AS_TEST_CASE( TestByteStream01::test12_copy_http_404,              "test12_copy_http_404");

METHOD_AS_TEST_CASE( TestByteStream01::test20_copy_fed_ok_buff4k_feed1k,  "test20_copy_fed_ok_buff4k_feed1k");
METHOD_AS_TEST_CASE( TestByteStream01::test21_copy_fed_ok_buff32k,        "test21_copy_fed_ok_buff32k");
METHOD_AS_TEST_CASE( TestByteStream01::test22_copy_fed_ok_buff32k,        "test22_copy_fed_ok_buff32k");
METHOD_AS_TEST_CASE( TestByteStream01::test23_copy_fed_irq,               "test23_copy_fed_irq");
