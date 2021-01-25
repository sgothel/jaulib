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
package org.jau.sys;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.ShortBuffer;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.List;

import org.jau.lang.NioUtil;
import org.jau.lang.ReflectionUtil;
import org.jau.sys.PlatformTypes.ABIType;
import org.jau.sys.PlatformTypes.CPUType;
import org.jau.sys.PlatformTypes.OSType;
import org.jau.sys.elf.ElfHeaderPart1;
import org.jau.sys.elf.ElfHeaderPart2;
import org.jau.sys.elf.SectionArmAttributes;
import org.jau.util.VersionNumber;

/**
 * Runtime platform properties.
 */
public class PlatformProps {
    private static final String prt_name = "jau.pkg.PlatformRuntime";

    public static final boolean DEBUG = Debug.debug("Platform");

    /**
     * Returns {@code true} if the given {@link CPUType}s and {@link ABIType}s are compatible.
     */
    private static final boolean isCompatible(final CPUType cpu1, final ABIType abi1, final CPUType cpu2, final ABIType abi2) {
        return cpu1.isCompatible(cpu2) && abi1.isCompatible(abi2);
    }

    //
    // static initialization order:
    //

    public static final VersionNumber JAVA_VERSION_NUMBER;

    /**
     * True only if being compatible w/ language level 9, e.g. JRE 9.
     * <p>
     * Since JRE 9, the version string has dropped the major release number,
     * see JEP 223: http://openjdk.java.net/jeps/223
     * </p>
     */
    public static final boolean JAVA_9;

    public static final String NEWLINE;

    public static final String JAVA_VENDOR;
    public static final String JAVA_VM_NAME;
    public static final String JAVA_RUNTIME_NAME;

    public static final VersionNumber os_version;
    public static final OSType OS;

    public static final boolean LITTLE_ENDIAN;
    public static final CPUType CPU;
    public static final ABIType ABI;

    /** Lower case system property '{@code os.name}'. */
    public static final String os_name;

    /** Lower case system property '{@code os.arch}' */
    public static final String os_arch;

    /** Static (not runtime) determined {@link MachineDataInfo}. */
    public static final MachineDataInfo MACH_DESC_STAT;

    /** Runtime determined {@link MachineDataInfo}, null if not available (i.e. no JNI libs loaded). */
    public static final MachineDataInfo MACH_DESC_RT;

    /**
     * Unique platform denominator composed as '{@link #os_name}' + '-' + '{@link #os_arch}'.
     */
    public static final String os_and_arch;

