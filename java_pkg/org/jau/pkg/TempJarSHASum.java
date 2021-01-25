/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2019 Gothel Software e.K.
 * Copyright (c) 2019 JogAmp Community.
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
package org.jau.pkg;

import java.io.IOException;
import java.net.URISyntaxException;
import java.security.MessageDigest;
import java.util.Arrays;
import java.util.List;
import java.util.regex.Pattern;

import org.jau.io.IOUtil;
import org.jau.pkg.cache.TempJarCache;
import org.jau.sec.SHASum;

/**
 * {@link SHASum} specialization utilizing {@link TempJarCache} to access jar file content for SHA computation
 */
public class TempJarSHASum extends SHASum {
    /**
     * Instance to ensure proper {@link #compute(boolean)} of identical SHA sums over same contents within given paths across machines.
     * <p>
     * Instantiation of this class is lightweight, {@link #compute(boolean)} performs all operations.
     * </p>
     * <p>
     * {@link TempJarCache#getTempFileCache()}'s {@link TempFileCache#getTempDir()} is used as origin for {@link IOUtil#filesOf(List, List, List)}
     * </p>
     *
     * @param digest the SHA algorithm
     * @param jarclazz a class from the desired classpath jar file used for {@link TempJarCache#addAll(Class, com.jogamp.common.net.Uri)}
     * @param excludes the optional exclude patterns to be used for {@link IOUtil#filesOf(List, List, List)}
     * @param includes the optional include patterns to be used for {@link IOUtil#filesOf(List, List, List)}
     * @throws SecurityException
     * @throws IllegalArgumentException
     * @throws IOException
     * @throws URISyntaxException
     */
    public TempJarSHASum(final MessageDigest digest, final Class<?> jarclazz, final List<Pattern> excludes, final List<Pattern> includes)
            throws SecurityException, IllegalArgumentException, IOException, URISyntaxException
    {
        super(digest, Arrays.asList(IOUtil.slashify(TempJarCache.getTempFileCache().getTempDir().getAbsolutePath(), false, false)),
              excludes, includes);
        TempJarCache.addAll(jarclazz, JarUtil.getJarFileUri(jarclazz.getName(), jarclazz.getClassLoader()));
    }

    public final String getOrigin() { return getOrigins().get(0); }
}