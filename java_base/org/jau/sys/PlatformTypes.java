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

/**
 * Exposing types describing the underlying platform.
 */
public final class PlatformTypes {
    public enum OSType {
        LINUX, FREEBSD, ANDROID, MACOS, SUNOS, HPUX, WINDOWS, OPENKODE, IOS, UNDEFINED;

        public static final OSType query(final String osNameLower) {
            if ( osNameLower.startsWith("linux") ) {
                return OSType.LINUX;
            }
            else if ( osNameLower.startsWith("freebsd") ) {
                return OSType.FREEBSD;
            }
            else if ( osNameLower.startsWith("android") ) {
                return OSType.ANDROID;
            }
            else if ( osNameLower.startsWith("mac os x") ||
                    osNameLower.startsWith("darwin") ) {
                return OSType.MACOS;
            }
            else if ( osNameLower.startsWith("sunos") ) {
                return OSType.SUNOS;
            }
            else if ( osNameLower.startsWith("hp-ux") ) {
                return OSType.HPUX;
            }
            else if ( osNameLower.startsWith("windows") ) {
                return OSType.WINDOWS;
            }
            else if ( osNameLower.startsWith("kd") ) {
                return OSType.OPENKODE;
            }
            else if ( osNameLower.startsWith("ios") ) {
                return OSType.IOS;
            } else {
                return OSType.UNDEFINED;
            }
        }
    }

    public enum CPUFamily {
        /** AMD/Intel */
        X86,
        /** ARM 32bit */
        ARM32,
        /** ARM 64bit */
        ARM64,
        /** Power PC */
        PPC,
        /** SPARC */
        SPARC,
        /** Mips */
        MIPS,
        /** PA RISC */
        PA_RISC,
        /** Itanium */
        IA64,
        /** Hitachi SuperH */
        SuperH;
    }
    public enum CPUType {
        /** ARM 32bit default, usually little endian */
        ARM(       CPUFamily.ARM32,     true),
        /** ARM7EJ, ARM9E, ARM10E, XScale, usually little endian */
        ARMv5(     CPUFamily.ARM32,     true),
        /** ARM11, usually little endian */
        ARMv6(     CPUFamily.ARM32,     true),
        /** ARM Cortex, usually little endian */
        ARMv7(     CPUFamily.ARM32,     true),
        // 4

        /** X86 32bit, little endian */
        X86_32(    CPUFamily.X86,     true),
        /** PPC 32bit default, usually big endian */
        PPC(       CPUFamily.PPC,     true),
        /** MIPS 32bit, big endian (mips) or little endian (mipsel) */
        MIPS_32(   CPUFamily.MIPS,    true),
        /** Hitachi SuperH 32bit default, ??? endian */
        SuperH(    CPUFamily.SuperH,  true),
        /** SPARC 32bit, big endian */
        SPARC_32(  CPUFamily.SPARC,   true),
        // 9

        /** ARM64 default (64bit), usually little endian */
        ARM64(     CPUFamily.ARM64,     false),
        /** ARM AArch64 (64bit), usually little endian */
        ARMv8_A(   CPUFamily.ARM64,     false),
        /** X86 64bit, little endian */
        X86_64(    CPUFamily.X86,     false),
        /** PPC 64bit default, usually big endian */
        PPC64(     CPUFamily.PPC,     false),
        /** MIPS 64bit, big endian (mips64) or little endian (mipsel64) ? */
        MIPS_64(   CPUFamily.MIPS,    false),
        /** Itanium 64bit default, little endian */
        IA64(      CPUFamily.IA64,    false),
        /** SPARC 64bit, big endian */
        SPARCV9_64(CPUFamily.SPARC,   false),
        /** PA_RISC2_0 64bit, ??? endian */
        PA_RISC2_0(CPUFamily.PA_RISC, false);
        // 17

        public final CPUFamily family;
        public final boolean is32Bit;

        CPUType(final CPUFamily type, final boolean is32Bit){
            this.family = type;
            this.is32Bit = is32Bit;
        }

        /**
         * Returns {@code true} if the given {@link CPUType} is compatible
         * w/ this one, i.e. at least {@link #family} and {@link #is32Bit} is equal.
         */
        public final boolean isCompatible(final CPUType other) {
            if( null == other ) {
                return false;
            } else if( other == this ) {
                return true;
            } else {
                return this.family == other.family &&
                       this.is32Bit == other.is32Bit;
            }
        }

