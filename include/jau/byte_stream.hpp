/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021-2023 Gothel Software e.K.
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

#include <string>
#include <cstdint>
#include <thread>

#include <jau/basic_types.hpp>
#include <jau/ringbuffer.hpp>
#include <jau/file_util.hpp>
#include <jau/enum_util.hpp>

#include <jau/io_util.hpp>

using namespace jau::fractions_i64_literals;

namespace jau::io {

    using namespace jau::enums;

    /** \addtogroup IOUtils
     *
     *  @{
     */

    /*
     * Mimic std::ios_base::iostate for state functionality, see iostate_func.
     *
     * This `enum class` type fulfills `C++ named requirements: BitmaskType`.
     */
    enum class iostate : uint32_t {
      /** No error occurred nor has EOS being reached. Value is no bit set! */
      none = 0,

      /** No error occurred nor has EOS being reached. Value is no bit set! */
      goodbit = 0,

      /** Irrecoverable stream error, including loss of integrity of the underlying stream or media. */
      badbit  = 1U << 0,

      /** An input operation reached the end of its stream. */
      eofbit  = 1U << 1,

      /** Input or output operation failed (formatting or extraction error). */
      failbit = 1U << 2,

      /** Input or output operation failed due to timeout. */
      timeout = 1U << 3
    };
    JAU_MAKE_BITFIELD_ENUM_STRING(iostate, goodbit, badbit, eofbit, failbit, timeout);

    /**
     * Supporting std::basic_ios's iostate functionality for all ByteInStream implementations.
     */
    class iostate_func {
        private:
            iostate m_state;

        protected:
            constexpr iostate rdstate_impl() const noexcept { return m_state; }
            constexpr void setstate_impl(iostate state) const noexcept { const_cast<iostate_func*>(this)->m_state |= state; }

        public:
            iostate_func() noexcept
            : m_state( iostate::goodbit ) {}

            iostate_func(const iostate_func& o) noexcept = default;
            iostate_func(iostate_func&& o) noexcept = default;
            iostate_func& operator=(const iostate_func &o) noexcept = default;
            iostate_func& operator=(iostate_func &&o) noexcept = default;

            virtual ~iostate_func() noexcept = default;

            /** Clears state flags by assignment to the given value. */
            virtual void clear(const iostate state = iostate::goodbit) noexcept { m_state = state; }

            /**
             * Returns the current state flags.
             *
             * Method is marked `virtual` to allow implementations with asynchronous resources
             * to determine or update the current iostate.
             *
             * Method is used throughout all query members and setstate(),
             * hence they all will use the updated state from a potential override implementation.
             */
            virtual iostate rdstate() const noexcept { return m_state; }

            /** Sets state flags, by keeping its previous bits. */
            void setstate(const iostate state) noexcept { clear( rdstate() | state ); }

            /** Checks if no error nor eof() has occurred i.e. I/O operations are available. */
            bool good() const noexcept
            { return iostate::goodbit == rdstate(); }

            /** Checks if end-of-file has been reached. */
            bool eof() const noexcept
            { return iostate::none != ( rdstate() & iostate::eofbit ); }

            /** Checks if an error has occurred. */
            bool fail() const noexcept
            { return iostate::none != ( rdstate() & ( iostate::badbit | iostate::failbit | iostate::timeout) ); }

            /** Checks if an error has occurred, synonym of fail(). */
            bool operator!() const noexcept { return fail(); }

            /** Checks if no error has occurred, synonym of !fail(). */
            explicit operator bool() const noexcept { return !fail(); }

            /** Checks if a non-recoverable error has occurred. */
            bool bad() const noexcept
            { return iostate::none != ( rdstate() & iostate::badbit ); }

            /** Checks if a timeout (non-recoverable) has occurred. */
            bool timeout() const noexcept
            { return iostate::none != ( rdstate() & iostate::timeout ); }
    };

