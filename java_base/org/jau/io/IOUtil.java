/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2010 Gothel Software e.K.
 * Copyright (c) 2010 JogAmp Community.
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

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.Closeable;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FilePermission;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.Reader;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLConnection;
import java.nio.ByteBuffer;
import java.security.PrivilegedAction;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.regex.Pattern;

import org.jau.lang.ExceptionUtils;
import org.jau.lang.InterruptSource;
import org.jau.lang.ReflectionUtil;
import org.jau.sec.SecurityUtil;
import org.jau.sys.AndroidUtil;
import org.jau.sys.Debug;
import org.jau.sys.MachineDataInfo;
import org.jau.sys.PlatformProps;
import org.jau.sys.PlatformTypes;
import org.jau.sys.PropertyAccess;
import org.jau.sys.PlatformTypes.OSType;

public class IOUtil {
    public static final boolean DEBUG;
    private static final boolean DEBUG_EXE;
    private static final boolean DEBUG_EXE_NOSTREAM;
    private static final boolean DEBUG_EXE_EXISTING_FILE;
    private static final boolean testTempDirExec;
    private static final boolean useNativeExeFile;

    private static final String auc_name = "org.jau.net.AssetURLContext";
    private static final ReflectionUtil.MethodAccessor aucGetRes;

    static {
        final boolean _props[] = { false, false, false, false, false, false };
        SecurityUtil.doPrivileged(new PrivilegedAction<Object>() {
            @Override
            public Object run() {
                try {
                    int i=0;
                    _props[i++] = Debug.debug("IOUtil");
                    _props[i++] = PropertyAccess.isPropertyDefined("jau.debug.IOUtil.Exe", true);
                    _props[i++] =  PropertyAccess.isPropertyDefined("jau.debug.IOUtil.Exe.NoStream", true);
                    // For security reasons, we have to hardcode this, i.e. disable this manual debug feature!
                    _props[i++] = false; // PropertyAccess.isPropertyDefined("jau.debug.IOUtil.Exe.ExistingFile", true);
                    _props[i++] = PropertyAccess.getBooleanProperty("jau.gluegen.TestTempDirExec", true, true);
                    _props[i++] = PropertyAccess.getBooleanProperty("jau.gluegen.UseNativeExeFile", true, false);
                } catch (final Throwable t) {
                    if(_props[0]) {
                        ExceptionUtils.dumpThrowable("ioutil-init", t);
                    }
                }
                return null;
            }
        });
        {
            int i=0;
            DEBUG = _props[i++];
            DEBUG_EXE = _props[i++];
            DEBUG_EXE_NOSTREAM = _props[i++];
            DEBUG_EXE_EXISTING_FILE = _props[i++];
            testTempDirExec = _props[i++];
            useNativeExeFile = _props[i++];
        }
        {
            Class<?> auc = null;
            try {
                auc = ReflectionUtil.getClass(auc_name, false /* initializeClazz */, IOUtil.class.getClassLoader());
            } catch (final Throwable t) {}
            if( null != auc ) {
                aucGetRes = new ReflectionUtil.MethodAccessor(auc, "getResource", String.class, ClassLoader.class);
                if( DEBUG ) {
                    System.err.println("IOUtil: Available <"+auc_name+">, getResource avail "+(null != aucGetRes ? aucGetRes.available() : false));
                }
            } else {
                aucGetRes = null;
                if( DEBUG ) {
                    System.err.println("IOUtil: Not available <"+auc_name+">");
                }
            }
        }
    }

    /** Std. temporary directory property key <code>java.io.tmpdir</code>. */
    private static final String java_io_tmpdir_propkey = "java.io.tmpdir";
    private static final String user_home_propkey = "user.home";
    private static final String XDG_CACHE_HOME_envkey = "XDG_CACHE_HOME";

    /** Subdirectory within platform's temporary root directory where all JogAmp related temp files are being stored: {@code jau} */
    public static final String tmpSubDir = "jau";

    private IOUtil() {}

    /***
     *
     * STREAM COPY STUFF
     *
     */

    /**
     * Copy the specified URL resource to the specified output file. The total
     * number of bytes written is returned.
     *
     * @param conn the open URLConnection
     * @param outFile the destination
     * @return
     * @throws IOException
     */
    public static int copyURLConn2File(final URLConnection conn, final File outFile) throws IOException {
        conn.connect();  // redundant

        int totalNumBytes = 0;
        final InputStream in = new BufferedInputStream(conn.getInputStream());
        try {
            totalNumBytes = copyStream2File(in, outFile, conn.getContentLength());
        } finally {
            in.close();
        }
        return totalNumBytes;
    }

    /**
     * Copy the specified input stream to the specified output file. The total
     * number of bytes written is returned.
     *
     * @param in the source
     * @param outFile the destination
     * @param totalNumBytes informal number of expected bytes, maybe used for user feedback while processing. -1 if unknown
     * @return
     * @throws IOException
     */
    public static int copyStream2File(final InputStream in, final File outFile, int totalNumBytes) throws IOException {
        final OutputStream out = new BufferedOutputStream(new FileOutputStream(outFile));
        try {
            totalNumBytes = copyStream2Stream(in, out, totalNumBytes);
        } finally {
            out.close();
        }
        return totalNumBytes;
    }

    /**
     * Copy the specified input stream to the specified output stream. The total
     * number of bytes written is returned.
     *
     * @param in the source
     * @param out the destination
     * @param totalNumBytes informal number of expected bytes, maybe used for user feedback while processing. -1 if unknown
     * @return
     * @throws IOException
     */
    public static int copyStream2Stream(final InputStream in, final OutputStream out, final int totalNumBytes) throws IOException {
        return copyStream2Stream(PlatformProps.MACH_DESC_STAT.pageSizeInBytes(), in, out, totalNumBytes);
    }

    /**
     * Copy the specified input stream to the specified output stream. The total
     * number of bytes written is returned.
     *
     * @param bufferSize the intermediate buffer size, should be {@link MachineDataInfo#pageSizeInBytes()} for best performance.
     * @param in the source
     * @param out the destination
     * @param totalNumBytes informal number of expected bytes, maybe used for user feedback while processing. -1 if unknown
     * @return
     * @throws IOException
     */
    public static int copyStream2Stream(final int bufferSize, final InputStream in, final OutputStream out, final int totalNumBytes) throws IOException {
        final byte[] buf = new byte[bufferSize];
        int numBytes = 0;
        while (true) {
            int count;
            if ((count = in.read(buf)) == -1) {
                break;
            }
            out.write(buf, 0, count);
            numBytes += count;
        }
        return numBytes;
    }

