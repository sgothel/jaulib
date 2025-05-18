/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021-2025 Gothel Software e.K.
 * Copyright (c) 2021 ZAFENA AB
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */

#ifndef JAU_IO_IO_UTIL_HPP_
#define JAU_IO_IO_UTIL_HPP_

#include <string>
#include <cstdint>
#include <thread>
#include <vector>

#include <jau/basic_types.hpp>
#include <jau/callocator_sec.hpp>
#include <jau/ringbuffer.hpp>
#include <jau/functional.hpp>

namespace jau::io {
    /** @defgroup IOUtils IO Utilities
     *  Input and Output (IO) types and functionality.
     *
     *  @{
     */

    template<typename T> using secure_vector = std::vector<T, jau::callocator_sec<T>>;

    typedef std::basic_string<char, std::char_traits<char>, jau::callocator_sec<char>> secure_string;

    typedef jau::ringbuffer<uint8_t, size_t> ByteRingbuffer;

    extern const size_t BEST_URLSTREAM_RINGBUFFER_SIZE;

    /**
     * I/O direction, read or write
     */
    enum class io_dir_t : int8_t {
        /** Read Operation */
        READ  = 0,

        /** Write Operation */
        WRITE =  1
    };

    /**
     * I/O operation result value
     */
    enum class io_result_t : int8_t {
        /** Operation failed. */
        FAILED  = -1,

        /** Operation still in progress. */
        NONE    =  0,

        /** Operation succeeded. */
        SUCCESS =  1
    };
    typedef jau::ordered_atomic<io_result_t, std::memory_order_relaxed> relaxed_atomic_io_result_t;

    inline std::string toString(io_result_t v) noexcept {
        switch(v) {
            case io_result_t::SUCCESS: return "SUCCESS";
            case io_result_t::NONE: return "NONE";
            default: return "FAILED";
        }
    }
    inline std::ostream& operator<<(std::ostream& os, io_result_t v) { return os << toString(v); }

    /**
     * Stream consumer function
     * - `bool consumer(secure_vector<uint8_t>& data, bool is_final)`
     *
     * Returns true to signal continuation, false to end streaming.
     */
    typedef jau::function<bool(secure_vector<uint8_t>& /* data */, bool /* is_final */)> StreamConsumerFunc;

    /**
     * Synchronous byte input stream reader from given file path using the given StreamConsumerFunc consumer_fn.
     *
     * To abort streaming, user may return `false` from the given `consumer_func`.
     *
     * It is guaranteed that consumer_fn() is called with `is_final=true` once at the end,
     * even if `input_file` stream has zero size.
     *
     * @param input_file input file name path, `-` denotes std::stdin.
     * @param buffer secure std::vector buffer, passed down to consumer_fn
     * @param consumer_fn StreamConsumerFunc consumer for each received heap of bytes, returning true to continue stream of false to abort.
     * @return total bytes read or 0 if error
     */
    uint64_t read_file(const std::string& input_file,
            secure_vector<uint8_t>& buffer,
            const StreamConsumerFunc& consumer_fn) noexcept;

    class ByteStream; // fwd

    /**
     * Synchronous byte input stream reader using the given StreamConsumerFunc consumer_fn.
     *
     * To abort streaming, user may return `false` from the given `consumer_func`.
     *
     * It is guaranteed that consumer_fn() is called with `is_final=true` once at the end,
     * even input stream has zero size.
     *
     * @param in the input byte stream to read from
     * @param buffer secure std::vector buffer, passed down to consumer_fn
     * @param consumer_fn StreamConsumerFunc consumer for each received heap of bytes, returning true to continue stream of false to abort.
     * @return total bytes read or 0 if error
     */
    uint64_t read_stream(ByteStream& in,
            secure_vector<uint8_t>& buffer,
            const StreamConsumerFunc& consumer_fn) noexcept;

    /**
     * Synchronous double-buffered byte input stream reader using the given StreamConsumerFunc consumer_fn.
     *
     * To abort streaming, user may return `false` from the given `consumer_func`.
     *
     * It is guaranteed that consumer_fn() is called with `is_final=true` once at the end,
     * even if input stream has zero size.
     *
     * Implementation reads one buffer ahead in respect to consumer_fn(). <br/>
     * If reading zero bytes on the next buffer,
     * it propagates the end-of-file (EOF) to the previous buffer which will be send via consumer_fn() next.<br/>
     *
     * This way, the consumer_fn() will always receive its `is_final` flag on the last sent bytes (size > 0)
     * even if the content-size is unknown (pipe). <br/>
     * Hence it allows e.g. decryption to work where the final data chunck must be processed as such.
     *
     * @param in the input byte stream to read from
     * @param buffer1 secure std::vector buffer, passed down to consumer_fn
     * @param buffer2 secure std::vector buffer, passed down to consumer_fn
     * @param consumer_fn StreamConsumerFunc consumer for each received heap of bytes, returning true to continue stream of false to abort.
     * @return total bytes read or 0 if error
     */
    uint64_t read_stream(ByteStream& in,
            secure_vector<uint8_t>& buffer1, secure_vector<uint8_t>& buffer2,
            const StreamConsumerFunc& consumer_fn) noexcept;