    /**
     * Abstract byte input stream object.
     *
     * @anchor byte_in_stream_properties
     * ### ByteInStream Properties
     * The byte input stream can originate from a local source w/o delay,
     * remote URL like http connection or even from another thread feeding the input buffer.<br />
     * Both latter asynchronous resources may expose blocking properties
     * in available().
     *
     * Asynchronous resources benefit from knowing their content size,
     * as their available() implementation may avoid
     * blocking and waiting for requested bytes available
     * if the stream is already beyond its scope.
     *
     * All method implementations are of `noexcept`.
     *
     * One may use fail() to detect whether an error has occurred,
     * while end_of_data() not only covers the end-of-stream (EOS) case but includes fail().
     *
     * @see @ref byte_in_stream_properties "ByteInStream Properties"
     */
    class ByteInStream : public iostate_func
    {
        public:
            ByteInStream() noexcept
            : iostate_func() {}

            ~ByteInStream() noexcept override = default;
            ByteInStream& operator=(const ByteInStream&) = delete;
            ByteInStream(const ByteInStream&) = delete;

            /** Checks if the stream has an associated file. */
            virtual bool is_open() const noexcept = 0;

            /**
             * Close the stream if supported by the underlying mechanism.
             */
            virtual void close() noexcept = 0;

            /**
             * Return whether n bytes are available in the input stream,
             * if has_content_size() or using an asynchronous source.
             *
             * If !has_content_size() and not being an asynchronous source,
             * !end_of_data() is returned.
             *
             * Method may be blocking when using an asynchronous source
             * up until the requested bytes are available.
             *
             * A subsequent call to read() shall return immediately with at least
             * the requested numbers of bytes available,
             * if has_content_size() or using an asynchronous source.
             *
             * See details of the implementing class.
             *
             * @param n byte count to wait for
             * @return true if n bytes are available, otherwise false
             *
             * @see has_content_size()
             * @see read()
             * @see @ref byte_in_stream_properties "ByteInStream Properties"
             */
            virtual bool available(size_t n) noexcept  = 0;

            /**
             * Read from the source. Moves the internal offset so that every
             * call to read will return a new portion of the source.
             *
             * Use available() to try to wait for a certain amount of bytes available.
             *
             * This method shall only block until `min(available, length)` bytes are transfered.
             *
             * See details of the implementing class.
             *
             * @param out the byte array to write the result to
             * @param length the length of the byte array out
             * @return length in bytes that was actually read and put into out
             *
             * @see available()
             * @see @ref byte_in_stream_properties "ByteInStream Properties"
             */
            [[nodiscard]] virtual size_t read(void* out, size_t length) noexcept = 0;

            /**
             * Read one byte.
             * @param out the byte to read to
             * @return true if one byte has been read, false otherwise
             */
            [[nodiscard]] bool read(uint8_t& out) noexcept;

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
            [[nodiscard]] virtual size_t peek(void* out, size_t length, size_t peek_offset) noexcept = 0;

            /**
             * Peek at one byte.
             * @param out an output byte
             * @return true if one byte has been peeked, false otherwise
             */
            [[nodiscard]] bool peek(uint8_t& out) noexcept;

            /**
             * Discard the next N bytes of the data
             * @param N the number of bytes to discard
             * @return number of bytes actually discarded
             */
            [[nodiscard]] size_t discard(size_t N) noexcept;

            /**
             * return the id of this data source
             * @return std::string representing the id of this data source
             */
            virtual std::string id() const noexcept { return ""; }

            /**
             * Returns the input position indicator, similar to std::basic_istream.
             *
             * @return number of bytes read so far.
             */
            virtual uint64_t tellg() const noexcept = 0;

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
     * Secure Memory-Based byte input stream
     */
    class ByteInStream_SecMemory final : public ByteInStream {
       public:
            [[nodiscard]] size_t read(void*, size_t) noexcept override;

            [[nodiscard]] size_t peek(void*, size_t, size_t) noexcept override;

          bool available(size_t n) noexcept override;

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
          explicit ByteInStream_SecMemory(io::secure_vector<uint8_t>  in)
          : m_source(std::move(in)), m_offset(0) {}

          /**
          * Construct a secure memory source that reads from a std::vector
          * @param in the MemoryRegion to read from
          */
          explicit ByteInStream_SecMemory(const std::vector<uint8_t>& in)
          : m_source(in.begin(), in.end()), m_offset(0) {}

          bool is_open() const noexcept override { return true; }

          void close() noexcept override;

          ~ByteInStream_SecMemory() noexcept override { close(); }

          uint64_t tellg() const noexcept override { return m_offset; }

