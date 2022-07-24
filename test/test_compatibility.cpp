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
 * [Operating Systems](https://sourceforge.net/p/predef/wiki/OperatingSystems/)
 * predefined macros.
 * - BSD  included from <sys/param.h>
 */
#include <sys/param.h>

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
    // don't include anything yes as we like to grab the vanilla values first
    extern int printf(const char * format, ...);
}
int my_strcmp(const char *, const char *);

#define MACRO_STRINGIFY(item) "" #item
#define COND_CODE(macro, code)                           \
    do {                                                 \
        if (my_strcmp("" #macro, MACRO_STRINGIFY(macro))) { \
            (code);                                      \
        }                                                \
    } while (0)

#define PRINT_COND(macro)                                \
    do {                                                 \
        if (my_strcmp("" #macro, MACRO_STRINGIFY(macro))) { \
            printf("- %s\t%s\n", #macro, MACRO_STRINGIFY(macro)); \
        } else {                                         \
            printf("- %s\t-\n", #macro);              \
        }                                                \
    } while (0)

void print_unix_std() {
    printf("Operating System\n");
    PRINT_COND(BSD);
    PRINT_COND(__FreeBSD__);
    PRINT_COND(__NetBSD__);
    PRINT_COND(__OpenBSD__);
    PRINT_COND(__bsdi__);
    PRINT_COND(__DragonFly__);
    PRINT_COND(_SYSTYPE_BSD);

    PRINT_COND(__CYGWIN__);

    PRINT_COND(__GNU__);
    PRINT_COND(__gnu_hurd__);

    PRINT_COND(__gnu_linux__);
    PRINT_COND(__linux__);
    PRINT_COND(__APPLE__);

    PRINT_COND(__QNX__);
    PRINT_COND(__QNXNTO__);

    PRINT_COND(sun);
    PRINT_COND(__sun);
    printf("\n");

    printf("Unix Standards Inputs\n");
    PRINT_COND(_POSIX_C_SOURCE);
    PRINT_COND(_FILE_OFFSET_BITS);
    PRINT_COND(_LARGEFILE64_SOURCE);
    PRINT_COND(_TIME_BITS);
    PRINT_COND(__TIMESIZE);
    printf("\n");

    printf("Unix Standards Outputs\n");
    PRINT_COND(_POSIX_VERSION);
    PRINT_COND(_POSIX2_C_VERSION);
    PRINT_COND(_XOPEN_VERSION);
    PRINT_COND(__LSB_VERSION__);
    printf("\n");
}

void print_libc() {
    printf("GLIBC  C Library Outputs\n");
    PRINT_COND(__GNU_LIBRARY__);
    PRINT_COND(__GLIBC__);
    PRINT_COND(__GLIBC_MINOR__);
    printf("\n");

    printf("Bionic C Library Outputs\n");
    PRINT_COND(__BIONIC__);
    printf("\n");

    printf("uClibc C Library Outputs\n");
    PRINT_COND(__UCLIBC__);
    printf("\n");

    printf("GNU C++ Library Outputs\n");
    PRINT_COND(__GLIBCPP__);
    PRINT_COND(__GLIBCXX__);
    printf("\n");

    printf("C++ Library Outputs\n");
    PRINT_COND(_LIBCPP_VERSION);
    PRINT_COND(_LIBCPP_ABI_VERSION);
    printf("\n");

    printf("\n");

}


#include <jau/test/catch2_ext.hpp>

#include <jau/basic_types.hpp>

#include <string.h>

int my_strcmp(const char *s1, const char *s2) {
    return ::strcmp(s1, s2);
}

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
