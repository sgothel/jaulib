/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 *
 * ByteInStream, ByteInStream_SecMemory and ByteInStream_istream are derived from Botan under same license:
 * - Copyright (c) 1999-2007 Jack Lloyd
 * - Copyright (c) 2005 Matthew Gregan
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

#ifndef JAU_BYTE_STREAM_HPP_
#define JAU_BYTE_STREAM_HPP_

#include <fstream>
#include <string>
#include <cstdint>
#include <functional>
#include <thread>

#include <jau/basic_types.hpp>
#include <jau/ringbuffer.hpp>

// Include Botan header files before this one to be integrated w/ Botan!
// #include <botan_all.h>

#include <jau/io_util.hpp>

using namespace jau::fractions_i64_literals;

namespace jau::io {

    /** \addtogroup IOUtils
     *
     *  @{
     */

#ifdef BOTAN_VERSION_MAJOR
    #define VIRTUAL_BOTAN
    #define OVERRIDE_BOTAN override
    #define NOEXCEPT_BOTAN
#else
    #define VIRTUAL_BOTAN virtual
    #define OVERRIDE_BOTAN
    #define NOEXCEPT_BOTAN noexcept
#endif

    /**
     * This class represents an abstract byte input stream object.
     *
     * @anchor byte_in_stream_properties
     * ### ByteInStream Properties
     * The byte input stream can originate from a local source w/o delay,
     * remote URL like http connection or even from another thread feeding the input buffer.<br />
     * Both latter asynchronous resources may expose blocking properties
     * in check_available().
     *
     * Asynchronous resources benefit from knowing their content size,
     * as their check_available() implementation may avoid
     * blocking and waiting for requested bytes available
     * if the stream is already beyond its scope.
     *
     * All method implementations are of `noexcept`
     * and declared as such if used standalone w/o Botan.<br/>
     * However, if using Botan, the `noexcept` qualifier is dropped
     * for compatibility with Botan::DataSource virtual function table.
     *
     * One may use error() to detect whether an error has occurred,
     * while end_of_data() not only covered the EOS case but includes error().
     *
     * @see @ref byte_in_stream_properties "ByteInStream Properties"
     */
    class ByteInStream
#ifdef BOTAN_VERSION_MAJOR
    : public Botan::DataSource
#endif
    {
        public:
            ByteInStream() = default;
            virtual ~ByteInStream() NOEXCEPT_BOTAN = default;
            ByteInStream& operator=(const ByteInStream&) = delete;
            ByteInStream(const ByteInStream&) = delete;

            /**
             * Close the stream if supported by the underlying mechanism.
             */
            virtual void close() noexcept = 0;

            /**
             * Check whether n bytes are available in the input stream.
             *
             * This method may be blocking when using an asynchronous source
             * up until the requested bytes are actually available.
             *
             * A subsequent call to read() shall return immediately with at least
             * the requested numbers of bytes available.
             *
             * @param n byte count to wait for
             * @return true if n bytes are available, otherwise false
             *
             * @see read()
             * @see @ref byte_in_stream_properties "ByteInStream Properties"
             */
            VIRTUAL_BOTAN bool check_available(size_t n) NOEXCEPT_BOTAN OVERRIDE_BOTAN = 0;

            /**
             * Read from the source. Moves the internal offset so that every
             * call to read will return a new portion of the source.
             *
             * Use check_available() to wait and ensure a certain amount of bytes are available.
             *
             * This method is not blocking.
             *
             * @param out the byte array to write the result to
             * @param length the length of the byte array out
             * @return length in bytes that was actually read and put into out
             *
             * @see check_available()
             * @see @ref byte_in_stream_properties "ByteInStream Properties"
             */
            [[nodiscard]] VIRTUAL_BOTAN size_t read(uint8_t out[], size_t length) NOEXCEPT_BOTAN OVERRIDE_BOTAN = 0;

            /**
             * Read from the source but do not modify the internal
             * offset. Consecutive calls to peek() will return portions of
             * the source starting at the same position.
             *
             * @param out the byte array to write the output to
             * @param length the length of the byte array out
             * @param peek_offset the offset into the stream to read at
             * @return length in bytes that was actually read and put into out
             */
            [[nodiscard]] VIRTUAL_BOTAN size_t peek(uint8_t out[], size_t length, size_t peek_offset) const NOEXCEPT_BOTAN OVERRIDE_BOTAN = 0;

            /**
             * Test whether the source still has data that can be read.
             *
             * This may include a failure and/or error in the underlying implementation, see error()
             *
             * @return true if there is no more data to read, false otherwise
             * @see error()
             */
            VIRTUAL_BOTAN bool end_of_data() const NOEXCEPT_BOTAN OVERRIDE_BOTAN = 0;

            /**
             * Return whether an error has occurred, excluding end_of_data().
             *
             * @return true if an error has occurred, false otherwise
             */
            virtual bool error() const noexcept = 0;

#ifndef BOTAN_VERSION_MAJOR

            /**
             * return the id of this data source
             * @return std::string representing the id of this data source
             */
            virtual std::string id() const noexcept { return ""; }

            /**
             * Read one byte.
             * @param out the byte to read to
             * @return length in bytes that was actually read and put
             * into out
             */
            size_t read_byte(uint8_t& out) noexcept;

            /**
             * Peek at one byte.
             * @param out an output byte
             * @return length in bytes that was actually read and put
             * into out
             */
            size_t peek_byte(uint8_t& out) const noexcept;

            /**
             * Discard the next N bytes of the data
             * @param N the number of bytes to discard
             * @return number of bytes actually discarded
             */
            size_t discard_next(size_t N) noexcept;

#endif /* BOTAN_VERSION_MAJOR */

            /**
             * @return number of bytes read so far.
             */
            VIRTUAL_BOTAN size_t get_bytes_read() const NOEXCEPT_BOTAN OVERRIDE_BOTAN = 0;

            /**
             * Variant of Botan's get_byte_read() using uint64_t for big files on a 32-bit platform.
             * @return number of bytes read so far.
             */
            virtual uint64_t bytes_read() const noexcept = 0;

            /**
             * Returns true if implementation is aware of content_size(), otherwise false.
             * @see content_size()
             */
            virtual bool has_content_size() const noexcept = 0;

            /**
             * Returns the content_size if known.
             * @see has_content_size()
             */
            virtual uint64_t content_size() const noexcept = 0;

            virtual std::string to_string() const noexcept = 0;
    };