    public static StringBuilder appendCharStream(final StringBuilder sb, final Reader r) throws IOException {
        final char[] cbuf = new char[1024];
        int count;
        while( 0 < ( count = r.read(cbuf) ) ) {
            sb.append(cbuf, 0, count);
        }
        return sb;
    }

    /**
     * Copy the specified input stream to a byte array, which is being returned.
     */
    public static byte[] copyStream2ByteArray(InputStream stream) throws IOException {
        if( !(stream instanceof BufferedInputStream) ) {
            stream = new BufferedInputStream(stream);
        }
        int totalRead = 0;
        int avail = stream.available();
        byte[] data = new byte[avail];
        int numRead = 0;
        do {
            if (totalRead + avail > data.length) {
                final byte[] newData = new byte[totalRead + avail];
                System.arraycopy(data, 0, newData, 0, totalRead);
                data = newData;
            }
            numRead = stream.read(data, totalRead, avail);
            if (numRead >= 0) {
                totalRead += numRead;
            }
            avail = stream.available();
        } while (avail > 0 && numRead >= 0);

        // just in case the announced avail > totalRead
        if (totalRead != data.length) {
            final byte[] newData = new byte[totalRead];
            System.arraycopy(data, 0, newData, 0, totalRead);
            data = newData;
        }
        return data;
    }

    /**
     * Copy the specified input stream to a NIO ByteBuffer w/ native byte order, which is being returned.
     * <p>The implementation creates the ByteBuffer w/ {@link #copyStream2ByteArray(InputStream)}'s returned byte array.</p>
     *
     * @param stream input stream, which will be wrapped into a BufferedInputStream, if not already done.
     */
    public static ByteBuffer copyStream2ByteBuffer(final InputStream stream) throws IOException {
        return copyStream2ByteBuffer(stream, -1);
    }

    /**
     * Copy the specified input stream to a NIO ByteBuffer w/ native byte order, which is being returned.
     * <p>The implementation creates the ByteBuffer w/ {@link #copyStream2ByteArray(InputStream)}'s returned byte array.</p>
     *
     * @param stream input stream, which will be wrapped into a BufferedInputStream, if not already done.
     * @param initialCapacity initial buffer capacity in bytes, if &gt; available bytes
     */
    public static ByteBuffer copyStream2ByteBuffer(InputStream stream, int initialCapacity) throws IOException {
        if( !(stream instanceof BufferedInputStream) ) {
            stream = new BufferedInputStream(stream);
        }
        int avail = stream.available();
        if( initialCapacity < avail ) {
            initialCapacity = avail;
        }
        final MachineDataInfo machine = PlatformProps.MACH_DESC_STAT;
        ByteBuffer data = Buffers.newDirectByteBuffer( machine.pageAlignedSize( initialCapacity ) );
        final byte[] chunk = new byte[machine.pageSizeInBytes()];
        int chunk2Read = Math.min(machine.pageSizeInBytes(), avail);
        int numRead = 0;
        do {
            if (avail > data.remaining()) {
                final ByteBuffer newData = Buffers.newDirectByteBuffer(
                                               machine.pageAlignedSize(data.position() + avail) );
                newData.put(data);
                data = newData;
            }

            numRead = stream.read(chunk, 0, chunk2Read);
            if (numRead > 0) {
                data.put(chunk, 0, numRead);
            }
            avail = stream.available();
            chunk2Read = Math.min(machine.pageSizeInBytes(), avail);
        } while ( numRead > 0 ); // EOS: -1 == numRead, EOF maybe reached earlier w/ 0 == numRead

        data.flip();
        return data;
    }

    /***
     *
     * RESOURCE / FILE NAME STUFF
     *
     */

    private static final Pattern patternSingleBS = Pattern.compile("\\\\{1}");

    /**
     *
     * @param path
     * @param startWithSlash
     * @param endWithSlash
     * @return
     * @throws URISyntaxException if path is empty or has no parent directory available while resolving <code>../</code>
     */
    public static String slashify(final String path, final boolean startWithSlash, final boolean endWithSlash) throws URISyntaxException {
        String p = patternSingleBS.matcher(path).replaceAll("/");
        if (startWithSlash && !p.startsWith("/")) {
            p = "/" + p;
        }
        if (endWithSlash && !p.endsWith("/")) {
            p = p + "/";
        }
        return cleanPathString(p);
    }

    /**
     * Returns the lowercase suffix of the given file name (the text
     * after the last '.' in the file name). Returns null if the file
     * name has no suffix. Only operates on the given file name;
     * performs no I/O operations.
     *
     * @param file name of the file
     * @return lowercase suffix of the file name
     * @throws NullPointerException if file is null
     */

    public static String getFileSuffix(final File file) {
        return getFileSuffix(file.getName());
    }

    /**
     * Returns the lowercase suffix of the given file name (the text
     * after the last '.' in the file name). Returns null if the file
     * name has no suffix. Only operates on the given file name;
     * performs no I/O operations.
     *
     * @param filename name of the file
     * @return lowercase suffix of the file name
     * @throws NullPointerException if filename is null
     */
    public static String getFileSuffix(final String filename) {
        final int lastDot = filename.lastIndexOf('.');
        if (lastDot < 0) {
            return null;
        }
        return toLowerCase(filename.substring(lastDot + 1));
    }
    private static String toLowerCase(final String arg) {
        if (arg == null) {
            return null;
        }

        return arg.toLowerCase();
    }

    /***
     * @param file
     * @param allowOverwrite
     * @return outputStream The resulting output stream
     * @throws IOException if the file already exists and <code>allowOverwrite</code> is false,
     *                     the class {@link java.io.FileOutputStream} is not accessible or
     *                     the user does not have sufficient rights to access the local filesystem.
     */
    public static FileOutputStream getFileOutputStream(final File file, final boolean allowOverwrite) throws IOException {
        if (file.exists() && !allowOverwrite) {
            throw new IOException("File already exists (" + file + ") and overwrite=false");
        }
        try {
            return new FileOutputStream( file );
        } catch (final Exception e) {
            throw new IOException("error opening " + file + " for write. ", e);
        }
    }

