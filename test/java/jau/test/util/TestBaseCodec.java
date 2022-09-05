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

package jau.test.util;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

import org.jau.util.BaseCodec;
import org.jau.util.BasicTypes;
import org.jau.util.BaseCodec.Alphabet;
import org.junit.Assert;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestBaseCodec {

    static final Alphabet natural86_alphabet = new BaseCodec.Natural86Alphabet();

    private static void testRadix_3digits_int32(final int base, final Alphabet aspec) {
        Assert.assertTrue( 1 < base );
        Assert.assertTrue( base <= aspec.max_base() );

        final char min_cp = aspec.charAt(0); // minimum code-point
        final char max_cp = aspec.charAt(base-1); // maximum code-point

        final int min = (int)BaseCodec.decode(""+min_cp, base, aspec);
        final int max = (int)BaseCodec.decode(""+max_cp+max_cp+max_cp, base, aspec);
        final int max_s = (int)BaseCodec.decode(""+max_cp, base, aspec);

        final double machine_epsilon = BasicTypes.machineEpsilonDouble();
        Assert.assertEquals(0, min);
        Assert.assertEquals(base-1, max_s);
        Assert.assertTrue( Math.abs( Math.pow(base, 3)-1 - max ) <=  machine_epsilon );

        final String r1_min = BaseCodec.encode(0, base, aspec, 3);
        final String r1_min_s = BaseCodec.encode(0, base, aspec);
        Assert.assertEquals(""+min_cp+min_cp+min_cp, r1_min);
        Assert.assertEquals(""+min_cp, r1_min_s);

        final String r1_max = BaseCodec.encode(base-1, base, aspec, 3);
        final String r1_max_s = BaseCodec.encode(base-1, base, aspec);
        Assert.assertEquals(""+min_cp+min_cp+max_cp, r1_max);
        Assert.assertEquals(""+max_cp, r1_max_s);

        final String r3_max = BaseCodec.encode((int)Math.pow(base, 3)-1, base, aspec, 3);
        Assert.assertEquals(""+max_cp+max_cp+max_cp, r3_max);

        System.err.printf("Test base %d, %s: [%d .. %d] <-> ['%s' .. '%s'], %d years (max/365d)\n",
                base, aspec, min, max,
                BaseCodec.encode(min, base, aspec),
                BaseCodec.encode(max, base, aspec), (max/365));

        Assert.assertEquals(0, BaseCodec.decode(""+min_cp+min_cp+min_cp, base, aspec));
        Assert.assertEquals(""+min_cp, BaseCodec.encode(0, base, aspec));
        Assert.assertEquals(""+min_cp+min_cp+min_cp, BaseCodec.encode(0, base, aspec, 3));

        Assert.assertEquals(max, BaseCodec.decode(""+max_cp+max_cp+max_cp, base, aspec));
        Assert.assertEquals(""+max_cp+max_cp+max_cp, BaseCodec.encode(max, base, aspec, 3));
        Assert.assertEquals(max_s, BaseCodec.decode(""+max_cp, base, aspec));
        Assert.assertEquals(""+min_cp+min_cp+max_cp, BaseCodec.encode(max_s, base, aspec, 3));

        {
            final int v0_d = (int)BaseCodec.decode(r1_max, base, aspec);
            final String v1_s = BaseCodec.encode(base-1, base, aspec, 3);
            // System.err.printf("r1_max '%s' ('%s'), base-1 %d (%d)\n", r1_max, v1_s, base-1, v0_d);
            Assert.assertEquals(r1_max, v1_s);
            Assert.assertEquals(base-1, v0_d);
        }
        {
            final int v0_d = (int)BaseCodec.decode(r3_max, base, aspec);
            final String v1_s = BaseCodec.encode(max, base, aspec, 3);
            Assert.assertEquals(r3_max, v1_s);
            Assert.assertEquals(max, v0_d);
        }
        for(int iter=min; iter<=max; ++iter) {
            final String rad = BaseCodec.encode(iter, base, aspec, 3);
            final int dec = (int)BaseCodec.decode(rad, base, aspec);
            Assert.assertEquals(iter, dec);
        }
        if( natural86_alphabet.equals( aspec ) ) {
            // Test 0-9 ..
            System.err.printf("Natural 0-9: ");
            for(int iter=0; iter<=9; ++iter) {
                final String rad = BaseCodec.encode(iter, base, aspec);
                System.err.printf("%s, ", rad);
                final char c = (char)('0'+iter);
                Assert.assertEquals(""+c, rad);
            }
            System.err.println();
        }
    }

    private static void testRadix_int64(final int base, final Alphabet aspec, final long test_min, final long test_max) {
        final int int64_max_enc_width = 11; // 9223372036854775807 ==  '7__________' (base 64, natural)

        Assert.assertTrue( 1 < base );
        Assert.assertTrue( base <= aspec.max_base() );

        final char min_cp = aspec.charAt(0); // minimum code-point
        final char max_cp = aspec.charAt(base-1); // maximum code-point

        final String max_radix = BaseCodec.encode(Long.MAX_VALUE, base, aspec, int64_max_enc_width);

        final long min = BaseCodec.decode(""+min_cp, base, aspec);
        final long max = BaseCodec.decode(max_radix, base, aspec);
        final long max_s = BaseCodec.decode(""+max_cp, base, aspec);

        Assert.assertEquals(0, min);
        Assert.assertEquals(base-1, max_s);
        Assert.assertEquals(Long.MAX_VALUE, max);

        final String r1_min = BaseCodec.encode(0, base, aspec, int64_max_enc_width);
        final String r1_min_s = BaseCodec.encode(0, base, aspec);
        Assert.assertEquals(""+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp, r1_min);
        Assert.assertEquals(""+min_cp, r1_min_s);

        final String r1_max = BaseCodec.encode(base-1, base, aspec, int64_max_enc_width);
        final String r1_max_s = BaseCodec.encode(base-1, base, aspec);
        Assert.assertEquals(""+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+max_cp, r1_max);
        Assert.assertEquals(""+max_cp, r1_max_s);

        System.err.printf("Test base %d, %s: [%d .. %d] <-> ['%s' .. '%s'], %d years (max/365d)\n",
                base, aspec, min, max,
                BaseCodec.encode(min, base, aspec),
                BaseCodec.encode(max, base, aspec), (max/365));

        System.err.printf("- range: [%d .. %d] <-> ['%s' .. '%s']\n",
                test_min, test_max,
                BaseCodec.encode(test_min, base, aspec), BaseCodec.encode(test_max, base, aspec));

        Assert.assertEquals(0, BaseCodec.decode(""+min_cp+min_cp+min_cp, base, aspec));
        Assert.assertEquals(""+min_cp, BaseCodec.encode(0, base, aspec));

        {
            final long v0_d = BaseCodec.decode(r1_max, base, aspec);
            final String v1_s = BaseCodec.encode(base-1, base, aspec, int64_max_enc_width);
            Assert.assertEquals(r1_max, v1_s);
            Assert.assertEquals(base-1, v0_d);
        }
        for(long iter=Math.max(0L, test_min-1); iter<test_max; ) {
            ++iter;
            final String rad = BaseCodec.encode(iter, base, aspec, int64_max_enc_width);
            final long dec = BaseCodec.decode(rad, base, aspec);
            if( false ) {
                System.err.printf("test base %d: iter %d, rad '%s', dec %d\n", base, iter, rad, dec);
            }
            Assert.assertEquals(iter, dec);
        }
    }

    private static void testIntegerBase64(final Alphabet alphabet) {
        testRadix_3digits_int32(64, alphabet);
        testRadix_int64(64, alphabet, 0x7fffff00L, 0x80000100L);
        testRadix_int64(64, alphabet, 0xFFFFFFF0L, 0x100000010L);
        testRadix_int64(64, alphabet, 0x7FFFFFFFFFFFFFF0L, 0x7FFFFFFFFFFFFFFFL);
    }

    private static void testIntegerBase86(final Alphabet alphabet) {
        testRadix_3digits_int32(86, alphabet);
        testRadix_int64(86, alphabet, 0x7fffff00L, 0x80000100L);
        testRadix_int64(86, alphabet, 0xFFFFFFF0L, 0x100000010L);
        testRadix_int64(86, alphabet, 0x7FFFFFFFFFFFFFF0L, 0x7FFFFFFFFFFFFFFFL);
    }

    @Test
    public void test01IntegerBase64() {
        testIntegerBase64(new BaseCodec.Base64Alphabet());
        testIntegerBase64(new BaseCodec.Base64urlAlphabet());
        testIntegerBase64(new BaseCodec.Natural86Alphabet());
        testIntegerBase64(new BaseCodec.Ascii64Alphabet());
        testIntegerBase64(new BaseCodec.Ascii86Alphabet());
    }

    @Test
    public void test02IntegerBase86() {
        testIntegerBase86(new BaseCodec.Natural86Alphabet());
        testIntegerBase86(new BaseCodec.Ascii86Alphabet());
    }

    public static class Base64AlphabetNopadding extends Alphabet {
        private static final String data = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        @Override
        public int code_point(final char c) {
                if ('A' <= c && c <= 'Z') {
                    return c - 'A';
                } else if ('a' <= c && c <= 'z') {
                    return c - 'a' + 26;
                } else if ('0' <= c && c <= '9') {
                    return c - '0' + 52;
                } else if ('+' == c) {
                    return 62;
                } else if ('/' == c) {
                    return 63;
                } else {
                    return -1;
                }
            }

        public Base64AlphabetNopadding() {
            super("base64", 64, data, (char)0);
        }
    }

    private static byte[] to_array(final ByteBuffer bb) {
        final byte[] res = new byte[bb.remaining()];
        bb.get(bb.position(), res, 0, res.length);
        return res;
    }
    private static void testBinaryBase64() {
        final BaseCodec.Base64Alphabet aspec = new BaseCodec.Base64Alphabet();
        final BaseCodec.Base64urlAlphabet aspec_url = new BaseCodec.Base64urlAlphabet();
        final Base64AlphabetNopadding aspec_nopadding = new Base64AlphabetNopadding();

        // Test Vectors taken from `base64` [RFC 4648](https://www.rfc-editor.org/rfc/rfc4648.html)
        {
            final byte[] octets = { };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( "", encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
            Assert.assertEquals( ByteBuffer.wrap(octets), dec_octets );
        }
        {
            final byte[] octets = { 'f' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( "Zg==", encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
            Assert.assertEquals( ByteBuffer.wrap(octets), dec_octets );
        }
        {
            final byte[] octets = { 'f', 'o' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( "Zm8=", encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }
        {
            final byte[] octets = { 'f', 'o', 'o' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( "Zm9v", encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }
        {
            final byte[] octets = { 'f', 'o', 'o', 'b' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( "Zm9vYg==", encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }
        {
            final byte[] octets = { 'f', 'o', 'o', 'b', 'a' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( "Zm9vYmE=", encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }

        // Further encoding tests
        {
            final byte[] octets = { 'a' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( "YQ==", encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }
        {
            final byte[] octets = { 'a', 'b' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( "YWI=", encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }
        {
            final byte[] octets = { 'a', 'b', 'c' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( "YWJj", encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }
        {
            final byte[] octets = { 'a', 'b', 'c', 'd' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( "YWJjZA==", encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }
        {
            final byte[] octets = { 'a', 'b', 'c', 'd', 'e' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( "YWJjZGU=", encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }
        {
            final byte[] octets = { 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( "YWJjZGVmZw==", encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }
        {
            // Test no-padding accept and error, double padding dropped '=='
            final byte[] octets = { 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec_nopadding).toString();
            Assert.assertEquals( "YWJjZGVmZw", encstr);
            {
                // accept no padding
                final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec_nopadding);
                Assert.assertArrayEquals( octets, to_array( dec_octets ) );
            }
            {
                // not accepting lack of padding
                final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
                Assert.assertEquals( 0, dec_octets.remaining() );
            }
        }
        {
            // Test no-padding accept and error, double padding dropped '=='
            final byte[] octets = { 'a' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec_nopadding).toString();
            Assert.assertEquals( "YQ", encstr);
            {
                // accept no padding
                final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec_nopadding);
                Assert.assertArrayEquals( octets, to_array( dec_octets ) );
            }
            {
                // not accepting lack of padding
                final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
                Assert.assertEquals( 0, dec_octets.remaining() );
            }
        }
        {
            // Test no-padding accept and error, single padding dropped '='
            final byte[] octets = { 'a', 'b', 'c', 'd', 'e' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec_nopadding).toString();
            Assert.assertEquals( "YWJjZGU", encstr);
            {
                // accept no padding
                final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec_nopadding);
                Assert.assertArrayEquals( octets, to_array( dec_octets ) );
            }
            {
                // not accepting lack of padding
                final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
                Assert.assertEquals( 0, dec_octets.remaining() );
            }
        }
        {
            // Test no-padding accept and error, single padding dropped '='
            final byte[] octets = { 'a', 'b' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec_nopadding).toString();
            Assert.assertEquals( "YWI", encstr);
            {
                // accept no padding
                final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec_nopadding);
                Assert.assertArrayEquals( octets, to_array( dec_octets ) );
            }
            {
                // not accepting lack of padding
                final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
                Assert.assertEquals( 0, dec_octets.remaining() );
            }
        }
        {
            // Test no-padding accept and error, zero padding dropped
            final byte[] octets = { 'a', 'b', 'c' };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec_nopadding).toString();
            Assert.assertEquals( "YWJj", encstr);
            {
                // accept no padding
                final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec_nopadding);
                Assert.assertArrayEquals( octets, to_array( dec_octets ) );
            }
            {
                // accept no padding
                final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
                Assert.assertArrayEquals( octets, to_array( dec_octets ) );
            }
        }
        {
            final String in_str = "aaaaaaaaaaaaaaaaa"; // a17
            final byte[] octets = in_str.getBytes(StandardCharsets.UTF_8);
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( "YWFhYWFhYWFhYWFhYWFhYWE=", encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }

        {
            // Test code-points 63 and 64 of base64
            final byte[] octets = { (byte)0x03, (byte)0xef, (byte)0xff, (byte)0xf9 };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( "A+//+Q==", encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }
        {
            // Test code-points 63 and 64 of base64url
            final byte[] octets = { (byte)0x03, (byte)0xef, (byte)0xff, (byte)0xf9 };
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec_url).toString();
            Assert.assertEquals( "A-__-Q==", encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec_url);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }

        {
            final String in_str = "one two three four five six seven eight nine ten eleven twelve thirteen fourteen fivteen sixteen seventeen eighteen nineteen twenty twenty-one";
            final String exp_encstr = "b25lIHR3byB0aHJlZSBmb3VyIGZpdmUgc2l4IHNldmVuIGVpZ2h0IG5pbmUgdGVuIGVsZXZlbiB0"+
                                      "d2VsdmUgdGhpcnRlZW4gZm91cnRlZW4gZml2dGVlbiBzaXh0ZWVuIHNldmVudGVlbiBlaWdodGVl"+
                                      "biBuaW5ldGVlbiB0d2VudHkgdHdlbnR5LW9uZQ==";
            final byte[] octets = in_str.getBytes(StandardCharsets.UTF_8);
            final String encstr = BaseCodec.encode64(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( exp_encstr, encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }
        {
            final String in_str = "one two three four five six seven eight nine ten eleven twelve thirteen fourteen fivteen sixteen seventeen eighteen nineteen twenty twenty-one";
            final String exp_encstr = "b25lIHR3byB0aHJlZSBmb3VyIGZpdmUgc2l4IHNldmVuIGVpZ2h0IG5pbmUgdGVuIGVsZXZlbiB0\n"+
                                      "d2VsdmUgdGhpcnRlZW4gZm91cnRlZW4gZml2dGVlbiBzaXh0ZWVuIHNldmVudGVlbiBlaWdodGVl\n"+
                                      "biBuaW5ldGVlbiB0d2VudHkgdHdlbnR5LW9uZQ==";
            final byte[] octets = in_str.getBytes(StandardCharsets.UTF_8);
            final String encstr = BaseCodec.encode64_mime(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( exp_encstr, encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64_lf(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }
        {
            final String in_str = "one two three four five six seven eight nine ten eleven twelve thirteen fourteen fivteen sixteen seventeen eighteen nineteen twenty twenty-one";
            final String exp_encstr = "b25lIHR3byB0aHJlZSBmb3VyIGZpdmUgc2l4IHNldmVuIGVpZ2h0IG5pbmUgdGVu\n"+
                                      "IGVsZXZlbiB0d2VsdmUgdGhpcnRlZW4gZm91cnRlZW4gZml2dGVlbiBzaXh0ZWVu\n"+
                                      "IHNldmVudGVlbiBlaWdodGVlbiBuaW5ldGVlbiB0d2VudHkgdHdlbnR5LW9uZQ==";
            final byte[] octets = in_str.getBytes(StandardCharsets.UTF_8);
            final String encstr = BaseCodec.encode64_pem(octets, 0, octets.length, aspec).toString();
            Assert.assertEquals( exp_encstr, encstr);
            final ByteBuffer dec_octets = BaseCodec.decode64_lf(encstr, aspec);
            Assert.assertArrayEquals( octets, to_array( dec_octets ) );
        }

        // Erroneous coded string in decoding
        {
            final String encstr = "!@#$%^&*()"; // non-alphebet error
            final ByteBuffer dec_octets = BaseCodec.decode64(encstr, aspec);
            Assert.assertEquals( 0, dec_octets.remaining() );
        }

    }

    @Test
    public void test11BinaryBase86() {
        testBinaryBase64();
    }

    public static void main(final String args[]) {
        org.junit.runner.JUnitCore.main(TestBaseCodec.class.getName());
    }

}
