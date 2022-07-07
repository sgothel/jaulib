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

import java.util.List;

/**
 * Limited URI toolkit to query handled protocols by the IO implementation.
 *
 * The URI scheme functionality exposed here is limited and only provided to decide whether the used implementation
 * is able to handle the protocol. This is not a replacement for a proper URI class.
 */
public class UriTk {
    /**
     * Returns a list of supported protocol supported by [*libcurl* network protocols](https://curl.se/docs/url-syntax.html),
     * queried at runtime.
     * @see protocol_supported()
     */
    public static native List<String> supported_protocols();

    /**
     * Returns the valid uri-scheme from given uri,
     * which is empty if no valid scheme is included.
     *
     * The given uri must included at least a colon after the uri-scheme part.
     *
     * @param uri an uri
     * @return valid uri-scheme, empty if non found
     */
    public static native String get_scheme(final String uri);

    /**
     * Returns true if the uri-scheme of given uri matches a supported by [*libcurl* network protocols](https://curl.se/docs/url-syntax.html) otherwise false.
     *
     * The uri-scheme is retrieved via get_scheme() passing given uri, hence must included at least a colon after the uri-scheme part.
     *
     * The *libcurl* supported protocols is queried at runtime, see supported_protocols().
     *
     * @param uri an uri to test
     * @return true if the uri-scheme of given uri is supported, otherwise false.
     * @see supported_protocols()
     * @see get_scheme()
     */
    public static native boolean protocol_supported(final String uri);

    /**
     * Returns true if the uri-scheme of given uri matches the local `file` protocol, i.e. starts with `file://`.
     * @param uri an uri to test
     */
    public static native boolean is_local_file_protocol(final String uri);
}