    public static String getClassFileName(final String clazzBinName) {
        // or return clazzBinName.replace('.', File.separatorChar) + ".class"; ?
        return clazzBinName.replace('.', '/') + ".class";
    }

    /**
     * @param clazzBinName com.jogamp.common.util.cache.TempJarCache
     * @param cl ClassLoader to locate the JarFile
     * @return jar:file:/usr/local/projects/JOGL/gluegen/build-x86_64/gluegen-rt.jar!/com/jogamp/common/util/cache/TempJarCache.class
     * @throws IOException if the jar file could not been found by the ClassLoader
     */
    public static URL getClassURL(final String clazzBinName, final ClassLoader cl) throws IOException {
        final URL url = cl.getResource(getClassFileName(clazzBinName));
        if(null == url) {
            throw new IOException("Cannot not find: "+clazzBinName);
        }
        return url;
    }

    /**
     * Returns the basename of the given fname w/o directory part
     * @throws URISyntaxException if path is empty or has no parent directory available while resolving <code>../</code>
     */
    public static String getBasename(String fname) throws URISyntaxException {
        fname = slashify(fname, false /* startWithSlash */, false /* endWithSlash */);
        final int lios = fname.lastIndexOf('/');  // strip off dirname
        if(lios>=0) {
            fname = fname.substring(lios+1);
        }
        return fname;
    }

    /**
     * Returns unified '/' dirname including the last '/'
     * @throws URISyntaxException if path is empty or has no parent directory available while resolving <code>../</code>
     */
    public static String getDirname(String fname) throws URISyntaxException {
        fname = slashify(fname, false /* startWithSlash */, false /* endWithSlash */);
        final int lios = fname.lastIndexOf('/');  // strip off dirname
        if(lios>=0) {
            fname = fname.substring(0, lios+1);
        }
        return fname;
    }

    /***
     *
     * RESOURCE LOCATION HELPER
     *
     */

    /**
     * Helper compound associating a class instance and resource paths
     * to be {@link #resolve(int) resolved} at a later time.
     */
    public static class ClassResources {
        /** Optional {@link ClassLoader} used to {@link #resolve(int)} {@link #resourcePaths}. */
        public final ClassLoader classLoader;

        /** Optional class instance used to {@link #resolve(int)} relative {@link #resourcePaths}. */
        public final Class<?> contextCL;

        /** Resource paths, see {@link #resolve(int)}. */
        public final String[] resourcePaths;

        /** Returns the number of resources, i.e. <code>resourcePaths.length</code>. */
        public final int resourceCount() { return resourcePaths.length; }

        /**
         * @param resourcePaths multiple relative or absolute resource locations
         * @param classLoader optional {@link ClassLoader}, see {@link IOUtil#getResource(String, ClassLoader, Class)}
         * @param relContext optional relative context, see {@link IOUtil#getResource(String, ClassLoader, Class)}
         */
        public ClassResources(final String[] resourcePaths, final ClassLoader classLoader, final Class<?> relContext) {
            for(int i=resourcePaths.length-1; i>=0; i--) {
                if( null == resourcePaths[i] ) {
                    throw new IllegalArgumentException("resourcePath["+i+"] is null");
                }
            }
            this.classLoader = classLoader;
            this.contextCL = relContext;
            this.resourcePaths = resourcePaths;
        }

        /**
         * Resolving one of the {@link #resourcePaths} indexed by <code>uriIndex</code> using
         * {@link #classLoader}, {@link #contextCL} through {@link IOUtil#getResource(String, ClassLoader, Class)}.
         * @throws ArrayIndexOutOfBoundsException if <code>uriIndex</code> is < 0 or >= {@link #resourceCount()}.
         */
        public URLConnection resolve(final int uriIndex) throws ArrayIndexOutOfBoundsException {
            return getResource(resourcePaths[uriIndex], classLoader, contextCL);
        }
    }

    /**
     * Locating a resource using {@link #getResource(String, ClassLoader)}:
     * <ul>
     *   <li><i>relative</i>: <code>relContext</code>'s package name-path plus <code>resourcePath</code> via <code>classLoader</code>.
     *       This allows locations relative to JAR- and other URLs.
     *       The <code>resourcePath</code> may start with <code>../</code> to navigate to parent folder.
     *       This attempt is skipped if {@code relContext} is {@code null}.</li>
     *   <li><i>absolute</i>: <code>resourcePath</code> as is via <code>classLoader</code>.
     * </ul>
     * <p>
     * Returns the resolved and open URLConnection or null if not found.
     * </p>
     *
     * @param resourcePath the resource path to locate relative or absolute
     * @param classLoader the optional {@link ClassLoader}, recommended
     * @param relContext relative context, i.e. position, of the {@code resourcePath},
     *                   to perform the relative lookup, if not {@code null}.
     * @see #getResource(String, ClassLoader)
     * @see ClassLoader#getResource(String)
     * @see ClassLoader#getSystemResource(String)
     */
    public static URLConnection getResource(final String resourcePath, final ClassLoader classLoader, final Class<?> relContext) {
        if(null == resourcePath) {
            return null;
        }
        URLConnection conn = null;
        if(null != relContext) {
            // scoping the path within the class's package
            final String className = relContext.getName().replace('.', '/');
            final int lastSlash = className.lastIndexOf('/');
            if (lastSlash >= 0) {
                final String pkgName = className.substring(0, lastSlash + 1);
                conn = getResource(pkgName + resourcePath, classLoader);
                if(DEBUG) {
                    System.err.println("IOUtil: found <"+resourcePath+"> within class package <"+pkgName+"> of given class <"+relContext.getName()+">: "+(null!=conn));
                }
            }
        } else if(DEBUG) {
            System.err.println("IOUtil: null context, skip rel. lookup");
        }
        if(null == conn) {
            conn = getResource(resourcePath, classLoader);
            if(DEBUG) {
                System.err.println("IOUtil: found <"+resourcePath+"> by classloader: "+(null!=conn));
            }
        }
        return conn;
    }

