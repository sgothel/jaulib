package jau.pkg;

import java.security.AccessController;
import java.security.PrivilegedAction;

import org.jau.net.Uri;
import org.jau.pkg.JNIJarLibrary;
import org.jau.pkg.JarUtil;
import org.jau.pkg.cache.TempJarCache;
import org.jau.sys.JNILibrary;
import org.jau.sys.MachineDataInfo;
import org.jau.sys.PlatformProps;
import org.jau.sys.PlatformTypes.OSType;

import org.jau.sys.PropertyAccess;
import jau.sys.MachineDataInfoRuntime;

/**
 * Initialized by {@link org.jau.sys.PlatformProps}
 */
public class PlatformRuntime {
    private static final String useTempJarCachePropName = "jau.pkg.UseTempJarCache";
    private static final String[] libBaseNames = { "jaulib_jni_jni", "jaulib_pkg_jni"} ;

    //
    // static initialization order:
    //

    /**
     * System property: 'jogamp.gluegen.UseTempJarCache',
     * defaults to true if {@link #OS_TYPE} is not {@link OSType#ANDROID}.
     */
    public static final boolean USE_TEMP_JAR_CACHE;

    //
    // post loading native lib:
    //

    public static final boolean isRunningFromJarURL;

    /** Runtime determined {@link MachineDataInfo}. */
    public static final MachineDataInfo MACH_DESC_RT;

    static {
        final boolean[] _isRunningFromJarURL = new boolean[] { false };
        final boolean[] _USE_TEMP_JAR_CACHE = new boolean[] { false };

        AccessController.doPrivileged(new PrivilegedAction<Object>() {
            @Override
            public Object run() {

                final ClassLoader cl = PlatformRuntime.class.getClassLoader();

                final Uri platformClassJarURI;
                {
                    Uri _platformClassJarURI = null;
                    try {
                        _platformClassJarURI = JarUtil.getJarUri(PlatformRuntime.class.getName(), cl);
                    } catch (final Exception e) { }
                    platformClassJarURI = _platformClassJarURI;
                }
                _isRunningFromJarURL[0] = null != platformClassJarURI;

                _USE_TEMP_JAR_CACHE[0] = ( PlatformProps.OS != OSType.ANDROID ) && ( PlatformProps.OS != OSType.IOS ) &&
                        ( null != platformClassJarURI ) &&
                        PropertyAccess.getBooleanProperty(useTempJarCachePropName, true, true);

                // load GluegenRT native library
                if(_USE_TEMP_JAR_CACHE[0] && TempJarCache.initSingleton() && TempJarCache.isInitialized(true) ) {
                    try {
                        JNIJarLibrary.addNativeJarLibs(new Class<?>[] { org.jau.sys.Debug.class }, null);
                    } catch (final Exception e0) {
                        // IllegalArgumentException, IOException
                        System.err.println("Caught "+e0.getClass().getSimpleName()+": "+e0.getMessage()+", while JNILibLoaderBase.addNativeJarLibs(..)");
                    }
                }
                for(final String libBaseName : libBaseNames) {
                    JNILibrary.loadLibrary(libBaseName, false, cl);
                }
                return null;
            } } );
        isRunningFromJarURL = _isRunningFromJarURL[0];
        USE_TEMP_JAR_CACHE = _USE_TEMP_JAR_CACHE[0];

        //
        // Validate and setup MachineDataInfo.StaticConfig
        //
        MachineDataInfoRuntime.initialize();
        MACH_DESC_RT = MachineDataInfoRuntime.getRuntime();
    }

    /**
     * kick off static initialization of <i>platform property information</i> and <i>native gluegen_rt lib loading</i>
     */
    public static void initSingleton() { }

    /**
     * Returns the MachineDataInfo of the running machine.
     */
    public static MachineDataInfo getMachineDataInfo() {
        return MACH_DESC_RT;
    }
}
