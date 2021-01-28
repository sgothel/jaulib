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

package org.jau.util;

import java.util.Iterator;
import java.util.Set;
import java.util.jar.Attributes;
import java.util.jar.Manifest;

import org.jau.sys.AndroidUtil;
import org.jau.sys.PlatformProps;

public class JauVersion {

    /** See {@link #getImplementationBuild()} */
    public static final Attributes.Name IMPLEMENTATION_BUILD = new Attributes.Name("Implementation-Build");
    /** See {@link #getImplementationBranch()} */
    public static final Attributes.Name IMPLEMENTATION_BRANCH = new Attributes.Name("Implementation-Branch");
    /** See {@link #getImplementationCommit()} */
    public static final Attributes.Name IMPLEMENTATION_COMMIT = new Attributes.Name("Implementation-Commit");
    /** See {@link #getImplementationSHASources()} */
    public static final Attributes.Name IMPLEMENTATION_SHA_SOURCES = new Attributes.Name("Implementation-SHA-Sources");
    /** See {@link #getImplementationSHAClasses()} */
    public static final Attributes.Name IMPLEMENTATION_SHA_CLASSES = new Attributes.Name("Implementation-SHA-Classes");
    /** See {@link #getImplementationSHAClassesThis()} */
    public static final Attributes.Name IMPLEMENTATION_SHA_CLASSES_THIS = new Attributes.Name("Implementation-SHA-Classes-this");
    /** See {@link #getImplementationSHANatives()} */
    public static final Attributes.Name IMPLEMENTATION_SHA_NATIVES = new Attributes.Name("Implementation-SHA-Natives");
    /** See {@link #getImplementationSHANativesThis()} */
    public static final Attributes.Name IMPLEMENTATION_SHA_NATIVES_THIS = new Attributes.Name("Implementation-SHA-Natives-this");

    /** For FAT jaulib jar file */
    private static final String packageNameFAT = "org.jau";

    private final String packageName;
    private final Manifest mf;
    private final int hash;
    private final Attributes mainAttributes;
    private final Set<?>/*<Attributes.Name>*/ mainAttributeNames;

    private final String androidPackageVersionName;

    protected JauVersion(final String packageName, final Manifest mf) {
        if( null != mf ) {
            // use provided valid data
            this.mf = mf;
            this.packageName = packageName;
        } else {
            // try FAT jar file
            final Manifest fatMF = VersionUtil.getManifest(JauVersion.class.getClassLoader(), packageNameFAT);
            if( null != fatMF ) {
                // use FAT jar file
                this.mf = fatMF;
                this.packageName = packageNameFAT;
            } else {
                // use faulty data, unresolvable ..
                this.mf = new Manifest();
                this.packageName = packageName;
            }
        }
        this.hash = this.mf.hashCode();
        mainAttributes = this.mf.getMainAttributes();
        mainAttributeNames = mainAttributes.keySet();
        androidPackageVersionName = AndroidUtil.getPackageInfoVersionName(this.packageName); // null if !Android
    }

    @Override
    public final int hashCode() {
        return hash;
    }

    @Override
    public final boolean equals(final Object o) {
        if (o instanceof JauVersion) {
            return mf.equals(((JauVersion) o).getManifest());
        }
        return false;
    }

    public final Manifest getManifest() {
        return mf;
    }

    public final String getPackageName() {
        return packageName;
    }

    public final String getAttribute(final Attributes.Name attributeName) {
        return (null != attributeName) ? (String) mainAttributes.get(attributeName) : null;
    }

    public final String getAttribute(final String attributeName) {
        return getAttribute(getAttributeName(attributeName));
    }

    public final Attributes.Name getAttributeName(final String attributeName) {
        for (final Iterator<?> iter = mainAttributeNames.iterator(); iter.hasNext();) {
            final Attributes.Name an = (Attributes.Name) iter.next();
            if (an.toString().equals(attributeName)) {
                return an;
            }
        }
        return null;
    }

    /**
     * @return set of type {@link Attributes.Name}, disguised as anonymous
     */
    public final Set<?>/*<Attributes.Name>*/ getAttributeNames() {
        return mainAttributeNames;
    }

    public final String getExtensionName() {
        if(null != androidPackageVersionName) {
            return packageName;
        }
        return this.getAttribute(Attributes.Name.EXTENSION_NAME);
    }

    /**
     * Returns the implementation build number, e.g. <code>2.0-b456-20130328</code>.
     */
    public final String getImplementationBuild() {
        return this.getAttribute(JauVersion.IMPLEMENTATION_BUILD);
    }

    /**
     * Returns the SCM branch name
     */
    public final String getImplementationBranch() {
        return this.getAttribute(JauVersion.IMPLEMENTATION_BRANCH);
    }