    /**
     * Locating a resource using the ClassLoader's facilities and {@link org.jau.net.AssetURLContext}.
     * <p>
     * Returns the resolved and connected URLConnection or null if not found.
     * </p>
     * <p>
     * Return null if {@link org.jau.net.AssetURLContext} is not available.
     * </p>
     *
     * @see ClassLoader#getResource(String)
     * @see ClassLoader#getSystemResource(String)
     * @see URL#URL(String)
     * @see File#File(String)
     */
    public static URLConnection getResource(final String resourcePath, final ClassLoader cl) {
        if( null != aucGetRes && aucGetRes.available() ) {
            return aucGetRes.callStaticMethod(resourcePath, cl);
        } else {
            return null;
        }
    }

    /**
     * Generates a path for the 'relativeFile' relative to the 'baseLocation'.
     *
     * @param baseLocation denotes a directory
     * @param relativeFile denotes a relative file to the baseLocation
     * @throws URISyntaxException if path is empty or has no parent directory available while resolving <code>../</code>
     */
    public static String getRelativeOf(final File baseLocation, final String relativeFile) throws URISyntaxException {
        if(null == relativeFile) {
            return null;
        }

        if (baseLocation != null) {
            final File file = new File(baseLocation, relativeFile);
            // Handle things on Windows
            return slashify(file.getPath(), false /* startWithSlash */, false /* endWithSlash */);
        }
        return null;
    }

    /**
     * @param path assuming a slashified path, either denotes a file or directory, either relative or absolute.
     * @return parent of path
     * @throws URISyntaxException if path is empty or has no parent directory available
     */
    public static String getParentOf(final String path) throws URISyntaxException {
        final int pl = null!=path ? path.length() : 0;
        if(pl == 0) {
            throw new IllegalArgumentException("path is empty <"+path+">");
        }

        final int e = path.lastIndexOf("/");
        if( e < 0 ) {
            throw new URISyntaxException(path, "path contains no '/': <"+path+">");
        }
        if( e == 0 ) {
            // path is root directory
            throw new URISyntaxException(path, "path has no parents: <"+path+">");
        }
        if( e <  pl - 1 ) {
            // path is file, return it's parent directory
            return path.substring(0, e+1);
        }
        final int j = path.lastIndexOf("!") + 1; // '!' Separates JARFile entry -> local start of path
        // path is a directory ..
        final int p = path.lastIndexOf("/", e-1);
        if( p >= j) {
            // parent itself has '/' - post '!' or no '!' at all
            return path.substring(0, p+1);
        } else {
            // parent itself has no '/'
            final String parent = path.substring(j, e);
            if( parent.equals("..") ) {
                throw new URISyntaxException(path, "parent is unresolved: <"+path+">");
            } else {
                // parent is '!' or empty (relative path)
                return path.substring(0, j);
            }
        }
    }

    /**
     * @param path assuming a slashified path, either denoting a file or directory, either relative or absolute.
     * @return clean path string where {@code ./} and {@code ../} is resolved,
     *         while keeping a starting {@code ../} at the beginning of a relative path.
     * @throws URISyntaxException if path is empty or has no parent directory available while resolving <code>../</code>
     */
    public static String cleanPathString(String path) throws URISyntaxException {
        // Resolve './' before '../' to handle case 'parent/./../a.txt' properly.
        int idx = path.length() - 1;
        while ( idx >= 1 && ( idx = path.lastIndexOf("./", idx) ) >= 0 ) {
            if( 0 < idx && path.charAt(idx-1) == '.' ) {
                idx-=2; // skip '../' -> idx upfront
            } else {
                path = path.substring(0, idx) + path.substring(idx+2);
                idx--; // idx upfront
            }
        }
        idx = 0;
        while ( ( idx = path.indexOf("../", idx) ) >= 0 ) {
            if( 0 == idx ) {
                idx += 3; // skip starting '../'
            } else {
                path = getParentOf(path.substring(0, idx)) + path.substring(idx+3);
                idx = 0;
            }
        }
        return path;
    }

    public static final Pattern patternSpaceEnc = Pattern.compile("%20");

    /**
     * Returns the connected URLConnection, or null if not url is not available
     */
    public static URLConnection openURL(final URL url) {
        return openURL(url, ".");
    }

    /**
     * Returns the connected URLConnection, or null if not url is not available
     */
    public static URLConnection openURL(final URL url, final String dbgmsg) {
        if(null!=url) {
            try {
                final URLConnection c = url.openConnection();
                c.connect(); // redundant
                if(DEBUG) {
                    System.err.println("IOUtil: urlExists("+url+") ["+dbgmsg+"] - true");
                }
                return c;
            } catch (final IOException ioe) {
                if(DEBUG) {
                    ExceptionUtils.dumpThrowable("IOUtil: urlExists("+url+") ["+dbgmsg+"] - false -", ioe);
                }
            }
        } else if(DEBUG) {
            System.err.println("IOUtil: no url - urlExists(null) ["+dbgmsg+"]");
        }

        return null;
    }

    private static String getExeTestFileSuffix() {
        switch(PlatformProps.OS) {
            case WINDOWS:
              if( useNativeExeFile && PlatformTypes.CPUFamily.X86 == PlatformProps.CPU.family ) {
                  return ".exe";
              } else {
                  return ".bat";
              }
            default:
              return ".sh";
        }
    }
    private static String getExeTestShellCode() {
        switch(PlatformProps.OS) {
            case WINDOWS:
              return "echo off"+PlatformProps.NEWLINE;
            case FREEBSD:
              return "#!/bin/test"+PlatformProps.NEWLINE;
            default:
              return "#!/bin/true"+PlatformProps.NEWLINE;
        }
    }
    private static String[] getExeTestCommandArgs(final String scriptFile) {
        switch(PlatformProps.OS) {
            case WINDOWS:
            //   return new String[] { "cmd", "/c", scriptFile };
            default:
              return new String[] { scriptFile };
        }
    }

