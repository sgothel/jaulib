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

package org.jau.util;

/**
 * {@link VersionNumber} specialization, holding the <code>versionString</code>
 * this instance is derived from.
 */
public class VersionNumberString extends VersionNumber {

    /**
     * A {@link #isZero() zero} version instance, w/o any component defined explicitly.
     * @see #hasMajor()
     * @see #hasMinor()
     * @see #hasSub()
     */
    public static final VersionNumberString zeroVersion = new VersionNumberString(0, 0, 0, -1, (short)0, "n/a");

    protected final String strVal;

    protected VersionNumberString(final int majorRev, final int minorRev, final int subMinorRev, final int strEnd, final short _state, final String versionString) {
        super(majorRev, minorRev, subMinorRev, strEnd, _state);
        strVal = versionString;
    }

    /**
     * See {@link VersionNumber#VersionNumber(int, int, int)}.
     */
    public VersionNumberString(final int majorRev, final int minorRev, final int subMinorRev, final String versionString) {
        this(majorRev, minorRev, subMinorRev, -1, (short)(HAS_MAJOR | HAS_MINOR | HAS_SUB), versionString);
    }

    /**
     * See {@link VersionNumber#VersionNumber(String)}.
     */
    public VersionNumberString(final String versionString) {
        super( versionString);
        strVal = versionString;
    }

    /**
     * See {@link VersionNumber#VersionNumber(String, String)}.
     */
    public VersionNumberString(final String versionString, final String delim) {
        super( versionString, delim);
        strVal = versionString;
    }

    /**
     * See {@link VersionNumber#VersionNumber(String, java.util.regex.Pattern)}.
     */
    public VersionNumberString(final String versionString, final java.util.regex.Pattern versionPattern) {
        super( versionString, versionPattern);
        strVal = versionString;
    }

    /** Returns the version string this version number is derived from. */
    public final String getVersionString() { return strVal; }

    @Override
    public String toString() {
        return super.toString() + " ("+strVal+")" ;
    }
}
