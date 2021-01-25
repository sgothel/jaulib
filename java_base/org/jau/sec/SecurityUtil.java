/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2012 Gothel Software e.K.
 * Copyright (c) 2012 JogAmp Community.
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
package org.jau.sec;

import java.security.AccessController;
import java.security.AllPermission;
import java.security.CodeSource;
import java.security.Permission;
import java.security.PrivilegedAction;
import java.security.ProtectionDomain;
import java.security.cert.Certificate;

public class SecurityUtil {
    private static final SecurityManager securityManager;
    private static final Permission allPermissions;
    private static final boolean DEBUG = false;

    static {
        allPermissions = new AllPermission();
        securityManager = System.getSecurityManager();

        if( DEBUG ) {
            final boolean hasAllPermissions;
            {
                final ProtectionDomain insecPD = AccessController.doPrivileged(new PrivilegedAction<ProtectionDomain>() {
                                                @Override
                                                public ProtectionDomain run() {
                                                    return SecurityUtil.class.getProtectionDomain();
                                                } } );
                boolean _hasAllPermissions;
                try {
                    insecPD.implies(allPermissions);
                    _hasAllPermissions = true;
                } catch( final SecurityException ace ) {
                    _hasAllPermissions = false;
                }
                hasAllPermissions = _hasAllPermissions;
            }

            System.err.println("SecurityUtil: Has SecurityManager: "+ ( null != securityManager ) ) ;
            System.err.println("SecurityUtil: Has AllPermissions: "+hasAllPermissions);
            final Certificate[] certs = AccessController.doPrivileged(new PrivilegedAction<Certificate[]>() {
                                                @Override
                                                public Certificate[] run() {
                                                    return getCerts(SecurityUtil.class);
                                                } } );
            System.err.println("SecurityUtil: Cert count: "+ ( null != certs ? certs.length : 0 ));
            if( null != certs ) {
                for(int i=0; i<certs.length; i++) {
                    System.err.println("\t cert["+i+"]: "+certs[i].toString());
                }
            }
        }
    }

    /**
     * Returns <code>true</code> if no {@link SecurityManager} has been installed
     * or the installed {@link SecurityManager}'s <code>checkPermission(new AllPermission())</code>
     * passes. Otherwise method returns <code>false</code>.
     */
    public static final boolean hasAllPermissions() {
        return hasPermission(allPermissions);
    }

    /**
     * Returns <code>true</code> if no {@link SecurityManager} has been installed
     * or the installed {@link SecurityManager}'s <code>checkPermission(perm)</code>
     * passes. Otherwise method returns <code>false</code>.
     */
    public static final boolean hasPermission(final Permission perm) {
        try {
            checkPermission(perm);
            return true;
        } catch( final SecurityException ace ) {
            return false;
        }
    }

    /**
     * Throws an {@link SecurityException} if an installed {@link SecurityManager}
     * does not permit the requested {@link AllPermission}.
     */
    public static final void checkAllPermissions() throws SecurityException {
        checkPermission(allPermissions);
    }

    /**
     * Throws an {@link SecurityException} if an installed {@link SecurityManager}
     * does not permit the requested {@link Permission}.
     */
    public static final void checkPermission(final Permission perm) throws SecurityException {
        if( null != securityManager ) {
            securityManager.checkPermission(perm);
        }
    }

    /**
     * Returns <code>true</code> if no {@link SecurityManager} has been installed
     * or the installed {@link SecurityManager}'s <code>checkLink(libName)</code>
     * passes. Otherwise method returns <code>false</code>.
     */
    public static final boolean hasLinkPermission(final String libName) {
        try {
            checkLinkPermission(libName);
            return true;
        } catch( final SecurityException ace ) {
            return false;
        }
    }

    /**
     * Throws an {@link SecurityException} if an installed {@link SecurityManager}
     * does not permit to dynamically link the given libName.
     */
    public static final void checkLinkPermission(final String libName) throws SecurityException {
        if( null != securityManager ) {
            securityManager.checkLink(libName);
        }
    }

    /**
     * Throws an {@link SecurityException} if an installed {@link SecurityManager}
     * does not permit to dynamically link to all libraries.
     */
    public static final void checkAllLinkPermission() throws SecurityException {
        if( null != securityManager ) {
            securityManager.checkPermission(allLinkPermission);
        }
    }
    private static final RuntimePermission allLinkPermission = new RuntimePermission("loadLibrary.*");

    /**
     * @param clz
     * @return
     * @throws SecurityException if the caller has no permission to access the ProtectedDomain of the given class.
     */
    public static final Certificate[] getCerts(final Class<?> clz) throws SecurityException {
        final ProtectionDomain pd = clz.getProtectionDomain();
        final CodeSource cs = (null != pd) ? pd.getCodeSource() : null;
        final Certificate[] certs = (null != cs) ? cs.getCertificates() : null;
        return (null != certs && certs.length>0) ? certs : null;
    }

    public static final boolean equals(final Certificate[] a, final Certificate[] b) {
        if(a == b) {
            return true;
        }
        if(a==null || b==null) {
            return false;
        }
        if(a.length != b.length) {
            return false;
        }

        int i = 0;
        while( i < a.length && a[i].equals(b[i]) ) {
            i++;
        }
        return i == a.length;
    }
}