    static {
        final VersionNumber _version9 = new VersionNumber(9, 0, 0);
        JAVA_VERSION_NUMBER = new VersionNumber(System.getProperty("java.version"));
        JAVA_9 = JAVA_VERSION_NUMBER.compareTo(_version9) >= 0;

        NEWLINE = System.getProperty("line.separator");

        JAVA_VENDOR = System.getProperty("java.vendor");
        JAVA_VM_NAME = System.getProperty("java.vm.name");

        final String os_name_prop;
        final String os_arch_prop;
        {
            final String[] props =
                    AccessController.doPrivileged(new PrivilegedAction<String[]>() {
                        @Override
                        public String[] run() {
                            final String[] props = new String[4];
                            int i=0;
                            props[i++] = System.getProperty("java.runtime.name"); // 0
                            props[i++] = System.getProperty("os.name").toLowerCase(); // 1
                            props[i++] = System.getProperty("os.arch").toLowerCase(); // 2
                            props[i++] = System.getProperty("os.version"); // 3
                            return props;
                        }
                    });
            int i=0;
            JAVA_RUNTIME_NAME = props[i++];
            os_name_prop = props[i++];
            os_arch_prop = props[i++];
            os_version = new VersionNumber(props[i++]);
        }
        OS = OSType.query(os_name_prop);

        // Hard values, i.e. w/ probing binaries
        final String elfCpuName;
        final CPUType elfCpuType;
        final ABIType elfABIType;
        final int elfLittleEndian;
        final boolean elfValid;
        {
            final String[] _elfCpuName = { null };
            final CPUType[] _elfCpuType = { null };
            final ABIType[] _elfAbiType = { null };
            final int[] _elfLittleEndian = { 0 }; // 1 - little, 2 - big
            final boolean[] _elfValid = { false };
            AccessController.doPrivileged(new PrivilegedAction<Object>() {
                @Override
                public Object run() {
                    RandomAccessFile in = null;
                    try {
                        final File file = queryElfFile(OS);
                        if(DEBUG) {
                            System.err.println("ELF-1: Using "+file);
                        }
                        in = new RandomAccessFile(file, "r");
                        final ElfHeaderPart1 eh1 = readElfHeaderPart1(OS, in);
                        if(DEBUG) {
                            System.err.println("ELF-1: Got "+eh1);
                        }
                        if( null != eh1 ) {
                            final ElfHeaderPart2 eh2 = readElfHeaderPart2(eh1, in);
                            if(DEBUG) {
                                System.err.println("ELF-2: Got "+eh2);
                            }
                            if( null != eh2 ) {
                                _elfCpuName[0] = eh2.cpuName;
                                _elfCpuType[0] = eh2.cpuType;
                                _elfAbiType[0] = eh2.abiType;
                                if( eh1.isLittleEndian() ) {
                                    _elfLittleEndian[0] = 1;
                                } else if( eh1.isBigEndian() ) {
                                    _elfLittleEndian[0] = 2;
                                }
                                _elfValid[0] = true;
                            }
                        }
                    } catch (final Throwable t) {
                        if(DEBUG) {
                            t.printStackTrace();
                        }
                    } finally {
                        if(null != in) {
                            try {
                                in.close();
                            } catch (final IOException e) { }
                        }
                    }
                    return null;
                } });
            elfCpuName = _elfCpuName[0];
            elfCpuType = _elfCpuType[0];
            elfABIType = _elfAbiType[0];
            elfLittleEndian = _elfLittleEndian[0];
            elfValid = _elfValid[0];
            if( DEBUG ) {
                System.err.println("Platform.Elf: valid "+elfValid+", elfCpuName "+elfCpuName+", cpuType "+elfCpuType+", abiType "+elfABIType+", elfLittleEndian "+elfLittleEndian);
            }
        }

        // Determine endianess, favor ELF value
        if( elfValid ) {
            switch( elfLittleEndian ) {
                case 1:
                    LITTLE_ENDIAN = true;
                    break;
                case 2:
                    LITTLE_ENDIAN = false;
                    break;
                default:
                    LITTLE_ENDIAN = queryIsLittleEndianImpl();
                    break;
            }
        } else {
            LITTLE_ENDIAN = queryIsLittleEndianImpl();
        }
        if( DEBUG ) {
            System.err.println("Platform.Endian: test-little "+queryIsLittleEndianImpl()+", elf[valid "+elfValid+", val "+elfLittleEndian+"] -> LITTLE_ENDIAN "+LITTLE_ENDIAN);
        }

        // Property values for comparison
        // We might take the property values even if ELF values are available,
        // since the latter only reflect the CPU/ABI version of the binary files!
        final CPUType propCpuType = CPUType.query(os_arch_prop);
        final ABIType propABIType = ABIType.query(propCpuType, os_arch_prop);
        if( DEBUG ) {
            System.err.println("Platform.Property: ARCH "+os_arch_prop+", CpuType "+propCpuType+", ABIType "+propABIType);
        }

        final int strategy;
        if( elfValid ) {
            if( isCompatible(elfCpuType, elfABIType, propCpuType, propABIType) ) {
                // Use property ARCH, compatible w/ ELF
                CPU = propCpuType;
                ABI = propABIType;
                strategy = 210;
            } else {
                // use ELF ARCH
                CPU = elfCpuType;
                ABI = elfABIType;
                strategy = 211;
            }
        } else {
            // Last resort: properties
            CPU = propCpuType;
            ABI = propABIType;
            strategy = 220;
        }
        {
            final String _os_name2 = getOSName(OS);
            os_name = null != _os_name2 ? _os_name2 : os_name_prop;
        }
        {
            final String _os_arch2 = getArchName(CPU, ABI, LITTLE_ENDIAN);
            os_arch = null != _os_arch2 ? _os_arch2 : os_arch_prop;
        }
        os_and_arch = os_name+"-"+os_arch;

        MACH_DESC_STAT = MachineDataInfo.guessStaticMachineDataInfo(OS, CPU).md;
        {
            Class<?> prt = null;
            try {
                prt = ReflectionUtil.getClass(prt_name, true /* initializeClazz */, PlatformProps.class.getClassLoader());
            } catch (final Throwable t) {
                if( DEBUG ) {
                    System.err.println("Platform.RT: Exception: "+t.getMessage());
                    t.printStackTrace();
                }
            }
            if( null != prt ) {
                final ReflectionUtil.MethodAccessor prtGetMachDesc = new ReflectionUtil.MethodAccessor(prt, "getMachineDataInfo");
                if( null != prtGetMachDesc && prtGetMachDesc.available() ) {
                    MACH_DESC_RT = prtGetMachDesc.callStaticMethod();
                    if( DEBUG ) {
                        System.err.println("Platform.RT: Available <"+prt_name+">");
                    }
                } else {
                    MACH_DESC_RT = null;
                    if( DEBUG ) {
                        System.err.println("Platform.RT: Not available (2) <"+prt_name+">");
                    }
                }
            } else {
                MACH_DESC_RT = null;
                if( DEBUG ) {
                    System.err.println("Platform.RT: Not available (1) <"+prt_name+">");
                }
            }
        }

        if( DEBUG ) {
            System.err.println("Platform.OS: os_name "+os_name+", os_arch "+os_arch+", os_version "+os_version);
            System.err.println("Platform.Hard: CPU_ARCH "+CPU+", ABI_TYPE "+ABI+" - strategy "+strategy+"(elfValid "+elfValid+"), little "+LITTLE_ENDIAN);
            System.err.println("Platform.MD.ST: "+MACH_DESC_STAT);
            System.err.println("Platform.MD.RT: "+MACH_DESC_RT);
        }
    }
    public static void initSingleton() { }