    /**
     * Synchronous URL stream reader using the given StreamConsumerFunc consumer_fn.
     *
     * To abort streaming, user may return `false` from the given `consumer_func`.
     *
     * Standard implementation uses [curl](https://curl.se/),
     * hence all [*libcurl* network protocols](https://curl.se/docs/url-syntax.html) are supported,
     * see jau::io::uri::supported_protocols().
     *
     * If the uri-sheme doesn't match a supported protocol, see jau::io::uri::protocol_supported(),
     * function returns immediately with zero bytes.
     *
     * Environment variables:
     * - `jau_io_net_ssl_verifypeer=[true|false]` to enable or disable SSL peer verification. Defaults to `true`.
     * - `jau_io_net_verbose=[true|false]` to enable or disable verbose on stderr stream communication. Defaults to `false`.
     *
     * @param url the URL to open a connection to and stream bytes from
     * @param buffer secure std::vector buffer, passed down to consumer_fn
     * @param consumer_fn StreamConsumerFunc consumer for each received heap of bytes, returning true to continue stream of false to abort.
     * @return total bytes read or 0 if transmission error or protocol of given url is not supported
     */
    uint64_t read_url_stream(const std::string& url,
            secure_vector<uint8_t>& buffer,
            const StreamConsumerFunc& consumer_fn) noexcept;

    /**
     * Synchronized URL header response completion
     * as used by asynchronous read_url_stream().
     *
     * @see url_header_sync::completed()
     */
    class url_header_resp {
        private:
            std::mutex m_sync;
            std::condition_variable m_cv;
            jau::relaxed_atomic_bool m_completed;
            jau::relaxed_atomic_int32 m_response_code;

        public:
            url_header_resp() noexcept // NOLINT(modernize-use-equals-default)
            : m_completed(false)
            { }

            /**
             * Returns whether URL header is completed.
             *
             * Completion is reached in any of the following cases
             * - Final (http) CRLF message received
             * - Any http header error response received
             * - First data package received
             * - End of operation
             */
            bool completed() const noexcept { return m_completed; }

            int32_t response_code() const noexcept { return m_response_code; }

            /**
             * Notify completion, see completed()
             */
            void notify_complete(const int32_t response_code=200) noexcept;

            /**
             * Wait until completed() has been reached.
             * @param timeout maximum duration in fractions of seconds to wait, where fractions_i64::zero waits infinitely
             * @return true if completed within timeout, otherwise false
             */
            bool wait_until_completion(const jau::fraction_i64& timeout) noexcept;
    };

    namespace http {
        struct PostRequest {
            std::unordered_map<std::string, std::string> header;
            std::string body;
        };
        using PostRequestPtr = std::unique_ptr<PostRequest>;
    }

    using net_tk_handle = void*;

    /// creates a reusable handle, free with free_net_tk_handle() after use.
    net_tk_handle create_net_tk_handle() noexcept;
    /// frees a handle after use created by create_net_tk_handle()
    void free_net_tk_handle(net_tk_handle handle) noexcept;

    /** Synchronous stream response */
    struct SyncStreamResponse {
        SyncStreamResponse(net_tk_handle handle_)
        : handle(handle_), header_resp(),
          has_content_length(false),
          content_length(0),
          total_read(0),
          result(io::io_result_t::NONE),
          result_data(), result_text() {}

        SyncStreamResponse()
        : SyncStreamResponse(nullptr) {}

        /** Stream failed and is aborted, i.e. io_result_t::FAILED == result */
        constexpr_atomic bool failed() const noexcept { return io_result_t::FAILED == result; }
        /** Stream processing in progress, i.e. io_result_t::NONE == result */
        constexpr_atomic bool processing() const noexcept { return io_result_t::NONE == result; }
        /** Stream completed successfully, i.e. io_result_t::SUCCESS == result */
        constexpr_atomic bool success() const noexcept { return io_result_t::SUCCESS == result; }

