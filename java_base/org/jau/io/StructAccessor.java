/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Author: Kenneth Bradley Russell
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2015 Gothel Software e.K.
 * Copyright (c) 2015 JogAmp Community.
 * Copyright (c) 2003 Sun Microsystems.
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
import java.nio.ByteOrder;

public class StructAccessor {

    private final ByteBuffer bb;

    public StructAccessor(final ByteBuffer bb) {
        // Setting of byte order is concession to native code which needs
        // to instantiate these
        this.bb = bb.order(ByteOrder.nativeOrder());
    }

    public final ByteBuffer getBuffer() {
        return bb;
    }

    /**
     * Returns a slice of the current ByteBuffer starting at the
     * specified byte offset and extending the specified number of
     * bytes. Note that this method is not thread-safe with respect to
     * the other methods in this class.
     */
    public final ByteBuffer slice(final int byteOffset, final int byteLength) {
        bb.position(byteOffset);
        bb.limit(byteOffset + byteLength);
        final ByteBuffer newBuf = bb.slice().order(bb.order()); // slice and duplicate may change byte order
        bb.position(0);
        bb.limit(bb.capacity());
        return newBuf;
    }

    /** Retrieves the byte at the specified byteOffset. */
    public final byte getByteAt(final int byteOffset) {
        return bb.get(byteOffset);
    }

    /** Puts a byte at the specified byteOffset. */
    public final void setByteAt(final int byteOffset, final byte v) {
        bb.put(byteOffset, v);
    }

    /** Retrieves the boolean at the specified byteOffset. */
    public final boolean getBooleanAt(final int byteOffset) {
        return (byte)0 != bb.get(byteOffset);
    }

    /** Puts a boolean at the specified byteOffset. */
    public final void setBooleanAt(final int byteOffset, final boolean v) {
        bb.put(byteOffset, v?(byte)1:(byte)0);
    }

    /** Retrieves the char at the specified byteOffset. */
    public final char getCharAt(final int byteOffset) {
        return bb.getChar(byteOffset);
    }

    /** Puts a char at the specified byteOffset. */
    public final void setCharAt(final int byteOffset, final char v) {
        bb.putChar(byteOffset, v);
    }

    /** Retrieves the short at the specified byteOffset. */
    public final short getShortAt(final int byteOffset) {
        return bb.getShort(byteOffset);
    }

    /** Puts a short at the specified byteOffset. */
    public final void setShortAt(final int byteOffset, final short v) {
        bb.putShort(byteOffset, v);
    }

    /** Retrieves the int at the specified byteOffset. */
    public final int getIntAt(final int byteOffset) {
        return bb.getInt(byteOffset);
    }

    /** Puts a int at the specified byteOffset. */
    public final void setIntAt(final int byteOffset, final int v) {
        bb.putInt(byteOffset, v);
    }

    /** Retrieves the int at the specified byteOffset. */
    public final int getIntAt(final int byteOffset, final int nativeSizeInBytes) {
        switch(nativeSizeInBytes) {
            case 2:
                return bb.getShort(byteOffset) & 0x0000FFFF ;
            case 4:
                return bb.getInt(byteOffset);
            case 8:
                return (int) ( bb.getLong(byteOffset) & 0x00000000FFFFFFFFL ) ;
            default:
                throw new InternalError("invalid nativeSizeInBytes "+nativeSizeInBytes);
        }
    }

    /** Puts a int at the specified byteOffset. */
    public final void setIntAt(final int byteOffset, final int v, final int nativeSizeInBytes) {
        switch(nativeSizeInBytes) {
            case 2:
                bb.putShort(byteOffset, (short) ( v & 0x0000FFFF ) );
                break;
            case 4:
                bb.putInt(byteOffset, v);
                break;
            case 8:
                bb.putLong(byteOffset, v & 0x00000000FFFFFFFFL );
                break;
            default:
                throw new InternalError("invalid nativeSizeInBytes "+nativeSizeInBytes);
        }
    }

    /** Retrieves the float at the specified byteOffset. */
    public final float getFloatAt(final int byteOffset) {
        return bb.getFloat(byteOffset);
    }

    /** Puts a float at the specified byteOffset. */
    public final void setFloatAt(final int byteOffset, final float v) {
        bb.putFloat(byteOffset, v);
    }

    /** Retrieves the double at the specified byteOffset. */
    public final double getDoubleAt(final int byteOffset) {
        return bb.getDouble(byteOffset);
    }

    /** Puts a double at the specified byteOffset. */
    public final void setDoubleAt(final int byteOffset, final double v) {
        bb.putDouble(byteOffset, v);
    }

