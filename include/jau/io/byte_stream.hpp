/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021-2025 Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */

#ifndef JAU_IO_BYTE_STREAM_HPP_
#define JAU_IO_BYTE_STREAM_HPP_

#include <cstdint>
#include <string>

#include <jau/basic_types.hpp>
#include <jau/enum_util.hpp>
#include <jau/io/file_util.hpp>
#include <jau/io/io_util.hpp>
#include <jau/ringbuffer.hpp>

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
    enum class iostate_t : uint32_t {
        /** No error occurred nor has EOS being reached. Value is no bit set! */
        none = 0,

        /** No error occurred nor has EOS being reached. Value is no bit set! */
        goodbit = 0,

        /** Irrecoverable stream error, including loss of integrity of the underlying stream or media. */
        badbit = 1U << 0,

        /** An input operation reached the end of its stream (EOS). */
        eofbit = 1U << 1,

        /** Input or output operation failed (formatting or extraction error). */
        failbit = 1U << 2,

        /** Input or output operation failed due to timeout. */
        timeout = 1U << 3
    };
    JAU_MAKE_BITFIELD_ENUM_STRING(iostate_t, goodbit, badbit, eofbit, failbit, timeout);

    /** Stream I/O mode, e.g. read and/or write. */
    enum class iomode_t : uint32_t {
        /** No capabilities */
        none = 0,
        /** Read capabilities */
        read = 1U << 0,
        /** Write capabilities */
        write = 1U << 1,
        /** Read and write capabilities, i.e. `read|write` */
        rw = read | write,
        /** Seek to end of (file) stream when opened */
        atend = 1U << 2,
        /** Truncate existing (file) stream when opened with write */
        trunc = 1U << 3,
        /** Write capabilities and truncate existing (file) stream, i.e. `write|trunc` */
        writetrunc = write | trunc,
    };
    JAU_MAKE_BITFIELD_ENUM_STRING(iomode_t, read, write, atend, trunc);

    /**
     * Supporting std::basic_ios's iostate capabilities for all ByteStream implementations.
     */
    class IOStateCap {
      private:
        iostate_t m_state;

      protected:
        constexpr iostate_t rdstate_impl() const noexcept { return m_state; }
        constexpr void addstate_impl(iostate_t state) const noexcept { const_cast<IOStateCap*>(this)->m_state |= state; }

      public:
        IOStateCap() noexcept
        : m_state(iostate_t::goodbit) { }

        IOStateCap(const IOStateCap& o) noexcept = default;
        IOStateCap(IOStateCap&& o) noexcept = default;
        IOStateCap& operator=(const IOStateCap& o) noexcept = default;
        IOStateCap& operator=(IOStateCap&& o) noexcept = default;

        virtual ~IOStateCap() noexcept = default;

        /**
         * Clears state flags by assignment to the given value.
         *
         * \deprecated { Use assignState() for clarity. }
         *
         * @see assignState()
         */
        void clear(const iostate_t state = iostate_t::goodbit) noexcept { assignState(state); }

        /** Assigns given state to current value. */
        virtual void assignState(const iostate_t state = iostate_t::goodbit) noexcept { m_state = state; }

        /** Clears given state flags from current value. */
        void clearStateFlags(const iostate_t clr) noexcept { assignState(rdstate() & ~clr); }

        /**
         * Returns the current state flags.
         *
         * Method is marked `virtual` to allow implementations with asynchronous resources
         * to determine or update the current iostate.
         *
         * Method is used throughout all query members and addState(),
         * hence they all will use the updated state from a potential override implementation.
         */
        virtual iostate_t rdstate() const noexcept { return rdstate_impl(); }

        /**
         * Sets state flags, by keeping its previous bits.
         *
         * \deprecated { Use addState() for clarity. }
         *
         * @see addState()
         */
        void setstate(const iostate_t state) noexcept { addState(state); }

        /** Adds given state flags to existing rdstate() bits. */
        void addState(const iostate_t state) noexcept { assignState(rdstate() | state); }

        /** Checks if no error nor eof() has occurred i.e. I/O operations are available. */
        bool good() const noexcept { return iostate_t::goodbit == rdstate(); }

        /** Checks if end-of-file has been reached. */
        bool eof() const noexcept { return iostate_t::none != (rdstate() & iostate_t::eofbit); }

        /** Checks if an error has occurred. */
        bool fail() const noexcept { return iostate_t::none != (rdstate() & (iostate_t::badbit | iostate_t::failbit | iostate_t::timeout)); }

        /** Checks if an error has occurred, synonym of fail(). */
        bool operator!() const noexcept { return fail(); }

        /** Checks if no error has occurred, synonym of !fail(). */
        explicit operator bool() const noexcept { return !fail(); }

        /** Checks if a non-recoverable error has occurred. */
        bool bad() const noexcept { return iostate_t::none != (rdstate() & iostate_t::badbit); }

        /** Checks if a timeout (non-recoverable) has occurred. */
        bool timeout() const noexcept { return iostate_t::none != (rdstate() & iostate_t::timeout); }
    };

    /**
     * Byte stream interface.
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
    class ByteStream : public IOStateCap {
      protected:
        iomode_t m_iomode;

        /// Fallback slow discard implementation usind read() in case of unknown stream size.
        size_t discardRead(size_t n) noexcept;

      public:
        /** Invalid position constant, denoting unset mark() or invalid position. Value: `std::numeric_limits<uint64_t>::max()` */
        constexpr static uint64_t npos = std::numeric_limits<uint64_t>::max();

        ByteStream(iomode_t mode) noexcept
        : IOStateCap(), m_iomode(mode) { }

        ~ByteStream() noexcept override = default;

        constexpr iomode_t mode() const noexcept { return m_iomode; }

        /** Returns true in case stream has iomode::read capabilities. */
        constexpr bool canRead() const noexcept { return is_set(m_iomode, iomode_t::read); }

        /** Returns true in case stream has iomode::write capabilities. */
        constexpr bool canWrite() const noexcept { return is_set(m_iomode, iomode_t::write); }

        /** Checks if the stream has an associated file. */
        virtual bool isOpen() const noexcept = 0;

        /**
         * Close the stream if supported by the underlying mechanism.
         */
        virtual void close() noexcept = 0;

        /**
         * return the id of this data source
         * @return std::string representing the id of this data source
         */
        virtual std::string id() const noexcept { return ""; }

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

        /**
         * Returns the position indicator, similar to e.g. std::basic_istream.
         *
         * @return number of bytes read or written so far.
         */
        virtual uint64_t position() const noexcept = 0;

        /**
         * Sets position indicator for output-streams or input-streams with known length, similar to e.g. std::basic_istream.
         *
         * No change occurs if fail() was set or the input-stream has no known length.
         *
         * If newPos is >= stream-length, iostate::eofbit is set and position set to stream-length,
         * otherwise iostate::eofbit is cleared.
         *
         * Certain implementations may not allow random rewinding of the stream
         * but only support rewinding back to `markpos` (see setMark()) and may return ByteStream::npos if no set or exceeding range.
         *
         * A ByteInStream's mark is cleared if > newPos.
         *
         * @param newPos desired absolute byte-offset (position)
         * @return resulting position if successful (incl eofbit) or ByteStream::npos otherwise having an unchanged position().
         */
        [[nodiscard]] virtual uint64_t seek(uint64_t newPos) noexcept = 0;

        virtual std::string toString() const noexcept = 0;

        //
        // I/O
        //

        /**
         * Set `markpos` to current position, allowing the stream to be seekMark().
         *
         * seek() will clear `markpos` if > newPos.
         *
         * For implementations where seek() doesn't allow random rewinding of the stream,
         * setMark() will allow rewinding back to `markpos` if not exceeding `readLimit`.
         *
         * @param readlimit maximum number of bytes able to read before invalidating the `markpos`.
         * @return true if marks is set successfully, otherwise false
         */
        [[nodiscard]] virtual bool setMark(uint64_t readLimit) noexcept = 0;

        /** Returns the `markpos` set via setMark() or ByteStream::npos if unset. */
        virtual uint64_t mark() const noexcept = 0;

        /** Returns the `readLimit` set via setMark(). If unset either 0 or implicit limit. */
        virtual uint64_t markReadLimit() const noexcept = 0;

        /**
         * Seeks stream position to `markpos` as set via setMark().
         *
         * `markpos` is kept, hence seekMark() can be called multiple times.
         *
         * @return true if successful (incl eofbit),
         *         otherwise false with unchanged position due to I/O failure (iostate::fail set) or setMark() not set.
         */
        [[nodiscard]] virtual bool seekMark() noexcept = 0;

        //
        // Input Stream
        //

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
         * Input stream operation, returns false if !is_input().
         *
         * @param n byte count to wait for
         * @return true if n bytes are available, otherwise false
         *
         * @see has_content_size()
         * @see read()
         * @see @ref byte_in_stream_properties "ByteInStream Properties"
         */
        virtual bool available(size_t n) noexcept = 0;

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
         * Input stream operation, returns zero if !is_input().
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
         *
         * Input stream operation, returns false if !is_input().
         *
         * @param out the byte to read to
         * @return true if one byte has been read, false otherwise
         */
        [[nodiscard]] bool read(uint8_t& out) noexcept;

        /**
         * Read from the source but do not modify the internal
         * offset. Consecutive calls to peek() will return portions of
         * the source starting at the same position.
         *
         * Input stream operation, returns zero if !is_input().
         *
         * @param out the byte array to write the output to
         * @param length the length of the byte array out
         * @param peek_offset offset from current stream position to read at
         * @return length in bytes that was actually read and put into out
         */
        [[nodiscard]] virtual size_t peek(void* out, size_t length, uint64_t peek_offset) noexcept = 0;

        /**
         * Peek one byte at current position
         *
         * Input stream operation, returns false if !is_input().
         *
         * @param out an output byte
         * @return true if one byte has been peeked, false otherwise
         */
        [[nodiscard]] bool peek(uint8_t& out) noexcept;

        /**
         * Discard the next N bytes of the data
         *
         * Input stream operation, returns zero if !is_input().
         *
         * @param N the number of bytes to discard
         * @return number of bytes actually discarded
         */
        [[nodiscard]] virtual size_t discard(size_t N) noexcept = 0;

        //
        // Output Stream
        //

        /**
         * Write to the data sink. Moves the internal offset so that every
         * call to write will be appended to the sink.
         *
         * This method is not blocking beyond the transfer length bytes.
         *
         * Output stream operation, returns zero if !is_output().
         *
         * @param in the input bytes to write out
         * @param length the length of the byte array in
         * @return length in bytes that were actually written
         */
        [[nodiscard]] virtual size_t write(const void* in, size_t length) noexcept = 0;

        /**
         * Write one byte.
         *
         * Output stream operation, returns false if !is_output().
         *
         * @param in the byte to be written
         * @return true if one byte has been written, otherwise false
         */
        [[nodiscard]] bool write(const uint8_t& in) noexcept;

        /**
         * Synchronizes all output operations, or do nothing.
         */
        virtual void flush() noexcept = 0;
    };

    inline std::ostream& operator<<(std::ostream& os, const ByteStream& v) { return os << v.toString(); }

    /**
     * Secure Memory-Based byte input stream
     */
    class ByteStream_SecMemory final : public ByteStream {
      public:
        /**
         * Construct a secure memory source that reads from a string, iomode_t::read.
         * @param in the string to read from
         */
        explicit ByteStream_SecMemory(const std::string& in);

        /**
         * Construct a secure memory source that reads from a byte array
         * @param in the byte array to read from, copied over
         * @param length the length of the byte array
         * @param iomode determines whether file should be opened iomode_t::read, iomode_t::write or iomode_t::rw
         */
        ByteStream_SecMemory(const uint8_t in[], size_t length, iomode_t mode)
        : ByteStream(mode), m_source(in, in + length), m_offset(0), m_mark(npos) { }

        /**
         * Construct a secure memory source that reads from a secure_vector
         * @param in the MemoryRegion to read from, this instance assumes ownership
         * @param iomode determines whether file should be opened iomode_t::read, iomode_t::write or iomode_t::rw
         */
        explicit ByteStream_SecMemory(io::secure_vector<uint8_t> && in, iomode_t mode)
        : ByteStream(mode), m_source(std::move(in)), m_offset(0), m_mark(npos) { }

        /**
         * Construct a secure memory source that reads from a std::vector
         * @param in the MemoryRegion to read from, copied over
         * @param iomode determines whether file should be opened iomode_t::read, iomode_t::write or iomode_t::rw
         */
        explicit ByteStream_SecMemory(const std::vector<uint8_t>& in, iomode_t mode)
        : ByteStream(mode), m_source(in.begin(), in.end()), m_offset(0), m_mark(npos) { }

        ~ByteStream_SecMemory() noexcept override { close(); }

        bool isOpen() const noexcept override { return true; }

        void close() noexcept override;

        bool has_content_size() const noexcept override { return true; }

        uint64_t content_size() const noexcept override { return m_source.size(); }

        uint64_t position() const noexcept override { return m_offset; }

        [[nodiscard]] uint64_t seek(uint64_t newPos) noexcept override;

        std::string toString() const noexcept override;

        [[nodiscard]] bool setMark(uint64_t readLimit) noexcept override;
        uint64_t mark() const noexcept override { return m_mark; }
        uint64_t markReadLimit() const noexcept override { return content_size(); }
        [[nodiscard]] bool seekMark() noexcept override;

        bool available(size_t n) noexcept override;
        [[nodiscard]] size_t read(void*, size_t) noexcept override;
        [[nodiscard]] size_t peek(void*, size_t, uint64_t) noexcept override;
        [[nodiscard]] size_t discard(size_t N) noexcept override;

        [[nodiscard]] size_t write(const void*, size_t) noexcept override;
        void flush() noexcept override {}

      private:
        io::secure_vector<uint8_t> m_source;
        size_t m_offset;
        uint64_t m_mark;
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
    class ByteStream_File final : public ByteStream {
      private:
        jau::io::fs::file_stats m_stats;

        int m_fd;

        bool m_has_content_length;
        uint64_t m_content_size;
        uint64_t m_offset;
        uint64_t m_mark;
        uint64_t get_available() const noexcept { return m_has_content_length ? m_content_size - m_offset : 0; }

      public:
        /**
         * Construct a stream based byte stream from filesystem path,
         * either an existing or new file.
         *
         * In case the file already exists and iomode has iomode_t::atend, the underlying file offset is positioned at the end of the file.
         *
         * In case the given path is a local file URI starting with `file://`, see jau::io::uri::is_local_file_protocol(),
         * the leading `file://` is cut off and the remainder being used.
         *
         * @param path the path to the file, maybe a local file URI
         * @param iomode determines whether file should be opened iomode_t::read, iomode_t::write or iomode_t::rw
         * @param fmode file protection mode for a new file, otherwise ignored.
         */
        ByteStream_File(const std::string& path, iomode_t iomode, const jau::io::fs::fmode_t fmode = fs::fmode_t::def_file_prot) noexcept;

        /**
         * Construct a stream based byte stream from filesystem path and parent directory file descriptor,
         * either an existing or new file.
         *
         * In case the file already exists and iomode has iomode_t::atend, the underlying file offset is positioned at the end of the file.
         *
         * In case the given path is a local file URI starting with `file://`, see jau::io::uri::is_local_file_protocol(),
         * the leading `file://` is cut off and the remainder being used.
         *
         * @param dirfd parent directory file descriptor
         * @param path the path to the file, maybe a local file URI
         * @param iomode determines whether file should be opened iomode_t::read, iomode_t::write or iomode_t::rw
         * @param fmode file protection mode for a new file, otherwise ignored.
         */
        ByteStream_File(const int dirfd, const std::string& path, iomode_t iomode, const jau::io::fs::fmode_t fmode = fs::fmode_t::def_file_prot) noexcept;

        /**
         * Construct a stream based byte stream by duplicating given file descriptor
         *
         * In case the given path is a local file URI starting with `file://`, see jau::io::uri::is_local_file_protocol(),
         * the leading `file://` is cut off and the remainder being used.
         *
         * @param fd file descriptor to duplicate leaving the given `fd` untouched
         * @param iomode determines whether file descriptor is iomode_t::read, iomode_t::write or iomode_t::rw
         */
        ByteStream_File(const int fd, iomode_t mode) noexcept;

        ByteStream_File(const ByteStream_File&) = delete;

        ByteStream_File& operator=(const ByteStream_File&) = delete;

        ~ByteStream_File() noexcept override { close(); }

        const jau::io::fs::file_stats& stats() const noexcept { return m_stats; }

        /**
         * Returns the file descriptor if is_open(), otherwise -1 for no file descriptor.
         *
         * @see is_open()
         */
        int fd() const noexcept { return m_fd; }


        bool isOpen() const noexcept override { return 0 <= m_fd; }
        void close() noexcept override;
        std::string id() const noexcept override { return m_stats.path(); }

        bool has_content_size() const noexcept override { return m_has_content_length; }
        uint64_t content_size() const noexcept override { return m_content_size; }
        uint64_t position() const noexcept override { return m_offset; }
        [[nodiscard]] uint64_t seek(uint64_t newPos) noexcept override;
        std::string toString() const noexcept override;

        [[nodiscard]] bool setMark(uint64_t readLimit) noexcept override;
        uint64_t mark() const noexcept override { return m_mark; }
        uint64_t markReadLimit() const noexcept override { return content_size(); }
        [[nodiscard]] bool seekMark() noexcept override;

        bool available(size_t n) noexcept override;
        [[nodiscard]] size_t read(void*, size_t) noexcept override;
        [[nodiscard]] size_t peek(void*, size_t, uint64_t) noexcept override;
        [[nodiscard]] size_t discard(size_t N) noexcept override;

        [[nodiscard]] size_t write(const void*, size_t) noexcept override;
        void flush() noexcept override;
    };

    namespace impl {
        /**
         * Rewind buffer support for mark/setMark, read and reset/seekMark
         *
         * m - m_mark            (stream space)
         * o - m_offset          (stream space)
         * p - m_offset          (stream space)
         *
         * g - got bytes         (relative)
         *
         * q - m_offset - m_mark (buffer space, relative stream)
         * r - m_end             (buffer space, relative stream)
         *
         *               <-- q = o-m  ->
         *                     <- g ->
         * stream [0 ... m ... p ... o)
         * buffer       [0 ... q ... r)
         */
        class RewindBuffer {
          private:
            std::vector<uint8_t> m_buffer;
            size_t m_end;

          public:
            typedef jau::function<size_t(void* out, size_t length)> DataProvider;

            RewindBuffer() noexcept : m_buffer(0), m_end(0) {}

            constexpr bool covered(const uint64_t m, uint64_t o) const noexcept {
                return ByteStream::npos != m && m <= o && o < m+m_end;
            }

            constexpr uint64_t capacity() const noexcept { return m_buffer.size(); }
            constexpr uint64_t end() const noexcept { return m_end; }
            std::string toString() const noexcept {
                return "Rew[end "+std::to_string(end())+", capacity "+std::to_string(capacity())+"]";
            }

            bool setMark(const uint64_t m, uint64_t o, uint64_t readLimit) noexcept {
                // DBG_PRINT("RewindBuffer.setMark.0 mark %" PRIu64 ", offset %" PRIu64 ", %s", m, o, toString().c_str());
                PRAGMA_DISABLE_WARNING_PUSH
                PRAGMA_DISABLE_WARNING_TYPE_RANGE_LIMIT
                if( readLimit > std::numeric_limits<size_t>::max() ) {
                    return false;
                }
                PRAGMA_DISABLE_WARNING_POP
                if( covered(m, o) ) {
                    const size_t q = o - m;
                    const size_t l0 = m_end - q;
                    if( q > 0 ) {
                        std::memmove(m_buffer.data(), m_buffer.data()+q, l0);
                    }
                    m_end = l0;
                    // DBG_PRINT("RewindBuffer.setMark.1 src-q %" PRIu64 ", len %" PRIu64 ", %s.", q, l0, toString().c_str());
                } else {
                    m_end = 0;
                }
                if( readLimit > m_buffer.size() ) {
                    m_buffer.resize(readLimit, 0);
                }
                return true;
            }
            size_t read(uint64_t& m, uint64_t& o, DataProvider newData, void* out, const size_t length) noexcept {
                [[maybe_unused]] size_t l0 = length;
                size_t g1 = 0, g2 = 0;
                if( covered(m, o) ) {
                    const size_t p0 = o - m;
                    g1 = std::min(m+m_end - o, l0);
                    std::memcpy(out, m_buffer.data()+p0, g1);
                    o += g1;
                    l0 -= g1;
                }
                if( l0 > 0 ) {
                    g2 = newData((char*)out+g1, l0);
                    if( g2 > 0 && ByteStream::npos != m ) {
                        if( m_end + g2 > m_buffer.size() ) {
                            m = ByteStream::npos;
                            m_end = 0;
                        } else {
                            std::memcpy(m_buffer.data()+m_end, (const char*)out+g1, g2);
                            m_end += g2;
                        }
                    }
                    o += g2;
                    l0 -= g2;
                }
                // DBG_PRINT("ByteInStream::read.X: size (%zu + %zu = %zu) / %zu (-> %zu) bytes, %s",
                //           g1, g2, g1+g2, length, l0, toString().c_str() );
                return g1+g2;
            }
        };
    }

    /**
     * Ringbuffer-Based byte input stream with a URL connection provisioned data feed.
     *
     * Standard implementation uses [curl](https://curl.se/),
     * hence all [*libcurl* network protocols](https://curl.se/docs/url-syntax.html) are supported,
     * jau::io::uri::supported_protocols().
     */
    class ByteInStream_URL final : public ByteStream {
      public:
        /**
         * Construct a ringbuffer backed Http byte input stream
         * @param url the URL of the data to read
         * @param timeout maximum duration in fractions of seconds to wait @ available() for next bytes, where fractions_i64::zero waits infinitely
         */
        ByteInStream_URL(std::string url, const jau::fraction_i64& timeout) noexcept;

        ByteInStream_URL(const ByteInStream_URL&) = delete;

        ByteInStream_URL& operator=(const ByteInStream_URL&) = delete;

        ~ByteInStream_URL() noexcept override { close(); }

        iostate_t rdstate() const noexcept override;

        bool isOpen() const noexcept override;
        void close() noexcept override;
        std::string id() const noexcept override { return m_url; }

        bool has_content_size() const noexcept override;
        uint64_t content_size() const noexcept override { return m_stream_resp->content_length; }
        uint64_t position() const noexcept override { return m_offset; }

        /// newPos < position() limited to `markpos`, see setMark().
        [[nodiscard]] uint64_t seek(uint64_t newPos) noexcept override;

        std::string toString() const noexcept override;

        [[nodiscard]] bool setMark(uint64_t readLimit) noexcept override;
        uint64_t mark() const noexcept override { return m_mark; }
        uint64_t markReadLimit() const noexcept override { return m_rewindbuf.capacity(); }
        [[nodiscard]] bool seekMark() noexcept override;

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

        [[nodiscard]] size_t peek(void* out, size_t length, uint64_t peek_offset) noexcept override;

        /// Implemented by skipping input stream via read
        [[nodiscard]] size_t discard(size_t N) noexcept override;

        [[nodiscard]] size_t write(const void*, size_t) noexcept override { return 0; }
        void flush() noexcept override { }

      private:
        uint64_t get_available() const noexcept { return m_stream_resp->has_content_length ? m_stream_resp->content_length - m_offset : 0; }
        std::string to_string_int() const noexcept;

        const std::string m_url;
        jau::fraction_i64 m_timeout;
        ByteRingbuffer m_buffer;
        jau::io::AsyncStreamResponseRef m_stream_resp;

        uint64_t m_offset;
        uint64_t m_mark;
        impl::RewindBuffer m_rewindbuf;

        impl::RewindBuffer::DataProvider newData = [&](void* out, size_t length) -> size_t {
            bool timeout_occured = false;
            const size_t g = m_buffer.getBlocking(static_cast<uint8_t*>(out), length, 1, m_timeout, timeout_occured);
            if( timeout_occured ) {
                addstate_impl( iostate_t::timeout );
                if( m_stream_resp->processing() ) {
                    m_stream_resp->result = io_result_t::FAILED;
                }
                m_buffer.interruptWriter();
            }
            return g;
        };
    };

    /**
     * Parses the given path_or_uri, if it matches a supported protocol, see jau::io::uri::protocol_supported(),
     * but is not a local file, see jau::io::uri::is_local_file_protocol(), ByteInStream_URL is being attempted.
     *
     * If the above fails, ByteStream_File (read-only) is attempted.
     *
     * If non of the above leads to a ByteStream without ByteStream::fail(), nullptr is returned.
     *
     * @param path_or_uri given path or uri for with a ByteInStream instance shall be established.
     * @param timeout in case `path_or_uri` resolves to ByteInStream_URL, timeout is being used as maximum duration to wait for next bytes at ByteInStream_URL::available(), defaults to 20_s
     * @return a working ByteInStream w/o ByteInStream::fail() or nullptr
     */
    std::unique_ptr<ByteStream> to_ByteInStream(const std::string& path_or_uri, jau::fraction_i64 timeout = 20_s) noexcept;

    /**
     * Ringbuffer-Based byte input stream with an externally provisioned data feed.
     */
    class ByteInStream_Feed final : public ByteStream {
      public:
        /**
         * Construct a ringbuffer backed externally provisioned byte input stream
         * @param id_name arbitrary identifier for this instance
         * @param timeout maximum duration in fractions of seconds to wait @ available() and write(), where fractions_i64::zero waits infinitely
         */
        ByteInStream_Feed(std::string id_name, const jau::fraction_i64& timeout) noexcept;

        ByteInStream_Feed(const ByteInStream_Feed&) = delete;

        ByteInStream_Feed& operator=(const ByteInStream_Feed&) = delete;

        ~ByteInStream_Feed() noexcept override { close(); }

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

        iostate_t rdstate() const noexcept override;

        bool isOpen() const noexcept override;

        void close() noexcept override;

        std::string id() const noexcept override { return m_id; }

        bool has_content_size() const noexcept override { return m_has_content_length; }

        uint64_t content_size() const noexcept override { return m_content_size; }

        uint64_t position() const noexcept override { return m_offset; }

        /// newPos < position() limited to `markpos`, see setMark().
        [[nodiscard]] uint64_t seek(uint64_t newPos) noexcept override;

        std::string toString() const noexcept override;

        [[nodiscard]] bool setMark(uint64_t readLimit) noexcept override;
        uint64_t mark() const noexcept override { return m_mark; }
        uint64_t markReadLimit() const noexcept override { return m_rewindbuf.capacity(); }
        [[nodiscard]] bool seekMark() noexcept override;

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

        /// Not implemented, returns 0
        [[nodiscard]] size_t peek(void* out, size_t length, uint64_t peek_offset) noexcept override;

        /// Implemented by skipping input stream via read
        [[nodiscard]] size_t discard(size_t N) noexcept override;

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
        [[nodiscard]] size_t write(const void* in, size_t length, const jau::fraction_i64& timeout) noexcept;

        /**
         * Write given bytes to the async ringbuffer, feeding the input stream.
         *
         * Wait up to timeout duration set in constructor until ringbuffer space is available, where fractions_i64::zero waits infinitely.
         *
         * This method is blocking.
         *
         * @param in the byte array to transfer to the async ringbuffer
         * @param length the length of the byte array in
         * @return length if successful, otherwise zero on timeout or stopped feeder and subsequent calls to good() will return false.
         */
        [[nodiscard]] size_t write(const void* in, size_t length) noexcept override {
            return write(in, length, m_timeout);
        }


        void flush() noexcept override { }

      private:
        uint64_t get_available() const noexcept { return m_has_content_length ? m_content_size - m_offset : 0; }
        std::string to_string_int() const noexcept;

        const std::string m_id;
        jau::fraction_i64 m_timeout;
        ByteRingbuffer m_buffer;
        jau::relaxed_atomic_bool m_has_content_length;
        jau::relaxed_atomic_uint64 m_content_size;
        jau::relaxed_atomic_uint64 m_total_xfered;
        relaxed_atomic_io_result_t m_result;

        uint64_t m_offset;
        uint64_t m_mark;
        impl::RewindBuffer m_rewindbuf;

        impl::RewindBuffer::DataProvider newData = [&](void* out, size_t length) -> size_t {
            bool timeout_occured = false;
            // set_eof() unblocks ringbuffer via set_end_of_input(true) permanently, hence blocking call on !m_has_content_length is OK.
            const size_t g = m_buffer.getBlocking(static_cast<uint8_t*>(out), length, 1, m_timeout, timeout_occured);
            if( timeout_occured ) {
                addstate_impl( iostate_t::timeout );
                if( io_result_t::NONE == m_result ) {
                    m_result = io_result_t::FAILED;
                }
                m_buffer.interruptWriter();
            }
            return g;
        };
    };

    /**
     * Wrapped byte input stream with the capability
     * to record the read byte stream at will.
     *
     * Peek'ed, seek'ed or discard'ed bytes won't be recorded, only read bytes.
     */
    class ByteStream_Recorder final : public ByteStream {
      public:
        /**
         * Construct a byte input stream wrapper using the given parent ByteInStream.
         * @param parent the parent ByteInStream
         * @param buffer a user defined buffer for the recording
         */
        ByteStream_Recorder(ByteStream& parent, io::secure_vector<uint8_t>& buffer) noexcept
        : ByteStream(iomode_t::read), m_parent(parent), m_offset(0), m_buffer(buffer), m_rec_offset(0), m_is_recording(false) {};

        ByteStream_Recorder(const ByteStream_Recorder&) = delete;

        ByteStream_Recorder& operator=(const ByteStream_Recorder&) = delete;

        ~ByteStream_Recorder() noexcept override { close(); }

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

        void assignState(const iostate_t state = iostate_t::goodbit) noexcept override { m_parent.assignState(state); }
        iostate_t rdstate() const noexcept override { return m_parent.rdstate(); }

        bool isOpen() const noexcept override { return m_parent.isOpen(); }

        void close() noexcept override;

        uint64_t position() const noexcept override { return m_offset; }

        bool has_content_size() const noexcept override { return m_parent.has_content_size(); }

        uint64_t content_size() const noexcept override { return m_parent.content_size(); }

        std::string toString() const noexcept override;

        std::string id() const noexcept override { return m_parent.id(); }

        [[nodiscard]] uint64_t seek(uint64_t newPos) noexcept override {
            m_offset = m_parent.seek(newPos);
            return m_offset;
        }
        [[nodiscard]] size_t discard(size_t N) noexcept override { size_t n = m_parent.discard(N); m_offset = m_parent.position(); return n; }

        [[nodiscard]] bool setMark(uint64_t readLimit) noexcept override { return m_parent.setMark(readLimit); }
        uint64_t mark() const noexcept override { return m_parent.mark(); }
        uint64_t markReadLimit() const noexcept override { return m_parent.markReadLimit(); }
        [[nodiscard]] bool seekMark() noexcept override {
            if( m_parent.seekMark() ) {
                m_offset = m_parent.position();
                return true;
            } else {
                return false;
            }
        }

        [[nodiscard]] size_t read(void*, size_t) noexcept override;

        [[nodiscard]] size_t peek(void* out, size_t length, uint64_t peek_offset) noexcept override {
            return m_parent.peek(out, length, peek_offset);
        }

        bool available(size_t n) noexcept override { return m_parent.available(n); }

        [[nodiscard]] size_t write(const void*, size_t) noexcept override;
        void flush() noexcept override { m_parent.flush(); }

      private:
        ByteStream& m_parent;
        uint64_t m_offset;
        io::secure_vector<uint8_t>& m_buffer;
        uint64_t m_rec_offset;
        bool m_is_recording;
    };

    /**@}*/

}  // namespace jau::io

#endif /* JAU_IO_BYTE_STREAM_HPP_ */
