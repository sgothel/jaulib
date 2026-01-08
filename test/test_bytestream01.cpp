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
#include <cstdint>
#include <cstring>
#include <vector>

#include <jau/test/catch2_ext.hpp>

#include <jau/debug.hpp>
#include <jau/io/file_util.hpp>
#include <jau/io/byte_stream.hpp>
#include <jau/io/io_util.hpp>

#include "test_httpd.hpp"

extern "C" {
    #include <unistd.h>
}

using namespace jau::fractions_i64_literals;

class TestByteStream01 {
    private:
        const size_t IDX_11kiB = 0;
        const size_t IDX_65MiB = 1;
        // const size_t IDX_SKIPFILE1 = 2;
        // const size_t IDX_SKIPFILE2 = 3;
        static std::vector<std::string> fname_payload_lst;
        static std::vector<std::string> fname_payload_copy_lst;
        static std::vector<uint64_t> fname_payload_size_lst;
        const std::string url_input_root = "http://localhost:8080/";

        class data {
            private:
                static bool add_test_file(const std::string& name, const size_t size_limit) {
                    jau::io::fs::remove(name);
                    jau::io::fs::remove(name+".enc");
                    jau::io::fs::remove(name+".enc.dec");
                    jau::io::fs::remove(name+".copy");
                    size_t size;
                    {
                        static const std::string one_line = "Hello World, this is a test and I like it. Exactly 100 characters long. 0123456780 abcdefghjklmnop..";
                        jau::io::ByteStream_File ofs(name, jau::io::iomode_t::write);

                        REQUIRE( ofs.good() == true );
                        REQUIRE( ofs.isOpen() == true );

                        for(size=0; size < size_limit; size+=one_line.size()) {
                            if( one_line.size() != ofs.write(one_line.data(), one_line.size()) ) {
                                ERR_PRINT("Write %zu bytes to test file failed: %s", one_line.size(), ofs.toString());
                                return false;
                            }
                        }
                        if( 1 != ofs.write("X", 1) ) { // make it odd
                            ERR_PRINT("Write %d bytes to test file failed: %s", 1, ofs.toString());
                            return false;
                        }
                        size += 1;
                    }
                    fname_payload_lst.push_back(name);
                    fname_payload_copy_lst.push_back(name+".copy");
                    fname_payload_size_lst.push_back( size );
                    return true;
                }
                static bool add_test_skipfile(const std::string& name_in, const std::string& name_out, const size_t take, const size_t skip) {
                    jau::io::fs::remove(name_out);

                    jau::io::ByteStream_File in(name_in, jau::io::iomode_t::read);
                    jau::io::ByteStream_File out(name_out, jau::io::iomode_t::write);
                    REQUIRE( true == out.good() );
                    REQUIRE( true == out.isOpen() );

                    REQUIRE( true == in.isOpen() );
                    REQUIRE( false == in.canWrite() );
                    REQUIRE( true == in.hasContentSize() );

                    char buffer[1024];
                    while( !in.eof() ) {
                        uint64_t in_left = in.contentSize() - in.position();
                        uint64_t l0 = std::min<uint64_t>(in_left, std::min<size_t>(sizeof(buffer), take));
                        REQUIRE( l0 == in.read(buffer, l0) );
                        REQUIRE( l0 == out.write(buffer, l0) );

                        in_left = in.contentSize() - in.position();
                        l0 = std::min<uint64_t>(in_left, std::min<size_t>(sizeof(buffer), skip));
                        const uint64_t p0 = in.position() + l0;
                        // std::cout << "XXX: " << in << ": l0 " << l0 << ", p0 " << p0 << "\n";
                        REQUIRE(p0 == in.seek(p0) );
                    }

                    fname_payload_lst.push_back(name_out);
                    return true;
                }
                data() {
                    REQUIRE( true == add_test_file("testfile_blob_01_11kiB.bin", (size_t)(1024*11)) );
                    REQUIRE( true == add_test_file("testfile_blob_02_65MiB.bin", (size_t)(1024*1024*65)) );
                    REQUIRE( true == add_test_skipfile("testfile_blob_01_11kiB.bin", "testfile_blob_03_skip1.bin", 10,  9) );
                    REQUIRE( true == add_test_skipfile("testfile_blob_01_11kiB.bin", "testfile_blob_04_skip2.bin",  9, 10) );
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
                const std::string cwd = jau::io::fs::get_cwd();
                const std::string cmd = std::string(mini_httpd_exe)+" -p 8080 -l "+cwd+"/mini_httpd.log";
                PLAIN_PRINT(true, "%s", cmd);
                res = std::system(cmd.c_str());
                (void)res;
            }
        }