    protected PlatformProps() {}

    /**
     * Returns the {@link ABIType} of the current platform using given {@link CPUType cpuType}
     * and {@link OSType osType} as a hint.
     * <p>
     * For Elf parsing one of the following binaries is used:
     * <ul>
     *  <li>Linux: Current executable</li>
     *  <li>Android: Found gluegen_rt library</li>
     *  <li>Other: A found java/jvm native library.</li>
     * </ul>
     * </p>
     * <p>
     * Elf ARM Tags are read using {@link ElfHeader}, .. and {@link SectionArmAttributes#abiVFPArgsAcceptsVFPVariant(byte)}.
     * </p>
     */
    private static final File queryElfFile(final OSType osType) {
        File file = null;
        try {
            if( OSType.LINUX == osType ) {
                file = new File("/proc/self/exe");
                if( !checkFileReadAccess(file) ) {
                    file = null;
                }
            }
            if( null == file ) {
                file = findSysLib("java");
            }
            if( null == file ) {
                file = findSysLib("jvm");
            }
        } catch(final Throwable t) {
            if(DEBUG) {
                t.printStackTrace();
            }
        }
        return file;
    }
    private static final ElfHeaderPart1 readElfHeaderPart1(final OSType osType, final RandomAccessFile in) {
        ElfHeaderPart1 res = null;
        try {
            res = ElfHeaderPart1.read(osType, in);
        } catch(final Throwable t) {
            if(DEBUG) {
                System.err.println("Caught: "+t.getMessage());
                t.printStackTrace();
            }
        }
        return res;
    }
    private static final ElfHeaderPart2 readElfHeaderPart2(final ElfHeaderPart1 eh1, final RandomAccessFile in) {
        ElfHeaderPart2 res = null;
        try {
            res = ElfHeaderPart2.read(eh1, in);
        } catch(final Throwable t) {
            if(DEBUG) {
                System.err.println("Caught: "+t.getMessage());
                t.printStackTrace();
            }
        }
        return res;
    }
    private static boolean checkFileReadAccess(final File file) {
        try {
            return file.isFile() && file.canRead();
        } catch (final Throwable t) { }
        return false;
    }
    private static File findSysLib(final String libName) {
        final ClassLoader cl = PlatformProps.class.getClassLoader();
        final List<String> possibleLibPaths = JNILibrary.enumerateLibraryPaths(libName,
                true /* searchSystemPath */, true /* searchSystemPathFirst */, cl);
        for(int i=0; i<possibleLibPaths.size(); i++) {
            final String libPath = possibleLibPaths.get(i);
            final File lib = new File(libPath);
            if(DEBUG) {
                System.err.println("findSysLib #"+i+": test "+lib);
            }
            if( checkFileReadAccess(lib) ) {
                return lib;
            }
            if(DEBUG) {
                System.err.println("findSysLib #"+i+": "+lib+" not readable");
            }
        }
        return null;
    }

    private static final String getArchName(final CPUType cpuType, final ABIType abiType, final boolean littleEndian) {
        switch( abiType ) {
            case EABI_GNU_ARMEL:
                return "arm"; // actually not supported!
            case EABI_GNU_ARMHF:
                return "armhf";
            case EABI_AARCH64:
                return "arm64";
            default:
                break;
        }

        switch( cpuType ) {
            case X86_32:
                return "i386";
            case PPC:
                return "ppc";
            case MIPS_32:
                return littleEndian ? "mipsel" : "mips";
            case SuperH:
                return "superh";
            case SPARC_32:
                return "sparc";

            case X86_64:
                return "amd64";
            case PPC64:
                return littleEndian ? "ppc64le" : "ppc64";
            case MIPS_64:
                return "mips64";
            case IA64:
                return "ia64";
            case SPARCV9_64:
                return "sparcv9";
            case PA_RISC2_0:
                return "risc2.0";
            default:
                return null;
        }
    }

    private static final String getOSName(final OSType osType) {
        switch( osType ) {
            case ANDROID:
              return "android";
            case MACOS:
              return "macosx";
            case IOS:
              return "ios";
            case WINDOWS:
              return "windows";
            case OPENKODE:
              return "openkode";
            case LINUX:
              return "linux";
            case FREEBSD:
              return "freebsd";
            case SUNOS:
              return "solaris";
            case HPUX:
              return "hpux";
            default:
              return "undefined";
        }
    }
    private static final boolean queryIsLittleEndianImpl() {
        final ByteBuffer tst_b = NioUtil.newNativeByteBuffer(NioUtil.SIZEOF_INT); // 32bit in native order
        final IntBuffer tst_i = tst_b.asIntBuffer();
        final ShortBuffer tst_s = tst_b.asShortBuffer();
        tst_i.put(0, 0x0A0B0C0D);
        return 0x0C0D == tst_s.get(0);
    }
}
