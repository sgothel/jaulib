/**
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
package org.jau.io;

import java.nio.ByteBuffer;

/**
 * This class represents an abstract byte input stream object.
 *
 * Its specializations utilize a native C++ implementation
 * derived from `jau::io::ByteInStream`.
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
 * One may use error() to detect whether an error has occurred,
 * while end_of_data() not only covered the EOS case but includes error().
 *
 * @see @ref byte_in_stream_properties "ByteInStream Properties"
 */
public interface ByteInStream extends AutoCloseable  {
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
    boolean check_available(long n);

    /**
     * Read from the source. Moves the internal offset so that every
     * call to read will return a new portion of the source.
     *
     * Use check_available() to wait and ensure a certain amount of bytes are available.
     *
     * This method is not blocking.
     *
     * @param out the byte array to write the result to
     * @param offset offset to in byte array to read into
     * @param length number of in bytes to read into starting at offset
     * @return length in bytes that was actually read and put into out
     *
     * @see check_available()
     * @see @ref byte_in_stream_properties "ByteInStream Properties"
     */
    int read(byte out[], final int offset, final int length);

    /**
     * Read from the source. Moves the internal offset so that every
     * call to read will return a new portion of the source.
     *
     * Use check_available() to wait and ensure a certain amount of bytes are available.
     *
     * This method is not blocking.
     *
     * @param out the direct {@link ByteBuffer} to write the result starting at its {@link ByteBuffer#position() position} up to its {@link ByteBuffer#capacity() capacity}.
     *            {@link ByteBuffer#limit() Limit} will be set to {@link ByteBuffer#position() position} + read-bytes.
     * @return length in bytes that was actually read and put into out,
     *         equal to its {@link ByteBuffer#limit() limit} - {@link ByteBuffer#position() position}, i.e. {@link ByteBuffer#remaining() remaining}.
     *
     * @see check_available()
     * @see @ref byte_in_stream_properties "ByteInStream Properties"
     *
     * @param out
     * @return
     */
    int read(ByteBuffer out);

    /**
     * Read from the source but do not modify the internal
     * offset. Consecutive calls to peek() will return portions of
     * the source starting at the same position.
     *
     * @param out the byte array to write the output to
     * @param offset offset to in byte array to read into
     * @param length number of in bytes to read into starting at offset
     * @param peek_offset the offset into the stream to read at
     * @return length in bytes that was actually read and put into out
     */
    int peek(byte out[], final int offset, final int length, final long peek_offset);

    /**
     * Test whether the source still has data that can be read.
     *
     * This may include a failure and/or error in the underlying implementation, see error()
     *
     * @return true if there is no more data to read, false otherwise
     * @see error()
     */
    boolean end_of_data();

    /**
     * Return whether an error has occurred, excluding end_of_data().
     *
     * @return true if an error has occurred, false otherwise
     */
    boolean error();

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
     * @return number of bytes read so far.
     */
    long bytes_read();

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