    private static void fillExeTestFile(final File exefile) throws IOException {
        final String shellCode = getExeTestShellCode();
        if( isStringSet(shellCode) ) {
            final FileWriter fout = new FileWriter(exefile);
            try {
                fout.write(shellCode);
                try {
                    fout.flush();
                } catch (final IOException sfe) {
                    ExceptionUtils.dumpThrowable("", sfe);
                }
            } finally {
                fout.close();
            }
        }
    }
    private static boolean getOSHasNoexecFS() {
        switch(PlatformProps.OS) {
            case OPENKODE:
              return false;

            default:
              return true;
        }
    }

    /**
     * @see <a href="http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html">Free-Desktop - XDG Base Directory Specification</a>
     */
    private static boolean getOSHasFreeDesktopXDG() {
        switch(PlatformProps.OS) {
            case ANDROID:
            case MACOS:
            case IOS:
            case WINDOWS:
            case OPENKODE:
              return false;

            default:
              return true;
        }
    }

    /**
     * Test whether {@code file} exists and matches the given requirements
     *
     * @param file
     * @param shallBeDir
     * @param shallBeWritable
     * @return
     */
    public static boolean testFile(final File file, final boolean shallBeDir, final boolean shallBeWritable) {
        if (!file.exists()) {
            if(DEBUG) {
                System.err.println("IOUtil.testFile: <"+file.getAbsolutePath()+">: does not exist");
            }
            return false;
        }
        if (shallBeDir && !file.isDirectory()) {
            if(DEBUG) {
                System.err.println("IOUtil.testFile: <"+file.getAbsolutePath()+">: is not a directory");
            }
            return false;
        }
        if (shallBeWritable && !file.canWrite()) {
            if(DEBUG) {
                System.err.println("IOUtil.testFile: <"+file.getAbsolutePath()+">: is not writable");
            }
            return false;
        }
        return true;
    }

    public static class StreamMonitor implements Runnable {
        private final InputStream[] istreams;
        private final boolean[] eos;
        private final PrintStream ostream;
        private final String prefix;
        public StreamMonitor(final InputStream[] streams, final PrintStream ostream, final String prefix) {
            this.istreams = streams;
            this.eos = new boolean[streams.length];
            this.ostream = ostream;
            this.prefix = prefix;
            final InterruptSource.Thread t = new InterruptSource.Thread(null, this, "StreamMonitor-"+Thread.currentThread().getName());
            t.setDaemon(true);
            t.start();
        }

        @Override
        public void run()
        {
            final byte[] buffer = new byte[4096];
            try {
                final int streamCount = istreams.length;
                int eosCount = 0;
                do {
                    for(int i=0; i<istreams.length; i++) {
                        if( !eos[i] ) {
                            final int numReadI = istreams[i].read(buffer);
                            if (numReadI > 0) {
                                if( null != ostream ) {
                                    if( null != prefix ) {
                                        ostream.write(prefix.getBytes());
                                    }
                                    ostream.write(buffer, 0, numReadI);
                                }
                            } else {
                                // numReadI == -1
                                eosCount++;
                                eos[i] = true;
                            }
                        }
                    }
                    if( null != ostream ) {
                        ostream.flush();
                    }
                } while ( eosCount < streamCount );
            } catch (final IOException e) {
            } finally {
                if( null != ostream ) {
                    ostream.flush();
                }
                // Should allow clean exit when process shuts down
            }
        }
    }

    private static final Boolean isNioExecutableFile(final File file) {
        try {
            return java.nio.file.Files.isExecutable( file.toPath() );
        } catch (final Throwable t) {
            throw new RuntimeException("error invoking Files.isExecutable(file.toPath())", t);
        }
    }

    /**
     * Returns true if the given {@code dir}
     * <ol>
     *   <li>exists, and</li>
     *   <li>is a directory, and</li>
     *   <li>is writeable, and</li>
     *   <li>files can be executed from the directory</li>
     * </ol>
     *
     * @throws SecurityException if file creation and process execution is not allowed within the current security context
     * @param dir
     */
    public static boolean testDirExec(final File dir)
            throws SecurityException
    {
        final boolean debug = DEBUG_EXE || DEBUG;

        if( !testTempDirExec ) {
            if(DEBUG) {
                System.err.println("IOUtil.testDirExec: <"+dir.getAbsolutePath()+">: Disabled TestTempDirExec");
            }
            return false;
        }
        if (!testFile(dir, true, true)) {
            if( debug ) {
                System.err.println("IOUtil.testDirExec: <"+dir.getAbsolutePath()+">: Not writeable dir");
            }
            return false;
        }
        if(!getOSHasNoexecFS()) {
            if( debug ) {
                System.err.println("IOUtil.testDirExec: <"+dir.getAbsolutePath()+">: Always executable");
            }
            return true;
        }

        final long t0 = debug ? System.currentTimeMillis() : 0;
        final File exeTestFile;
        final boolean existingExe;
        try {
            final File permExeTestFile = DEBUG_EXE_EXISTING_FILE ? new File(dir, "jau_exe_tst"+getExeTestFileSuffix()) : null;
            if( null != permExeTestFile && permExeTestFile.exists() ) {
                exeTestFile = permExeTestFile;
                existingExe = true;
            } else {
                exeTestFile = File.createTempFile("jau_exe_tst", getExeTestFileSuffix(), dir);
                existingExe = false;
                fillExeTestFile(exeTestFile);
            }
        } catch (final SecurityException se) {
            throw se; // fwd Security exception
        } catch (final IOException e) {
            if( debug ) {
                e.printStackTrace();
            }
            return false;
        }
        final long t1 = debug ? System.currentTimeMillis() : 0;
        long t2;
        int res = -1;
        int exitValue = -1;
        Boolean isNioExec = null;
        if( existingExe || exeTestFile.setExecutable(true /* exec */, true /* ownerOnly */) ) {
            t2 = debug ? System.currentTimeMillis() : 0;
            // First soft exec test via NIO's ACL check, if available
            isNioExec = isNioExecutableFile(exeTestFile);
            if( null != isNioExec ) {
                res = isNioExec.booleanValue() ? 0 : -1;
            }
            if( null == isNioExec || 0 <= res ) {
                // Hard exec test via actual execution, if NIO's ACL check succeeded or not available.
                // Required, since Windows 'Software Restriction Policies (SRP)' won't be triggered merely by NIO's ACL check.
                Process pr = null;
                try {
                    // Using 'Process.exec(String[])' avoids StringTokenizer of 'Process.exec(String)'
                    // and hence splitting up command by spaces!
                    // Note: All no-exec cases throw an IOExceptions at ProcessBuilder.start(), i.e. below exec() call!
                    pr = Runtime.getRuntime().exec( getExeTestCommandArgs( exeTestFile.getCanonicalPath() ), null, null );
                    if( DEBUG_EXE && !DEBUG_EXE_NOSTREAM ) {
                        new StreamMonitor(new InputStream[] { pr.getInputStream(), pr.getErrorStream() }, System.err, "Exe-Tst: ");
                    }
                    pr.waitFor();
                    exitValue = pr.exitValue(); // Note: Bug 1219 Comment 50: On reporter's machine exit value 1 is being returned
                    if( 0 == exitValue ) {
                        res++; // file has been executed and exited normally
                    } else {
                        res = -2; // abnormal termination
                    }
                } catch (final SecurityException se) {
                    throw se; // fwd Security exception
                } catch (final Throwable t) {
                    t2 = debug ? System.currentTimeMillis() : 0;
                    res = -3;
                    if( debug ) {
                        System.err.println("IOUtil.testDirExec: <"+exeTestFile.getAbsolutePath()+">: Caught "+t.getClass().getSimpleName()+": "+t.getMessage());
                        t.printStackTrace();
                    }
                } finally {
                    if( null != pr ) {
                        // Bug 1219 Comment 58: Ensure that the launched process gets terminated!
                        // This is Process implementation specific and varies on different platforms,
                        // hence it may be required.
                        try {
                            pr.destroy();
                        } catch (final Throwable t) {
                            ExceptionUtils.dumpThrowable("", t);
                        }
                    }
                }
            }
        } else {
            t2 = debug ? System.currentTimeMillis() : 0;
        }

        final boolean ok = 0 <= res;
        if( !DEBUG_EXE && !existingExe ) {
            exeTestFile.delete();
        }
        if( debug ) {
            final long t3 = System.currentTimeMillis();
            System.err.println("IOUtil.testDirExec(): test-exe <"+exeTestFile.getAbsolutePath()+">, existingFile "+existingExe+", isNioExec "+isNioExec+", returned "+exitValue);
            System.err.println("IOUtil.testDirExec(): abs-path <"+dir.getAbsolutePath()+">: res "+res+" -> "+ok);
            System.err.println("IOUtil.testDirExec(): total "+(t3-t0)+"ms, create "+(t1-t0)+"ms, fill "+(t2-t1)+"ms, execute "+(t3-t2)+"ms");
        }
        return ok;
    }

