/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
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

package org.jau.sys.dl;

import java.util.List;

import org.jau.util.parallel.RunnableExecutor;

public interface DynamicLibraryBundleInfo {
    public static final boolean DEBUG = DynamicLibraryBundle.DEBUG;

    /**
     * Returns {@code true} if tool libraries shall be searched in the system path <i>(default)</i>, otherwise {@code false}.
     * @since 0.3.0
     */
    public boolean searchToolLibInSystemPath();

    /**
     * Returns {@code true} if system path shall be searched <i>first</i> <i>(default)</i>, rather than searching it last.
     * <p>
     * If {@link #searchToolLibInSystemPath()} is {@code false} the return value is ignored.
     * </p>
     * @since 0.3.0
     */
    public boolean searchToolLibSystemPathFirst();

    /**
     * If a {@link SecurityManager} is installed, user needs link permissions
     * for the named libraries.
     *
     * @return a list of Tool library names or alternative library name lists.<br>
     * <ul>
     * <li>GL/GLU example Unix:   [ [ "libGL.so.1", "libGL.so", "GL" ], [ "libGLU.so", "GLU" ] ] </li>
     * <li>GL/GLU example Windows: [ "OpenGL32", "GLU32" ] </li>
     * <li>Cg/CgGL example: [ [ "libCg.so", "Cg" ], [ "libCgGL.so", "CgGL" ] ] </li>
     * </pre>
     */
    public List<List<String>> getToolLibNames();

    /**
     * If a {@link SecurityManager} is installed, user needs link permissions
     * for the named libraries.
     *
     * @return a list of Glue library names.<br>
     * <ul>
     * <li>GL:   [ "nativewindow_x11", "jogl_gl2es12", "jogl_desktop" ] </li>
     * <li>NEWT: [ "nativewindow_x11", "newt" ] </li>
     * <li>Cg:   [ "nativewindow_x11", "jogl_cg" ] </li>
     * </ul><br>
     * Only the last entry is crucial, ie all other are optional preload dependencies and may generate errors,
     * which are ignored.
     */
    public List<String> getGlueLibNames();

    /**
     * May return the native libraries <pre>GetProcAddressFunc</pre> names, the first found function is being used.<br>
     * This could be eg: <pre> glXGetProcAddressARB, glXGetProcAddressARB </pre>.<br>
     * If your Tool does not has this facility, just return null.
     * @see #toolGetProcAddress(long, String)
     */
    public List<String> getToolGetProcAddressFuncNameList() ;

    /**
     * May implement the lookup function using the Tools facility.<br>
     * The actual function pointer is provided to allow proper bootstrapping of the ProcAddressTable,
     * using one of the provided function names by {@link #getToolGetProcAddressFuncNameList()}.<br>
     */
    public long toolGetProcAddress(long toolGetProcAddressHandle, String funcName);

    /**
     * @param funcName
     * @return true if {@link #toolGetProcAddress(long, String)} shall be tried before
     *         the system loader for the given function lookup. Otherwise false.
     *         Default is <b>true</b>.
     */
    public boolean useToolGetProcAdressFirst(String funcName);

    /** @return true if the native library symbols shall be made available for symbol resolution of subsequently loaded libraries. */
    public boolean shallLinkGlobal();

    /**
     * If method returns <code>true</code> <i>and</i> if a {@link SecurityManager} is installed, user needs link permissions
     * for <b>all</b> libraries, i.e. for <code>new RuntimePermission("loadLibrary.*");</code>!
     *
     * @return true if the dynamic symbol lookup shall happen system wide, over all loaded libraries.
     * Otherwise only the loaded native libraries are used for lookup, which shall be the default.
     */
    public boolean shallLookupGlobal();

    /**
     * Returns a suitable {@link RunnableExecutor} implementation, which is being used
     * to load the <code>tool</code> and <code>glue</code> native libraries.
     * <p>
     * This allows the generic {@link DynamicLibraryBundle} implementation to
     * load the native libraries on a designated thread.
     * </p>
     * <p>
     * An implementation may return {@link DynamicLibraryBundle#getDefaultRunnableExecutor()}.
     * </p>
     */
    public RunnableExecutor getLibLoaderExecutor();
}