    /** Retrieves the long at the specified byteOffset. */
    public final long getLongAt(final int byteOffset) {
        return bb.getLong(byteOffset);
    }

    /** Puts a long at the specified byteOffset. */
    public final void setLongAt(final int byteOffset, final long v) {
        bb.putLong(byteOffset, v);
    }

    /** Retrieves the long at the specified byteOffset. */
    public final long getLongAt(final int byteOffset, final int nativeSizeInBytes) {
        switch(nativeSizeInBytes) {
            case 4:
                return bb.getInt(byteOffset) & 0x00000000FFFFFFFFL;
            case 8:
                return bb.getLong(byteOffset);
            default:
                throw new InternalError("invalid nativeSizeInBytes "+nativeSizeInBytes);
        }
    }

    /** Puts a long at the specified byteOffset. */
    public final void setLongAt(final int byteOffset, final long v, final int nativeSizeInBytes) {
        switch(nativeSizeInBytes) {
            case 4:
                bb.putInt(byteOffset, (int) ( v & 0x00000000FFFFFFFFL ) );
                break;
            case 8:
                bb.putLong(byteOffset, v);
                break;
            default:
                throw new InternalError("invalid nativeSizeInBytes "+nativeSizeInBytes);
        }
    }

    public final void setBytesAt(int byteOffset, final byte[] v) {
        for (int i = 0; i < v.length; i++) {
            bb.put(byteOffset++, v[i]);
        }
    }

    public final byte[] getBytesAt(int byteOffset, final byte[] v) {
        for (int i = 0; i < v.length; i++) {
            v[i] = bb.get(byteOffset++);
        }
        return v;
    }

    public final void setBooleansAt(int byteOffset, final boolean[] v) {
        for (int i = 0; i < v.length; i++) {
            bb.put(byteOffset++, v[i]?(byte)1:(byte)0);
        }
    }

    public final boolean[] getBooleansAt(int byteOffset, final boolean[] v) {
        for (int i = 0; i < v.length; i++) {
            v[i] = (byte)0 != bb.get(byteOffset++);
        }
        return v;
    }

    public final void setCharsAt(int byteOffset, final char[] v) {
        for (int i = 0; i < v.length; i++, byteOffset+=2) {
            bb.putChar(byteOffset, v[i]);
        }
    }

    public final char[] getCharsAt(int byteOffset, final char[] v) {
        for (int i = 0; i < v.length; i++, byteOffset+=2) {
            v[i] = bb.getChar(byteOffset);
        }
        return v;
    }

    public final void setShortsAt(int byteOffset, final short[] v) {
        for (int i = 0; i < v.length; i++, byteOffset+=2) {
            bb.putShort(byteOffset, v[i]);
        }
    }

    public final short[] getShortsAt(int byteOffset, final short[] v) {
        for (int i = 0; i < v.length; i++, byteOffset+=2) {
            v[i] = bb.getShort(byteOffset);
        }
        return v;
    }

    public final void setIntsAt(int byteOffset, final int[] v) {
        for (int i = 0; i < v.length; i++, byteOffset+=4) {
            bb.putInt(byteOffset, v[i]);
        }
    }

    public final int[] getIntsAt(int byteOffset, final int[] v) {
        for (int i = 0; i < v.length; i++, byteOffset+=4) {
            v[i] = bb.getInt(byteOffset);
        }
        return v;
    }

    public final void setFloatsAt(int byteOffset, final float[] v) {
        for (int i = 0; i < v.length; i++, byteOffset+=4) {
            bb.putFloat(byteOffset, v[i]);
        }
    }

    public final float[] getFloatsAt(int byteOffset, final float[] v) {
        for (int i = 0; i < v.length; i++, byteOffset+=4) {
            v[i] = bb.getFloat(byteOffset);
        }
        return v;
    }

    public final void setDoublesAt(int byteOffset, final double[] v) {
        for (int i = 0; i < v.length; i++, byteOffset+=8) {
            bb.putDouble(byteOffset, v[i]);
        }
    }

    public final double[] getDoublesAt(int byteOffset, final double[] v) {
        for (int i = 0; i < v.length; i++, byteOffset+=8) {
            v[i] = bb.getDouble(byteOffset);
        }
        return v;
    }

    public final void setLongsAt(int byteOffset, final long[] v) {
        for (int i = 0; i < v.length; i++, byteOffset+=8) {
            bb.putLong(byteOffset, v[i]);
        }
    }

    public final long[] getLongsAt(int byteOffset, final long[] v) {
        for (int i = 0; i < v.length; i++, byteOffset+=8) {
            v[i] = bb.getLong(byteOffset);
        }
        return v;
    }
}