    private static File testDirImpl(final File dir, final boolean create, final boolean executable, final String dbgMsg)
            throws SecurityException
    {
        final File res;
        if (create && !dir.exists()) {
            dir.mkdirs();
        }
        if( executable ) {
            res = testDirExec(dir) ? dir : null;
        } else {
            res = testFile(dir, true, true) ? dir : null;
        }
        if(DEBUG) {
            System.err.println("IOUtil.testDirImpl("+dbgMsg+"): <"+dir.getAbsolutePath()+">, create "+create+", exec "+executable+": "+(null != res));
        }
        return res;
    }

    /**
     * Returns the directory {@code dir}, which is processed and tested as described below.
     * <ol>
     *   <li>If {@code create} is {@code true} and the directory does not exist yet, it is created incl. all sub-directories.</li>
     *   <li>If {@code dirName} exists, but is not a directory, {@code null} is being returned.</li>
     *   <li>If the directory does not exist or is not writeable, {@code null} is being returned.</li>
     *   <li>If {@code executable} is {@code true} and files cannot be executed from the directory, {@code null} is being returned.</li>
     * </ol>
     *
     * @param dir the directory to process
     * @param create true if the directory shall be created if not existing
     * @param executable true if the user intents to launch executables from the temporary directory, otherwise false.
     * @throws SecurityException if file creation and process execution is not allowed within the current security context
     */
    public static File testDir(final File dir, final boolean create, final boolean executable)
        throws SecurityException
    {
        return testDirImpl(dir, create, executable, "testDir");
    }

    private static boolean isStringSet(final String s) { return null != s && 0 < s.length(); }

    /**
     * This methods finds [and creates] an available temporary sub-directory:
     * <pre>
           File tmpBaseDir = null;
           if(null != testDir(tmpRoot, true, executable)) { // check tmpRoot first
               for(int i = 0; null == tmpBaseDir && i<=9999; i++) {
                   final String tmpDirSuffix = String.format("_%04d", i); // 4 digits for iteration
                   tmpBaseDir = testDir(new File(tmpRoot, tmpSubDirPrefix+tmpDirSuffix), true, executable);
               }
           } else {
               tmpBaseDir = null;
           }
           return tmpBaseDir;
     * </pre>
     * <p>
     * The iteration through [0000-9999] ensures that the code is multi-user save.
     * </p>
     * @param tmpRoot
     * @param executable
     * @param dbgMsg
     * @param tmpDirPrefix
     * @return a temporary directory, writable by this user
     * @throws SecurityException
     */
    private static File getSubTempDir(final File tmpRoot, final String tmpSubDirPrefix, final boolean executable, final String dbgMsg)
        throws SecurityException
    {
       File tmpBaseDir = null;
       if(null != testDirImpl(tmpRoot, true /* create */, executable, dbgMsg)) { // check tmpRoot first
           for(int i = 0; null == tmpBaseDir && i<=9999; i++) {
               final String tmpDirSuffix = String.format((Locale)null, "_%04d", i); // 4 digits for iteration
               tmpBaseDir = testDirImpl(new File(tmpRoot, tmpSubDirPrefix+tmpDirSuffix), true /* create */, executable, dbgMsg);
           }
       }
       return tmpBaseDir;
    }