        static bool transfer(jau::io::ByteStream& input, const std::string& output_fname, const size_t buffer_size=4096) {
            const jau::fraction_timespec _t0 = jau::getMonotonicTime();
            PLAIN_PRINT(true, "Transfer Start: %s", input.toString());
            {
                const jau::io::fs::file_stats output_stats(output_fname);
                if( output_stats.exists() ) {
                    if( output_stats.is_file() ) {
                        if( !jau::io::fs::remove(output_fname) ) {
                            ERR_PRINT2("ByteStream copy failed: Failed deletion of existing output file %s", output_fname);
                            return false;
                        }
                    } else {
                        ERR_PRINT2("ByteStream copy failed: Not overwriting existing output file %s", output_fname);
                        return false;
                    }
                }
            }
            jau::io::ByteStream_File outfile(output_fname, jau::io::iomode_t::write);
            if ( !outfile.good() || !outfile.isOpen() ) {
                ERR_PRINT2("ByteStream copy failed: Couldn't open output file %s", output_fname);
                return false;
            }

            uint64_t out_bytes_payload = 0;
            jau::io::StreamConsumerFunc consume_data = [&](jau::io::secure_vector<uint8_t>& data, bool is_final) -> bool {
                if( !is_final && ( !input.hasContentSize() || out_bytes_payload + data.size() < input.contentSize() ) ) {
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
                IRQ_PRINT("ByteStream copy failed: Input file read failed in %s, out %s", input.toString(), outfile.toString());
                return false;
            }
            if ( outfile.fail() ) {
                IRQ_PRINT("ByteStream copy failed: Output file write failed in %s, out %s", input.toString(), outfile.toString());
                return false;
            }

            const jau::fraction_i64 _td = ( jau::getMonotonicTime() - _t0 ).to_fraction_i64();
            jau::io::print_stats("Transfer "+output_fname, out_bytes_payload, _td);
            PLAIN_PRINT(true, "Transfer End: %s", input.toString());

            return true;
        }

        void test00a_protocols_error() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            const bool http_support_expected = jau::io::uri_tk::protocol_supported("http:");
            const bool file_support_expected = jau::io::uri_tk::protocol_supported("file:");
            httpd_start();
            {
                std::vector<std::string_view> protos = jau::io::uri_tk::supported_protocols();
                PLAIN_PRINT(true, "test00_protocols: Supported protocols: %zu: %s", protos.size(), jau::to_string(protos, ","));
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

                std::unique_ptr<jau::io::ByteStream> in = jau::io::to_ByteInStream(url);
                if( nullptr != in ) {
                    PLAIN_PRINT(true, "test00_protocols: not_exiting_file: %s", in->toString());
                }
                REQUIRE( nullptr == in );
            }
            {
                const std::string url = "file://not_exiting_file_uri.txt";
                REQUIRE( true == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( file_support_expected == jau::io::uri_tk::protocol_supported(url) );

                std::unique_ptr<jau::io::ByteStream> in = jau::io::to_ByteInStream(url);
                if( nullptr != in ) {
                    PLAIN_PRINT(true, "test00_protocols: not_exiting_file_uri: %s", in->toString());
                }
                REQUIRE( nullptr == in );
            }
            {
                const std::string url = "lala://localhost:8080/" + fname_payload_lst[file_idx];
                REQUIRE( false == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( false == jau::io::uri_tk::protocol_supported(url) );

                std::unique_ptr<jau::io::ByteStream> in = jau::io::to_ByteInStream(url);
                if( nullptr != in ) {
                    PLAIN_PRINT(true, "test00_protocols: not_exiting_protocol_uri: %s", in->toString());
                }
                REQUIRE( nullptr == in );
            }
            {
                const std::string url = url_input_root + "not_exiting_http_uri.txt";
                REQUIRE( false == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( http_support_expected == jau::io::uri_tk::protocol_supported(url) );

                std::unique_ptr<jau::io::ByteStream> in = jau::io::to_ByteInStream(url);
                if( http_support_expected ) {
                    REQUIRE( nullptr != in );
                    jau::sleep_for( 100_ms ); // time to read 404 response
                    PLAIN_PRINT(true, "test00_protocols: not_exiting_http_uri: %s", in->toString());
                    REQUIRE( false == in->good() );
                    REQUIRE( true == in->fail() );
                    REQUIRE( 0 == in->contentSize() );
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
                const std::string& url = fname_payload_lst[file_idx];
                REQUIRE( false == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( false == jau::io::uri_tk::protocol_supported(url) );

                std::unique_ptr<jau::io::ByteStream> in = jau::io::to_ByteInStream(url);
                if( nullptr != in ) {
                    PLAIN_PRINT(true, "test00_protocols: local-file-0: %s", in->toString());
                }
                REQUIRE( nullptr != in );
                REQUIRE( false == in->fail() );

                bool res = transfer(*in, fname_payload_copy_lst[file_idx]);
                REQUIRE( true == res );

                jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                REQUIRE( true == out_stats.exists() );
                REQUIRE( true == out_stats.is_file() );
                REQUIRE( in->contentSize() == out_stats.size() );
                REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                REQUIRE( true == jau::io::fs::compare(in->id(), out_stats.path(), true /* verbose */) );
            }
            {
                const std::string url = "file://" + fname_payload_lst[file_idx];
                REQUIRE( true == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( file_support_expected == jau::io::uri_tk::protocol_supported(url) );

                std::unique_ptr<jau::io::ByteStream> in = jau::io::to_ByteInStream(url);
                if( nullptr != in ) {
                    PLAIN_PRINT(true, "test00_protocols: local-file-1: %s", in->toString());
                } else {
                    PLAIN_PRINT(true, "test00_protocols: local-file-1: NULL from url '%s'", url);
                }
                REQUIRE( nullptr != in );
                REQUIRE( false == in->fail() );

                bool res = transfer(*in, fname_payload_copy_lst[file_idx]);
                REQUIRE( true == res );

                jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                REQUIRE( true == out_stats.exists() );
                REQUIRE( true == out_stats.is_file() );
                REQUIRE( in->contentSize() == out_stats.size() );
                REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                REQUIRE( true == jau::io::fs::compare(fname_payload_lst[file_idx], out_stats.path(), true /* verbose */) );
            }
            {
                const std::string url = url_input_root + fname_payload_lst[file_idx];
                REQUIRE( false == jau::io::uri_tk::is_local_file_protocol(url) );
                REQUIRE( http_support_expected == jau::io::uri_tk::protocol_supported(url) );

                std::unique_ptr<jau::io::ByteStream> in = jau::io::to_ByteInStream(url);
                if( nullptr != in ) {
                    PLAIN_PRINT(true, "test00_protocols: http: %s", in->toString());
                }
                if( http_support_expected ) {
                    REQUIRE( nullptr != in );
                    REQUIRE( false == in->fail() );

                    bool res = transfer(*in, fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == res );

                    jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( in->contentSize() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::io::fs::compare(fname_payload_lst[file_idx], out_stats.path(), true /* verbose */) );
                } else {
                    REQUIRE( nullptr == in );
                }
            }
        }

        void test01_copy_file_ok_11kiB_buff4k() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            const size_t file_idx = IDX_11kiB;
            jau::io::ByteStream_File data_stream(fname_payload_lst[file_idx], jau::io::iomode_t::read);

            bool res = transfer(data_stream, fname_payload_copy_lst[file_idx], 4096);
            REQUIRE( true == res );

            jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
            REQUIRE( true == out_stats.exists() );
            REQUIRE( true == out_stats.is_file() );
            REQUIRE( data_stream.contentSize() == out_stats.size() );
            REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
            REQUIRE( true == jau::io::fs::compare(data_stream.id(), out_stats.path(), true /* verbose */) );
        }

        void test02_copy_file_ok_65MiB_buff4k() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            const size_t file_idx = IDX_65MiB;
            jau::io::ByteStream_File data_stream(fname_payload_lst[file_idx], jau::io::iomode_t::read);

            bool res = transfer(data_stream, fname_payload_copy_lst[file_idx], 4096);
            REQUIRE( true == res );

            jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
            REQUIRE( true == out_stats.exists() );
            REQUIRE( true == out_stats.is_file() );
            REQUIRE( data_stream.contentSize() == out_stats.size() );
            REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
            REQUIRE( true == jau::io::fs::compare(data_stream.id(), out_stats.path(), true /* verbose */) );
        }

        void test04_copy_file_ok_65MiB_buff32k() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            const size_t file_idx = IDX_65MiB;
            jau::io::ByteStream_File data_stream(fname_payload_lst[file_idx], jau::io::iomode_t::read);

            bool res = transfer(data_stream, fname_payload_copy_lst[file_idx], 32768);
            REQUIRE( true == res );

            jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
            REQUIRE( true == out_stats.exists() );
            REQUIRE( true == out_stats.is_file() );
            REQUIRE( data_stream.contentSize() == out_stats.size() );
            REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
            REQUIRE( true == jau::io::fs::compare(data_stream.id(), out_stats.path(), true /* verbose */) );
        }

        void test11_copy_http_ok_buff32k() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            if( !jau::io::uri_tk::protocol_supported("http:") ) {
                PLAIN_PRINT(true, "http not supported, abort\n");
                return;
            }
            httpd_start();
            {
                const size_t file_idx = IDX_11kiB;

                const std::string uri_original = url_input_root + fname_payload_lst[file_idx];

                jau::io::ByteInStream_URL data_stream(uri_original, 500_ms);

                bool res = transfer(data_stream, fname_payload_copy_lst[file_idx], 32768);
                REQUIRE( true == res );

                jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                REQUIRE( true == out_stats.exists() );
                REQUIRE( true == out_stats.is_file() );
                REQUIRE( data_stream.contentSize() == out_stats.size() );
                REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                REQUIRE( true == jau::io::fs::compare(fname_payload_lst[file_idx], out_stats.path(), true /* verbose */) );
            }
            {
                const size_t file_idx = IDX_65MiB;

                const std::string uri_original = url_input_root + fname_payload_lst[file_idx];

                jau::io::ByteInStream_URL data_stream(uri_original, 500_ms);

                bool res = transfer(data_stream, fname_payload_copy_lst[file_idx], 32768);
                REQUIRE( true == res );

                jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                REQUIRE( true == out_stats.exists() );
                REQUIRE( true == out_stats.is_file() );
                REQUIRE( data_stream.contentSize() == out_stats.size() );
                REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                REQUIRE( true == jau::io::fs::compare(fname_payload_lst[file_idx], out_stats.path(), true /* verbose */) );
            }
        }

        void test12_copy_http_404() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            if( !jau::io::uri_tk::protocol_supported("http:") ) {
                PLAIN_PRINT(true, "http not supported, abort\n");
                return;
            }
            httpd_start();
            {
                const size_t file_idx = IDX_11kiB;

                const std::string uri_original = url_input_root + "doesnt_exists.txt";

                jau::io::ByteInStream_URL data_stream(uri_original, 500_ms);

                bool res = transfer(data_stream, fname_payload_copy_lst[file_idx]);
                REQUIRE( false == res );

                jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                REQUIRE( true == out_stats.exists() );
                REQUIRE( true == out_stats.is_file() );
                REQUIRE( data_stream.fail() == true );
                REQUIRE( data_stream.hasContentSize() == false );
                REQUIRE( data_stream.contentSize() == 0 );
                REQUIRE( 0 == out_stats.size() );
            }
        }

        // throttled, no content size, interruptReader() via set_eof() will avoid timeout
        static void feed_source_00(jau::io::ByteInStream_Feed * data_feed, const size_t feed_size=1024) {
            uint64_t xfer_total = 0;
            jau::io::ByteStream_File data_stream(data_feed->id(), jau::io::iomode_t::read);
            std::vector<uint8_t> buffer;
            buffer.resize(feed_size);
            while( data_stream.good() ) {
                size_t count = data_stream.read(buffer.data(), buffer.size());
                if( 0 < count ) {
                    xfer_total += count;
                    if( data_feed->write(buffer.data(), count) ) {
                        jau::sleep_for( 16_ms );
                    } else {
                        break;
                    }
                }
            }
            // probably set after transfering due to above sleep, which also ends when total size has been reached.
            data_feed->setEOF( data_feed->fail() ? jau::io::io_result_t::FAILED : jau::io::io_result_t::SUCCESS );
            (void)xfer_total; // not used yet ..
        }

        // throttled, with content size
        static void feed_source_01(jau::io::ByteInStream_Feed * data_feed, const size_t feed_size=1024) {
            uint64_t xfer_total = 0;
            jau::io::ByteStream_File data_stream(data_feed->id(), jau::io::iomode_t::read);
            const uint64_t file_size = data_stream.contentSize();
            data_feed->setContentSize( file_size );
            std::vector<uint8_t> buffer;
            buffer.resize(feed_size);
            while( data_stream.good() && xfer_total < file_size ) {
                size_t count = data_stream.read(buffer.data(), buffer.size());
                if( 0 < count ) {
                    xfer_total += count;
                    if( data_feed->write(buffer.data(), count) ) {
                        jau::sleep_for( 16_ms );
                    } else {
                        break;
                    }
                }
            }
            // probably set after transfering due to above sleep, which also ends when total size has been reached.
            data_feed->setEOF( !data_feed->fail() && xfer_total == file_size ? jau::io::io_result_t::SUCCESS : jau::io::io_result_t::FAILED );
        }

        // full speed, with content size
        static void feed_source_10(jau::io::ByteInStream_Feed * data_feed, const size_t feed_size=1024) {
            uint64_t xfer_total = 0;
            jau::io::ByteStream_File data_stream(data_feed->id(), jau::io::iomode_t::read);
            const uint64_t file_size = data_stream.contentSize();
            data_feed->setContentSize( data_stream.contentSize() );
            std::vector<uint8_t> buffer;
            buffer.resize(feed_size);
            while( data_stream.good() && xfer_total < file_size ) {
                size_t count = data_stream.read(buffer.data(), buffer.size());
                if( 0 < count ) {
                    xfer_total += count;
                    if( !data_feed->write(buffer.data(), count) ) {
                        break;
                    }
                }
            }
            data_feed->setEOF( !data_feed->fail() && xfer_total == file_size ? jau::io::io_result_t::SUCCESS : jau::io::io_result_t::FAILED );
        }

        // full speed, no content size, interrupting @ 1024 bytes within our header
        static void feed_source_20(jau::io::ByteInStream_Feed * data_feed, const size_t feed_size=1024) {
            uint64_t xfer_total = 0;
            jau::io::ByteStream_File data_stream(data_feed->id(), jau::io::iomode_t::read);
            std::vector<uint8_t> buffer;
            buffer.resize(feed_size);
            while( data_stream.good() ) {
                size_t count = data_stream.read(buffer.data(), buffer.size());
                if( 0 < count ) {
                    xfer_total += count;
                    if( data_feed->write(buffer.data(), count) ) {
                        if( xfer_total >= 1024 ) {
                            data_feed->setEOF( jau::io::io_result_t::FAILED ); // calls data_feed->interruptReader();
                            return;
                        }
                    } else {
                        break;
                    }
                }
            }
            // probably set after transfering due to above sleep, which also ends when total size has been reached.
            // data_feed->set_eof( jau::io::async_io_result_t::SUCCESS );
        }

        // full speed, with content size, interrupting 1/4 way
        static void feed_source_21(jau::io::ByteInStream_Feed * data_feed, const size_t feed_size=1024) {
            uint64_t xfer_total = 0;
            jau::io::ByteStream_File data_stream(data_feed->id(), jau::io::iomode_t::read);
            const uint64_t file_size = data_stream.contentSize();
            data_feed->setContentSize( data_stream.contentSize() );
            std::vector<uint8_t> buffer;
            buffer.resize(feed_size);
            while( data_stream.good() ) {
                size_t count = data_stream.read(buffer.data(), buffer.size());
                if( 0 < count ) {
                    xfer_total += count;
                    if( data_feed->write(buffer.data(), count) ) {
                        if( xfer_total >= file_size/4 ) {
                            data_feed->setEOF( jau::io::io_result_t::FAILED ); // calls data_feed->interruptReader();
                            return;
                        }
                    } else {
                        break;
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

                    jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.contentSize() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::io::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
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

                    jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.contentSize() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::io::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
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

                    jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.contentSize() == 0 );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::io::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
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

                    jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.contentSize() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::io::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
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

                    jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.contentSize() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::io::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
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

                    jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.contentSize() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::io::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
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

                    jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.contentSize() == 0 );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::io::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
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

                    jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( data_feed.contentSize() == out_stats.size() );
                    REQUIRE( fname_payload_size_lst[file_idx] == out_stats.size() );
                    REQUIRE( true == jau::io::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
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

                    jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( false == data_feed.hasContentSize() );
                    REQUIRE( 0 == data_feed.contentSize() );
                    REQUIRE( fname_payload_size_lst[file_idx] > out_stats.size() ); // interrupted...
                    REQUIRE( false == jau::io::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
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

                    jau::io::fs::file_stats out_stats(fname_payload_copy_lst[file_idx]);
                    REQUIRE( true == out_stats.exists() );
                    REQUIRE( true == out_stats.is_file() );
                    REQUIRE( true == data_feed.hasContentSize() );
                    REQUIRE( fname_payload_size_lst[file_idx] == data_feed.contentSize() );
                    REQUIRE( data_feed.contentSize() > out_stats.size() ); // interrupted...
                    REQUIRE( false == jau::io::fs::compare(data_feed.id(), out_stats.path(), true /* verbose */) );
                }
            }
        }

        void test_stream_seek(jau::io::ByteStream& in) {
            REQUIRE( true == in.isOpen() );
            REQUIRE( false == in.canWrite() );
            REQUIRE( true == in.hasContentSize() );
            uint64_t p0 = 0;
            REQUIRE( p0 == in.position() );
            const uint64_t len = in.contentSize();
            REQUIRE( true == in.available(len) );
            p0 = len/4;
            REQUIRE( 0 < p0 );
            REQUIRE( p0 == in.seek(p0) );
            REQUIRE( p0 == in.position() );
            REQUIRE( true == in.available(len-p0) );
            REQUIRE( p0 == in.discard(p0) );
            p0 = len/2;
            REQUIRE( p0 == in.position() );
            REQUIRE( true == in.available(len-p0) );
        }
        void test30_mem_seek() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            // 4*40
            jau::io::ByteStream_SecMemory data_stream("0123456789abcdefghjklmnopqrstuvwxyz_()#$0123456789abcdefghjklmnopqrstuvwxyz_()#$0123456789abcdefghjklmnopqrstuvwxyz_()#$0123456789abcdefghjklmnopqrstuvwxyz_()#$");
            test_stream_seek(data_stream);
        }
        void test31_file_seek() {
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            const size_t file_idx = IDX_65MiB;
            jau::io::ByteStream_File data_stream(fname_payload_lst[file_idx], jau::io::iomode_t::read);
            test_stream_seek(data_stream);
        }

        constexpr static const char* test40_src_name = "test40_seek.src.in";
        constexpr static const char* test40_cmp1_name = "test40_seek.cmp1.in";
        constexpr static const char* test40_cmp2_name = "test40_seek.cmp2.in";
        constexpr static const char* test40_cmp3_name = "test40_seek.cmp3.in";

        void test40_seek_prep() {

            jau::io::fs::remove(test40_src_name);
            jau::io::fs::remove(test40_cmp1_name);
            jau::io::fs::remove(test40_cmp2_name);
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            // 4*40
            jau::io::ByteStream_SecMemory in("0123456789abcdefghjklmnopqrstuvwxyz_()#$0123456789abcdefghjklmnopqrstuvwxyz_()#$0123456789abcdefghjklmnopqrstuvwxyz_()#$0123456789abcdefghjklmnopqrstuvwxyz_()#$");
            REQUIRE( true == transfer(in, test40_src_name, 1024) );
            // s(41) + 40 + s(40) + 39
            jau::io::ByteStream_SecMemory cmp1("123456789abcdefghjklmnopqrstuvwxyz_()#$0123456789abcdefghjklmnopqrstuvwxyz_()#$");
            REQUIRE( true == transfer(cmp1, test40_cmp1_name, 1024) );
            // s(28) + 31 + s(41) + 60
            jau::io::ByteStream_SecMemory cmp2("tuvwxyz_()#$0123456789abcdefghjlmnopqrstuvwxyz_()#$0123456789abcdefghjklmnopqrstuvwxyz_()#$");
            REQUIRE( true == transfer(cmp2, test40_cmp2_name, 1024) );

            jau::io::ByteStream_SecMemory cmp3("0123456789abcde||0123456789abcde||#$||#$||#$||abcdefghjklmnopqrstuvwxy||abcdefghjklmnopqrstuvwxy||abcdefghjklmnopqrstuvwxyz||abcdefghjklmnopqrstuvwxyz||abcdefghjklmnopqrstuvwxyz||");
            REQUIRE( true == transfer(cmp3, test40_cmp3_name, 1024) );
        }

        void test_stream_seek(jau::io::ByteStream& in, const std::string& out_name, const std::string& cmp_name,
                              size_t skip1, size_t take1, size_t skip2, size_t take2) {
            REQUIRE( true == in.isOpen() );
            REQUIRE( false == in.canWrite() );
            REQUIRE( true == in.hasContentSize() );

            jau::io::ByteStream_File out(out_name, jau::io::iomode_t::writetrunc);
            REQUIRE( true == out.good() );
            REQUIRE( true == out.isOpen() );
            {
                char buffer[100];

                uint64_t p0 = 0;
                REQUIRE( p0 == in.position() );
                const uint64_t len = in.contentSize();
                REQUIRE( true == in.available(len) );
                {
                    p0 = skip1;
                    REQUIRE( p0 == in.seek(p0) );
                    REQUIRE( p0 == in.position() );
                    REQUIRE( true == in.available(len-p0) );
                }
                REQUIRE( take1 == in.read(buffer, take1) );
                p0 += take1;
                REQUIRE( p0 == in.position() );
                REQUIRE(take1 == out.write(buffer, take1) );
                REQUIRE(take1 == out.position() );
                {
                    REQUIRE( skip2 == in.discard(skip2) );
                    p0 += skip2;
                    REQUIRE( p0 == in.position() );
                    REQUIRE( true == in.available(len-p0) );
                }
                REQUIRE( true == in.available(take2) );
                REQUIRE( take2 == in.contentSize() - in.position() );
                REQUIRE( take2 == in.read(buffer, take2) );
                p0 += take2;
                REQUIRE( p0 == in.position() );
                REQUIRE(take2 == out.write(buffer, take2) );
                REQUIRE(take1+take2 == out.position() );
                REQUIRE( in.contentSize() == in.position() );
                REQUIRE( true == in.eof() );
            }
            REQUIRE( true == jau::io::fs::compare(out_name, cmp_name, true /* verbose */) );
        }

        void test41_seek_rw_file() {
            const std::string outfile_name01 = "test41_seek.01.out";
            const std::string outfile_name02 = "test41_seek.02.out";
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);

            jau::io::ByteStream_File in(test40_src_name, jau::io::iomode_t::read);
            REQUIRE( true == in.isOpen() );
            REQUIRE( false == in.canWrite() );
            REQUIRE( true == in.hasContentSize() );
            test_stream_seek(in, outfile_name01, test40_cmp1_name, 41, 40, 40, 39);
            REQUIRE(0 == in.seek(0) );
            test_stream_seek(in, outfile_name02, test40_cmp2_name, 28, 31, 41, 60);
        }

