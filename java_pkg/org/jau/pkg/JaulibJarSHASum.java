/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2010 Gothel Software e.K.
 * Copyright (c) 2010 JogAmp Community
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
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

import org.jau.base.JaulibVersion;
import org.jau.sec.SHASum;

/**
 * jaulib definition of {@link TempJarSHASum}'s specialization of {@link SHASum}.
 * <p>
 * Implementation uses {@link org.jau.pkg.cache.TempJarCache}.
 * </p>
 * <p>
 * Constructor defines the includes and excludes as used for jaulib {@link SHASum} computation.
 * </p>
 */
public class JaulibJarSHASum extends TempJarSHASum {
    /**
     * See {@link JaulibJarSHASum}
     * @throws SecurityException
     * @throws IllegalArgumentException
     * @throws NoSuchAlgorithmException
     * @throws IOException
     * @throws URISyntaxException
     */
    public JaulibJarSHASum()
            throws SecurityException, IllegalArgumentException, NoSuchAlgorithmException, IOException, URISyntaxException
    {
        super(MessageDigest.getInstance("SHA-256"), JaulibVersion.class, new ArrayList<Pattern>(), new ArrayList<Pattern>());
        final List<Pattern> excludes = getExcludes();
        final List<Pattern> includes = getIncludes();
        final String origin = getOrigin();
        excludes.add(Pattern.compile(origin+"/jau/sys/android"));
        includes.add(Pattern.compile(origin+"/org/jau/.*"));
        includes.add(Pattern.compile(origin+"/jau/.*"));
    }
}