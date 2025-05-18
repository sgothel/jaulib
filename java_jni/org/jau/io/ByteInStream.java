/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022-2023 Gothel Software e.K.
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
package org.jau.io;

import java.nio.ByteBuffer;

/**
 * Abstract byte input stream object.
 *
 * Its specializations utilize a native C++ implementation
 * derived from `jau::io::ByteInStream`.
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
public interface ByteInStream extends IOStateFunc, AutoCloseable  {
    /** Checks if the stream has an associated file. */
    boolean is_open();

    /**
     * Close the stream if supported by the underlying mechanism.
     *
     * Native instance will not be disposed.
     *
     * {@inheritDoc}
     */
    void closeStream();

    /**
     * Close the stream if supported by the underlying mechanism
     * and dispose the native instance.
     *
     * Instance is unusable after having this method called.
     *
     * {@inheritDoc}
     */
    @Override
    void close();

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
    boolean available(long n);

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
     * @param offset offset to byte array to read into
     * @param length the length of the byte array out
     * @return length in bytes that was actually read and put into out
     *
     * @see available()
     * @see @ref byte_in_stream_properties "ByteInStream Properties"
     */
    int read(byte out[], final int offset, final int length);

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
     * @param out the direct {@link ByteBuffer} to write the result starting at its {@link ByteBuffer#position() position} up to its {@link ByteBuffer#capacity() capacity}.
     *            {@link ByteBuffer#limit() Limit} will be set to {@link ByteBuffer#position() position} + read-bytes.
     * @return length in bytes that was actually read and put into out,
     *         equal to its {@link ByteBuffer#limit() limit} - {@link ByteBuffer#position() position}, i.e. {@link ByteBuffer#remaining() remaining}.
     *
     * @see available()
     * @see @ref byte_in_stream_properties "ByteInStream Properties"
     */
    int read(ByteBuffer out);

    /**
     * Read from the source but do not modify the internal
     * offset. Consecutive calls to peek() will return portions of
     * the source starting at the same position.
     *
     * @param out the byte array to write the output to
     * @param offset offset to byte array to read into
     * @param length number of in bytes to read into starting at offset
     * @param peek_offset the offset into the stream to read at
     * @return length in bytes that was actually read and put into out
     */
    int peek(byte out[], final int offset, final int length, final long peek_offset);

    /**
     * return the id of this data source
     * @return std::string representing the id of this data source
     */
    String id();

    /**
     * Discard the next N bytes of the data
     * @param N the number of bytes to discard
     * @return number of bytes actually discarded
     */
    long discard_next(long N);

    /**
     * Returns the input position indicator, similar to std::basic_istream.
     *
     * @return number of bytes read so far.
     */
    long position();

    /**
     * Returns true if implementation is aware of content_size(), otherwise false.
     * @see content_size()
     */
    boolean has_content_size();

    /**
     * Returns the content_size if known.
     * @see has_content_size()
     */
    long content_size();


    @Override
    String toString();
}
