/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2020 ZAFENA AB
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
package org.jau.net;

import org.jau.util.BasicTypes;
import java.nio.ByteOrder;

/**
 * A packed 48 bit EUI-48 identifier, formerly known as MAC-48
 * or simply network device MAC address (Media Access Control address).
 * <p>
 * Stores value in {@link ByteOrder#nativeOrder()} byte order.
 * </p>
 * <p>
 * Implementation caches the hash value {@link #hashCode()},
 * hence users shall take special care when mutating the
 * underlying data {@link #b}, read its API notes.
 * </p>
 */
public class EUI48 {
    /** EUI48 MAC address matching any device, i.e. '0:0:0:0:0:0'. */
    public static final EUI48 ANY_DEVICE = new EUI48( new byte[] { (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00 }, ByteOrder.LITTLE_ENDIAN );
    /** EUI48 MAC address matching all device, i.e. 'ff:ff:ff:ff:ff:ff'. */
    public static final EUI48 ALL_DEVICE = new EUI48( new byte[] { (byte)0xff, (byte)0xff, (byte)0xff, (byte)0xff, (byte)0xff, (byte)0xff }, ByteOrder.LITTLE_ENDIAN );
    /** EUI48 MAC address matching local device, i.e. '0:0:0:ff:ff:ff'. */
    public static final EUI48 LOCAL_DEVICE = new EUI48( new byte[] { (byte)0x00, (byte)0x00, (byte)0x00, (byte)0xff, (byte)0xff, (byte)0xff }, ByteOrder.LITTLE_ENDIAN );

    /**
     * The 6 byte EUI48 address.
     * <p>
     * If modifying, it is the user's responsibility to avoid data races.<br>
     * Further, call {@link #clearHash()} after mutation is complete.
     * </p>
     */
    public final byte b[/* 6 octets */];

    private volatile int hash; // default 0, cache

    /**
     * Size of the byte stream representation in bytes
     * @see #put(byte[], int)
     */
    /* pp */ static final int byte_size = 6;

    /**
     * Fills given EUI48 instance via given string representation.
     * <p>
     * Implementation is consistent with {@link #toString()}.
     * </p>
     * @param str a string of exactly 17 characters representing 6 bytes as hexadecimal numbers separated via colon {@code "01:02:03:0A:0B:0C"}.
     * @param dest EUI48 to set its value
     * @param errmsg error parsing message if returning false
     * @return true if successful, otherwise false
     * @see #EUI48(String)
     * @see #toString()
     */
    public static boolean scanEUI48(final String str, final EUI48 dest, final StringBuilder errmsg) {
        if( 17 != str.length() ) {
            errmsg.append("EUI48 string not of length 17 but "+str.length()+": "+str);
            return false;
        }
        try {
            if( ByteOrder.LITTLE_ENDIAN == ByteOrder.nativeOrder() ) {
                for(int i=0; i<byte_size; i++) {
                    dest.b[byte_size-1-i] = Integer.valueOf(str.substring(i*2+i, i*2+i+2), 16).byteValue();
                }
            } else {
                for(int i=0; i<byte_size; i++) {
                    dest.b[i] = Integer.valueOf(str.substring(i*2+i, i*2+i+2), 16).byteValue();
                }
            }
        } catch (final NumberFormatException e) {
            errmsg.append("EUI48 string not in format '01:02:03:0A:0B:0C' but "+str+"; "+e.getMessage());
            return false;
        }
        return true;
    }

    /**
     * Construct instance via given string representation.
     * <p>
     * Implementation is consistent with {@link #toString()}.
     * </p>
     * @param str a string of exactly 17 characters representing 6 bytes as hexadecimal numbers separated via colon {@code "01:02:03:0A:0B:0C"}.
     * @see #scanEUI48(String, byte[], StringBuilder)
     * @see #toString()
     * @throws IllegalArgumentException if given string doesn't comply with EUI48
     */
    public EUI48(final String str) throws IllegalArgumentException {
        final StringBuilder errmsg = new StringBuilder();
        b = new byte[byte_size];
        if( !scanEUI48(str, this, errmsg) ) {
            throw new IllegalArgumentException(errmsg.toString());
        }
    }

    /**
     * Copy address bytes from given source and store it in {@link ByteOrder#nativeOrder()} byte order.
     *
     * If given address bytes are not in {@link ByteOrder#nativeOrder()} byte order,
     * they are swapped.
     *
     * @param stream address bytes
     * @param pos position in stream at address
     * @param byte_order {@link ByteOrder#LITTLE_ENDIAN} or {@link ByteOrder#BIG_ENDIAN} byte order
     */
    public EUI48(final byte stream[], final int pos, final ByteOrder byte_order) {
        if( byte_size > ( stream.length - pos ) ) {
            throw new IllegalArgumentException("EUI48 stream ( "+stream.length+" - "+pos+" ) < "+byte_size+" bytes");
        }
        b = new byte[byte_size];
        if( byte_order == ByteOrder.nativeOrder() ) {
            System.arraycopy(stream, pos, b, 0,  byte_size);
        } else {
            BasicTypes.bswap_6bytes(stream, pos, b, 0);
        }
    }

    /**
     * Copy address bytes from given source and store it in {@link ByteOrder#nativeOrder()} byte order.
     *
     * If given address bytes are not in {@link ByteOrder#nativeOrder()} byte order,
     * they are swapped.
     *
     * @param address address bytes
     * @param byte_order {@link ByteOrder#LITTLE_ENDIAN} or {@link ByteOrder#BIG_ENDIAN} byte order
     */
    public EUI48(final byte address[], final ByteOrder byte_order) {
        if( byte_size != address.length ) {
            throw new IllegalArgumentException("EUI48 stream "+address.length+" != "+byte_size+" bytes");
        }
        b = new byte[byte_size];
        if( byte_order == ByteOrder.nativeOrder() ) {
            System.arraycopy(address, 0, b, 0,  byte_size);
        } else {
            BasicTypes.bswap_6bytes(address, 0, b, 0);
        }
    }

    /** Construct empty unset instance. */
    public EUI48() {
        b = new byte[byte_size];
    }

    @Override
    public final boolean equals(final Object obj) {
        if(this == obj) {
            return true;
        }
        if (obj == null || !(obj instanceof EUI48)) {
            return false;
        }
        final byte[] b2 = ((EUI48)obj).b;
        return b[0] == b2[0] &&
               b[1] == b2[1] &&
               b[2] == b2[2] &&
               b[3] == b2[3] &&
               b[4] == b2[4] &&
               b[5] == b2[5];
    }

    /**
     * {@inheritDoc}
     * <p>
     * Implementation uses a lock-free volatile cache.
     * </p>
     * @see #clearHash()
     */
    @Override
    public final int hashCode() {
        int h = hash;
        if( 0 == h ) {
            /**
            // final int p = 92821; // alternative with less collisions?
            final int p = 31; // traditional prime
            h = b[0];
            h = p * h + b[1];
            h = p * h + b[2];
            h = p * h + b[3];
            h = p * h + b[4];
            h = p * h + b[5];
            */
            // 31 * x == (x << 5) - x
            h = b[0];
            h = ( ( h << 5 ) - h ) + b[1];
            h = ( ( h << 5 ) - h ) + b[2];
            h = ( ( h << 5 ) - h ) + b[3];
            h = ( ( h << 5 ) - h ) + b[4];
            h = ( ( h << 5 ) - h ) + b[5];
            hash = h;
        }
        return h;
    }

    /**
     * Method clears the cached hash value.
     * @see #clear()
     */
    public void clearHash() { hash = 0; }

    /**
     * Method clears the underlying byte array {@link #b} and cached hash value.
     * @see #clearHash()
     */
    public void clear() {
        hash = 0;
        b[0] = 0; b[1] = 0; b[2] = 0;
        b[3] = 0; b[4] = 0; b[5] = 0;
    }

    /**
     * Method transfers all bytes representing this instance into the given
     * destination array at the given position and in the given byte order.
     * <p>
     * Implementation is consistent with {@link #EUI48(byte[], int, ByteOrder)}.
     * </p>
     * @param sink the destination array
     * @param sink_pos starting position in the destination array
     * @param byte_order destination buffer byte order
     * @throws IllegalArgumentException
     * @see #EUI48(byte[], int, ByteOrder)
     */
    public final void put(final byte[] sink, final int sink_pos, final ByteOrder byte_order) {
        if( byte_size > ( sink.length - sink_pos ) ) {
            throw new IllegalArgumentException("Stream ( "+sink.length+" - "+sink_pos+" ) < "+byte_size+" bytes");
        }
        if( byte_order == ByteOrder.nativeOrder() ) {
            System.arraycopy(b, 0, sink, sink_pos, byte_size);
        } else {
            BasicTypes.bswap_6bytes(b, 0, sink, sink_pos);
        }
    }

    /**
     * Finds the index of given EUI48Sub needle within this instance haystack in the given byte order.
     *
     * The returned index will be adjusted for the desired byte order.
     * - {@link ByteOrder#BIG_ENDIAN} will return index 0 for the leading byte like the {@link #toString()} representation from left (MSB) to right (LSB).
     * - {@link ByteOrder#LITTLE_ENDIAN} will return index 5 for the leading byte
     *
     * @param needle
     * @param byte_order byte order will adjust the returned index, {@link ByteOrder#BIG_ENDIAN} is equivalent with {@link #toString()} representation from left (MSB) to right (LSB).
     * @return index of first element of needle within this instance haystack or -1 if not found. If the needle length is zero, 0 (found) is returned.
     * @see #indexOf(byte[], int, byte[], int, ByteOrder)
     */
    public int indexOf(final EUI48Sub needle, final ByteOrder byte_order) {
        return EUI48Sub.indexOf(b, 6, needle.b, needle.length, byte_order);
    }

    /**
     * Returns true, if given EUI48Sub is contained in here.
     * <p>
     * If the sub is zero, true is returned.
     * </p>
     */
    public boolean contains(final EUI48Sub needle) {
        return 0 <= indexOf(needle, ByteOrder.nativeOrder());
    }

    /**
     * {@inheritDoc}
     * <p>
     * Returns the EUI48 string representation,
     * exactly 17 characters representing 6 bytes as upper case hexadecimal numbers separated via colon {@code "01:02:03:0A:0B:0C"}.
     * </p>
     * @see #EUI48(String)
     */
    @Override
    public final String toString() {
        final StringBuilder sb = new StringBuilder(17);
        if( ByteOrder.LITTLE_ENDIAN == ByteOrder.nativeOrder() ) {
            BasicTypes.byteHexString(sb, b[5], false /* lowerCase */);
            sb.append(":");
            BasicTypes.byteHexString(sb, b[4], false /* lowerCase */);
            sb.append(":");
            BasicTypes.byteHexString(sb, b[3], false /* lowerCase */);
            sb.append(":");
            BasicTypes.byteHexString(sb, b[2], false /* lowerCase */);
            sb.append(":");
            BasicTypes.byteHexString(sb, b[1], false /* lowerCase */);
            sb.append(":");
            BasicTypes.byteHexString(sb, b[0], false /* lowerCase */);
        } else {
            BasicTypes.byteHexString(sb, b[0], false /* lowerCase */);
            sb.append(":");
            BasicTypes.byteHexString(sb, b[1], false /* lowerCase */);
            sb.append(":");
            BasicTypes.byteHexString(sb, b[2], false /* lowerCase */);
            sb.append(":");
            BasicTypes.byteHexString(sb, b[3], false /* lowerCase */);
            sb.append(":");
            BasicTypes.byteHexString(sb, b[4], false /* lowerCase */);
            sb.append(":");
            BasicTypes.byteHexString(sb, b[5], false /* lowerCase */);
        }
        return sb.toString();
    }
}