    private static File getFile(final String fname) {
        if( isStringSet(fname) ) {
            return new File(fname);
        } else {
            return null;
        }

    }
    /**
     * Returns a platform independent writable directory for temporary files
     * consisting of the platform's {@code temp-root} + {@link #tmpSubDir},
     * e.g. {@code /tmp/jau_0000/}.
     * <p>
     * On standard Java the {@code temp-root} folder is specified by <code>java.io.tempdir</code>.
     * </p>
     * <p>
     * On Android the {@code temp-root} folder is relative to the applications local folder
     * (see {@link Context#getDir(String, int)}) is returned, if
     * the Android application/activity has registered it's Application Context
     * via {@link jogamp.common.os.android.StaticContext.StaticContext#init(Context, ClassLoader) StaticContext.init(..)}.
     * This allows using the temp folder w/o the need for <code>sdcard</code>
     * access, which would be the <code>java.io.tempdir</code> location on Android!
     * </p>
     * <p>
     * In case {@code temp-root} is the users home folder,
     * a dot is being prepended to {@link #tmpSubDir}, i.e.: {@code /home/user/.jau_0000/}.
     * </p>
     * @param executable true if the user intents to launch executables from the temporary directory, otherwise false.
     * @throws IOException if no temporary directory could be determined
     * @throws SecurityException if access to <code>java.io.tmpdir</code> is not allowed within the current security context
     *
     * @see PropertyAccess#getProperty(String, boolean)
     * @see Context#getDir(String, int)
     */
    public static File getTempDir(final boolean executable)
        throws SecurityException, IOException
    {
        if(!tempRootSet) { // volatile: ok
            synchronized(IOUtil.class) {
                if(!tempRootSet) {
                    tempRootSet = true;
                    {
                        final File ctxTempDir = AndroidUtil.getTempRoot(); // null if ( !Android || no android-ctx )
                        if(null != ctxTempDir) {
                            tempRootNoexec = getSubTempDir(ctxTempDir, tmpSubDir, false /* executable, see below */, "Android.ctxTemp");
                            tempRootExec = tempRootNoexec; // FIXME: Android temp root is always executable (?)
                            return tempRootExec;
                        }
                    }

                    final File java_io_tmpdir = getFile( PropertyAccess.getProperty(java_io_tmpdir_propkey, false) );
                    if(DEBUG) {
                        System.err.println("IOUtil.getTempRoot(): tempX1 <"+java_io_tmpdir+">, used "+(null!=java_io_tmpdir));
                    }

                    final File user_tmpdir; // only if diff than java_io_tmpdir
                    {
                        String __user_tmpdir = System.getenv("TMPDIR");
                        if( !isStringSet(__user_tmpdir) ) {
                            __user_tmpdir = System.getenv("TEMP");
                        }
                        final File _user_tmpdir = getFile(__user_tmpdir);
                        if( null != _user_tmpdir && !_user_tmpdir.equals(java_io_tmpdir) ) {
                            user_tmpdir = _user_tmpdir;
                        } else {
                            user_tmpdir = null;
                        }
                        if(DEBUG) {
                            System.err.println("IOUtil.getTempRoot(): tempX3 <"+_user_tmpdir+">, used "+(null!=user_tmpdir));
                        }
                    }

                    final File user_home = getFile( PropertyAccess.getProperty(user_home_propkey, false) );
                    if(DEBUG) {
                        System.err.println("IOUtil.getTempRoot(): tempX4 <"+user_home+">, used "+(null!=user_home));
                    }

                    final File xdg_cache_home;
                    {
                        String __xdg_cache_home;
                        if( getOSHasFreeDesktopXDG() ) {
                            __xdg_cache_home = System.getenv(XDG_CACHE_HOME_envkey);
                            if( !isStringSet(__xdg_cache_home) && null != user_home ) {
                                __xdg_cache_home = user_home.getAbsolutePath() + File.separator + ".cache" ; // default
                            }
                        } else {
                            __xdg_cache_home = null;
                        }
                        final File _xdg_cache_home = getFile(__xdg_cache_home);
                        if( null != _xdg_cache_home && !_xdg_cache_home.equals(java_io_tmpdir) ) {
                            xdg_cache_home = _xdg_cache_home;
                        } else {
                            xdg_cache_home = null;
                        }
                        if(DEBUG) {
                            System.err.println("IOUtil.getTempRoot(): tempX2 <"+_xdg_cache_home+">, used "+(null!=xdg_cache_home));
                        }
                    }

                    // 1) java.io.tmpdir/jau
                    if( null == tempRootExec && null != java_io_tmpdir ) {
                        if( PlatformTypes.OSType.MACOS == PlatformProps.OS || PlatformTypes.OSType.IOS == PlatformProps.OS ) {
                            // Bug 865: Safari >= 6.1 [OSX] May employ xattr on 'com.apple.quarantine' on 'PluginProcess.app'
                            // We attempt to fix this issue _after_ gluegen native lib is loaded, see JarUtil.fixNativeLibAttribs(File).
                            tempRootExec = getSubTempDir(java_io_tmpdir, tmpSubDir, false /* executable */, "tempX1");
                        } else {
                            tempRootExec = getSubTempDir(java_io_tmpdir, tmpSubDir, true /* executable */, "tempX1");
                        }
                    }

                    // 2) $XDG_CACHE_HOME/jau
                    if( null == tempRootExec && null != xdg_cache_home ) {
                        tempRootExec = getSubTempDir(xdg_cache_home, tmpSubDir, true /* executable */, "tempX2");
                    }

                    // 3) $TMPDIR/jau
                    if( null == tempRootExec && null != user_tmpdir ) {
                        tempRootExec = getSubTempDir(user_tmpdir, tmpSubDir, true /* executable */, "tempX3");
                    }

                    // 4) $HOME/.jau
                    if( null == tempRootExec && null != user_home ) {
                        tempRootExec = getSubTempDir(user_home, "." + tmpSubDir, true /* executable */, "tempX4");
                    }

                    if( null != tempRootExec ) {
                        tempRootNoexec = tempRootExec;
                    } else {
                        // 1) java.io.tmpdir/jau
                        if( null == tempRootNoexec && null != java_io_tmpdir ) {
                            tempRootNoexec = getSubTempDir(java_io_tmpdir, tmpSubDir, false /* executable */, "temp01");
                        }

                        // 2) $XDG_CACHE_HOME/jau
                        if( null == tempRootNoexec && null != xdg_cache_home ) {
                            tempRootNoexec = getSubTempDir(xdg_cache_home, tmpSubDir, false /* executable */, "temp02");
                        }

                        // 3) $TMPDIR/jau
                        if( null == tempRootNoexec && null != user_tmpdir ) {
                            tempRootNoexec = getSubTempDir(user_tmpdir, tmpSubDir, false /* executable */, "temp03");
                        }

                        // 4) $HOME/.jau
                        if( null == tempRootNoexec && null != user_home ) {
                            tempRootNoexec = getSubTempDir(user_home, "." + tmpSubDir, false /* executable */, "temp04");
                        }
                    }

                    if(DEBUG) {
                        final String tempRootExecAbsPath = null != tempRootExec ? tempRootExec.getAbsolutePath() : null;
                        final String tempRootNoexecAbsPath = null != tempRootNoexec ? tempRootNoexec.getAbsolutePath() : null;
                        System.err.println("IOUtil.getTempRoot(): temp dirs: exec: "+tempRootExecAbsPath+", noexec: "+tempRootNoexecAbsPath);
                    }
                }
            }
        }
        final File r = executable ? tempRootExec : tempRootNoexec ;
        if(null == r) {
            final String exe_s = executable ? "executable " : "";
            throw new IOException("Could not determine a temporary "+exe_s+"directory");
        }
        final FilePermission fp = new FilePermission(r.getAbsolutePath(), "read,write,delete");
        SecurityUtil.checkPermission(fp);
        return r;
    }
    private static File tempRootExec = null; // writeable and executable
    private static File tempRootNoexec = null; // writeable, maybe executable
    private static volatile boolean tempRootSet = false;