    /**
     * This class represents a secure Memory-Based byte input stream
     */
    class ByteInStream_SecMemory final : public ByteInStream {
       public:
          size_t read(uint8_t[], size_t) NOEXCEPT_BOTAN override;

          size_t peek(uint8_t[], size_t, size_t) const NOEXCEPT_BOTAN override;

          bool check_available(size_t n) NOEXCEPT_BOTAN override;

          bool end_of_data() const NOEXCEPT_BOTAN override;

          bool error() const noexcept override { return false; }

          /**
          * Construct a secure memory source that reads from a string
          * @param in the string to read from
          */
          explicit ByteInStream_SecMemory(const std::string& in);

          /**
          * Construct a secure memory source that reads from a byte array
          * @param in the byte array to read from
          * @param length the length of the byte array
          */
          ByteInStream_SecMemory(const uint8_t in[], size_t length)
          : m_source(in, in + length), m_offset(0) {}

          /**
          * Construct a secure memory source that reads from a secure_vector
          * @param in the MemoryRegion to read from
          */
          explicit ByteInStream_SecMemory(const io::secure_vector<uint8_t>& in)
          : m_source(in), m_offset(0) {}

          /**
          * Construct a secure memory source that reads from a std::vector
          * @param in the MemoryRegion to read from
          */
          explicit ByteInStream_SecMemory(const std::vector<uint8_t>& in)
          : m_source(in.begin(), in.end()), m_offset(0) {}

          void close() noexcept override;

