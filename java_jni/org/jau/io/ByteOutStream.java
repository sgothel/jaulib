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
 * Abstract byte output stream object,
 * to write data to a sink.
 *
 * Its specializations utilize a native C++ implementation
 * derived from `jau::io::ByteOutStream`.
 *
 * All method implementations are of `noexcept`.
 *
 * One may use fail() to detect whether an error has occurred.
 */
public interface ByteOutStream extends IOStateFunc, AutoCloseable  {
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
     * Write to the data sink. Moves the internal offset so that every
     * call to write will be appended to the sink.
     *
     * This method is not blocking beyond the transfer length bytes.
     *
     * @param in the input bytes to write out
     * @param offset offset to byte array to write out
     * @param length the length of the byte array in
     * @return length in bytes that were actually written
     */
    int write(byte in[], final int offset, final int length);

    /**
     * Write to the data sink. Moves the internal offset so that every
     * call to write will be appended to the sink.
     *
     * This method is not blocking beyond the transfer length bytes.
     *
     * @param in the direct {@link ByteBuffer} source to be written to the sink from {@link ByteBuffer#position() position} up to its {@link ByteBuffer#limit() limit},
     *           i.e. {@link ByteBuffer#remaining() remaining}.
     *           {@link ByteBuffer#position() Position} will be set to {@link ByteBuffer#position() position} + written-bytes.
     * @return length in bytes that were actually written,
     *         equal to its current {@link ByteBuffer#position() position} - previous {@link ByteBuffer#position() position}.
     */
    int write(ByteBuffer in);

    /**
     * return the id of this data source
     * @return std::string representing the id of this data source
     */
    String id();

    /**
     * Returns the output position indicator.
     *
     * @return number of bytes written so far.
     */
    long tellp();

    @Override
    String toString();
}
