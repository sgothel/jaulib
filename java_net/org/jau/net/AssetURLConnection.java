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
import java.net.JarURLConnection;
import java.net.URL;

/**
 * See base class {@link PiggybackURLConnection} for motivation.
 *
 * <p>
 * <i>asset</i> resource location protocol connection.
 * </p>
 *
 * <p>
 * See {@link AssetURLContext#resolve(String)} how resources are being resolved.
 * </p>
 *
 * <h3>Example:</h3>
 *
 * Assuming the plain <i>asset entry</i> <b><code>test/lala.txt</code></b> is being resolved by
 * a class <code>test.LaLaTest</code>, ie. using the <i>asset aware</i> ClassLoader,
 * one would use the following <i>asset</i> aware filesystem layout:
 *
 * <pre>
 *  test/LaLaTest.class
 *  assets/test/lala.txt
 * </pre>
 *
 * The above maybe on a plain filesystem, or within a JAR or an APK file,
 * e.g. <code>jogamp.test.apk</code>.
 *
 * The above would result in the following possible URLs
 * reflecting the plain and resolved state of the <i>asset URL</i>:
 * <pre>
 *  0 Entry          test/lala.txt
 *  1 Plain    asset:test/lala.txt
 *  2 Resolved asset:jar:file:/data/app/jogamp.test.apk!/assets/test/lala.txt
 * </pre>
 *
 * <p>
 * The sub protocol URL of the resolved <i>asset</i>
 * <pre>
 *  3 Sub-URL        jar:file:/data/app/jogamp.test.apk!/assets/test/lala.txt
 * </pre>
 * can be retrieved using {@link #getSubProtocol()}.
 * </p>
 *
 * In all above cases, the <i>asset entry</i> is <b><code>test/lala.txt</code></b>,
 * which can be retrieved via {@link #getEntryName()}.
 *
 * <p>
 * <h3>General Implementation Notes:</h3>
 * An <i>asset</i> URL is resolved using {@link AssetURLContext#getClassLoader()}.{@link ClassLoader#getResource(String) getResource(String)},
 * hence the only requirement for an implementation is to have an <i>asset</i> aware ClassLoader
 * as described in  {@link AssetURLContext#getClassLoader()}.
 * </p>
 * <p>
 * <h3>Warning:</h3>
 * Since the <i>asset</i> protocol is currently not being implemented
 * on all platform with an appropriate ClassLoader, a user shall not create the <i>asset</i> URL manually.<br>
 * </p>
 *
 * <h3>Android Implementation Notes:</h3>
 * <p>
 * The Android ClassLoader {@link jogamp.android.launcher.AssetDexClassLoader}
 * resolves the resource as an <i>asset</i> URL in it's {@link ClassLoader#findResource(String)} implementation.</p>
 * <p>
 * Currently we attach our <i>asset</i> {@link java.net.URLStreamHandlerFactory}
 * to allow {@link java.net.URL} to handle <i>asset</i> URLs via our <i>asset</i> {@link java.net.URLStreamHandler} implementation.
 * </p>
 */
public class AssetURLConnection extends PiggybackURLConnection<AssetURLContext> {

    public AssetURLConnection(final URL url, final AssetURLContext implHelper) {
        super(url, implHelper);
    }

    @Override
    public String getEntryName() throws IOException {
        if(!connected) {
            throw new IOException("not connected");
        }

        final String urlPath ;
        if(subConn instanceof JarURLConnection) {
            urlPath = ((JarURLConnection)subConn).getEntryName();
        } else {
            urlPath = subConn.getURL().getPath();
        }

        if(urlPath.startsWith(AssetURLContext.assets_folder)) {
            return urlPath.substring(AssetURLContext.assets_folder.length());
        } else {
            return urlPath;
        }
    }

}