          ~ByteInStream_SecMemory() NOEXCEPT_BOTAN override { close(); }

          size_t get_bytes_read() const NOEXCEPT_BOTAN override { return m_offset; }

          uint64_t bytes_read() const noexcept override { return m_offset; }

          bool has_content_size() const noexcept override { return true; }

          uint64_t content_size() const noexcept override { return m_source.size(); }

          std::string to_string() const noexcept override;

       private:
          io::secure_vector<uint8_t> m_source;
          size_t m_offset;
    };

    /**
     * This class represents a std::istream based byte input stream.
     */
    class ByteInStream_istream final : public ByteInStream {
       public:
          size_t read(uint8_t[], size_t) NOEXCEPT_BOTAN override;
          size_t peek(uint8_t[], size_t, size_t) const NOEXCEPT_BOTAN override;
          bool check_available(size_t n) NOEXCEPT_BOTAN override;
          bool end_of_data() const NOEXCEPT_BOTAN override;
          bool error() const noexcept override { return m_source.bad(); }
          std::string id() const NOEXCEPT_BOTAN override;

          ByteInStream_istream(std::istream&, const std::string& id = "<std::istream>") noexcept;

          ByteInStream_istream(const ByteInStream_istream&) = delete;

          ByteInStream_istream& operator=(const ByteInStream_istream&) = delete;

          void close() noexcept override;

          ~ByteInStream_istream() NOEXCEPT_BOTAN override { close(); }

          size_t get_bytes_read() const NOEXCEPT_BOTAN override { return m_bytes_consumed; }

          uint64_t bytes_read() const noexcept override { return m_bytes_consumed; }

          bool has_content_size() const noexcept override { return false; }

          uint64_t content_size() const noexcept override { return 0; }

          std::string to_string() const noexcept override;

       private:
          const std::string m_identifier;

          std::istream& m_source;
          size_t m_bytes_consumed;
    };


    /**
     * This class represents a file based byte input stream, including named file descriptor.
     *
     * If source path denotes a named file descriptor, i.e. jau::fs::file_stats::is_fd() returns true,
     * has_content_size() returns false and check_available() returns true as long the stream is open and EOS hasn't occurred.
     */
    class ByteInStream_File final : public ByteInStream {
       public:
          size_t read(uint8_t[], size_t) NOEXCEPT_BOTAN override;
          size_t peek(uint8_t[], size_t, size_t) const NOEXCEPT_BOTAN override;
          bool check_available(size_t n) NOEXCEPT_BOTAN override;
          bool end_of_data() const NOEXCEPT_BOTAN override;
          bool error() const noexcept override { return nullptr == m_source || m_source->bad(); }
          std::string id() const NOEXCEPT_BOTAN override;

          /**
           * Construct a Stream-Based byte input stream from filesystem path
           *
           * In case the given path is a local file URI starting with `file://`, see jau::io::uri::is_local_file_protocol(),
           * the leading `file://` is cut off and the remainder being used.
           *
           * @param path the path to the file, maybe a local file URI
           * @param use_binary whether to treat the file as binary (default) or use platform character conversion
           */
          ByteInStream_File(const std::string& path, bool use_binary = true) noexcept;

          ByteInStream_File(const ByteInStream_File&) = delete;

          ByteInStream_File& operator=(const ByteInStream_File&) = delete;

          void close() noexcept override;

          ~ByteInStream_File() NOEXCEPT_BOTAN override { close(); }

          size_t get_bytes_read() const NOEXCEPT_BOTAN override { return (size_t)m_bytes_consumed; }

          uint64_t bytes_read() const noexcept override { return m_bytes_consumed; }

          bool has_content_size() const noexcept override { return m_has_content_length; }

          uint64_t content_size() const noexcept override { return m_content_size; }

          std::string to_string() const noexcept override;

       private:
          uint64_t get_available() const noexcept { return m_has_content_length ? m_content_size - m_bytes_consumed : 0; }
          const std::string m_identifier;
          mutable std::unique_ptr<std::ifstream> m_source;
          bool m_has_content_length;
          uint64_t m_content_size;
          uint64_t m_bytes_consumed;
    };