    /**
     * Returns the SCM version of the last commit, e.g. git's sha1
     */
    public final String getImplementationCommit() {
        return this.getAttribute(JauVersion.IMPLEMENTATION_COMMIT);
    }

    /**
     * Returns the SHA of all concatenated source files of the whole project
     */
    public final String getImplementationSHASources() {
        return this.getAttribute(JauVersion.IMPLEMENTATION_SHA_SOURCES);
    }

    /**
     * Returns the SHA of all concatenated class files of all build classes
     */
    public final String getImplementationSHAClasses() {
        return this.getAttribute(JauVersion.IMPLEMENTATION_SHA_CLASSES);
    }

    /**
     * Returns the SHA of all concatenated class files of the local (jar) package subset
     */
    public final String getImplementationSHAClassesThis() {
        return this.getAttribute(JauVersion.IMPLEMENTATION_SHA_CLASSES_THIS);
    }

    /**
     * Returns the SHA of all concatenated native library files of all build libs
     */
    public final String getImplementationSHANatives() {
        return this.getAttribute(JauVersion.IMPLEMENTATION_SHA_NATIVES);
    }

    /**
     * Returns the SHA of all concatenated native library files of the local (jar) package subset
     */
    public final String getImplementationSHANativesThis() {
        return this.getAttribute(JauVersion.IMPLEMENTATION_SHA_NATIVES_THIS);
    }

    public final String getImplementationTitle() {
        return this.getAttribute(Attributes.Name.IMPLEMENTATION_TITLE);
    }

    public final String getImplementationVendor() {
        return this.getAttribute(Attributes.Name.IMPLEMENTATION_VENDOR);
    }

    /**
     * Returns the {@link Attributes.Name#IMPLEMENTATION_VERSION IMPLEMENTATION_VERSION}.
     * <p>
     * E.g. <code>2.0.2-rc-20130328</code> for snapshots prior to <code>2.0.2</code> release
     * and <code>2.0.2</code> for the upcoming release.
     * </p>
     */
    public final String getImplementationVersion() {
        return this.getAttribute(Attributes.Name.IMPLEMENTATION_VERSION);
    }

    public final String getAndroidPackageVersionName() {
        return androidPackageVersionName;
    }

    public final String getSpecificationTitle() {
        return this.getAttribute(Attributes.Name.SPECIFICATION_TITLE);
    }

    public final String getSpecificationVendor() {
        return this.getAttribute(Attributes.Name.SPECIFICATION_VENDOR);
    }

    public final String getSpecificationVersion() {
        return this.getAttribute(Attributes.Name.SPECIFICATION_VERSION);
    }

    public final StringBuilder getFullManifestInfo(final StringBuilder sb) {
        return VersionUtil.getFullManifestInfo(getManifest(), sb);
    }

    public StringBuilder getManifestInfo(StringBuilder sb) {
        if(null==sb) {
            sb = new StringBuilder();
        }
        final String nl = PlatformProps.NEWLINE;
        sb.append("Package: ").append(getPackageName()).append(nl);
        sb.append("Extension Name: ").append(getExtensionName()).append(nl);
        sb.append("Specification Title: ").append(getSpecificationTitle()).append(nl);
        sb.append("Specification Vendor: ").append(getSpecificationVendor()).append(nl);
        sb.append("Specification Version: ").append(getSpecificationVersion()).append(nl);
        sb.append("Implementation Title: ").append(getImplementationTitle()).append(nl);
        sb.append("Implementation Vendor: ").append(getImplementationVendor()).append(nl);
        sb.append("Implementation Version: ").append(getImplementationVersion()).append(nl);
        sb.append("Implementation Build: ").append(getImplementationBuild()).append(nl);
        sb.append("Implementation Branch: ").append(getImplementationBranch()).append(nl);
        sb.append("Implementation Commit: ").append(getImplementationCommit()).append(nl);
        sb.append("Implementation SHA Sources: ").append(getImplementationSHASources()).append(nl);
        sb.append("Implementation SHA Classes: ").append(getImplementationSHAClasses()).append(nl);
        sb.append("Implementation SHA Classes-this: ").append(getImplementationSHAClassesThis()).append(nl);
        sb.append("Implementation SHA Natives: ").append(getImplementationSHANatives()).append(nl);
        sb.append("Implementation SHA Natives-this: ").append(getImplementationSHANativesThis()).append(nl);
        if(null != getAndroidPackageVersionName()) {
            sb.append("Android Package Version: ").append(getAndroidPackageVersionName()).append(nl);
        }
        return sb;
    }

    public StringBuilder toString(StringBuilder sb) {
        if(null==sb) {
            sb = new StringBuilder();
        }

        sb.append(VersionUtil.SEPERATOR).append(PlatformProps.NEWLINE);
        getManifestInfo(sb);
        sb.append(VersionUtil.SEPERATOR);

        return sb;
    }

    @Override
    public String toString() {
        return toString(null).toString();
    }
}