        void test42_seek_rw_url() {
            const std::string uri_original = url_input_root + test40_src_name;
            const std::string outfile_name01 = "test42_seek.01.out";
            const std::string outfile_name02 = "test42_seek.02.out";

            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            if( !jau::io::uri_tk::protocol_supported("http:") ) {
                PLAIN_PRINT(true, "http not supported, abort\n");
                return;
            }
            httpd_start();
            {

                jau::io::ByteInStream_URL in(uri_original, 500_ms);
                REQUIRE( true == in.isOpen() );
                REQUIRE( false == in.canWrite() );
                REQUIRE( true == in.hasContentSize() );
                test_stream_seek(in, outfile_name01, test40_cmp1_name, 41, 40, 40, 39);
                REQUIRE(jau::io::ByteStream::npos == in.seek(0) );
            }
            {

                jau::io::ByteInStream_URL in(uri_original, 500_ms);
                REQUIRE( true == in.isOpen() );
                REQUIRE( false == in.canWrite() );
                REQUIRE( true == in.hasContentSize() );
                test_stream_seek(in, outfile_name02, test40_cmp2_name, 28, 31, 41, 60);
                REQUIRE(jau::io::ByteStream::npos == in.seek(0) );
            }
        }

        void test_stream_rewind(jau::io::ByteStream& in, const std::string& out_name, const std::string& cmp_name) {
            REQUIRE( true == in.isOpen() );
            REQUIRE( false == in.canWrite() );
            REQUIRE( true == in.hasContentSize() );

            jau::io::ByteStream_File out(out_name, jau::io::iomode_t::writetrunc);
            REQUIRE( true == out.good() );
            REQUIRE( true == out.isOpen() );

            // "0123456789abcdefghjklmnopqrstuvwxyz_()#$0123456789abcdefghjklmnopqrstuvwxyz_()#$0123456789abcdefghjklmnopqrstuvwxyz_()#$0123456789abcdefghjklmnopqrstuvwxyz_()#$"
            // 2x [ 40- 55): 0123456789abcde (15 = 30)
            // 3x [ 38- 40): #$ (2 = 6)
            // 2x [ 50- 74): abcdefghjklmnopqrstuvwxy (24 = 48)
            // 3x [130-155): abcdefghjklmnopqrstuvwxyz (25 = 75)
            // "0123456789abcde||0123456789abcde||#$||#$||#$||abcdefghjklmnopqrstuvwxy||abcdefghjklmnopqrstuvwxy||abcdefghjklmnopqrstuvwxyz||abcdefghjklmnopqrstuvwxyz||abcdefghjklmnopqrstuvwxyz"
            // 30 + 6 + 48 + 75 = 159 -> 177
            {
                char buffer[100];

                REQUIRE( 0 == in.position() );
                const uint64_t len = in.contentSize();
                REQUIRE( true == in.available(len) );

                {
                    REQUIRE( jau::io::ByteStream::npos == in.mark() );
                    REQUIRE( true == in.setMark(5) );
                    REQUIRE( 0 == in.mark() );
                    std::memset(buffer, 0, sizeof(buffer));
                    REQUIRE( 15 == in.read(buffer, 15) ); // if not unlimited: invalidates mark (readLimit exceeded)
                    if( 15 >= in.markReadLimit() ) {
                        REQUIRE( jau::io::ByteStream::npos == in.mark() );
                    }
                    REQUIRE( 15 == in.position() );
                }

                // read to 38
                std::memset(buffer, 0, sizeof(buffer));
                REQUIRE( 38-15 == in.read(buffer, 38-15) );
                REQUIRE( 38 == in.position() );

                REQUIRE( true == in.setMark(17) ); // covering [38-55), also allowing seek-back
                REQUIRE( 38 == in.mark() );
                REQUIRE( 38 == in.position() );

                // read to 40
                std::memset(buffer, 0, sizeof(buffer));
                REQUIRE( 40-38 == in.read(buffer, 40-38) );
                REQUIRE( 40 == in.position() );

                // x1: 15
                REQUIRE( 38 == in.mark() );
                REQUIRE( 40 == in.position() );
                std::memset(buffer, 0, sizeof(buffer));
                REQUIRE( 15 == in.read(buffer, 15) );
                REQUIRE( 55 == in.position() );
                REQUIRE( 38 == in.mark() );
                REQUIRE( 15 == out.write(buffer, 15) );
                REQUIRE(  2 == out.write("||", 2) );
                REQUIRE( 40 == in.seek(40) ); // seek back (allowed on all stream types due to mark readLimit)
                REQUIRE( 40 == in.position() );
                // x2: 15
                REQUIRE( 40 == in.position() );
                std::memset(buffer, 0, sizeof(buffer));
                REQUIRE( 15 == in.read(buffer, 15) );
                REQUIRE( 55 == in.position() );
                REQUIRE( 38 == in.mark() );
                REQUIRE( 15 == out.write(buffer, 15) );
                REQUIRE(  2 == out.write("||", 2) );

                REQUIRE( true == in.seekMark() );
                REQUIRE( 38 == in.position() );

                // x1: 2
                REQUIRE( 38 == in.mark() );
                REQUIRE( 38 == in.position() );
                std::memset(buffer, 0, sizeof(buffer));
                REQUIRE(  2 == in.read(buffer, 2) );
                REQUIRE( 40 == in.position() );
                REQUIRE( 38 == in.mark() );
                REQUIRE(  2 == out.write(buffer, 2) );
                REQUIRE(  2 == out.write("||", 2) );
                REQUIRE( true == in.seekMark() );
                // x2: 2
                REQUIRE( 38 == in.position() );
                std::memset(buffer, 0, sizeof(buffer));
                REQUIRE(  2 == in.read(buffer, 2) );
                REQUIRE( 40 == in.position() );
                REQUIRE( 38 == in.mark() );
                REQUIRE(  2 == out.write(buffer, 2) );
                REQUIRE(  2 == out.write("||", 2) );
                REQUIRE( true == in.seekMark() );
                // x3: 2
                REQUIRE( 38 == in.position() );
                std::memset(buffer, 0, sizeof(buffer));
                REQUIRE(  2 == in.read(buffer, 2) );
                REQUIRE( 40 == in.position() );
                REQUIRE( 38 == in.mark() );
                REQUIRE(  2 == out.write(buffer, 2) );
                REQUIRE(  2 == out.write("||", 2) );
                REQUIRE( true == in.seekMark() );

                // seek to 50
                std::memset(buffer, 0, sizeof(buffer));
                REQUIRE( 50 == in.seek(50) );
                REQUIRE( 50 == in.position() );

                // x1: 24
                REQUIRE( true == in.setMark(25) );
                REQUIRE( 50 == in.mark() );
                REQUIRE( 50 == in.position() );
                std::memset(buffer, 0, sizeof(buffer));
                REQUIRE( 24 == in.read(buffer, 24) );
                REQUIRE( 74 == in.position() );
                REQUIRE( 50 == in.mark() );
                REQUIRE( 24 == out.write(buffer, 24) );
                REQUIRE(  2 == out.write("||", 2) );
                REQUIRE( 50 == in.seek(50) ); // alternative to seekMark!
                // x2: 24
                REQUIRE( 50 == in.position() );
                std::memset(buffer, 0, sizeof(buffer));
                REQUIRE( 24 == in.read(buffer, 24) );
                REQUIRE( 74 == in.position() );
                REQUIRE( 50 == in.mark() );
                REQUIRE( 24 == out.write(buffer, 24) );
                REQUIRE(  2 == out.write("||", 2) );

                // read to 130
                std::memset(buffer, 0, sizeof(buffer));
                REQUIRE( 130-74 == in.read(buffer, 130-74) );
                REQUIRE( 130 == in.position() );

                // x1: 25
                REQUIRE( true == in.setMark(in.markReadLimit()) );
                REQUIRE( 130 == in.mark() );
                REQUIRE( 130 == in.position() );
                std::memset(buffer, 0, sizeof(buffer));
                REQUIRE(  25 == in.read(buffer, 25) );
                REQUIRE( 155 == in.position() );
                REQUIRE( 130 == in.mark() );
                REQUIRE(  25 == out.write(buffer, 25) );
                REQUIRE(   2 == out.write("||", 2) );
                REQUIRE( 130 == in.seek(130) ); // alternative to seekMark!
                // x2: 25
                REQUIRE( 130 == in.position() );
                std::memset(buffer, 0, sizeof(buffer));
                REQUIRE(  25 == in.read(buffer, 25) );
                REQUIRE( 155 == in.position() );
                REQUIRE( 130 == in.mark() );
                REQUIRE(  25 == out.write(buffer, 25) );
                REQUIRE(   2 == out.write("||", 2) );
                REQUIRE( true == in.seekMark() );
                // x3: 25
                REQUIRE( 130 == in.position() );
                std::memset(buffer, 0, sizeof(buffer));
                REQUIRE(  25 == in.read(buffer, 25) );
                REQUIRE( 155 == in.position() );
                REQUIRE( 130 == in.mark() );
                REQUIRE(  25 == out.write(buffer, 25) );
                REQUIRE(   2 == out.write("||", 2) );

                ///

                REQUIRE( 179 == out.position() );
                REQUIRE( false == in.eof() );
            }
            REQUIRE( true == jau::io::fs::compare(out_name, cmp_name, true /* verbose */) );
        }