    /**
     * This class represents a Ringbuffer-Based byte input stream with a URL connection provisioned data feed.
     *
     * Standard implementation uses [curl](https://curl.se/),
     * hence all [*libcurl* network protocols](https://curl.se/docs/url-syntax.html) are supported,
     * jau::io::uri::supported_protocols().
     */
    class ByteInStream_URL final : public ByteInStream {
        public:
            /**
             * Check whether n bytes are available in the input stream.
             *
             * Wait up to timeout duration given in constructor until n bytes become available, where fractions_i64::zero waits infinitely.
             *
             * This method is blocking.
             *
             * @param n byte count to wait for
             * @return true if n bytes are available, otherwise false
             *
             * @see read()
             * @see @ref byte_in_stream_properties "ByteInStream Properties"
             */
            bool check_available(size_t n) NOEXCEPT_BOTAN override;

            /**
             * Read from the source. Moves the internal offset so that every
             * call to read will return a new portion of the source.
             *
             * Use check_available() to wait and ensure a certain amount of bytes are available.
             *
             * This method is not blocking.
             *
             * @param out the byte array to write the result to
             * @param length the length of the byte array out
             * @return length in bytes that was actually read and put into out
             *
             * @see check_available()
             * @see @ref byte_in_stream_properties "ByteInStream Properties"
             */
            size_t read(uint8_t out[], size_t length) NOEXCEPT_BOTAN override;

            size_t peek(uint8_t out[], size_t length, size_t peek_offset) const NOEXCEPT_BOTAN override;

            bool end_of_data() const NOEXCEPT_BOTAN override;

            bool error() const noexcept override { return async_io_result_t::FAILED == m_result; }

            std::string id() const NOEXCEPT_BOTAN override { return m_url; }

            /**
             * Construct a ringbuffer backed Http byte input stream
             * @param url the URL of the data to read
             * @param timeout maximum duration in fractions of seconds to wait @ check_available() for next bytes, where fractions_i64::zero waits infinitely
             */
            ByteInStream_URL(const std::string& url, const jau::fraction_i64& timeout) noexcept;

            ByteInStream_URL(const ByteInStream_URL&) = delete;

            ByteInStream_URL& operator=(const ByteInStream_URL&) = delete;

            void close() noexcept override;

            ~ByteInStream_URL() NOEXCEPT_BOTAN override { close(); }

            size_t get_bytes_read() const NOEXCEPT_BOTAN override { return (size_t)m_bytes_consumed; }

            uint64_t bytes_read() const noexcept override { return m_bytes_consumed; }

            bool has_content_size() const noexcept override { return m_has_content_length; }

            uint64_t content_size() const noexcept override { return m_content_size; }

            std::string to_string() const noexcept override;

        private:
            uint64_t get_available() const noexcept { return m_has_content_length ? m_content_size - m_bytes_consumed : 0; }
            std::string to_string_int() const noexcept;

            const std::string m_url;
            jau::fraction_i64 m_timeout;
            ByteRingbuffer m_buffer;
            jau::relaxed_atomic_bool m_has_content_length;
            jau::relaxed_atomic_uint64 m_content_size;
            jau::relaxed_atomic_uint64 m_total_xfered;
            relaxed_atomic_async_io_result_t m_result;
            std::unique_ptr<std::thread> m_url_thread;
            uint64_t m_bytes_consumed;
    };

    /**
     * Parses the given path_or_uri, if it matches a supported protocol, see jau::io::uri::protocol_supported(),
     * but is not a local file, see jau::io::uri::is_local_file_protocol(), ByteInStream_URL is being attempted.
     *
     * If the above fails, ByteInStream_File is attempted.
     *
     * If non of the above leads to a ByteInStream without ByteInStream::error(), nullptr is returned.
     *
     * @param path_or_uri given path or uri for with a ByteInStream instance shall be established.
     * @param timeout in case `path_or_uri` resolves to ByteInStream_URL, timeout is being used as maximum duration to wait for next bytes at ByteInStream_URL::check_available(), defaults to 20_s
     * @return a working ByteInStream w/o ByteInStream::error() or nullptr
     */
    std::unique_ptr<ByteInStream> to_ByteInStream(const std::string& path_or_uri, jau::fraction_i64 timeout=20_s) noexcept;

