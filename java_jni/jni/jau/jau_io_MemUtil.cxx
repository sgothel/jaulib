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

#include "org_jau_io_MemUtil.h"

#include <cstdint>
#include <cinttypes>

#include <ctime>

#include <jau/environment.hpp>

#include "jau/jni/helper_jni.hpp"

void Java_org_jau_io_MemUtil_zeroByteBuffer(JNIEnv *env, jclass clazz, jobject jbuf) {
    (void)clazz;
    if( nullptr != jbuf ) {
        void* address = env->GetDirectBufferAddress(jbuf);
        jlong capacity = env->GetDirectBufferCapacity(jbuf);
        if( nullptr != address && 0 < capacity ) {
            ::explicit_bzero(address, capacity);
        }
    }
}

#if 0

jboolean Java_org_jau_io_MemUtil_zeroString(JNIEnv *env, jclass clazz, jstring jstr) {
    (void)clazz;
    if( nullptr != jstr ) {
        size_t len = env->GetStringLength(jstr);
        if( 0 == len ) {
            return JNI_TRUE;
        }
        jboolean is_copy = JNI_FALSE;
        const jchar* str_u16 = env->GetStringCritical(jstr, &is_copy);
        jau_DBG_PRINT("zeroString.0: jstr %p, copy %d", str_u16, is_copy);
        if( nullptr != str_u16 ) {
            ::explicit_bzero((void*)str_u16, len * sizeof(jchar));
            env->ReleaseStringCritical(jstr, str_u16);
        } else {
            jau_ERR_PRINT("GetStringCritical() return null");
        }
        if( nullptr == str_u16 || JNI_TRUE == is_copy ) {
            // try harder ..
            jclass string_clazz = jau::jni::search_class(env, "java/lang/String");
            jfieldID f_value = jau::jni::search_field(env, string_clazz, "value", "[B", false /* is_static */);
            jbyteArray jstr_value = (jbyteArray)env->GetObjectField(jstr, f_value);
            jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);

            if( nullptr == jstr_value ) {
                jau_ERR_PRINT("GetObjectField(value) is null");
                return JNI_FALSE;
            }
            const size_t jstr_value_size = env->GetArrayLength(jstr_value);
            if( 0 == jstr_value_size ) {
                jau_ERR_PRINT("GetArrayLength(address byte array) is null");
                return JNI_FALSE;
            }
            jau::jni::JNICriticalArray<uint8_t, jbyteArray> criticalArray(env); // RAII - release
            uint8_t * ptr = criticalArray.get(jstr_value, criticalArray.Mode::UPDATE_AND_RELEASE);
            jau_DBG_PRINT("zeroString.1: value: %p, len %zu, is_copy %d", ptr, jstr_value_size, criticalArray.getIsCopy());
            if( nullptr == ptr ) {
                jau_ERR_PRINT("GetPrimitiveArrayCritical(address byte array) is null");
                return JNI_FALSE;
            }
            ::explicit_bzero((void*)ptr, jstr_value_size);
        }
    }
    return JNI_TRUE;
}

#include <locale>
#include <codecvt>

// utility wrapper to adapt locale-bound facets for wstring/wbuffer convert
template<class Facet>
struct deletable_facet : Facet
{
    template<class ...Args>
    deletable_facet(Args&& ...args) : Facet(std::forward<Args>(args)...) {}
    ~deletable_facet() {}
};

jobject Java_org_jau_io_MemUtil_toByteBufferImpl(JNIEnv *env, jclass clazz, jstring jstr) {
    (void)clazz;
    if( nullptr == jstr ) {
        return nullptr;
    }
    size_t jstr_len = env->GetStringLength(jstr);
    if( 0 == jstr_len ) {
        return nullptr;
    }
    jboolean isCopy = JNI_TRUE;
    const jchar* jstr_u16 = env->GetStringCritical(jstr, &isCopy);
    if( nullptr == jstr_u16 ) {
        return nullptr;
    }
    std::u16string str_u16(jstr_u16, jstr_u16+jstr_len);
    if( JNI_TRUE == isCopy ) {
        ::explicit_bzero((void*)jstr_u16, jstr_len * sizeof(jchar));
    }
    env->ReleaseStringCritical(jstr, jstr_u16);

    // UTF-16/char16_t to UTF-8
    std::wstring_convert<deletable_facet<std::codecvt<char16_t, char, std::mbstate_t>>, char16_t> conv16;
    // std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv16;
    std::string u8_conv = conv16.to_bytes(str_u16);
    ::explicit_bzero((void*)str_u16.data(), str_u16.size() * sizeof(char16_t));
    const size_t u8_size = u8_conv.size();

    jmethodID buffer_new = jau::jni::search_method(env, clazz, "newDirectByteBuffer", "(I)Ljava/nio/ByteBuffer;", true);
    jobject jdest = env->CallStaticObjectMethod(clazz, buffer_new, (jint)u8_size);
    jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);
    if( nullptr == jdest ) {
        jau_ERR_PRINT("Couldn't allocated ByteBuffer w/ capacity %zu", u8_size);
        return nullptr;
    }
    char* jdest_address = (char*)env->GetDirectBufferAddress(jdest);
    size_t jdest_capacity = (size_t)env->GetDirectBufferCapacity(jdest);
    if( nullptr == jdest_address || 0 == jdest_capacity ) {
        jau_ERR_PRINT("ByteBuffer w/ capacity %zu has zero address (%p) or length (%zu)", u8_size, jdest_address, jdest_capacity);
        return nullptr;
    }
    if( u8_size > jdest_capacity ) {
        jau_ERR_PRINT("ByteBuffer w/ capacity %zu < required size %zu)", jdest_capacity, u8_size);
        return nullptr;
    }
    ::memcpy(jdest_address, u8_conv.data(), u8_size);
    ::explicit_bzero((void*)u8_conv.data(), u8_size);
    return jdest;
}

#endif
