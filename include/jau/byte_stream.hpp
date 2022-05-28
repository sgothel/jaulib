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
#include <jau/callocator_sec.hpp>
#include <jau/ringbuffer.hpp>

// Include Botan header files before this one to be integrated w/ Botan!
// #include <botan_all.h>

namespace jau::io {

    /** \addtogroup IOUtils
     *
     *  @{
     */

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
     * Asynchronous I/O operation result value
     */
    enum class async_io_result_t : int8_t {
        /** Operation failed. */
        FAILED  = -1,

        /** Operation still in progress. */
        NONE    =  0,

        /** Operation succeeded. */
        SUCCESS =  1
    };
    typedef jau::ordered_atomic<async_io_result_t, std::memory_order::memory_order_relaxed> relaxed_atomic_result_t;

    typedef jau::ringbuffer<uint8_t, size_t> ByteRingbuffer;

    extern const size_t BEST_URLSTREAM_RINGBUFFER_SIZE;

#ifdef BOTAN_VERSION_MAJOR
    #define VIRTUAL_BOTAN
    #define OVERRIDE_BOTAN override
    template<typename T> using secure_vector = std::vector<T, Botan::secure_allocator<T>>;
#else
    #define VIRTUAL_BOTAN virtual
    #define OVERRIDE_BOTAN
    template<typename T> using secure_vector = std::vector<T, jau::callocator_sec<T>>;
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
     * @see @ref byte_in_stream_properties "ByteInStream Properties"
     */
    class ByteInStream
#ifdef BOTAN_VERSION_MAJOR
    : public Botan::DataSource
#endif
    {
        public:
            ByteInStream() = default;
            virtual ~ByteInStream() = default;
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
            VIRTUAL_BOTAN bool check_available(size_t n) OVERRIDE_BOTAN = 0;

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
            [[nodiscard]] VIRTUAL_BOTAN size_t read(uint8_t out[], size_t length) OVERRIDE_BOTAN = 0;

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
            [[nodiscard]] VIRTUAL_BOTAN size_t peek(uint8_t out[], size_t length, size_t peek_offset) const OVERRIDE_BOTAN = 0;

            /**
             * Test whether the source still has data that can be read.
             * @return true if there is no more data to read, false otherwise
             */
            VIRTUAL_BOTAN bool end_of_data() const OVERRIDE_BOTAN = 0;

#ifndef BOTAN_VERSION_MAJOR

            /**
             * return the id of this data source
             * @return std::string representing the id of this data source
             */
            virtual std::string id() const OVERRIDE_BOTAN { return ""; }

            /**
             * Read one byte.
             * @param out the byte to read to
             * @return length in bytes that was actually read and put
             * into out
             */
            size_t read_byte(uint8_t& out);

            /**
             * Peek at one byte.
             * @param out an output byte
             * @return length in bytes that was actually read and put
             * into out
             */
            size_t peek_byte(uint8_t& out) const;

            /**
             * Discard the next N bytes of the data
             * @param N the number of bytes to discard
             * @return number of bytes actually discarded
             */
            size_t discard_next(size_t N);

#endif /* BOTAN_VERSION_MAJOR */

            /**
             * @return number of bytes read so far.
             */
            VIRTUAL_BOTAN size_t get_bytes_read() const OVERRIDE_BOTAN = 0;

            virtual std::string to_string() const = 0;
    };

    /**
     * This class represents a secure Memory-Based byte input stream
     */
    class ByteInStream_SecMemory final : public ByteInStream {
       public:
          /**
           * Read from a memory buffer
           */
          size_t read(uint8_t[], size_t) override;

          /**
           * Peek into a memory buffer
           */
          size_t peek(uint8_t[], size_t, size_t) const override;

          bool check_available(size_t n) override;

          /**
           * Check if the memory buffer is empty
           */
          bool end_of_data() const override;

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

          ~ByteInStream_SecMemory() override { close(); }

          size_t get_bytes_read() const override { return m_offset; }

          std::string to_string() const override;

