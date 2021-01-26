/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2014 Gothel Software e.K.
 * Copyright (c) 2014 JogAmp Community.
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
package jau.test.junit.util;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import org.jau.io.IOUtil;

public class MiscUtils {
    public static class CopyStats {
        public int totalBytes = 0;
        public int totalFiles = 0;
        public int totalFolders = 0;
        public int currentDepth = 0;
        public int maxDepth = 0;
        public boolean trackFiles;
        public final List<File> srcFiles = new ArrayList<File>();
        public final List<File> dstFiles = new ArrayList<File>();

        public void dump(final String prefix, final boolean folderOnly) {
            System.err.println(prefix+"Total bytes: "+totalBytes);
            System.err.println(prefix+"Total files: "+totalFiles);
            System.err.println(prefix+"Total folder: "+totalFolders);
            System.err.println(prefix+"Depth: "+currentDepth/maxDepth);
            System.err.println(prefix+"Tracking: "+trackFiles);
            if( trackFiles ) {
                for(int i=0; i<srcFiles.size(); i++) {
                    final File src = srcFiles.get(i);
                    final File dst = dstFiles.get(i);
                    if( !folderOnly || src.isDirectory() ) {
                        System.err.printf("%s\t src %4d: %s%n", prefix, i, src.toString());
                        System.err.printf("%s\t dst %4d: %s%n%n", prefix, i, dst.toString());
                    }
                }
            }
        }
    }
    public static CopyStats copy(final File src, final File dest, final int maxDepth, final boolean trackFiles) throws IOException {
        final CopyStats cs = new CopyStats();
        cs.maxDepth = maxDepth;
        cs.trackFiles = trackFiles;
        copy(src, dest, cs);
        return cs;
    }
    private static void copy(final File src, final File dest, final CopyStats stats) throws IOException {
        if(src.isDirectory()){
            if( stats.maxDepth >= 0 && stats.currentDepth >= stats.maxDepth ) {
                return;
            }
            stats.totalFolders++;
            if( stats.trackFiles ) {
                stats.srcFiles.add(src);
                stats.dstFiles.add(dest);
            }
            stats.currentDepth++;
            if(!dest.exists()){
                dest.mkdirs();
            }
            final String fileNames[] = src.list();
            for (int i=0; i<fileNames.length; i++) {
                final String fileName = fileNames[i];
                final File srcFile = new File(src, fileName);
                final File destFile = new File(dest, fileName);
                copy(srcFile, destFile, stats);
            }
            stats.currentDepth--;
        } else {
            stats.totalFiles++;
            if( stats.trackFiles ) {
                stats.srcFiles.add(src);
                stats.dstFiles.add(dest);
            }
            final InputStream in = new BufferedInputStream(new FileInputStream(src));
            try {
                stats.totalBytes += IOUtil.copyStream2File(in, dest, 0);
            } finally {
                in.close();
            }
        }
    }
}
