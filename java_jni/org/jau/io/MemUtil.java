package org.jau.io;

import java.nio.ByteBuffer;

public class MemUtil {
    /**
     * Zeros all bytes of given direct NIO byte buffer.
     */
    public static native void zeroByteBuffer(final ByteBuffer buf);

    /**
     * Zeros all underlying bytes of given Java string.
     *
     * Implementation either uses the JNI GetStringCritical() with is_copy == false or the String backing array `value`.
     *
     * @return true if successful, i.e. the underlying bytes could have been zeroed, otherwise returns false w/o zero'ed content.
    public static native boolean zeroString(final String str);
     */

    /**
     * Converts the Java string to a native direct ByteBuffer as UTF-8
     * allowing better control over its memory region to {@link #zeroByteBuffer(ByteBuffer) zero} it after usage.
     *
     * @param str the string to convert
     * @param zerostr pass true to zero the bytes of the given Java string afterwards (recommended!)
     * @return direct ByteBuffer instance suitable for Cipherpack.
    public static ByteBuffer to_ByteBuffer(final String str, final boolean zerostr) {
        final ByteBuffer res = toByteBufferImpl(str);
        res.limit(res.capacity());
        res.position(0);
        if( zerostr ) {
            zeroString(str);
        }
        return res;
    }
    private static native ByteBuffer toByteBufferImpl(final String str);

    public static String to_String(final ByteBuffer bb, final Charset cs) {
        final int p0 = bb.position();
        bb.position(0);
        final byte[] bb_bytes = new byte[bb.limit()];
        bb.get(bb_bytes);
        bb.position(p0);
        return new String(bb_bytes, cs);
    }
     */

}
