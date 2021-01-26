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

package jau.test.util.parallel.locks.impl;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.channels.FileLock;

import org.jau.lang.InterruptSource;

import jau.test.util.parallel.locks.SingletonInstance;

public class SingletonInstanceFileLock extends SingletonInstance {

    static final String temp_file_path;

    static {
        String s = null;
        try {
            final File tmpFile = File.createTempFile("TEST", "tst");
            final String absTmpFile = tmpFile.getCanonicalPath();
            tmpFile.delete();
            s = absTmpFile.substring(0, absTmpFile.lastIndexOf(File.separator));
        } catch (final IOException ex) {
            ex.printStackTrace();
        }
        temp_file_path = s;
    }

    public static String getCanonicalTempPath() {
        return temp_file_path;
    }

    public static String getCanonicalTempLockFilePath(final String basename) {
        return getCanonicalTempPath() + File.separator + basename;
    }

    public SingletonInstanceFileLock(final long poll_ms, final String lockFileBasename) {
        super(poll_ms);
        file = new File ( getCanonicalTempLockFilePath ( lockFileBasename ) );
        setupFileCleanup();
    }

    public SingletonInstanceFileLock(final long poll_ms, final File lockFile) {
        super(poll_ms);
        file = lockFile ;
        setupFileCleanup();
    }

    @Override
    public final String getName() { return file.getPath(); }

    private void setupFileCleanup() {
        file.deleteOnExit();
        Runtime.getRuntime().addShutdownHook(new InterruptSource.Thread() {
            @Override
            public void run() {
                if(isLocked()) {
                    System.err.println(infoPrefix()+" XXX "+SingletonInstanceFileLock.this.getName()+" - Unlock @ JVM Shutdown");
                }
                unlock();
            }
        });
    }

    @Override
    protected boolean tryLockImpl() {
        try {
            randomAccessFile = new RandomAccessFile(file, "rw");
            fileLock = randomAccessFile.getChannel().tryLock();

            if (fileLock != null) {
                return true;
            }
        } catch (final Exception e) {
            System.err.println(infoPrefix()+" III "+getName()+" - Unable to create and/or lock file");
            e.printStackTrace();
        }
        return false;
    }

    @Override
    protected boolean unlockImpl() {
        try {
            if(null != fileLock) {
                fileLock.release();
                fileLock = null;
            }
            if(null != randomAccessFile) {
                randomAccessFile.close();
                randomAccessFile = null;
            }
            if(null != file) {
                file.delete();
            }
            return true;
        } catch (final Exception e) {
            System.err.println(infoPrefix()+" EEE "+getName()+" - Unable to remove lock file");
            e.printStackTrace();
        } finally {
            fileLock = null;
            randomAccessFile = null;
        }
        return false;
    }

    private final File file;
    private RandomAccessFile randomAccessFile = null;
    private FileLock fileLock = null;
}
