/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2020 ZAFENA AB
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

#include "org_jau_sys_Clock.h"

#include <cstdint>
#include <cinttypes>

#include <time.h>

#include <jau/environment.hpp>

#include "jau/jni/helper_jni.hpp"


static const int64_t NanoPerMilli = 1000000L;
static const int64_t MilliPerOne = 1000L;

void Java_org_jau_sys_Clock_getMonotonicTimeImpl(JNIEnv *env, jclass clazz, jlongArray jval) {
    (void)clazz;
    try {
        if( nullptr == jval ) {
            throw jau::IllegalArgumentException("val null", E_FILE_LINE);
        }
        const size_t in_size = env->GetArrayLength(jval);
        if( 2 > in_size ) {
            throw jau::IllegalArgumentException("val size "+std::to_string(in_size)+" < 2", E_FILE_LINE);
        }
        jau::jni::JNICriticalArray<int64_t, jlongArray> criticalArray(env); // RAII - release
        int64_t * val_ptr = criticalArray.get(jval, criticalArray.Mode::UPDATE_AND_RELEASE);
        if( NULL == val_ptr ) {
            throw jau::InternalError("GetPrimitiveArrayCritical(address val array) is null", E_FILE_LINE);
        }
        struct timespec t { 0, 0 };
        clock_gettime(CLOCK_MONOTONIC, &t);
        val_ptr[0] = (int64_t)t.tv_sec;
        val_ptr[1] = (int64_t)t.tv_nsec;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

void Java_org_jau_sys_Clock_getWallClockTimeImpl(JNIEnv *env, jclass clazz, jlongArray jval) {
    (void)clazz;
    try {
        if( nullptr == jval ) {
            throw jau::IllegalArgumentException("val null", E_FILE_LINE);
        }
        const size_t in_size = env->GetArrayLength(jval);
        if( 2 > in_size ) {
            throw jau::IllegalArgumentException("val size "+std::to_string(in_size)+" < 2", E_FILE_LINE);
        }
        jau::jni::JNICriticalArray<int64_t, jlongArray> criticalArray(env); // RAII - release
        int64_t * val_ptr = criticalArray.get(jval, criticalArray.Mode::UPDATE_AND_RELEASE);
        if( NULL == val_ptr ) {
            throw jau::InternalError("GetPrimitiveArrayCritical(address val array) is null", E_FILE_LINE);
        }
        struct timespec t { 0, 0 };
        clock_gettime(CLOCK_REALTIME, &t);
        val_ptr[0] = (int64_t)t.tv_sec;
        val_ptr[1] = (int64_t)t.tv_nsec;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

/**
 * See <http://man7.org/linux/man-pages/man2/clock_gettime.2.html>
 * <p>
 * Regarding avoiding kernel via VDSO,
 * see <http://man7.org/linux/man-pages/man7/vdso.7.html>,
 * clock_gettime seems to be well supported at least on kernel >= 4.4.
 * Only bfin and sh are missing, while ia64 seems to be complicated.
 */
jlong Java_org_jau_sys_Clock_currentTimeMillis(JNIEnv *env, jclass clazz) {
    (void)env;
    (void)clazz;

    struct timespec t { 0, 0 };
    clock_gettime(CLOCK_MONOTONIC, &t);
    int64_t res = static_cast<int64_t>( t.tv_sec ) * MilliPerOne +
                  static_cast<int64_t>( t.tv_nsec ) / NanoPerMilli;
    return (jlong)res;
}

jlong Java_org_jau_sys_Clock_wallClockSeconds(JNIEnv *env, jclass clazz) {
    (void)env;
    (void)clazz;

    struct timespec t { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &t);
    return (jlong)( static_cast<int64_t>( t.tv_sec ) );
}

jlong Java_org_jau_sys_Clock_startupTimeMillisImpl(JNIEnv *env, jclass clazz) {
    (void)env;
    (void)clazz;

    return jau::environment::startupTimeMilliseconds;
}

