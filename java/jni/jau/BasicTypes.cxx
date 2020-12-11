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

#include "org_jau_BasicTypes.h"

#include <cstdint>
#include <cinttypes>

#include <time.h>

#include <jau/dfa_utf8_decode.hpp>

#include "helper_base.hpp"

jstring Java_org_jau_BasicTypes_decodeUTF8String(JNIEnv *env, jclass clazz, jbyteArray jbuffer, jint offset, jint size) {
    (void)clazz;

    const int buffer_size = env->GetArrayLength(jbuffer);
    if( 0 == buffer_size ) {
        return env->NewStringUTF("");
    }
    if( buffer_size < offset+size ) {
        std::string msg("buffer.length "+std::to_string(buffer_size)+
                        " < offset "+std::to_string(offset)+
                        " + size "+std::to_string(size));
        throw std::invalid_argument(msg.c_str());
    }

    std::string sres;
    {
        JNICriticalArray<uint8_t, jbyteArray> criticalArray(env); // RAII - release
        uint8_t * buffer_ptr = criticalArray.get(jbuffer, criticalArray.Mode::NO_UPDATE_AND_RELEASE);
        if( NULL == buffer_ptr ) {
            throw std::invalid_argument("GetPrimitiveArrayCritical(byte array) is null");
        }
        sres = jau::dfa_utf8_decode(buffer_ptr+offset, static_cast<size_t>(size));
    }
    return jau::from_string_to_jstring(env, sres);
}
