/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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

/**
 * [Unix standards](https://sourceforge.net/p/predef/wiki/Standards/)
 * require the existence macros in the <unistd.h> header file.
 */
#include <unistd.h>

/**
 * [GNU glibc](https://sourceforge.net/p/predef/wiki/Libraries/)
 *
 * GLIBC macros have to be included from the <features.h> header file.
 * Include <limits.h> header file instead, which included <features.h> on GLIBC (see e.g. paragraph 4/6 in ISO/IEC 9899:1999).
 */
#include <limits.h>

/**
 * [glibc 1.3.4 Feature Test Macros](https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html)
 * _FILE_OFFSET_BITS
 * - available if _POSIX_C_SOURCE >= 200112L
 * - _FILE_OFFSET_BITS == 64 implies using all 64-bit file-function and -type variants on 32-bit platforms
 * - _FILE_OFFSET_BITS == 64 has no effect on 64-bit platforms, already using the 64-bit variants
 * - _FILE_OFFSET_BITS is favored over _LARGEFILE64_SOURCE
 *
 * _TIME_BITS
 * - introduced in glibc 2.34, tackling year 2038 issue
 * - _TIME_BITS  available for Linux with kernel >= 5.1
 * - _TIME_BITS == 64 requires _FILE_OFFSET_BITS to be 64 as well
 *
 * glibc D.2.1 64-bit time symbol handling in the GNU C Library
 * __TIMESIZE == 64 uses 64-bit time_t version
 */
extern "C" {
    extern int printf(const char * format, ...);
}

void print_unix_std() {
    printf("Unix Standards Inputs\n");
    #ifdef _POSIX_C_SOURCE
        printf("- _POSIX_C_SOURCE     %ld\n", _POSIX_C_SOURCE);
    #else
        printf("- _POSIX_C_SOURCE     -\n");
    #endif
    #ifdef _FILE_OFFSET_BITS
        printf("- _FILE_OFFSET_BITS   %d\n", _FILE_OFFSET_BITS);
    #else
        printf("- _FILE_OFFSET_BITS   -\n");
    #endif
    #ifdef _LARGEFILE64_SOURCE
        printf("- _LARGEFILE64_SOURCE %d\n", _LARGEFILE64_SOURCE);
    #else
        printf("- _LARGEFILE64_SOURCE -\n");
    #endif
    #ifdef _TIME_BITS
        printf("- _TIME_BITS          %d\n", _TIME_BITS);
    #else
        printf("- _TIME_BITS          -\n");
    #endif
    #ifdef __TIMESIZE
        printf("- __TIMESIZE          %d\n", __TIMESIZE);
    #else
        printf("- __TIMESIZE          -\n");
    #endif
    printf("\n");

    printf("Unix Standards Outputs\n");
    #ifdef _POSIX_VERSION
        printf("- _POSIX_VERSION      %ld\n", _POSIX_VERSION);
    #else
        printf("- _POSIX_VERSION      -\n");
    #endif
    #ifdef _POSIX2_C_VERSION
        printf("- _POSIX2_C_VERSION   %ld\n", _POSIX2_C_VERSION);
    #else
        printf("- _POSIX2_C_VERSION   -\n");
    #endif
    #ifdef _XOPEN_VERSION
        printf("- _XOPEN_VERSION      %d\n", _XOPEN_VERSION);
    #else
        printf("- _XOPEN_VERSION      -\n");
    #endif
    #ifdef __LSB_VERSION__
        printf("- __LSB_VERSION__     %ld\n", __LSB_VERSION__);
    #else
        printf("- __LSB_VERSION__     -\n");
    #endif
    printf("\n");
}

void print_libc() {
    printf("GLIBC  C Library Outputs\n");
    #ifdef __GNU_LIBRARY__
        printf("- __GNU_LIBRARY__      %d\n", __GNU_LIBRARY__);
    #else
        printf("- __GNU_LIBRARY__      -\n",);
    #endif
    #ifdef __GLIBC__
        printf("- __GLIBC__            %d\n", __GLIBC__);
    #else
        printf("- __GLIBC__      -\n");
    #endif
    #ifdef __GLIBC_MINOR__
        printf("- __GLIBC_MINOR__      %d\n", __GLIBC_MINOR__);
    #else
        printf("- __GLIBC_MINOR__      -\n");
    #endif
    printf("\n");

    printf("Bionic C Library Outputs\n");
    #ifdef __BIONIC__
        printf("- __BIONIC__           %d\n", __BIONIC__);
    #else
        printf("- __BIONIC__           -\n");
    #endif
    printf("\n");

    printf("uClibc C Library Outputs\n");
    #ifdef __UCLIBC__
        printf("- __UCLIBC__           %d\n", __UCLIBC__);
        printf("- __UCLIBC__           %d.%d.%d\n", __UCLIBC_MAJOR__, __UCLIBC_MINOR__, __UCLIBC_SUBLEVEL__);
    #else
        printf("- __UCLIBC__           -\n");
    #endif
    printf("\n");

}


#include <jau/test/catch2_ext.hpp>

#include <jau/basic_types.hpp>

/**
 * Resembling the GNU/Linux bits/types.h,
 * documenting whether time_t is 32-bit (arm-32) or 64-bit (arm-64, x86_64, ..).
 */
static int sizeof_time_t() {
/* X32 kernel interface is 64-bit.  */
#if defined __x86_64__ && defined __ILP32__
    // 64 bit size
    #if __WORDSIZE == 32
        return sizeof( __int64_t );
    #else
        return sizeof( long int );
    #endif
#else
    // 32 bit or 64 bit
    return sizeof( long int );
#endif
}

/**
 * Resembling the GNU/Linux bits/types.h,
 * documenting whether tv_nsec of struct timespec is 32-bit (arm-32) or 64-bit (arm-64, x86_64, ..).
 */
static int sizeof_tv_nsec() {
#if __WORDSIZE == 64 \
  || (defined __SYSCALL_WORDSIZE && __SYSCALL_WORDSIZE == 64) \
  || __TIMESIZE == 32
    // 32 bit or 64 bit: __syscall_slong_t
    return sizeof( int64_t );
#else
    // 32 bit or 64 bit
    return sizeof( long int );
#endif
}

TEST_CASE( "Unix StandardsTest 01.00", "[unix][posix]" ) {
    print_unix_std();
    {
        using time_t_type = decltype(timespec::tv_sec);
        INFO_STR(" tv_sec: sizeof=" + std::to_string( sizeof( time_t_type ) ) + ", signed " + std::to_string( std::is_signed_v<time_t_type>) );
        CHECK( sizeof_time_t() == sizeof( time_t_type ) );
        CHECK( true == std::is_signed_v<time_t_type> );

        using ns_type = decltype(timespec::tv_nsec);
        INFO_STR(" tv_nsec: sizeof=" + std::to_string( sizeof( ns_type ) ) + ", signed " + std::to_string( std::is_signed_v<ns_type>) );
        CHECK( sizeof_tv_nsec() == sizeof( ns_type ) );
        CHECK( true == std::is_signed_v<ns_type> );
    }
    {
        INFO_STR(" off_t sizeof=" + std::to_string( sizeof( off_t ) ) + ", signed " + std::to_string( std::is_signed_v<off_t>) );
        INFO_STR(" off64_t sizeof=" + std::to_string( sizeof( off64_t ) ) + ", signed " + std::to_string( std::is_signed_v<off64_t>) );
        REQUIRE( 8 == sizeof(off64_t) );
    }
}

TEST_CASE( "Standard C Library 01.01", "[libc]" ) {
    print_libc();
}
