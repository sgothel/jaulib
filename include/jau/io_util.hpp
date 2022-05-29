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

#ifndef JAU_IO_UTIL_HPP_
#define JAU_IO_UTIL_HPP_

#include <string>
#include <cstdint>
#include <functional>
#include <thread>
#include <vector>

#include <jau/basic_types.hpp>
#include <jau/ringbuffer.hpp>

// Include Botan header files before this one to be integrated w/ Botan!
// #include <botan_all.h>

#include <jau/byte_stream.hpp>

namespace jau::io {
        /** @defgroup IOUtils IO Utilities
         *  Input and Output (IO) types and functionality.
         *
         *  @{
         */

        /**
         * Stream consumer function
         * - `bool consumer(secure_vector<uint8_t>& data, bool is_final)`
         *
         * Returns true to signal continuation, false to end streaming.
         */
        typedef std::function<bool (secure_vector<uint8_t>& /* data */, bool /* is_final */)> StreamConsumerFunc;

        /**
         * Synchronous byte input stream reader from given file path using the given StreamConsumerFunc consumer_fn.
         *
         * To abort streaming, user may return `false` from the given `consumer_func`.
         *
         * @param input_file input file name path, `-` denotes std::stdin.
         * @param exp_size if > 0 it is additionally used to determine EOF, otherwise the underlying EOF mechanism is being used only.
         * @param buffer secure std::vector buffer, passed down to consumer_fn
         * @param consumer_fn StreamConsumerFunc consumer for each received heap of bytes, returning true to continue stream of false to abort.
         * @return total bytes read or 0 if error
         */
        uint64_t read_file(const std::string& input_file, const uint64_t exp_size,
                           secure_vector<uint8_t>& buffer,
                           StreamConsumerFunc consumer_fn) noexcept;

        /**
         * Synchronous byte input stream reader using the given StreamConsumerFunc consumer_fn.
         *
         * To abort streaming, user may return `false` from the given `consumer_func`.
         *
         * @param in the input byte stream to read from
         * @param exp_size if > 0 it is additionally used to determine EOF, otherwise the underlying EOF mechanism is being used only.
         * @param buffer secure std::vector buffer, passed down to consumer_fn
         * @param consumer_fn StreamConsumerFunc consumer for each received heap of bytes, returning true to continue stream of false to abort.
         * @return total bytes read or 0 if error
         */
        uint64_t read_stream(ByteInStream& in, const uint64_t exp_size,
                             secure_vector<uint8_t>& buffer,
                             StreamConsumerFunc consumer_fn) noexcept;

        /**
         * Synchronous URL stream reader using the given StreamConsumerFunc consumer_fn.
         *
         * To abort streaming, user may return `false` from the given `consumer_func`.
         *
         * @param url the URL to open a connection to and stream bytes from
         * @param exp_size if > 0 it is additionally used to determine EOF, otherwise the underlying EOF mechanism is being used only.
         * @param buffer secure std::vector buffer, passed down to consumer_fn
         * @param consumer_fn StreamConsumerFunc consumer for each received heap of bytes, returning true to continue stream of false to abort.
         * @return total bytes read or 0 if error
         */
        uint64_t read_url_stream(const std::string& url, const uint64_t exp_size,
                                 secure_vector<uint8_t>& buffer,
                                 StreamConsumerFunc consumer_fn) noexcept;

        /**
         * Asynchronous URL read content using the given byte jau::ringbuffer, allowing parallel reading.
         *
         * To abort streaming, user may set given reference `results` to a value other than async_io_result_t::NONE.
         *
         * @param url the URL to open a connection to and stream bytes from
         * @param exp_size if > 0 it is additionally used to determine EOF, otherwise the underlying EOF mechanism is being used only.
         * @param buffer the ringbuffer destination to write into
         * @param has_content_length indicating whether content_length is known from server
         * @param content_length tracking the content_length
         * @param total_read tracking the total_read
         * @param result reference to tracking async_io_result_t. If set to other than async_io_result_t::NONE while streaming, streaming is aborted.
         * @return the url background reading thread
         */
        std::thread read_url_stream(const std::string& url, const uint64_t& exp_size,
                                    ByteRingbuffer& buffer,
                                    jau::relaxed_atomic_bool& has_content_length,
                                    jau::relaxed_atomic_uint64& content_length,
                                    jau::relaxed_atomic_uint64& total_read,
                                    relaxed_atomic_async_io_result_t& result) noexcept;

        void print_stats(const std::string& prefix, const uint64_t& out_bytes_total, const jau::fraction_i64& td) noexcept;

        /**@}*/

} // namespace elevator::io

#endif /* JAU_IO_UTIL_HPP_ */