       private:
          io::secure_vector<uint8_t> m_source;
          size_t m_offset;
    };

    /**
     * This class represents a std::istream based byte input stream.
     */
    class ByteInStream_istream final : public ByteInStream {
       public:
          size_t read(uint8_t[], size_t) override;
          size_t peek(uint8_t[], size_t, size_t) const override;
          bool check_available(size_t n) override;
          bool end_of_data() const override;
          std::string id() const override;

          ByteInStream_istream(std::istream&, const std::string& id = "<std::istream>");

          ByteInStream_istream(const ByteInStream_istream&) = delete;

          ByteInStream_istream& operator=(const ByteInStream_istream&) = delete;

          void close() noexcept override;

          ~ByteInStream_istream() override { close(); }

          size_t get_bytes_read() const override { return m_bytes_consumed; }

          std::string to_string() const override;

       private:
          const std::string m_identifier;

          std::istream& m_source;
          size_t m_bytes_consumed;
    };


    /**
     * This class represents a file based byte input stream.
     */
    class ByteInStream_File final : public ByteInStream {
       public:
          size_t read(uint8_t[], size_t) override;
          size_t peek(uint8_t[], size_t, size_t) const override;
          bool check_available(size_t n) override;
          bool end_of_data() const override;
          std::string id() const override;

          /**
           * Construct a Stream-Based byte input stream from filesystem path
           * @param file the path to the file
           * @param use_binary whether to treat the file as binary (default) or use platform character conversion
           */
          ByteInStream_File(const std::string& file, bool use_binary = true);

          ByteInStream_File(const ByteInStream_File&) = delete;

          ByteInStream_File& operator=(const ByteInStream_File&) = delete;

          void close() noexcept override;

          ~ByteInStream_File() override { close(); }

          size_t get_bytes_read() const override { return (size_t)m_bytes_consumed; }

          /**
           * Botan's get_bytes_read() API uses `size_t`,
           * which only covers 32bit or 4GB on 32bit systems.
           * @return uint64_t bytes read
           */
          uint64_t get_bytes_read_u64() const { return m_bytes_consumed; }

          std::string to_string() const override;

       private:
          const std::string m_identifier;
          std::unique_ptr<std::ifstream> m_source;
          uint64_t m_content_size;
          uint64_t m_bytes_consumed;
    };

    /**
     * This class represents a Ringbuffer-Based byte input stream with a URL connection provisioned data feed.
     *
     * Standard implementation uses curl, hence all protocols supported by curl are supported.
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
            bool check_available(size_t n) override;

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
            size_t read(uint8_t out[], size_t length) override;

            size_t peek(uint8_t out[], size_t length, size_t peek_offset) const override;

            bool end_of_data() const override;

            std::string id() const override { return m_url; }

            /**
             * Construct a ringbuffer backed Http byte input stream
             * @param url the URL of the data to read
             * @param timeout maximum duration in fractions of seconds to wait @ check_available(), where fractions_i64::zero waits infinitely
             * @param exp_size if > 0 it is additionally used to determine EOF, otherwise the underlying EOF mechanism is being used only (default).
             */
            ByteInStream_URL(const std::string& url, jau::fraction_i64 timeout, const uint64_t exp_size=0);

            ByteInStream_URL(const ByteInStream_URL&) = delete;

            ByteInStream_URL& operator=(const ByteInStream_URL&) = delete;

            void close() noexcept override;

            ~ByteInStream_URL() override { close(); }

            size_t get_bytes_read() const override { return (size_t)m_bytes_consumed; }

            /**
             * Botan's get_bytes_read() API uses `size_t`,
             * which only covers 32bit or 4GB on 32bit systems.
             * @return uint64_t bytes read
             */
            uint64_t get_bytes_read_u64() const { return m_bytes_consumed; }

            std::string to_string() const override;

        private:
            uint64_t get_available() const noexcept { return m_has_content_length ? m_content_size - m_bytes_consumed : 0; }
            std::string to_string_int() const;

