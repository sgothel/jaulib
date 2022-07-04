/*
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

package jau.test.nio;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.Charset;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.jau.fs.FileUtil;
import org.jau.io.Buffers;
import org.jau.io.PrintUtil;
import org.jau.nio.ByteInStream;
import org.jau.nio.ByteInStream_Feed;
import org.jau.nio.ByteInStream_File;
import org.jau.nio.ByteInStream_URL;
import org.jau.nio.NativeIO;
import org.jau.nio.Uri;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

import jau.pkg.PlatformRuntime;
import jau.test.junit.util.JunitTracer;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestByteStream01 extends JunitTracer {
    static final boolean DEBUG = false;

    static final String payload_version = "0";
    static final String payload_version_parent = "0";

    static final int IDX_11kiB = 0;
    static final int IDX_65MiB = 1;
    static List<String> fname_payload_lst = new ArrayList<String>();
    static List<String> fname_payload_copy_lst = new ArrayList<String>();
    static List<Long> fname_payload_size_lst = new ArrayList<Long>();

    static boolean file_exists(final String name) {
        final File file = new File( name );
        return file.exists();
    }

    static long file_size(final String name) {
        try (ByteInStream_File f = new ByteInStream_File(name)) {
            return f.content_size();
        }
    }

    static boolean remove_file(final String name) {
        final File file = new File( name );
        try {
            if( file.exists() ) {
                if( !file.delete() ) {
                    PrintUtil.println(System.err, "Remove.1: Failed deletion of existing file "+name);
                    return false;
                }
            }
            return true;
        } catch (final Exception ex) {
            PrintUtil.println(System.err, "Remove.2: Failed deletion of existing file "+name+": "+ex.getMessage());
            ex.printStackTrace();
        }
        return false;
    }

    static void add_test_file(final String name, final long size_limit) {
        Assert.assertTrue( remove_file(name) );
        Assert.assertTrue( remove_file(name+".enc") );
        Assert.assertTrue( remove_file(name+".enc.dec") );
        long size = 0;
        {
            final String one_line = "Hello World, this is a test and I like it. Exactly 100 characters long. 0123456780 abcdefghjklmnop..";
            final Charset charset = Charset.forName("ASCII");
            final byte[] one_line_bytes = one_line.getBytes(charset);

            final File file = new File( name );
            OutputStream out = null;
            try {
                Assert.assertFalse( file.exists() );

                out = new FileOutputStream(file);

                for(size=0; size < size_limit; size+=one_line_bytes.length) {
                    out.write( one_line_bytes );
                }
                out.write( (byte)'X' ); // make it odd
                size += 1;

            } catch (final Exception ex) {
                PrintUtil.println(System.err, "Write test file: Failed "+name+": "+ex.getMessage());
                ex.printStackTrace();
            } finally {
                try {
                    if( null != out ) {
                        out.close();
                    }
                } catch (final IOException e) {
                    e.printStackTrace();
                }
            }

        }
        fname_payload_lst.add(name);
        fname_payload_copy_lst.add(name+".copy");
        fname_payload_size_lst.add( Long.valueOf(size) );
    }

    static {
        add_test_file("test_cipher_01_11kiB.bin", 1024*11);
        add_test_file("test_cipher_02_65MiB.bin", 1024*1024*65);
    }

    static boolean system(final String[] command) {
        Process proc = null;
        try{
            proc = Runtime.getRuntime().exec(command);
            proc.waitFor();
            return true;
        }
        catch(final Exception ex)
        {
            if(proc!=null) {
                proc.destroy();
            }
            ex.printStackTrace();
        }
        return false;
    }

    @AfterClass
    public static void httpd_stop() {
        Assert.assertTrue( system(new String[]{"killall", "mini_httpd"}) );
    }

    static void httpd_start() {
        Assert.assertTrue( system(new String[]{"killall", "mini_httpd"}) );
        final Path path = Paths.get("");
        final String directoryName = path.toAbsolutePath().toString();
        final String[] cmd = new String[]{"/usr/sbin/mini_httpd", "-p", "8080", "-l", directoryName+"/mini_httpd.log"};
        PrintUtil.fprintf_td(System.err, "%s%n", Arrays.toString(cmd));
        Assert.assertTrue( system(cmd) );
    }

    final static String url_input_root = "http://localhost:8080/";


    static boolean transfer_std(final ByteInStream input, final String output_fname, final int buffer_size) {
        final long _t0 = org.jau.sys.Clock.currentTimeMillis();
        PrintUtil.fprintf_td(System.err, "Transfer Start: %s%n", input);
        remove_file(output_fname);

        final File file = new File( output_fname );
        if( file.exists() ) {
            return false;
        }

        final OutputStream out[] = { null };
        try {
            Assert.assertFalse( file.exists() );
            out[0] = new FileOutputStream(file);
        } catch (final Exception ex) {
            PrintUtil.fprintf_td(System.err, "Opening output file : Failed %s: %s%n", file, ex.getMessage());
            ex.printStackTrace();
            return false;
        }

        final long[] out_bytes_payload = { 0 };
        final boolean[] out_failure = { false };
        final NativeIO.StreamConsumer1 consumer = (final byte[] data, final int data_len, final boolean is_final) -> {
                try {
                    if( !is_final && ( !input.has_content_size() || out_bytes_payload[0] + data_len < input.content_size() ) ) {
                        out[0].write( data, 0, data_len );
                        out_bytes_payload[0] += data_len;
                        return true; // continue ..
                    } else {
                        out[0].write( data, 0, data_len );
                        out_bytes_payload[0] += data_len;
                        return false; // EOS
                    }
                } catch (final Exception ex) {
                    PrintUtil.fprintf_td(System.err, "Write input bytes: Failed %s: %s%n", input, ex.getMessage());
                    ex.printStackTrace();
                    out_failure[0] = true;
                    return false;
                }
        };
        final byte[] io_buffer = new byte[buffer_size];
        final long in_bytes_total = NativeIO.read_stream(input, io_buffer, consumer);
        input.closeStream();
        try {
            out[0].close();
            out[0] = null;
        } catch (final IOException e) {
            e.printStackTrace();
        }

        if ( 0==in_bytes_total || out_failure[0] ) {
            PrintUtil.fprintf_td(System.err, "ByteStream copy failed: Output file write failed %s%n", output_fname);
            return false;
        }

        final long _td = org.jau.sys.Clock.currentTimeMillis() - _t0;
        NativeIO.print_stats("Transfer "+output_fname, out_bytes_payload[0], _td);
        PrintUtil.fprintf_td(System.err, "Transfer End: %s%n", input.toString());

        return true;
    }

    static boolean transfer_nio(final ByteInStream input, final String output_fname, final int buffer_size) {
        final long _t0 = org.jau.sys.Clock.currentTimeMillis();
        PrintUtil.fprintf_td(System.err, "Transfer Start: %s%n", input);
        remove_file(output_fname);

        final File file = new File( output_fname );
        if( file.exists() ) {
            return false;
        }

        final OutputStream out[] = { null };
        final FileChannel outc[] = { null };
        try {
            Assert.assertFalse( file.exists() );
            out[0] = new FileOutputStream(file);
            outc[0] = ((FileOutputStream)out[0]).getChannel();
        } catch (final Exception ex) {
            PrintUtil.fprintf_td(System.err, "Opening output file : Failed %s: %s%n", file, ex.getMessage());
            ex.printStackTrace();
            if( null != outc[0] ) {
                try { outc[0].close(); } catch (final IOException e) { }
            }
            if( null != out[0] ) {
                try { out[0].close(); } catch (final IOException e) { }
            }
            return false;
        }

        final long[] out_bytes_payload = { 0 };
        final boolean[] out_failure = { false };
        final NativeIO.StreamConsumer2 consumer = (final ByteBuffer data, final boolean is_final) -> {
                try {
                    final int data_len = data.remaining();
                    if( !is_final && ( !input.has_content_size() || out_bytes_payload[0] + data_len < input.content_size() ) ) {
                        outc[0].write(data);
                        data.rewind();
                        out_bytes_payload[0] += data_len;
                        return true; // continue ..
                    } else {
                        outc[0].write(data);
                        data.rewind();
                        out_bytes_payload[0] += data_len;
                        return false; // EOS
                    }
                } catch (final Exception ex) {
                    PrintUtil.fprintf_td(System.err, "Write input bytes: Failed %s: %s%n", input, ex.getMessage());
                    ex.printStackTrace();
                    out_failure[0] = true;
                    return false;
                }
        };
        final ByteBuffer io_buffer = Buffers.newDirectByteBuffer(buffer_size);
        final long in_bytes_total = NativeIO.read_stream(input, io_buffer, consumer);
        input.closeStream();
        if( null != outc[0] ) {
            try { outc[0].close(); outc[0]=null; } catch (final IOException e) { e.printStackTrace(); }
        }
        if( null != out[0] ) {
            try { out[0].close(); out[0]=null; } catch (final IOException e) { e.printStackTrace(); }
        }

        if ( 0==in_bytes_total || out_failure[0] ) {
            PrintUtil.fprintf_td(System.err, "ByteStream copy failed: Output file write failed %s%n", output_fname);
            return false;
        }

        final long _td = org.jau.sys.Clock.currentTimeMillis() - _t0;
        NativeIO.print_stats("Transfer "+output_fname, out_bytes_payload[0], _td);
        PrintUtil.fprintf_td(System.err, "Transfer End: %s%n", input.toString());

        return true;
    }

    @Test(timeout = 10000)
    public final void test00a_protocols_error() {
        PlatformRuntime.checkInitialized();
        final boolean http_support_expected = Uri.protocol_supported("http:");
        final boolean file_support_expected = Uri.protocol_supported("file:");
        httpd_start();
        {
            final List<String> protos = Uri.supported_protocols();
            PrintUtil.fprintf_td(System.err, "test00_protocols: Supported protocols: %d: %s%n", protos.size(), protos);
            if( http_support_expected ) { // assume no http -> no curl
                Assert.assertTrue( 0 < protos.size() );
            } else {
                Assert.assertTrue( 0 == protos.size() );
            }
        }
        final int file_idx = IDX_11kiB;
        {
            final String url = "not_exiting_file.txt";
            Assert.assertFalse( Uri.is_local_file_protocol(url) );
            Assert.assertFalse( Uri.protocol_supported(url) );

            try( final ByteInStream in = NativeIO.to_ByteInStream(url) ) {
                if( null != in ) {
                    PrintUtil.fprintf_td(System.err, "test00_protocols: not_exiting_file: %s%n", in);
                }
                Assert.assertNull(in);
            }
        }
        {
            final String url = "file://not_exiting_file_uri.txt";
            Assert.assertTrue( Uri.is_local_file_protocol(url) );
            Assert.assertEquals( file_support_expected, Uri.protocol_supported(url) );

            try( final ByteInStream in = NativeIO.to_ByteInStream(url) ) {
                if( null != in ) {
                    PrintUtil.fprintf_td(System.err, "test00_protocols: not_exiting_file_uri: %s%n", in);
                }
                Assert.assertNull(in);
            }
        }
        {
            final String url = "lala://localhost:8080/" + fname_payload_lst.get(file_idx);
            Assert.assertFalse( Uri.is_local_file_protocol(url) );
            Assert.assertFalse( Uri.protocol_supported(url) );

            try( final ByteInStream in = NativeIO.to_ByteInStream(url) ) {
                if( null != in ) {
                    PrintUtil.fprintf_td(System.err, "test00_protocols: not_exiting_protocol_uri: %s%n", in);
                }
                Assert.assertNull(in);
            }
        }
        {
            final String url = url_input_root + "not_exiting_http_uri.txt";
            Assert.assertFalse( Uri.is_local_file_protocol(url) );
            Assert.assertEquals( http_support_expected, Uri.protocol_supported(url) );

            try( final ByteInStream in = NativeIO.to_ByteInStream(url) ) {
                if( http_support_expected ) {
                    Assert.assertNotNull(in);
                    try { Thread.sleep(10); } catch (final Throwable t) {}
                    PrintUtil.fprintf_td(System.err, "test00_protocols: not_exiting_http_uri: %s%n", in);
                    Assert.assertTrue( in.end_of_data() );
                    Assert.assertTrue( in.error() );
                    Assert.assertEquals( 0, in.content_size() );
                } else {
                    Assert.assertNull(in);
                }
            }
        }
    }

    @Test(timeout = 10000)
    public final void test00b_protocols_ok() {
        PlatformRuntime.checkInitialized();
        final boolean http_support_expected = Uri.protocol_supported("http:");
        final boolean file_support_expected = Uri.protocol_supported("file:");
        httpd_start();
        final int file_idx = IDX_11kiB;
        {
            final String url = fname_payload_lst.get(file_idx);
            Assert.assertFalse( Uri.is_local_file_protocol(url) );
            Assert.assertFalse( Uri.protocol_supported(url) );

            try( final ByteInStream in = NativeIO.to_ByteInStream(url) ) {
                if( null != in ) {
                    PrintUtil.fprintf_td(System.err, "test00_protocols: local-file-0: %s%n", in);
                }
                Assert.assertNotEquals( null, in );
                Assert.assertFalse( in.error() );

                final boolean res = transfer_nio(in, fname_payload_copy_lst.get(file_idx), 4096);
                Assert.assertTrue( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertEquals( in.content_size(), copy_size );
                Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
                Assert.assertTrue( FileUtil.compare(in.id(), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
            }
        }
        {
            final String url = "file://" + fname_payload_lst.get(file_idx);
            Assert.assertTrue( Uri.is_local_file_protocol(url) );
            Assert.assertEquals( file_support_expected, Uri.protocol_supported(url) );

            try( final ByteInStream in = NativeIO.to_ByteInStream(url) ) {
                if( null != in ) {
                    PrintUtil.fprintf_td(System.err, "test00_protocols: local-file-1: %s%n", in);
                }
                Assert.assertNotNull( in );
                Assert.assertFalse( in.error() );

                final boolean res = transfer_nio(in, fname_payload_copy_lst.get(file_idx), 4096);
                Assert.assertTrue( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertEquals( in.content_size(), copy_size );
                Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
                Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
            }
        }
        {
            final String url = url_input_root + fname_payload_lst.get(file_idx);
            Assert.assertFalse( Uri.is_local_file_protocol(url) );
            Assert.assertEquals( http_support_expected, Uri.protocol_supported(url) );

            try( final ByteInStream in = NativeIO.to_ByteInStream(url) ) {
                if( null != in ) {
                    PrintUtil.fprintf_td(System.err, "test00_protocols: http: %s%n", in);
                }
                if( http_support_expected ) {
                    Assert.assertNotNull( in );
                    Assert.assertFalse( in.error() );

                    final boolean res = transfer_nio(in, fname_payload_copy_lst.get(file_idx), 4096);
                    Assert.assertTrue( res );

                    Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                    final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                    Assert.assertEquals( in.content_size(), copy_size );
                    Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
                    Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
                } else {
                    Assert.assertNull( in );
                }
            }
        }
    }

    @Test(timeout = 10000)
    public void test01_copy_file_ok_11kiB() {
        PlatformRuntime.checkInitialized();
        final int file_idx = IDX_11kiB;
        try( final ByteInStream_File data_stream = new ByteInStream_File(fname_payload_lst.get(file_idx)) ) {
            final boolean res = transfer_nio(data_stream, fname_payload_copy_lst.get(file_idx), 4096);
            Assert.assertTrue( res );

            Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
            final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
            Assert.assertEquals( data_stream.content_size(), copy_size );
            Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
            Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
        }
    }

    @Test(timeout = 10000)
    public void test02_copy_file_ok_65MiB_nio_buff4k() {
        PlatformRuntime.checkInitialized();
        final int file_idx = IDX_65MiB;
        try( final ByteInStream_File data_stream = new ByteInStream_File(fname_payload_lst.get(file_idx)) ) {
            final boolean res = transfer_nio(data_stream, fname_payload_copy_lst.get(file_idx), 4096);
            Assert.assertTrue( res );

            Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
            final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
            Assert.assertEquals( data_stream.content_size(), copy_size );
            Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
            Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
        }
    }

    @Test(timeout = 10000)
    public void test03_copy_file_ok_65MiB_std_buff4k() {
        PlatformRuntime.checkInitialized();
        final int file_idx = IDX_65MiB;
        try( final ByteInStream_File data_stream = new ByteInStream_File(fname_payload_lst.get(file_idx)) ) {
            final boolean res = transfer_std(data_stream, fname_payload_copy_lst.get(file_idx), 4096);
            Assert.assertTrue( res );

            Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
            final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
            Assert.assertEquals( data_stream.content_size(), copy_size );
            Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
            Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
        }
    }

    @Test(timeout = 10000)
    public void test04_copy_file_ok_65MiB_nio_buff32k() {
        PlatformRuntime.checkInitialized();
        final int file_idx = IDX_65MiB;
        try( final ByteInStream_File data_stream = new ByteInStream_File(fname_payload_lst.get(file_idx)) ) {
            final boolean res = transfer_nio(data_stream, fname_payload_copy_lst.get(file_idx), 32768);
            Assert.assertTrue( res );

            Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
            final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
            Assert.assertEquals( data_stream.content_size(), copy_size );
            Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
            Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
        }
    }

    @Test(timeout = 10000)
    public void test05_copy_file_ok_65MiB_std_buff32k() {
        PlatformRuntime.checkInitialized();
        final int file_idx = IDX_65MiB;
        try( final ByteInStream_File data_stream = new ByteInStream_File(fname_payload_lst.get(file_idx)) ) {
            final boolean res = transfer_std(data_stream, fname_payload_copy_lst.get(file_idx), 32768);
            Assert.assertTrue( res );

            Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
            final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
            Assert.assertEquals( data_stream.content_size(), copy_size );
            Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
            Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
        }
    }

    @Test(timeout = 10000)
    public void test11_copy_http_ok_nio_buff32k() {
        PlatformRuntime.checkInitialized();
        if( !Uri.protocol_supported("http:") ) {
            PrintUtil.fprintf_td(System.err, "http not supported, abort%n");
            return;
        }
        httpd_start();
        {
            final int file_idx = IDX_11kiB;

            final String uri_original = url_input_root + fname_payload_lst.get(file_idx);

            try( final ByteInStream_URL data_stream = new ByteInStream_URL(uri_original, 500) ) {
                final boolean res = transfer_nio(data_stream, fname_payload_copy_lst.get(file_idx), 32768);
                Assert.assertTrue( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertEquals( data_stream.content_size(), copy_size );
                Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
                Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
            }
        }
        {
            final int file_idx = IDX_65MiB;

            final String uri_original = url_input_root + fname_payload_lst.get(file_idx);

            try( final ByteInStream_URL data_stream = new ByteInStream_URL(uri_original, 500) ) {
                final boolean res = transfer_nio(data_stream, fname_payload_copy_lst.get(file_idx), 32768);
                Assert.assertTrue( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertEquals( data_stream.content_size(), copy_size );
                Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
                Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
            }
        }
    }

    @Test(timeout = 10000)
    public void test12_copy_http_404() {
        PlatformRuntime.checkInitialized();
        if( !Uri.protocol_supported("http:") ) {
            PrintUtil.fprintf_td(System.err, "http not supported, abort%n");
            return;
        }
        httpd_start();
        {
            final int file_idx = IDX_11kiB;

            final String uri_original = url_input_root + "doesnt_exists.txt";

            try( final ByteInStream_URL data_stream = new ByteInStream_URL(uri_original, 500) ) {
                final boolean res = transfer_nio(data_stream, fname_payload_copy_lst.get(file_idx), 4096);
                Assert.assertFalse( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertTrue( data_stream.error() );
                Assert.assertFalse( data_stream.has_content_size() );
                Assert.assertEquals( data_stream.content_size(), 0 );
                Assert.assertEquals( 0, copy_size );
            }
        }
    }

    static Thread executeOffThread(final Runnable runobj, final String threadName, final boolean detach) {
        final Thread t = new Thread( runobj, threadName );
        t.setDaemon( detach );
        t.start();
        return t;
    }

    // throttled, no content size, interruptReader() via set_eof() will avoid timeout
    static void feed_source_00(final ByteInStream_Feed data_feed, final int feed_size) {
        // long xfer_total = 0;
        try( final ByteInStream_File data_stream = new ByteInStream_File(data_feed.id() ) ) {
            final byte buffer[] = new byte[feed_size];
            while( !data_stream.end_of_data() ) {
                final int count = data_stream.read(buffer, 0, buffer.length);
                if( 0 < count ) {
                    // xfer_total += count;
                    data_feed.write(buffer, 0, count);
                    try { Thread.sleep(16); } catch (final Throwable t) {}
                }
            }
            // probably set after transfering due to above sleep, which also ends when total size has been reached.
            data_feed.set_eof( 1 /* SUCCESS */ );
        }
    }

    // throttled, with content size
    static void feed_source_01(final ByteInStream_Feed data_feed, final int feed_size) {
        long xfer_total = 0;
        try( final ByteInStream_File data_stream = new ByteInStream_File(data_feed.id() ) ) {
            final long file_size = data_stream.content_size();
            data_feed.set_content_size( file_size );
            final byte buffer[] = new byte[feed_size];
            while( !data_stream.end_of_data() && xfer_total < file_size ) {
                final int count = data_stream.read(buffer, 0, buffer.length);
                if( 0 < count ) {
                    xfer_total += count;
                    data_feed.write(buffer, 0, count);
                    try { Thread.sleep(16); } catch (final Throwable t) {}
                }
            }
            // probably set after transfering due to above sleep, which also ends when total size has been reached.
            data_feed.set_eof( xfer_total == file_size ? 1 /* SUCCESS */ : -1 /* FAILED */);
        }
    }

    // full speed, with content size
    static void feed_source_10_nio(final ByteInStream_Feed data_feed, final int feed_size) {
        long xfer_total = 0;
        try( final ByteInStream_File data_stream = new ByteInStream_File(data_feed.id() ) ) {
            final long file_size = data_stream.content_size();
            data_feed.set_content_size( file_size );
            final ByteBuffer buffer = Buffers.newDirectByteBuffer(feed_size);
            while( !data_stream.end_of_data() && xfer_total < file_size ) {
                final int count = data_stream.read(buffer);
                if( 0 < count ) {
                    xfer_total += count;
                    data_feed.write(buffer);
                }
            }
            data_feed.set_eof( xfer_total == file_size ? 1 /* SUCCESS */ : -1 /* FAILED */);
        }
    }

    // full speed, with content size
    static void feed_source_10_std(final ByteInStream_Feed data_feed, final int feed_size) {
        long xfer_total = 0;
        try( final ByteInStream_File data_stream = new ByteInStream_File(data_feed.id() ) ) {
            final long file_size = data_stream.content_size();
            data_feed.set_content_size( file_size );
            final byte buffer[] = new byte[feed_size];
            while( !data_stream.end_of_data() && xfer_total < file_size ) {
                final int count = data_stream.read(buffer, 0, buffer.length);
                if( 0 < count ) {
                    xfer_total += count;
                    data_feed.write(buffer, 0, count);
                }
            }
            data_feed.set_eof( xfer_total == file_size ? 1 /* SUCCESS */ : -1 /* FAILED */);
        }
    }

    // full speed, no content size, interrupting @ 1024 bytes within our header
    static void feed_source_20(final ByteInStream_Feed data_feed, final int feed_size) {
        long xfer_total = 0;
        try( final ByteInStream_File data_stream = new ByteInStream_File(data_feed.id() ) ) {
            final byte buffer[] = new byte[feed_size];
            while( !data_stream.end_of_data() ) {
                final int count = data_stream.read(buffer, 0, buffer.length);
                if( 0 < count ) {
                    xfer_total += count;
                    data_feed.write(buffer, 0, count);
                    if( xfer_total >= 1024 ) {
                        data_feed.set_eof( -1 /* FAILED */ ); // calls data_feed->interruptReader();
                        return;
                    }
                }
            }
            // probably set after transfering due to above sleep, which also ends when total size has been reached.
            data_feed.set_eof( 1 /* SUCCESS */ );
        }
    }

    // full speed, with content size, interrupting 1/4 way
    static void feed_source_21(final ByteInStream_Feed data_feed, final int feed_size) {
        long xfer_total = 0;
        try( final ByteInStream_File data_stream = new ByteInStream_File(data_feed.id() ) ) {
            final long file_size = data_stream.content_size();
            data_feed.set_content_size( file_size );
            final byte buffer[] = new byte[feed_size];
            while( !data_stream.end_of_data() ) {
                final int count = data_stream.read(buffer, 0, buffer.length);
                if( 0 < count ) {
                    xfer_total += count;
                    data_feed.write(buffer, 0, count);
                    if( xfer_total >= file_size/4 ) {
                        data_feed.set_eof( -1 /* FAILED */ ); // calls data_feed->interruptReader();
                        return;
                    }
                }
            }
            // probably set after transfering due to above sleep, which also ends when total size has been reached.
            data_feed.set_eof( 1 /* SUCCESS */ );
        }
    }

    @Test(timeout = 10000)
    public void test20_copy_fed_ok_buff4k_feed1k() {
        PlatformRuntime.checkInitialized();
        final int buffer_size = 4096;
        final int feed_size = 1024;
        {
            final int file_idx = IDX_11kiB;

            // full speed, with content size
            try( final ByteInStream_Feed data_feed = new ByteInStream_Feed(fname_payload_lst.get(file_idx), 500) ) {
                final Thread feeder_thread = executeOffThread( () -> { feed_source_10_nio(data_feed, feed_size); }, "test21_copy_fed_ok::feed_source_10", false /* detach */);

                final boolean res = transfer_nio(data_feed, fname_payload_copy_lst.get(file_idx), buffer_size);
                try {
                    feeder_thread.join(1000);
                } catch (final InterruptedException e) { }
                Assert.assertTrue( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertEquals( data_feed.content_size(), copy_size );
                Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
                Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
            }

            // throttled, with content size
            try( final ByteInStream_Feed data_feed = new ByteInStream_Feed(fname_payload_lst.get(file_idx), 500) ) {
                final Thread feeder_thread = executeOffThread( () -> { feed_source_01(data_feed, feed_size); }, "test21_copy_fed_ok::feed_source_01", false /* detach */);

                final boolean res = transfer_nio(data_feed, fname_payload_copy_lst.get(file_idx), buffer_size);
                try {
                    feeder_thread.join(1000);
                } catch (final InterruptedException e) { }
                Assert.assertTrue( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertEquals( data_feed.content_size(), copy_size );
                Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
                Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
            }

            // throttled, no content size, interruptReader() via set_eof() will avoid timeout
            try( final ByteInStream_Feed data_feed = new ByteInStream_Feed(fname_payload_lst.get(file_idx), 500) ) {
                final Thread feeder_thread = executeOffThread( () -> { feed_source_00(data_feed, feed_size); }, "test21_copy_fed_ok::feed_source_00", false /* detach */);

                final boolean res = transfer_nio(data_feed, fname_payload_copy_lst.get(file_idx), buffer_size);
                try {
                    feeder_thread.join(1000);
                } catch (final InterruptedException e) { }
                Assert.assertTrue( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertEquals( data_feed.content_size(), 0 );
                Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
                Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
            }
        }
        {
            final int file_idx = IDX_65MiB;

            // full speed, with content size
            try( final ByteInStream_Feed data_feed = new ByteInStream_Feed(fname_payload_lst.get(file_idx), 500) ) {
                final Thread feeder_thread = executeOffThread( () -> { feed_source_10_nio(data_feed, feed_size); }, "test21_copy_fed_ok2::feed_source_10", false /* detach */);

                final boolean res = transfer_nio(data_feed, fname_payload_copy_lst.get(file_idx), buffer_size);
                try {
                    feeder_thread.join(1000);
                } catch (final InterruptedException e) { }
                Assert.assertTrue( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertEquals( data_feed.content_size(), copy_size );
                Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
                Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
            }
        }
    }

    @Test(timeout = 10000)
    public void test21_copy_fed_ok_buff32k() {
        PlatformRuntime.checkInitialized();
        final int buffer_size = 32768;
        final int feed_size = 32768;
        {
            final int file_idx = IDX_11kiB;

            // full speed, with content size
            try( final ByteInStream_Feed data_feed = new ByteInStream_Feed(fname_payload_lst.get(file_idx), 500) ) {
                final Thread feeder_thread = executeOffThread( () -> { feed_source_10_nio(data_feed, feed_size); }, "test21_copy_fed_ok::feed_source_10", false /* detach */);

                final boolean res = transfer_nio(data_feed, fname_payload_copy_lst.get(file_idx), buffer_size);
                try {
                    feeder_thread.join(1000);
                } catch (final InterruptedException e) { }
                Assert.assertTrue( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertEquals( data_feed.content_size(), copy_size );
                Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
                Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
            }

            // throttled, with content size
            try( final ByteInStream_Feed data_feed = new ByteInStream_Feed(fname_payload_lst.get(file_idx), 500) ) {
                final Thread feeder_thread = executeOffThread( () -> { feed_source_01(data_feed, feed_size); }, "test21_copy_fed_ok::feed_source_01", false /* detach */);

                final boolean res = transfer_nio(data_feed, fname_payload_copy_lst.get(file_idx), buffer_size);
                try {
                    feeder_thread.join(1000);
                } catch (final InterruptedException e) { }
                Assert.assertTrue( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertEquals( data_feed.content_size(), copy_size );
                Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
                Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
            }

            // throttled, no content size, interruptReader() via set_eof() will avoid timeout
            try( final ByteInStream_Feed data_feed = new ByteInStream_Feed(fname_payload_lst.get(file_idx), 500) ) {
                final Thread feeder_thread = executeOffThread( () -> { feed_source_00(data_feed, feed_size); }, "test21_copy_fed_ok::feed_source_00", false /* detach */);

                final boolean res = transfer_nio(data_feed, fname_payload_copy_lst.get(file_idx), buffer_size);
                try {
                    feeder_thread.join(1000);
                } catch (final InterruptedException e) { }
                Assert.assertTrue( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertEquals( data_feed.content_size(), 0 );
                Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
                Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
            }
        }
    }

    @Test(timeout = 10000)
    public void test22_copy_fed_ok_buff32k_nio() {
        PlatformRuntime.checkInitialized();
        final int buffer_size = 32768;
        final int feed_size = 32768;
        {
            final int file_idx = IDX_65MiB;

            // full speed, with content size
            try( final ByteInStream_Feed data_feed = new ByteInStream_Feed(fname_payload_lst.get(file_idx), 500) ) {
                final Thread feeder_thread = executeOffThread( () -> { feed_source_10_nio(data_feed, feed_size); }, "test21_copy_fed_ok2::feed_source_10", false /* detach */);

                final boolean res = transfer_nio(data_feed, fname_payload_copy_lst.get(file_idx), buffer_size);
                try {
                    feeder_thread.join(1000);
                } catch (final InterruptedException e) { }
                Assert.assertTrue( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertEquals( data_feed.content_size(), copy_size );
                Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
                Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
            }
        }
    }

    @Test(timeout = 10000)
    public void test23_copy_fed_ok_buff32k_std() {
        PlatformRuntime.checkInitialized();
        final int buffer_size = 32768;
        final int feed_size = 32768;
        {
            final int file_idx = IDX_65MiB;

            // full speed, with content size
            try( final ByteInStream_Feed data_feed = new ByteInStream_Feed(fname_payload_lst.get(file_idx), 500) ) {
                final Thread feeder_thread = executeOffThread( () -> { feed_source_10_std(data_feed, feed_size); }, "test23_copy_fed_ok2::feed_source_10", false /* detach */);

                final boolean res = transfer_std(data_feed, fname_payload_copy_lst.get(file_idx), buffer_size);
                try {
                    feeder_thread.join(1000);
                } catch (final InterruptedException e) { }
                Assert.assertTrue( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertEquals( data_feed.content_size(), copy_size );
                Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), copy_size );
                Assert.assertTrue( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
            }
        }
    }

    @Test(timeout = 10000)
    public void test24_copy_fed_irq() {
        PlatformRuntime.checkInitialized();
        final int buffer_size = 4096;
        final int feed_size = 1024;
        {
            final int file_idx = IDX_65MiB;

            // full speed, no content size, interrupting @ 1024 bytes within our header
            try( final ByteInStream_Feed data_feed = new ByteInStream_Feed(fname_payload_lst.get(file_idx), 500) ) {
                final Thread feeder_thread = executeOffThread( () -> { feed_source_20(data_feed, feed_size); }, "test22_copy_fed_irq::feed_source_20", false /* detach */);

                final boolean res = transfer_nio(data_feed, fname_payload_copy_lst.get(file_idx), buffer_size);
                try {
                    feeder_thread.join(1000);
                } catch (final InterruptedException e) { }
                Assert.assertTrue( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertFalse( data_feed.has_content_size() );
                Assert.assertEquals( data_feed.content_size(), 0 );
                Assert.assertTrue( fname_payload_size_lst.get(file_idx).longValue() > copy_size );
                Assert.assertFalse( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
            }

            // full speed, with content size, interrupting 1/4 way
            try( final ByteInStream_Feed data_feed = new ByteInStream_Feed(fname_payload_lst.get(file_idx), 500) ) {
                final Thread feeder_thread = executeOffThread( () -> { feed_source_21(data_feed, feed_size); }, "test22_copy_fed_irq::feed_source_21", false /* detach */);

                final boolean res = transfer_nio(data_feed, fname_payload_copy_lst.get(file_idx), buffer_size);
                try {
                    feeder_thread.join(1000);
                } catch (final InterruptedException e) { }
                Assert.assertTrue( res );

                Assert.assertTrue( file_exists( fname_payload_copy_lst.get(file_idx) ) );
                final long copy_size = file_size(fname_payload_copy_lst.get(file_idx));
                Assert.assertTrue( data_feed.has_content_size() );
                Assert.assertEquals( fname_payload_size_lst.get(file_idx).longValue(), data_feed.content_size() );
                Assert.assertTrue( data_feed.content_size() > copy_size );
                Assert.assertFalse( FileUtil.compare(fname_payload_lst.get(file_idx), fname_payload_copy_lst.get(file_idx), true /* verbose */) );
            }
        }
    }

    public static void main(final String args[]) {
        org.junit.runner.JUnitCore.main(TestByteStream01.class.getName());
    }
};