        /// used network tookit handle, if owned by caller
        net_tk_handle handle;
        /// synchronized URL header response completion
        url_header_resp header_resp;
        /// indicating whether content_length is known from server
        bool has_content_length;
        /// content_length tracking the content_length
        uint64_t content_length;
        /// tracking the total_read
        uint64_t total_read;
        /// tracking io_result_t. If set to other than io_result_t::NONE while streaming, streaming is aborted. See failed(), processing() and success()
        relaxed_atomic_io_result_t result;
        /// piggy-bag result data compiled by user, e.g. via AsyncStreamConsumerFunc
        std::vector<uint8_t> result_data;
        /// piggy-bag result data compiled by user, e.g. via AsyncStreamConsumerFunc
        std::string result_text;
    };
    using SyncStreamResponseRef = std::shared_ptr<SyncStreamResponse>;

    /**
     * Synchronous stream consumer function
     * - `bool consumer(AsyncStreamResponse& resp, const uint8_t* data , size_t len, bool is_final)`
     *
     * Returns true to signal continuation, false to end streaming.
     */
    typedef jau::function<bool(SyncStreamResponse& /* resp */, const uint8_t* /* data */, size_t /* len */, bool /* is_final */)> SyncStreamConsumerFunc;

    /**
     * Synchronous URL stream reader using the given SyncStreamConsumerFunc consumer_fn.
     *
     * Function returns after completion.
     *
     * To abort streaming, (1) user may return `false` from the given `consumer_func`.
     * Asynchronous URL read content using the given byte jau::ringbuffer, allowing parallel reading.
     *
     * To abort streaming, (2) user may set given reference `results` to a value other than async_io_result_t::NONE.
     *
     * Standard implementation uses [curl](https://curl.se/),
     * hence all [*libcurl* network protocols](https://curl.se/docs/url-syntax.html) are supported,
     * see jau::io::uri::supported_protocols().
     *
     * If the uri-sheme doesn't match a supported protocol, see jau::io::uri::protocol_supported(),
     * SyncStreamResponse::failed() returns true.
     *
     * Environment variables:
     * - `jau_io_net_ssl_verifypeer=[true|false]` to enable or disable SSL peer verification. Defaults to `true`.
     * - `jau_io_net_verbose=[true|false]` to enable or disable verbose on stderr stream communication. Defaults to `false`.
     *
     * @param handle optional reused user-pooled net toolkit handle, see create_net_tk_handle(). Pass nullptr to use an own local handle.
     * @param url the URL to open a connection to and stream bytes from
     * @param httpPostReq optional HTTP POST request data, maybe nullptr
     * @param buffer optional jau::ringbuffer<uint8_t>, if not nullptr will be synchronously filled with received data
     * @param consumer_fn SyncStreamConsumerFunc consumer for each received heap of bytes, returning true to continue stream of false to abort.
     * @return new SyncStreamResponse reference. If protocol of given url is not supported, SyncStreamResponse::failed() returns true.
     */
    SyncStreamResponseRef read_url_stream_sync(net_tk_handle handle, const std::string& url,
            http::PostRequestPtr httpPostReq, ByteRingbuffer *buffer,
            const SyncStreamConsumerFunc& consumer_fn) noexcept;

    /** Asynchronous stream response */
    struct AsyncStreamResponse {
        AsyncStreamResponse(net_tk_handle handle_)
        : handle(handle_), thread(), header_resp(),
          has_content_length(false),
          content_length(0),
          total_read(0),
          result(io::io_result_t::NONE),
          result_data(), result_text() {}

        AsyncStreamResponse()
        : AsyncStreamResponse(nullptr) {}

        /** Stream failed and is aborted, i.e. io_result_t::FAILED == result */
        constexpr_atomic bool failed() const noexcept { return io_result_t::FAILED == result; }
        /** Stream processing in progress, i.e. io_result_t::NONE == result */
        constexpr_atomic bool processing() const noexcept { return io_result_t::NONE == result; }
        /** Stream completed successfully, i.e. io_result_t::SUCCESS == result */
        constexpr_atomic bool success() const noexcept { return io_result_t::SUCCESS == result; }

        /// used network tookit handle, if owned by caller
        net_tk_handle handle;
        /// background reading thread unique-pointer
        std::thread thread;
        /// synchronized URL header response completion
        url_header_resp header_resp;
        /// indicating whether content_length is known from server
        relaxed_atomic_bool has_content_length;
        /// content_length tracking the content_length
        relaxed_atomic_uint64 content_length;
        /// tracking the total_read
        relaxed_atomic_uint64 total_read;
        /// tracking io_result_t. If set to other than io_result_t::NONE while streaming, streaming is aborted. See failed(), processing() and success()
        relaxed_atomic_io_result_t result;
        /// piggy-bag result data compiled by user, e.g. via AsyncStreamConsumerFunc
        std::vector<uint8_t> result_data;
        /// piggy-bag result data compiled by user, e.g. via AsyncStreamConsumerFunc
        std::string result_text;
    };
    using AsyncStreamResponseRef = std::shared_ptr<AsyncStreamResponse>;

