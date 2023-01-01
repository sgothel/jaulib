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

/**
 * Supporting std::basic_ios's iostate functionality for all ByteInStream implementations.
 */
public interface IOStateFunc {

    /** Clears state flags by assignment to the given value. */
    void clear(final IOState state);

    /**
     * Returns the current state flags.
     *
     * Method is marked `virtual` to allow implementations with asynchronous resources
     * to determine or update the current iostate.
     *
     * Method is used throughout all query members and setstate(),
     * hence they all will use the updated state from a potential override implementation.
     */
    IOState rdState();

    /** Sets state flags, by keeping its previous bits. */
    void setState(final IOState state);

    /** Checks if no error nor eof() has occurred i.e. I/O operations are available. */
    boolean good();

    /** Checks if end-of-file has been reached. */
    boolean eof();

    /** Checks if an error has occurred. */
    boolean fail();

    /** Checks if a non-recoverable error has occurred. */
    boolean bad();

    /** Checks if a timeout (non-recoverable) has occurred. */
    boolean timeout();
}
