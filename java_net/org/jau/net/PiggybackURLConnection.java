/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2012 Gothel Software e.K.
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
package org.jau.net;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;

/**
 * Generic resource location protocol connection,
 * using another sub-protocol as the vehicle for a piggyback protocol.
 * <p>
 * The details of the sub-protocol can be queried using {@link #getSubProtocol()}.
 * </p>
 * <p>
 * See example in {@link AssetURLConnection}.
 * </p>
 */
public abstract class PiggybackURLConnection<I extends PiggybackURLContext> extends URLConnection {
    protected URL subUrl;
    protected URLConnection subConn;
    protected I context;

    /**
     * @param url the specific URL for this instance
     * @param context the piggyback context, defining state independent code and constants
     */
    protected PiggybackURLConnection(final URL url, final I context) {
        super(url);
        this.context = context;
    }

    /**
     * <p>
     * Resolves the URL via {@link PiggybackURLContext#resolve(String)},
     * see {@link AssetURLContext#resolve(String)} for an example.
     * </p>
     *
     * {@inheritDoc}
     */
    @Override
    public synchronized void connect() throws IOException {
        if(!connected) {
            subConn = context.resolve(url.getPath());
            subUrl = subConn.getURL();
            connected = true;
        }
    }

    @Override
    public InputStream getInputStream() throws IOException {
        if(!connected) {
            throw new IOException("not connected");
        }
        return subConn.getInputStream();
    }

    /**
     * Returns the <i>entry name</i> of the asset.
     * <pre>
     * Plain     asset:test/lala.txt
     * Resolved  asset:jar:file:/data/app/jogamp.test.apk!/assets/test/lala.txt
     * Result          test/lala.txt
     * </pre>
     * @throws IOException is not connected
     **/
    public abstract String getEntryName() throws IOException;

    /**
     * Returns the resolved <i>sub protocol</i> of the asset or null, ie:
     * <pre>
     * Plain     asset:test/lala.txt
     * Resolved  asset:jar:file:/data/app/jogamp.test.apk!/assets/test/lala.txt
     * Result          jar:file:/data/app/jogamp.test.apk!/assets/test/lala.txt
     * </pre>
     *
     * @throws IOException is not connected
     */
    public URL getSubProtocol() throws IOException {
        if(!connected) {
            throw new IOException("not connected");
        }
        return subUrl;
    }
}