    /**
     * Asynchronous stream consumer function
     * - `bool consumer(AsyncStreamResponse& resp, const uint8_t* data , size_t len, bool is_final)`
     *
     * Returns true to signal continuation, false to end streaming.
     */
    typedef jau::function<bool(AsyncStreamResponse& /* resp */, const uint8_t* /* data */, size_t /* len */, bool /* is_final */)> AsyncStreamConsumerFunc;

    /**
     * Asynchronous URL stream reader using the given AsyncStreamConsumerFunc consumer_fn.
     *
     * Function returns immediately.
     *
     * To abort streaming, (1) user may return `false` from the given `consumer_func`.
     * Asynchronous URL read content using the given byte jau::ringbuffer, allowing parallel reading.
     *
     * To abort streaming, (2) user may set given reference `results` to a value other than async_io_result_t::NONE.
     *
     * Standard implementation uses [curl](https://curl.se/),
     * hence all [*libcurl* network protocols](https://curl.se/docs/url-syntax.html) are supported,
     * see jau::io::uri::supported_protocols().
     *
     * If the uri-sheme doesn't match a supported protocol, see jau::io::uri::protocol_supported(),
     * AsyncStreamResponse::failed() returns true.
     *
     * Environment variables:
     * - `jau_io_net_ssl_verifypeer=[true|false]` to enable or disable SSL peer verification. Defaults to `true`.
     * - `jau_io_net_verbose=[true|false]` to enable or disable verbose on stderr stream communication. Defaults to `false`.
     *
     * @param handle optional reused user-pooled net toolkit handle, see create_net_tk_handle(). Pass nullptr to use an own local handle.
     * @param url the URL to open a connection to and stream bytes from
     * @param httpPostReq optional HTTP POST request data, maybe nullptr
     * @param buffer optional jau::ringbuffer<uint8_t>, if not nullptr will be asynchronously filled with received data
     * @param consumer_fn AsyncStreamConsumerFunc consumer for each received heap of bytes, returning true to continue stream of false to abort.
     * @return new AsyncStreamResponse reference.  If protocol of given url is not supported, AsyncStreamResponse::failed() returns true.
     */
    AsyncStreamResponseRef read_url_stream_async(net_tk_handle handle, const std::string& url,
            http::PostRequestPtr httpPostReq, ByteRingbuffer *buffer,
            const AsyncStreamConsumerFunc& consumer_fn) noexcept;

    void print_stats(const std::string& prefix, const uint64_t& out_bytes_total, const jau::fraction_i64& td) noexcept;

    /**@}*/

    /**
     * Limited URI toolkit to query handled protocols by the IO implementation.
     *
     * The URI scheme functionality exposed here is limited and only provided to decide whether the used implementation
     * is able to handle the protocol. This is not a replacement for a proper URI class.
     */
    namespace uri_tk {
        /** \addtogroup IOUtils
         *
         *  @{
         */

        /**
         * Returns a list of supported protocol supported by [*libcurl* network protocols](https://curl.se/docs/url-syntax.html),
         * queried at runtime.
         * @see protocol_supported()
         */
        std::vector<std::string_view> supported_protocols() noexcept;

        /**
         * Returns the valid uri-scheme from given uri,
         * which is empty if no valid scheme is included.
         *
         * The given uri must include at least a colon after the uri-scheme part.
         *
         * @param uri an uri
         * @return valid uri-scheme, empty if non found
         */
        std::string_view get_scheme(const std::string_view& uri) noexcept;

        /**
         * Returns true if the uri-scheme of given uri matches a supported by [*libcurl* network protocols](https://curl.se/docs/url-syntax.html) otherwise false.
         *
         * The uri-scheme is retrieved via get_scheme() passing given uri, hence must include at least a colon after the uri-scheme part.
         *
         * The *libcurl* supported protocols is queried at runtime, see supported_protocols().
         *
         * @param uri an uri to test
         * @return true if the uri-scheme of given uri is supported, otherwise false.
         * @see supported_protocols()
         * @see get_scheme()
         */
        bool protocol_supported(const std::string_view& uri) noexcept;

        /**
         * Returns true if the uri-scheme of given uri matches the local `file` protocol, i.e. starts with `file://`.
         * @param uri an uri to test
         */
        bool is_local_file_protocol(const std::string_view& uri) noexcept;

        /**
         * Returns true if the uri-scheme of given uri matches the `http` or `https` protocol, i.e. starts with `http:` or `https:`.
         * @param uri an uri to test
         */
        bool is_httpx_protocol(const std::string_view& uri) noexcept;

        /**@}*/
    }

} // namespace jau::io

#endif /* JAU_IO_IO_UTIL_HPP_ */