          bool has_content_size() const noexcept override { return true; }

          uint64_t content_size() const noexcept override { return m_source.size(); }

          std::string to_string() const noexcept override;

       private:
          io::secure_vector<uint8_t> m_source;
          size_t m_offset;
    };

    /**
     * File based byte input stream, including named file descriptor.
     *
     * Implementation mimics std::ifstream via OS level file descriptor (FD) operations,
     * giving more flexibility, allowing reusing existing FD and enabling openat() operations.
     *
     * If source path denotes a named file descriptor, i.e. jau::fs::file_stats::is_fd() returns true,
     * has_content_size() returns false and available() returns true as long the stream is open and EOS hasn't occurred.
     */
    class ByteInStream_File final : public ByteInStream {
       private:
          jau::fs::file_stats stats;

          int m_fd;

          bool m_has_content_length;
          uint64_t m_content_size;
          uint64_t m_bytes_consumed;
          uint64_t get_available() const noexcept { return m_has_content_length ? m_content_size - m_bytes_consumed : 0; }

       public:
          [[nodiscard]] size_t read(void*, size_t) noexcept override;
          [[nodiscard]] size_t peek(void*, size_t, size_t) noexcept override;
          bool available(size_t n) noexcept override;

          bool is_open() const noexcept override { return 0 <= m_fd; }

          std::string id() const noexcept override { return stats.path(); }

          /**
           * Returns the file descriptor if is_open(), otherwise -1 for no file descriptor.
           *
           * @see is_open()
           */
          int fd() const noexcept { return m_fd; }

          /**
           * Construct a stream based byte input stream from filesystem path
           *
           * In case the given path is a local file URI starting with `file://`, see jau::io::uri::is_local_file_protocol(),
           * the leading `file://` is cut off and the remainder being used.
           *
           * @param path the path to the file, maybe a local file URI
           */
          ByteInStream_File(const std::string& path) noexcept;

          /**
           * Construct a stream based byte input stream from filesystem path and parent directory file descriptor
           *
           * In case the given path is a local file URI starting with `file://`, see jau::io::uri::is_local_file_protocol(),
           * the leading `file://` is cut off and the remainder being used.
           *
           * @param dirfd parent directory file descriptor
           * @param path the path to the file, maybe a local file URI
           */
          ByteInStream_File(const int dirfd, const std::string& path) noexcept;

          /**
           * Construct a stream based byte input stream by duplicating given file descriptor
           *
           * In case the given path is a local file URI starting with `file://`, see jau::io::uri::is_local_file_protocol(),
           * the leading `file://` is cut off and the remainder being used.
           *
           * @param fd file descriptor to duplicate leaving the given `fd` untouched
           */
          ByteInStream_File(const int fd) noexcept;

          ByteInStream_File(const ByteInStream_File&) = delete;

          ByteInStream_File& operator=(const ByteInStream_File&) = delete;

          void close() noexcept override;

          ~ByteInStream_File() noexcept override { close(); }

          uint64_t tellg() const noexcept override { return m_bytes_consumed; }

          bool has_content_size() const noexcept override { return m_has_content_length; }

          uint64_t content_size() const noexcept override { return m_content_size; }

          std::string to_string() const noexcept override;
    };

    /**
     * Ringbuffer-Based byte input stream with a URL connection provisioned data feed.
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
             * Wait up to timeout duration set in constructor until n bytes become available, where fractions_i64::zero waits infinitely.
             *
             * This method is blocking.
             *
             * @param n byte count to wait for
             * @return true if n bytes are available, otherwise false
             *
             * @see read()
             * @see @ref byte_in_stream_properties "ByteInStream Properties"
             */
            bool available(size_t n) noexcept override;

            /**
             * Read from the source. Moves the internal offset so that every
             * call to read will return a new portion of the source.
             *
             * Use available() to wait and ensure a certain amount of bytes are available.
             *
             * This method is not blocking beyond the transfer of `min(available, length)` bytes.
             *
             * @param out the byte array to write the result to
             * @param length the length of the byte array out
             * @return length in bytes that was actually read and put into out
             *
             * @see available()
             * @see @ref byte_in_stream_properties "ByteInStream Properties"
             */
            [[nodiscard]] size_t read(void* out, size_t length) noexcept override;