    /**
     * This class represents a Ringbuffer-Based byte input stream with an externally provisioned data feed.
     */
    class ByteInStream_Feed final : public ByteInStream {
        public:
            /**
             * Check whether n bytes are available in the input stream.
             *
             * Wait up to timeout duration given in constructor until n bytes become available, where fractions_i64::zero waits infinitely.
             *
             * This method is blocking.
             *
             * @param n byte count to wait for
             * @return true if n bytes are available, otherwise false
             *
             * @see read()
             * @see @ref byte_in_stream_properties "ByteInStream Properties"
             */
            bool check_available(size_t n) NOEXCEPT_BOTAN override;

            /**
             * Read from the source. Moves the internal offset so that every
             * call to read will return a new portion of the source.
             *
             * Use check_available() to wait and ensure a certain amount of bytes are available.
             *
             * This method is not blocking.
             *
             * @param out the byte array to write the result to
             * @param length the length of the byte array out
             * @return length in bytes that was actually read and put into out
             *
             * @see check_available()
             * @see @ref byte_in_stream_properties "ByteInStream Properties"
             */
            size_t read(uint8_t out[], size_t length) NOEXCEPT_BOTAN override;

            size_t peek(uint8_t out[], size_t length, size_t peek_offset) const NOEXCEPT_BOTAN override;

            bool end_of_data() const NOEXCEPT_BOTAN override;

            bool error() const noexcept override { return async_io_result_t::FAILED == m_result; }

            std::string id() const NOEXCEPT_BOTAN override { return m_id; }

            /**
             * Construct a ringbuffer backed externally provisioned byte input stream
             * @param id_name arbitrary identifier for this instance
             * @param timeout maximum duration in fractions of seconds to wait @ check_available() and write(), where fractions_i64::zero waits infinitely
             */
            ByteInStream_Feed(const std::string& id_name, const jau::fraction_i64& timeout) noexcept;

            ByteInStream_Feed(const ByteInStream_Feed&) = delete;

            ByteInStream_Feed& operator=(const ByteInStream_Feed&) = delete;

            void close() noexcept override;

            ~ByteInStream_Feed() NOEXCEPT_BOTAN override { close(); }

            size_t get_bytes_read() const NOEXCEPT_BOTAN override { return m_bytes_consumed; }

            uint64_t bytes_read() const noexcept override { return m_bytes_consumed; }

            bool has_content_size() const noexcept override { return m_has_content_length; }

            uint64_t content_size() const noexcept override { return m_content_size; }

            /**
             * Interrupt a potentially blocked reader.
             *
             * Call this method if intended to abort streaming and to interrupt the reader thread's potentially blocked check_available() call,
             * i.e. done at set_eof()
             *
             * @see set_eof()
             */
            void interruptReader() noexcept {
                m_buffer.interruptReader();
            }

            /**
             * Write given bytes to the async ringbuffer.
             *
             * Wait up to timeout duration given in constructor until ringbuffer space is available, where fractions_i64::zero waits infinitely.
             *
             * This method is blocking.
             *
             * @param n byte count to wait for
             * @param in the byte array to transfer to the async ringbuffer
             * @param length the length of the byte array in
             */
            void write(uint8_t in[], size_t length) noexcept;

            /**
             * Set known content size, informal only.
             * @param content_length the content size in bytes
             */
            void set_content_size(const uint64_t size) noexcept {
                m_content_size = size;
                m_has_content_length = true;
            }

