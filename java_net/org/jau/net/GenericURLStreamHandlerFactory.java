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

import java.net.URL;
import java.net.URLStreamHandler;
import java.net.URLStreamHandlerFactory;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.HashMap;
import java.util.Map;

public class GenericURLStreamHandlerFactory implements URLStreamHandlerFactory {
    private static GenericURLStreamHandlerFactory factory = null;

    private final Map<String, URLStreamHandler> protocolHandlers;

    private GenericURLStreamHandlerFactory() {
        protocolHandlers = new HashMap<String, URLStreamHandler>();
    }

    /**
     * Sets the <code>handler</code> for <code>protocol</code>.
     *
     * @return the previous set <code>handler</code>, or null if none was set.
     */
    public synchronized final URLStreamHandler setHandler(final String protocol, final URLStreamHandler handler) {
        return protocolHandlers.put(protocol, handler);
    }

    /**
     * Returns the <code>protocol</code> handler previously set via {@link #setHandler(String, URLStreamHandler)},
     * or null if none was set.
     */
    public synchronized final URLStreamHandler getHandler(final String protocol) {
        return protocolHandlers.get(protocol);
    }

    @Override
    public synchronized final URLStreamHandler createURLStreamHandler(final String protocol) {
        return getHandler(protocol);
    }

    /**
     * Returns the singleton instance of the registered GenericURLStreamHandlerFactory
     * or null if registration was not successful.
     * <p>
     * Registration is only performed once.
     * </p>
     */
    public synchronized static GenericURLStreamHandlerFactory register() {
        if(null == factory) {
            factory = AccessController.doPrivileged(new PrivilegedAction<GenericURLStreamHandlerFactory>() {
                @Override
                public GenericURLStreamHandlerFactory run() {
                    boolean ok = false;
                    final GenericURLStreamHandlerFactory f = new GenericURLStreamHandlerFactory();
                    try {
                        URL.setURLStreamHandlerFactory(f);
                        ok = true;
                    } catch (final Throwable e) {
                        System.err.println("GenericURLStreamHandlerFactory: Setting URLStreamHandlerFactory failed: "+e.getMessage());
                    }
                    return ok ? f : null;
                } } );
        }
        return factory;
    }
}