            [[nodiscard]] size_t peek(void* out, size_t length, size_t peek_offset) noexcept override;

            iostate rdstate() const noexcept override;

            std::string id() const noexcept override { return m_url; }

            /**
             * Construct a ringbuffer backed Http byte input stream
             * @param url the URL of the data to read
             * @param timeout maximum duration in fractions of seconds to wait @ available() for next bytes, where fractions_i64::zero waits infinitely
             */
            ByteInStream_URL(std::string url, const jau::fraction_i64& timeout) noexcept;

            ByteInStream_URL(const ByteInStream_URL&) = delete;

            ByteInStream_URL& operator=(const ByteInStream_URL&) = delete;

            bool is_open() const noexcept override;

            void close() noexcept override;

            ~ByteInStream_URL() noexcept override { close(); }

            uint64_t tellg() const noexcept override { return m_bytes_consumed; }

            bool has_content_size() const noexcept override { return m_stream_resp->has_content_length; }

            uint64_t content_size() const noexcept override { return m_stream_resp->content_length; }

            std::string to_string() const noexcept override;

        private:
            uint64_t get_available() const noexcept { return m_stream_resp->has_content_length ? m_stream_resp->content_length - m_bytes_consumed : 0; }
            std::string to_string_int() const noexcept;

            const std::string m_url;
            jau::fraction_i64 m_timeout;
            ByteRingbuffer m_buffer;
            jau::io::AsyncStreamResponseRef m_stream_resp;

            uint64_t m_bytes_consumed;
    };

    /**
     * Parses the given path_or_uri, if it matches a supported protocol, see jau::io::uri::protocol_supported(),
     * but is not a local file, see jau::io::uri::is_local_file_protocol(), ByteInStream_URL is being attempted.
     *
     * If the above fails, ByteInStream_File is attempted.
     *
     * If non of the above leads to a ByteInStream without ByteInStream::fail(), nullptr is returned.
     *
     * @param path_or_uri given path or uri for with a ByteInStream instance shall be established.
     * @param timeout in case `path_or_uri` resolves to ByteInStream_URL, timeout is being used as maximum duration to wait for next bytes at ByteInStream_URL::available(), defaults to 20_s
     * @return a working ByteInStream w/o ByteInStream::fail() or nullptr
     */
    std::unique_ptr<ByteInStream> to_ByteInStream(const std::string& path_or_uri, jau::fraction_i64 timeout=20_s) noexcept;

    /**
     * Ringbuffer-Based byte input stream with an externally provisioned data feed.
     */
    class ByteInStream_Feed final : public ByteInStream {
        public:
            /**
             * Check whether n bytes are available in the input stream.
             *
             * Wait up to timeout duration set in constructor until n bytes become available, where fractions_i64::zero waits infinitely.
             *
             * This method is blocking.
             *
             * @param n byte count to wait for
             * @return true if n bytes are available, otherwise false
             *
             * @see read()
             * @see @ref byte_in_stream_properties "ByteInStream Properties"
             */
            bool available(size_t n) noexcept override;

            /**
             * Read from the source. Moves the internal offset so that every
             * call to read will return a new portion of the source.
             *
             * Use available() to wait and ensure a certain amount of bytes are available.
             *
             * This method is not blocking beyond the transfer of `min(available, length)` bytes.
             *
             * @param out the byte array to write the result to
             * @param length the length of the byte array out
             * @return length in bytes that was actually read and put into out
             *
             * @see available()
             * @see @ref byte_in_stream_properties "ByteInStream Properties"
             */
            [[nodiscard]] size_t read(void* out, size_t length) noexcept override;

            [[nodiscard]] size_t peek(void* out, size_t length, size_t peek_offset) noexcept override;

            iostate rdstate() const noexcept override;

            std::string id() const noexcept override { return m_id; }

            /**
             * Construct a ringbuffer backed externally provisioned byte input stream
             * @param id_name arbitrary identifier for this instance
             * @param timeout maximum duration in fractions of seconds to wait @ available() and write(), where fractions_i64::zero waits infinitely
             */
            ByteInStream_Feed(std::string id_name, const jau::fraction_i64& timeout) noexcept;

