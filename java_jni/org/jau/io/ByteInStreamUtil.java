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
public interface ByteInStreamUtil {
    /**
     * Stream consumer using a byte array
     */
    public static interface StreamConsumer1 {
        /**
         *
         * @param data
         * @param data_len
         * @param is_final
         * @return true to signal continuation, false to end streaming.
         */
        boolean consume(byte[] data, int data_len, boolean is_final);
    }

    /**
     * Synchronous byte array input stream reader using the given {@link StreamConsumer1}.
     *
     * To abort streaming, user may return `false` from the given {@link StreamConsumer1#consume(byte[], long, boolean)}.
     *
     * @param in the input byte stream to read from
     * @param buffer byte buffer passed down to {@link StreamConsumer1#consume(byte[], long, boolean)}
     * @param consumer StreamConsumer consumer for each received heap of bytes, returning true to continue stream of false to abort.
     * @return total bytes read or 0 if error
     */
    public static long read_stream(final ByteInStream in,
                                   final byte buffer[],
                                   final StreamConsumer1 consumer)
    {
        long total = 0;
        boolean has_more = !in.end_of_data();
        while( has_more ) {
            if( in.check_available(1) ) { // at least one byte to stream ..
                final int got = in.read(buffer, 0, buffer.length);

                total += got;
                has_more = 1 <= got && !in.end_of_data() && ( !in.has_content_size() || total < in.content_size() );
                try {
                    if( !consumer.consume(buffer, got, !has_more) ) {
                        break; // end streaming
                    }
                } catch (final Throwable e) {
                    PrintUtil.fprintf_td(System.err, "org.jau.nio.read_stream: Caught exception: %s", e.getMessage());
                    break; // end streaming
                }
            } else {
                has_more = false;
                consumer.consume(buffer, 0, true); // forced final, zero size
            }
        }
        return total;
    }

    /**
     * Stream consumer using a direct ByteBuffer
     */
    public static interface StreamConsumer2 {
        /**
         *
         * @param data
         * @param is_final
         * @return true to signal continuation, false to end streaming.
         */
        boolean consume(ByteBuffer data, boolean is_final);
    }

    /**
     * Synchronous direct ByteBuffer input stream reader using the given {@link StreamConsumer2}.
     *
     * To abort streaming, user may return `false` from the given {@link StreamConsumer2#consume(ByteBuffer, boolean)}.
     *
     * @param in the input byte stream to read from
     * @param buffer byte buffer passed down to {@link StreamConsumer2#consume(ByteBuffer, boolean)}
     * @param consumer StreamConsumer2 consumer for each received heap of bytes, returning true to continue stream of false to abort.
     * @return total bytes read or 0 if error
     */
    public static long read_stream(final ByteInStream in,
                                   final ByteBuffer buffer,
                                   final StreamConsumer2 consumer)
    {
        long total = 0;
        boolean has_more = !in.end_of_data();
        while( has_more ) {
            if( in.check_available(1) ) { // at least one byte to stream ..
                final int got = in.read(buffer);

                total += got;
                has_more = 1 <= got && !in.end_of_data() && ( !in.has_content_size() || total < in.content_size() );
                try {
                    if( !consumer.consume(buffer, !has_more) ) {
                        break; // end streaming
                    }
                } catch (final Throwable e) {
                    PrintUtil.fprintf_td(System.err, "org.jau.nio.read_stream: Caught exception: %s", e.getMessage());
                    break; // end streaming
                }
            } else {
                has_more = false;
            }
        }
        return total;
    }

    /**
     * Parses the given path_or_uri, if it matches a supported protocol, see {@link org.jau.io.UriTk#protocol_supported(String)},
     * but is not a local file, see {@link org.jau.io.UriTk#is_local_file_protocol(String)}, ByteInStream_URL is being attempted.
     *
     * If the above fails, ByteInStream_File is attempted.
     *
     * If non of the above leads to a ByteInStream without {@link ByteInStreamUtil#error()}, null is returned.
     *
     * @param path_or_uri given path or uri for with a ByteInStream instance shall be established.
     * @param timeoutMS a timeout in case ByteInStream_URL is being used as maximum duration in milliseconds to wait for next bytes at {@link ByteInStream_URL#check_available(long)}, defaults to 20_s
     * @return a working ByteInStream w/o {@link ByteInStreamUtil#error()} or nullptr
     */
    public static ByteInStream to_ByteInStream(final String path_or_uri, final long timeoutMS) {
        if( !org.jau.io.UriTk.is_local_file_protocol(path_or_uri) &&
             org.jau.io.UriTk.protocol_supported(path_or_uri) )
        {
            final ByteInStream res = new ByteInStream_URL(path_or_uri, timeoutMS);
            if( null != res && !res.error() ) {
                return res;
            }
        }
        final ByteInStream res = new ByteInStream_File(path_or_uri);
        if( null != res && !res.error() ) {
            return res;
        }
        return null;
    }
    /**
     * Parses the given path_or_uri, if it matches a supported protocol, see {@link org.jau.io.UriTk#protocol_supported(String)},
     * but is not a local file, see {@link org.jau.io.UriTk#is_local_file_protocol(String)}, ByteInStream_URL is being attempted.
     *
     * If the above fails, ByteInStream_File is attempted.
     *
     * If non of the above leads to a ByteInStream without {@link ByteInStreamUtil#error()}, null is returned.
     *
     * Method uses a timeout of 20_s for maximum duration to wait for next bytes at {@link ByteInStream_URL#check_available(long)}
     *
     * @param path_or_uri given path or uri for with a ByteInStream instance shall be established.
     * @return a working ByteInStream w/o {@link ByteInStreamUtil#error()} or nullptr
     */
    public static ByteInStream to_ByteInStream(final String path_or_uri) {
        return to_ByteInStream(path_or_uri, 20000);
    }

    public static void print_stats(final String prefix, final long out_bytes_total, final long td_ms) {
        PrintUtil.fprintf_td(System.err, "%s: Duration %,d ms\n", prefix, td_ms);

        if( out_bytes_total >= 100000000 ) {

            PrintUtil.fprintf_td(System.err, "%s: Size %,d MB%n", prefix, Math.round(out_bytes_total/1000000.0));
        } else if( out_bytes_total >= 100000 ) {
            PrintUtil.fprintf_td(System.err, "%s: Size %,d KB%n", prefix, Math.round(out_bytes_total/1000.0));
        } else {
            PrintUtil.fprintf_td(System.err, "%s: Size %,d B%n", prefix, out_bytes_total);
        }

        final long _rate_bps = Math.round( out_bytes_total / ( td_ms / 1000.0 )); // bytes per second
        final long _rate_bitps = Math.round( ( out_bytes_total * 8.0 ) / ( td_ms / 1000.0 ) ); // bits per second

        if( _rate_bitps >= 100000000 ) {
            PrintUtil.fprintf_td(System.err, "%s: Bitrate %,d Mbit/s, %,d MB/s%n", prefix,
                    Math.round(_rate_bitps/1000000.0),
                    Math.round(_rate_bps/1000000.0));
        } else if( _rate_bitps >= 100000 ) {
            PrintUtil.fprintf_td(System.err, "%s: Bitrate %,d kbit/s, %,d kB/s%n", prefix,
                    Math.round(_rate_bitps/10000), Math.round(_rate_bps/10000));
        } else {
            PrintUtil.fprintf_td(System.err, "%s: Bitrate %,d bit/s, %,d B/s%n", prefix,
                    _rate_bitps, _rate_bps);
        }
    }
}