            const std::string m_url;
            const uint64_t m_exp_size;
            jau::fraction_i64 m_timeout;
            ByteRingbuffer m_buffer;
            jau::relaxed_atomic_bool m_has_content_length;
            jau::relaxed_atomic_uint64 m_content_size;
            jau::relaxed_atomic_uint64 m_total_xfered;
            relaxed_atomic_result_t m_result;
            std::thread m_url_thread;
            uint64_t m_bytes_consumed;
    };

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
            bool check_available(size_t n) override;

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
            size_t read(uint8_t out[], size_t length) override;

            size_t peek(uint8_t out[], size_t length, size_t peek_offset) const override;

            bool end_of_data() const override;

            std::string id() const override { return m_id; }

            /**
             * Construct a ringbuffer backed externally provisioned byte input stream
             * @param id_name arbitrary identifier for this instance
             * @param timeout maximum duration in fractions of seconds to wait @ check_available() and write(), where fractions_i64::zero waits infinitely
             * @param exp_size if > 0 it is additionally used to determine EOF, otherwise the underlying EOF mechanism is being used only (default).
             */
            ByteInStream_Feed(const std::string& id_name, jau::fraction_i64 timeout, const uint64_t exp_size=0);

            ByteInStream_Feed(const ByteInStream_URL&) = delete;

            ByteInStream_Feed& operator=(const ByteInStream_URL&) = delete;

            void close() noexcept override;

            ~ByteInStream_Feed() override { close(); }

            size_t get_bytes_read() const override { return m_bytes_consumed; }

            /**
             * Botan's get_bytes_read() API uses `size_t`,
             * which only covers 32bit or 4GB on 32bit systems.
             * @return uint64_t bytes read
             */
            uint64_t get_bytes_read_u64() const { return m_bytes_consumed; }

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
            void write(uint8_t in[], size_t length);

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
             * @param result should be either result_t::FAILED or result_t::SUCCESS.
             */
            void set_eof(const async_io_result_t result) noexcept { m_result = result; }

            std::string to_string() const override;

        private:
            uint64_t get_available() const noexcept { return m_has_content_length ? m_content_size - m_bytes_consumed : 0; }
            std::string to_string_int() const;

            const std::string m_id;
            const uint64_t m_exp_size;
            jau::fraction_i64 m_timeout;
            ByteRingbuffer m_buffer;
            jau::relaxed_atomic_bool m_has_content_length;
            jau::relaxed_atomic_uint64 m_content_size;
            jau::relaxed_atomic_uint64 m_total_xfered;
            relaxed_atomic_result_t m_result;
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
            size_t read(uint8_t[], size_t) override;

            size_t peek(uint8_t out[], size_t length, size_t peek_offset) const override {
                return m_parent.peek(out, length, peek_offset);
            }

            bool check_available(size_t n) override {
                return m_parent.check_available(n);
            }

            bool end_of_data() const override {
                return m_parent.end_of_data();
            }

            std::string id() const override { return m_parent.id(); }

            /**
             * Construct a byte input stream wrapper using the given parent ByteInStream.
             * @param parent the parent ByteInStream
             * @param buffer a user defined buffer for the recording
             */
            ByteInStream_Recorder(ByteInStream& parent, io::secure_vector<uint8_t>& buffer)
            : m_parent(parent), m_bytes_consumed(0), m_buffer(buffer), m_rec_offset(0), m_is_recording(false) {};

            ByteInStream_Recorder(const ByteInStream_Recorder&) = delete;

            ByteInStream_Recorder& operator=(const ByteInStream_Recorder&) = delete;

            void close() noexcept override;

            ~ByteInStream_Recorder() override { close(); }

            size_t get_bytes_read() const override { return m_parent.get_bytes_read(); }

            /**
             * Botan's get_bytes_read() API uses `size_t`,
             * which only covers 32bit or 4GB on 32bit systems.
             * @return uint64_t bytes read
             */
            uint64_t get_bytes_read_u64() const { return m_bytes_consumed; }

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

            std::string to_string() const override;

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