            ByteInStream_Feed(const ByteInStream_Feed&) = delete;

            ByteInStream_Feed& operator=(const ByteInStream_Feed&) = delete;

            bool is_open() const noexcept override;

            void close() noexcept override;

            ~ByteInStream_Feed() noexcept override { close(); }

            uint64_t tellg() const noexcept override { return m_bytes_consumed; }

            bool has_content_size() const noexcept override { return m_has_content_length; }

            uint64_t content_size() const noexcept override { return m_content_size; }

            /**
             * Interrupt a potentially blocked reader.
             *
             * Call this method if intended to abort streaming and to interrupt the reader thread's potentially blocked available() call,
             * i.e. done at set_eof()
             *
             * @see set_eof()
             */
            void interruptReader() noexcept {
                m_buffer.interruptReader();
            }

            /**
             * Write given bytes to the async ringbuffer using explicit given timeout.
             *
             * Wait up to explicit given timeout duration until ringbuffer space is available, where fractions_i64::zero waits infinitely.
             *
             * This method is blocking.
             *
             * @param n byte count to wait for
             * @param in the byte array to transfer to the async ringbuffer
             * @param length the length of the byte array in
             * @param timeout explicit given timeout for async ringbuffer put operation
             * @return true if successful, otherwise false on timeout or stopped feeder and subsequent calls to good() will return false.
             */
            [[nodiscard]] bool write(uint8_t in[], size_t length, const jau::fraction_i64& timeout) noexcept;

            /**
             * Write given bytes to the async ringbuffer.
             *
             * Wait up to timeout duration set in constructor until ringbuffer space is available, where fractions_i64::zero waits infinitely.
             *
             * This method is blocking.
             *
             * @param n byte count to wait for
             * @param in the byte array to transfer to the async ringbuffer
             * @param length the length of the byte array in
             * @return true if successful, otherwise false on timeout or stopped feeder and subsequent calls to good() will return false.
             */
            [[nodiscard]] bool write(uint8_t in[], size_t length) noexcept {
                return write(in, length, m_timeout);
            }

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
            void set_eof(const io_result_t result) noexcept;

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
            relaxed_atomic_io_result_t m_result;
            uint64_t m_bytes_consumed;
    };

    /**
     * Wrapped byte input stream with the capability
     * to record the read byte stream at will.
     *
     * Peek'ed bytes won't be recorded, only read bytes.
     */
    class ByteInStream_Recorder final : public ByteInStream {
        public:
            [[nodiscard]] size_t read(void*, size_t) noexcept override;

            [[nodiscard]] size_t peek(void* out, size_t length, size_t peek_offset) noexcept override {
                return m_parent.peek(out, length, peek_offset);
            }

            bool available(size_t n) noexcept override {
                return m_parent.available(n);
            }

            void clear(const iostate state = iostate::goodbit) noexcept override { m_parent.clear( state ); }
            iostate rdstate() const noexcept override { return m_parent.rdstate(); }

            std::string id() const noexcept override { return m_parent.id(); }

            /**
             * Construct a byte input stream wrapper using the given parent ByteInStream.
             * @param parent the parent ByteInStream
             * @param buffer a user defined buffer for the recording
             */
            ByteInStream_Recorder(ByteInStream& parent, io::secure_vector<uint8_t>& buffer) noexcept
            : m_parent(parent), m_bytes_consumed(0), m_buffer(buffer), m_rec_offset(0), m_is_recording(false) {};

            ByteInStream_Recorder(const ByteInStream_Recorder&) = delete;

            ByteInStream_Recorder& operator=(const ByteInStream_Recorder&) = delete;

            bool is_open() const noexcept override { return m_parent.is_open(); }

            void close() noexcept override;

            ~ByteInStream_Recorder() noexcept override { close(); }

            uint64_t tellg() const noexcept override { return m_bytes_consumed; }

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

    /**
     * Abstract byte output stream object,
     * to write data to a sink.
     *
     * All method implementations are of `noexcept`.
     *
     * One may use fail() to detect whether an error has occurred.
     */
    class ByteOutStream : public iostate_func
    {
        public:
            ByteOutStream() = default;
            ~ByteOutStream() noexcept override = default;
            ByteOutStream& operator=(const ByteOutStream&) = delete;
            ByteOutStream(const ByteOutStream&) = delete;

