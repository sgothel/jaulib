/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2013 Gothel Software e.K.
 * Copyright (c) 2013 JogAmp Community.
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
package org.jau.sys.elf;

import java.io.IOException;
import java.io.RandomAccessFile;

import org.jau.io.Bitstream;

class IOUtils {
    static final long MAX_INT_VALUE = ( Integer.MAX_VALUE & 0xffffffffL ) ;

    static String toHexString(final int i) { return "0x"+Integer.toHexString(i); }

    static String toHexString(final long i) { return "0x"+Long.toHexString(i); }

    static int shortToInt(final short s) {
        return s & 0x0000ffff;
    }

    static int long2Int(final long v) {
        if( MAX_INT_VALUE < v ) {
            throw new IllegalArgumentException("Read uint32 value "+toHexString(v)+" > int32-max "+toHexString(MAX_INT_VALUE));
        }
        return (int)v;
    }

    static void readBytes(final RandomAccessFile in, final byte[] out, final int offset, final int len)
            throws IOException, IllegalArgumentException
    {
        in.readFully(out, offset, len);
    }

    static void seek(final RandomAccessFile in, final long newPos) throws IOException {
        in.seek(newPos);
    }

    static int readUInt32(final boolean isBigEndian, final byte[] in, final int offset) {
        final int v = Bitstream.uint32LongToInt(Bitstream.readUInt32(isBigEndian, in, offset));
        if( 0 > v ) {
            throw new IllegalArgumentException("Read uint32 value "+toHexString(v)+" > int32-max "+toHexString(MAX_INT_VALUE));
        }
        return v;
    }

    /**
     * @param sb byte source buffer to parse
     * @param offset offset within byte source buffer to start parsing
     * @param remaining remaining numbers of bytes to parse beginning w/ <code>sb_off</code>,
     *                  which shall not exceed <code>sb.length - offset</code>.
     * @param offset_post optional integer array holding offset post parsing
     * @return the parsed string
     * @throws IndexOutOfBoundsException if <code>offset + remaining > sb.length</code>.
     */
    static String getString(final byte[] sb, final int offset, final int remaining, final int[] offset_post) throws IndexOutOfBoundsException {
        Bitstream.checkBounds(sb, offset, remaining);
        int strlen = 0;
        for(; strlen < remaining && sb[strlen + offset] != 0; strlen++) { }
        final String s = 0 < strlen ? new String(sb, offset, strlen) : "" ;
        if( null != offset_post ) {
            offset_post[0] = offset + strlen + 1; // incl. EOS
        }
        return s;
    }

    /**
     * @param sb byte source buffer to parse
     * @param offset offset within byte source buffer to start parsing
     * @param remaining remaining numbers of bytes to parse beginning w/ <code>sb_off</code>,
     *                  which shall not exceed <code>sb.length - offset</code>.
     * @return the number of parsed strings
     * @throws IndexOutOfBoundsException if <code>offset + remaining > sb.length</code>.
     */
    static int getStringCount(final byte[] sb, final int offset, final int remaining) throws IndexOutOfBoundsException {
        Bitstream.checkBounds(sb, offset, remaining);
        int strnum=0;
        for(int i=0; i < remaining; i++) {
            for(; i < remaining && sb[i + offset] != 0; i++) { }
            strnum++;
        }
        return strnum;
    }

    /**
     * @param sb byte source buffer to parse
     * @param offset offset within byte source buffer to start parsing
     * @param remaining remaining numbers of bytes to parse beginning w/ <code>sb_off</code>,
     *                  which shall not exceed <code>sb.length - offset</code>.
     * @return the parsed strings
     * @throws IndexOutOfBoundsException if <code>offset + remaining > sb.length</code>.
     */
    public static String[] getStrings(final byte[] sb, final int offset, final int remaining) throws IndexOutOfBoundsException  {
        final int strnum = getStringCount(sb, offset, remaining);
        // System.err.println("XXX: strnum "+strnum+", sb_off "+sb_off+", sb_len "+sb_len);

        final String[] sa = new String[strnum];
        final int[] io_off = new int[] { offset };
        for(int i=0; i < strnum; i++) {
            // System.err.print("XXX: str["+i+"] ["+io_off[0]);
            sa[i] = getString(sb, io_off[0], remaining - io_off[0], io_off);
            // System.err.println(".. "+io_off[0]+"[ "+sa[i]);
        }
        return sa;
    }

}