        public static final CPUType query(final String cpuNameLower) {
            if( null == cpuNameLower ) {
                throw new IllegalArgumentException("Null cpu name arg");
            }
            if(        cpuNameLower.equals("x86")  ||
                       cpuNameLower.equals("i386") ||
                       cpuNameLower.equals("i486") ||
                       cpuNameLower.equals("i586") ||
                       cpuNameLower.equals("i686") ) {
                return X86_32;
            } else if( cpuNameLower.equals("x86_64") ||
                       cpuNameLower.equals("amd64")  ) {
                return X86_64;
            } else if( cpuNameLower.equals("ia64") ) {
                return IA64;
            } else if( cpuNameLower.equals("aarch64") ) {
                return ARM64;
            } else if( cpuNameLower.startsWith("arm") ) {
                if(        cpuNameLower.equals("armv8-a")   ||
                           cpuNameLower.equals("arm-v8-a") ||
                           cpuNameLower.equals("arm-8-a") ||
                           cpuNameLower.equals("arm64-v8a") ) {
                    return ARMv8_A; // 64-bit, aarch64
                } else if( cpuNameLower.startsWith("arm64") ) {
                    return ARM64;   // 64-bit, aarch64
                } else if( cpuNameLower.startsWith("armv7") ||
                           cpuNameLower.startsWith("arm-v7") ||
                           cpuNameLower.startsWith("arm-7") ||
                           cpuNameLower.startsWith("armeabi-v7") ) {
                    return ARMv7;   // 32-bit, aarch32
                } else if( cpuNameLower.startsWith("armv5") ||
                           cpuNameLower.startsWith("arm-v5") ||
                           cpuNameLower.startsWith("arm-5") ) {
                    return ARMv5;   // 32-bit, aarch32
                } else if( cpuNameLower.startsWith("armv6") ||
                           cpuNameLower.startsWith("arm-v6") ||
                           cpuNameLower.startsWith("arm-6") ) {
                    return ARMv6;   // 32-bit, aarch32
                } else {
                    return ARM;     // 32-bit, aarch32
                }
            } else if( cpuNameLower.startsWith("cortex-a") && cpuNameLower.length() > 8 ) {
                final String versstr = cpuNameLower.substring(8);
                try {
                    final int vers = Integer.valueOf(versstr);
                    if( vers <= 32 ) {
                        return ARMv7;   // 32-bit, aarch32
                    } else {
                        return ARMv8_A; // 64-bit, aarch64
                    }
                } catch( final NumberFormatException nfe) {
                    throw new RuntimeException("'cortex-a' post-fix not an integer: '"+cpuNameLower+"', post-fix '"+versstr+"'");
                }
            } else if( cpuNameLower.startsWith("cortex-r") && cpuNameLower.length() > 8 ) {
                final String versstr = cpuNameLower.substring(8);
                try {
                    final int vers = Integer.valueOf(versstr);
                    if( vers < 80 ) {
                        return ARMv7;   // 32-bit, aarch32
                    } else { // >= 82
                        return ARMv8_A; // 64-bit, aarch64
                    }
                } catch( final NumberFormatException nfe) {
                    throw new RuntimeException("'cortex-r' post-fix not an integer: '"+cpuNameLower+"', post-fix '"+versstr+"'");
                }
            } else if( cpuNameLower.equals("sparcv9") ) {
                return SPARCV9_64;
            } else if( cpuNameLower.equals("sparc") ) {
                return SPARC_32;
            } else if( cpuNameLower.equals("pa_risc2.0") ) {
                return PA_RISC2_0;
            } else if( cpuNameLower.startsWith("ppc64") ) {
                return PPC64;
            } else if( cpuNameLower.startsWith("ppc") ) {
                return PPC;
            } else if( cpuNameLower.startsWith("mips64") ) {
                return MIPS_64;
            } else if( cpuNameLower.startsWith("mips") ) {
                return MIPS_32;
            } else if( cpuNameLower.startsWith("superh") ) {
                return SuperH;
            } else {
                throw new RuntimeException("Please port CPUType detection to your platform (CPU name string '" + cpuNameLower + "')");
            }
        }
    }
    public enum ABIType {
        GENERIC_ABI       ( 0x00 ),
        /** ARM GNU-EABI ARMEL -mfloat-abi=softfp */
        EABI_GNU_ARMEL    ( 0x01 ),
        /** ARM GNU-EABI ARMHF -mfloat-abi=hard */
        EABI_GNU_ARMHF    ( 0x02 ),
        /** ARM EABI AARCH64 (64bit) */
        EABI_AARCH64      ( 0x03 );

        public final int id;

        ABIType(final int id){
            this.id = id;
        }

        /**
         * Returns {@code true} if the given {@link ABIType} is compatible
         * w/ this one, i.e. they are equal.
         */
        public final boolean isCompatible(final ABIType other) {
            if( null == other ) {
                return false;
            } else {
                return other == this;
            }
        }

        public static final ABIType query(final CPUType cpuType, final String cpuABILower) {
            if( null == cpuType ) {
                throw new IllegalArgumentException("Null cpuType");
            } else if( null == cpuABILower ) {
                throw new IllegalArgumentException("Null cpuABILower");
            } else if( CPUFamily.ARM64 == cpuType.family ) {
                return EABI_AARCH64;
            } else if( CPUFamily.ARM32 == cpuType.family ) {
                // FIXME: We only support EABI_GNU_ARMHF on ARM 32bit for now!
                return EABI_GNU_ARMHF;
            } else {
                return GENERIC_ABI;
            }
        }
    }
}