            /** Checks if the stream has an associated file. */
            virtual bool is_open() const noexcept = 0;

            /**
             * Close the stream if supported by the underlying mechanism.
             */
            virtual void close() noexcept = 0;

            /**
             * Write to the data sink. Moves the internal offset so that every
             * call to write will be appended to the sink.
             *
             * This method is not blocking beyond the transfer length bytes.
             *
             * @param in the input bytes to write out
             * @param length the length of the byte array in
             * @return length in bytes that were actually written
             */
            [[nodiscard]] virtual size_t write(const void* in, size_t length) noexcept = 0;

            /**
             * Write one byte.
             * @param in the byte to be written
             * @return true if one byte has been written, otherwise false
             */
            [[nodiscard]] bool write(const uint8_t& in) noexcept;

            /**
             * return the id of this data source
             * @return std::string representing the id of this data source
             */
            virtual std::string id() const noexcept { return ""; }

            /**
             * Returns the output position indicator.
             *
             * @return number of bytes written so far.
             */
            virtual uint64_t tellp() const noexcept = 0;

            virtual std::string to_string() const noexcept = 0;
    };

    /**
     * File based byte output stream, including named file descriptor.
     */
    class ByteOutStream_File final : public ByteOutStream {
       private:
          jau::fs::file_stats stats;
          /**
           * We mimic std::ofstream via OS level file descriptor operations,
           * giving us more flexibility and enabling use of openat() operations.
           */
          int m_fd;

          // Remember: constexpr specifier used in a function or static data member (since C++17) declaration implies inline

       public:
          bool is_open() const noexcept override { return 0 <= m_fd; }

          [[nodiscard]] size_t write(const void*, size_t) noexcept override;

          std::string id() const noexcept override { return stats.path(); }

          /**
           * Returns the file descriptor if is_open(), otherwise -1 for no file descriptor.
           *
           * @see is_open()
           */
          int fd() const noexcept { return m_fd; }

          /**
           * Construct a stream based byte output stream from filesystem path,
           * either an existing or new file.
           *
           * In case the file already exists, the underlying file offset is positioned at the end of the file.
           *
           * In case the given path is a local file URI starting with `file://`, see jau::io::uri::is_local_file_protocol(),
           * the leading `file://` is cut off and the remainder being used.
           *
           * @param path the path to the file, maybe a local file URI
           * @param mode file protection mode for a new file, otherwise ignored.
           */
          ByteOutStream_File(const std::string& path, const jau::fs::fmode_t mode = jau::fs::fmode_t::def_file_prot) noexcept;

          /**
           * Construct a stream based byte output stream from filesystem path and parent directory file descriptor,
           * either an existing or new file.
           *
           * In case the file already exists, the underlying file offset is positioned at the end of the file.
           *
           * In case the given path is a local file URI starting with `file://`, see jau::io::uri::is_local_file_protocol(),
           * the leading `file://` is cut off and the remainder being used.
           *
           * @param dirfd parent directory file descriptor
           * @param path the path to the file, maybe a local file URI
           * @param mode file protection mode for a new file, otherwise ignored.
           */
          ByteOutStream_File(const int dirfd, const std::string& path, const jau::fs::fmode_t mode = jau::fs::fmode_t::def_file_prot) noexcept;

          /**
           * Construct a stream based byte output stream by duplicating given file descriptor
           *
           * In case the given path is a local file URI starting with `file://`, see jau::io::uri::is_local_file_protocol(),
           * the leading `file://` is cut off and the remainder being used.
           *
           * @param fd file descriptor to duplicate leaving the given `fd` untouched
           */
          ByteOutStream_File(const int fd) noexcept;

          ByteOutStream_File(const ByteOutStream_File&) = delete;

          ByteOutStream_File& operator=(const ByteOutStream_File&) = delete;

          void close() noexcept override;

          ~ByteOutStream_File() noexcept override { close(); }

          uint64_t tellp() const noexcept override { return m_bytes_consumed; }

          std::string to_string() const noexcept override;

       private:
          uint64_t m_bytes_consumed;
    };


    /**@}*/

} // namespace elevator::io

#endif /* JAU_BYTE_STREAM_HPP_ */