    /**
     * Utilizing {@link File#createTempFile(String, String, File)} using
     * {@link #getTempDir(boolean)} as the directory parameter, ie. location
     * of the root temp folder.
     *
     * @see File#createTempFile(String, String)
     * @see File#createTempFile(String, String, File)
     * @see #getTempDir(boolean)
     *
     * @param prefix
     * @param suffix
     * @param executable true if the temporary root folder needs to hold executable files, otherwise false.
     * @return
     * @throws IllegalArgumentException
     * @throws IOException if no temporary directory could be determined or temp file could not be created
     * @throws SecurityException
     */
    public static File createTempFile(final String prefix, final String suffix, final boolean executable)
        throws IllegalArgumentException, IOException, SecurityException
    {
        return File.createTempFile( prefix, suffix, getTempDir(executable) );
    }

    public static void close(final Closeable stream, final boolean throwRuntimeException) throws RuntimeException {
        if(null != stream) {
            try {
                stream.close();
            } catch (final IOException e) {
                if(throwRuntimeException) {
                    throw new RuntimeException(e);
                } else if(DEBUG) {
                    System.err.println("Caught Exception: ");
                    e.printStackTrace();
                }
            }
        }
    }

    /**
     * Helper to simplify closing {@link Closeable}s.
     *
     * @param stream the {@link Closeable} instance to close
     * @param saveOneIfFree cache for one {@link IOException} to store, if not already used (excess)
     * @param dumpExcess dump the excess {@link IOException} on this {@link PrintStream}
     * @return the excess {@link IOException} or {@code null}.
     */
    public static IOException close(final Closeable stream, final IOException[] saveOneIfFree, final PrintStream dumpExcess) {
        try {
            stream.close();
        } catch(final IOException e) {
            if( null == saveOneIfFree[0] ) {
                saveOneIfFree[0] = e;
            } else {
                if( null != dumpExcess ) {
                    dumpExcess.println("Caught "+e.getClass().getSimpleName()+": "+e.getMessage());
                    e.printStackTrace(dumpExcess);
                }
                return e;
            }
        }
        return null;
    }

    /**
     * Retrieve the list of all filenames traversing through given paths
     * @param paths list of paths to traverse through, containing directories and files
     * @param excludes optional list of exclude {@link Pattern}. All {@link Pattern#matcher(CharSequence) matching} files or directories will be omitted. Maybe be null or empty.
     * @param includes optional list of explicit include {@link Pattern}. If given, only {@link Pattern#matcher(CharSequence) matching} files will be returned, otherwise all occurring.
     * @return list of unsorted filenames within given paths
     */
    public static ArrayList<String> filesOf(final List<String> paths, final List<Pattern> excludes, final List<Pattern> includes) {
        final ArrayList<String> files = new ArrayList<String>(paths.size()*32);
        final ArrayList<String> todo = new ArrayList<String>(paths);
        while(todo.size() > 0) {
            final String p = todo.remove(0);
            if( null != excludes && excludes.size() > 0) {
                boolean exclude = false;
                for(int i=0; !exclude && i<excludes.size(); i++) {
                    exclude = excludes.get(i).matcher(p).matches();
                    if( DEBUG ) {
                        if( exclude ) {
                            System.err.println("IOUtil.filesOf(): excluding <"+p+"> (exclude["+i+"]: "+excludes.get(i)+")");
                        }
                    }
                }
                if( exclude ) {
                    continue; // skip further processing, continue w/ next path
                }
            }
            final File f = new File(p);
            if( !f.exists() ) {
                if( DEBUG ) {
                    System.err.println("IOUtil.filesOf(): not existing: "+f);
                }
                continue;
            } else if( f.isDirectory() ) {
                final String[] subs = f.list();
                if( null == subs ) {
                    if( DEBUG ) {
                        System.err.println("IOUtil.filesOf(): null list of directory: "+f);
                    }
                } else if( 0 == subs.length ) {
                    if( DEBUG ) {
                        System.err.println("IOUtil.filesOf(): empty list of directory: "+f);
                    }
                } else {
                    int j=0;
                    final String pp = p.endsWith("/") ? p : p+"/";
                    for(int i=0; i<subs.length; i++) {
                        todo.add(j++, pp+subs[i]); // add 'in-place' to soothe the later sorting algorithm
                    }
                }
            } else {
                if( null != includes && includes.size() > 0) {
                    boolean include = false;
                    for(int i=0; !include && i<includes.size(); i++) {
                        include = includes.get(i).matcher(p).matches();
                        if( DEBUG ) {
                            if( include ) {
                                System.err.println("IOUtil.filesOf(): including <"+p+"> (including["+i+"]: "+includes.get(i)+")");
                            }
                        }
                    }
                    if( include ) {
                        files.add(p);
                    }
                } else {
                    files.add(p);
                }
            }
        }
        return files;
    }
}