            /**
             * Set end-of-data (EOS), i.e. when feeder completed provisioning bytes.
             *
             * Implementation issues interruptReader() to unblock a potentially blocked reader thread.
             *
             * @param result should be either result_t::FAILED or result_t::SUCCESS.
             *
             * @see interruptReader()
             */
            void set_eof(const async_io_result_t result) noexcept { m_result = result; interruptReader(); }

            std::string to_string() const noexcept override;

        private:
            uint64_t get_available() const noexcept { return m_has_content_length ? m_content_size - m_bytes_consumed : 0; }
            std::string to_string_int() const noexcept;

            const std::string m_id;
            jau::fraction_i64 m_timeout;
            ByteRingbuffer m_buffer;
            jau::relaxed_atomic_bool m_has_content_length;
            jau::relaxed_atomic_uint64 m_content_size;
            jau::relaxed_atomic_uint64 m_total_xfered;
            relaxed_atomic_async_io_result_t m_result;
            uint64_t m_bytes_consumed;
    };

    /**
     * This class represents a wrapped byte input stream with the capability
     * to record the byte stream read out at will.
     *
     * Peek'ed bytes won't be recorded, only read bytes.
     */
    class ByteInStream_Recorder final : public ByteInStream {
        public:
            size_t read(uint8_t[], size_t) NOEXCEPT_BOTAN override;

            size_t peek(uint8_t out[], size_t length, size_t peek_offset) const NOEXCEPT_BOTAN override {
                return m_parent.peek(out, length, peek_offset);
            }

            bool check_available(size_t n) NOEXCEPT_BOTAN override {
                return m_parent.check_available(n);
            }

            bool end_of_data() const NOEXCEPT_BOTAN override { return m_parent.end_of_data(); }

            bool error() const noexcept override { return m_parent.error(); }

            std::string id() const NOEXCEPT_BOTAN override { return m_parent.id(); }

            /**
             * Construct a byte input stream wrapper using the given parent ByteInStream.
             * @param parent the parent ByteInStream
             * @param buffer a user defined buffer for the recording
             */
            ByteInStream_Recorder(ByteInStream& parent, io::secure_vector<uint8_t>& buffer) noexcept
            : m_parent(parent), m_bytes_consumed(0), m_buffer(buffer), m_rec_offset(0), m_is_recording(false) {};

            ByteInStream_Recorder(const ByteInStream_Recorder&) = delete;

            ByteInStream_Recorder& operator=(const ByteInStream_Recorder&) = delete;

            void close() noexcept override;

            ~ByteInStream_Recorder() NOEXCEPT_BOTAN override { close(); }

            size_t get_bytes_read() const NOEXCEPT_BOTAN override { return m_parent.get_bytes_read(); }

            uint64_t bytes_read() const noexcept override { return m_bytes_consumed; }

            bool has_content_size() const noexcept override { return m_parent.has_content_size(); }

            uint64_t content_size() const noexcept override { return m_parent.content_size(); }

            /**
             * Starts the recording.
             * <p>
             * A potential previous recording will be cleared.
             * </p>
             */
            void start_recording() noexcept;

            /**
             * Stops the recording.
             * <p>
             * The recording persists.
             * </p>
             */
            void stop_recording() noexcept;

            /**
             * Clears the recording.
             * <p>
             * If the recording was ongoing, also stops the recording.
             * </p>
             */
            void clear_recording() noexcept;

            /** Returns the reference of the recording buffer given by user. */
            io::secure_vector<uint8_t>& get_recording() noexcept { return m_buffer; }

            size_t get_bytes_recorded() noexcept { return m_buffer.size(); }

            /** Returns the recording start position. */
            uint64_t get_recording_start_pos() noexcept { return m_rec_offset; }

            bool is_recording() noexcept { return m_is_recording; }

            std::string to_string() const noexcept override;

        private:
            ByteInStream& m_parent;
            uint64_t m_bytes_consumed;
            io::secure_vector<uint8_t>& m_buffer;
            uint64_t m_rec_offset;
            bool m_is_recording;
    };

    /**@}*/

} // namespace elevator::io

#endif /* JAU_BYTE_STREAM_HPP_ */