        void test4a_rewind_rw_file() {
            const std::string outfile_name01 = "test4a_rewind.01.out";
            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);

            jau::io::ByteStream_File in(test40_src_name, jau::io::iomode_t::read);
            REQUIRE( true == in.isOpen() );
            REQUIRE( false == in.canWrite() );
            REQUIRE( true == in.hasContentSize() );
            test_stream_rewind(in, outfile_name01, test40_cmp3_name);
        }

        void test4b_rewind_rw_url() {
            const std::string uri_original = url_input_root + test40_src_name;
            const std::string outfile_name01 = "test4b_rewind.01.out";

            jau::fprintf_td(stderr, "\n"); jau::fprintf_td(stderr, "%s\n", __func__);
            if( !jau::io::uri_tk::protocol_supported("http:") ) {
                PLAIN_PRINT(true, "http not supported, abort\n");
                return;
            }
            httpd_start();
            {
                jau::io::ByteInStream_URL in(uri_original, 500_ms);
                REQUIRE( true == in.isOpen() );
                REQUIRE( false == in.canWrite() );
                REQUIRE( true == in.hasContentSize() );
                test_stream_rewind(in, outfile_name01, test40_cmp3_name);
                REQUIRE(jau::io::ByteStream::npos == in.seek(0) );
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

METHOD_AS_TEST_CASE( TestByteStream01::test30_mem_seek,                   "test30_mem_seek");
METHOD_AS_TEST_CASE( TestByteStream01::test31_file_seek,                  "test31_file_seek");
METHOD_AS_TEST_CASE( TestByteStream01::test40_seek_prep,                  "test40_seek_prep");
METHOD_AS_TEST_CASE( TestByteStream01::test41_seek_rw_file,               "test41_seek_rw_file");
METHOD_AS_TEST_CASE( TestByteStream01::test42_seek_rw_url,                "test42_seek_rw_url");
METHOD_AS_TEST_CASE( TestByteStream01::test4a_rewind_rw_file,             "test4a_rewind_rw_file");
METHOD_AS_TEST_CASE( TestByteStream01::test4b_rewind_rw_url,              "test4b_rewind_rw_url");
