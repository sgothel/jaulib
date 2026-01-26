/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2024 Gothel Software e.K.
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

#include <memory>
#include <thread>
#include <pthread.h>

#include <jau/test/catch2_ext.hpp>

#include <jau/io/file_util.hpp>
#include <jau/io/io_util.hpp>
#include <jau/io/byte_stream.hpp>

#include <jau/debug.hpp>

extern "C" {
    #include <unistd.h>
}

using namespace jau::fractions_i64_literals;
using namespace jau::int_literals;

class TestHttpStream01 {
    public:
        const std::string url_input_root = "http://httpbin.org/post";
        // const std::string url_input_root = "https://ssd.jpl.nasa.gov/api/horizons_file.api";
        const std::string HttpBoundarySep = "--";
        const std::string HttpBoundary = "affedeadbeaf";
        const std::string CRLF = "\r\n";
        const std::string HorizonCmd01 =
            "!$$SOF\n"
            "COMMAND='199'\n"
            "TABLE_TYPE='Vector'\n"
            "CENTER='@010'\n"
            "REF_PLANE='Ecliptic'\n"
            "START_TIME='2024-01-01 00:00:00'\n"
            "STOP_TIME='2024-01-01 00:00:01'\n";

        TestHttpStream01() = default;

        ~TestHttpStream01() =default;

        void test01_post_sync_ok() {
            if( !jau::io::uri_tk::protocol_supported("http:") ) {
                jau_PLAIN_PRINT(true, "http not supported, abort\n");
                return;
            }
            if( catch_auto_run ) {
                jau_PLAIN_PRINT(true, "not enabled on auto-run\n");
                return;
            }
            jau::io::http::PostRequestPtr postReq = std::make_unique<jau::io::http::PostRequest>();
            postReq->header.emplace("Content-Type", "multipart/form-data; boundary="+HttpBoundary);
            postReq->body.append(HttpBoundarySep).append(HttpBoundary).append(CRLF);
            postReq->body.append("Content-Disposition: form-data; name=\"format\"").append(CRLF).append(CRLF);
            postReq->body.append("text").append(CRLF);
            postReq->body.append(HttpBoundarySep).append(HttpBoundary).append(CRLF);
            postReq->body.append(R"(Content-Disposition: form-data; name="input"; filename="a.cmd")").append(CRLF);
            postReq->body.append("Content-type: application/octet-stream").append(CRLF).append(CRLF);
            postReq->body.append(HorizonCmd01).append(CRLF);
            postReq->body.append(HttpBoundarySep).append(HttpBoundary).append(HttpBoundarySep).append(CRLF);

            jau::io::net_tk_handle handle = jau::io::create_net_tk_handle();
            jau::relaxed_atomic_uint64 consumed_byte_count;
            jau::io::SyncStreamResponseRef res = jau::io::read_url_stream_sync(handle, url_input_root, std::move(postReq), nullptr,
                                                    [&consumed_byte_count](jau::io::SyncStreamResponse& response, const uint8_t* data , size_t len, bool is_final) -> bool {
                std::cout << "XXX: len " << len << "/" << response.content_length << ", final " << is_final << std::endl;
                if( nullptr != data && len > 0 ) {
                    std::string d(data, data+len);
                    std::cout << "  > " << d << std::endl;
                }
                consumed_byte_count.fetch_add(len);
                return true;
            });
            REQUIRE( nullptr != res );

            jau::io::free_net_tk_handle(handle);

            jau_PLAIN_PRINT(true, "test01_post_sync_ok.X Done: consumed %" PRIu64 " / total %" PRIu64 " / content_len %" PRIu64 ", result %d",
                    consumed_byte_count.load(), res->total_read, res->content_length, (int)res->result.load() );

            REQUIRE( res->header_resp.completed() == true );
            REQUIRE( res->header_resp.response_code() == 200 );
            if( res->has_content_length ) {
                REQUIRE( res->content_length == consumed_byte_count );
            }
            REQUIRE( res->total_read == consumed_byte_count );
            REQUIRE( res->success() );
        }

        void test11_post_async_ok() {
            if( !jau::io::uri_tk::protocol_supported("http:") ) {
                jau_PLAIN_PRINT(true, "http not supported, abort\n");
                return;
            }
            if( catch_auto_run ) {
                jau_PLAIN_PRINT(true, "not enabled on auto-run\n");
                return;
            }
            jau::io::http::PostRequestPtr postReq = std::make_unique<jau::io::http::PostRequest>();
            postReq->header.emplace("Content-Type", "multipart/form-data; boundary="+HttpBoundary);
            postReq->body.append(HttpBoundarySep).append(HttpBoundary).append(CRLF);
            postReq->body.append("Content-Disposition: form-data; name=\"format\"").append(CRLF).append(CRLF);
            postReq->body.append("text").append(CRLF);
            postReq->body.append(HttpBoundarySep).append(HttpBoundary).append(CRLF);
            postReq->body.append(R"(Content-Disposition: form-data; name="input"; filename="a.cmd")").append(CRLF);
            postReq->body.append("Content-type: application/octet-stream").append(CRLF).append(CRLF);
            postReq->body.append(HorizonCmd01).append(CRLF);
            postReq->body.append(HttpBoundarySep).append(HttpBoundary).append(HttpBoundarySep).append(CRLF);

            jau::io::net_tk_handle handle = jau::io::create_net_tk_handle();
            jau::relaxed_atomic_uint64 consumed_byte_count;
            jau::io::AsyncStreamResponseRef res = jau::io::read_url_stream_async(handle, url_input_root, std::move(postReq), nullptr,
                                                    [&consumed_byte_count](jau::io::AsyncStreamResponse& response, const uint8_t* data , size_t len, bool is_final) -> bool {
                std::cout << "XXX: len " << len << "/" << response.content_length << ", final " << is_final << std::endl;
                if( nullptr != data && len > 0 ) {
                    std::string d(data, data+len);
                    std::cout << "  > " << d << std::endl;
                }
                consumed_byte_count.fetch_add(len);
                return true;
            });
            REQUIRE( nullptr != res );

            res->thread.join();
            jau::io::free_net_tk_handle(handle);

            jau_PLAIN_PRINT(true, "test11_post_async_ok.X Done: consumed %" PRIu64 " / total %" PRIu64 " / content_len %" PRIu64 ", result %d",
                    consumed_byte_count.load(), res->total_read.load(), res->content_length.load(), (int)res->result.load() );

            REQUIRE( res->header_resp.completed() == true );
            REQUIRE( res->header_resp.response_code() == 200 );
            if( res->has_content_length ) {
                REQUIRE( res->content_length == consumed_byte_count );
                // REQUIRE( res->content_length == HorizonCmd01.length() );
            }
            REQUIRE( res->total_read == consumed_byte_count );
            REQUIRE( res->success() );
        }

};

METHOD_AS_TEST_CASE( TestHttpStream01::test01_post_sync_ok, "TestIOStream01 - test01_post_sync_ok");
METHOD_AS_TEST_CASE( TestHttpStream01::test11_post_async_ok, "TestIOStream01 - test11_post_async_ok");

