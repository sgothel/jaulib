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

#include "org_jau_nio_ByteInStream_URL.h"

#include <jau/debug.hpp>

#include "jau/jni/helper_jni.hpp"

#include "jau/byte_stream.hpp"

jlong Java_org_jau_nio_ByteInStream_1URL_ctorImpl(JNIEnv *env, jobject obj, jstring jurl, jlong jtimeoutMS) {
    try {
        (void)obj;
        // new instance
        const std::string url = jau::jni::from_jstring_to_string(env, jurl);
        const jau::fraction_i64 timeout = (int64_t)jtimeoutMS * 1_ms;
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_URL> ref( new jau::io::ByteInStream_URL(url, timeout) );
        return ref.release_to_jlong();
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return (jlong) (intptr_t)nullptr;
}

void Java_org_jau_nio_ByteInStream_1URL_closeStream(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_URL> ref(env, obj); // hold until done
        ref->close();
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

void Java_org_jau_nio_ByteInStream_1URL_dtorImpl(JNIEnv *env, jclass clazz, jlong nativeInstance) {
    (void)clazz;
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_URL> sref(nativeInstance, false /* throw_on_nullptr */); // hold copy until done
        if( nullptr != sref.pointer() ) {
            std::shared_ptr<jau::io::ByteInStream_URL>* sref_ptr = jau::jni::castInstance<jau::io::ByteInStream_URL>(nativeInstance);
            delete sref_ptr;
        }
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

jboolean Java_org_jau_nio_ByteInStream_1URL_check_1available(JNIEnv *env, jobject obj, jlong n) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_URL> ref(env, obj); // hold until done
        return ref->check_available((size_t)n) ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_FALSE;
}

jint Java_org_jau_nio_ByteInStream_1URL_read(JNIEnv *env, jobject obj, jbyteArray jout, jint joffset, jint jlength) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_URL> ref(env, obj); // hold until done

        if( nullptr == jout ) {
            throw jau::IllegalArgumentException("out buffer null", E_FILE_LINE);
        }
        const size_t in_size = env->GetArrayLength(jout);
        if( (size_t)joffset + (size_t)jlength > in_size ) {
            throw jau::IllegalArgumentException("output byte size "+std::to_string(in_size)+" < "+std::to_string(joffset)+" + "+std::to_string(jlength), E_FILE_LINE);
        }
        jau::jni::JNICriticalArray<uint8_t, jbyteArray> criticalArray(env); // RAII - release
        uint8_t * out_ptr = criticalArray.get(jout, criticalArray.Mode::UPDATE_AND_RELEASE);
        if( NULL == out_ptr ) {
            throw jau::InternalError("GetPrimitiveArrayCritical(address byte array) is null", E_FILE_LINE);
        }
        return (jint) ref->read(out_ptr + joffset, jlength);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return 0;
}

jint Java_org_jau_nio_ByteInStream_1URL_read2Impl(JNIEnv *env, jobject obj, jobject jout, jint out_offset) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_URL> ref(env, obj); // hold until done

        if( nullptr == jout ) {
            throw jau::IllegalArgumentException("out buffer null", E_FILE_LINE);
        }
        const jlong out_cap = env->GetDirectBufferCapacity(jout);
        uint8_t * out_ptr = static_cast<uint8_t *>( env->GetDirectBufferAddress(jout) );
        if( 0 > out_cap || nullptr == out_ptr ) {
            throw jau::IllegalArgumentException("out buffer access failure", E_FILE_LINE);
        }
        return (jint) ref->read(out_ptr + out_offset, out_cap - out_offset);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return 0;
}

jint Java_org_jau_nio_ByteInStream_1URL_peek(JNIEnv *env, jobject obj, jbyteArray jout, jint joffset, jint jlength, jlong jpeek_offset) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_URL> ref(env, obj); // hold until done

        if( nullptr == jout ) {
            throw jau::IllegalArgumentException("out buffer null", E_FILE_LINE);
        }
        const size_t in_size = env->GetArrayLength(jout);
        if( (size_t)joffset + (size_t)jlength > in_size ) {
            throw jau::IllegalArgumentException("output byte size "+std::to_string(in_size)+" < "+std::to_string(joffset)+" + "+std::to_string(jlength), E_FILE_LINE);
        }
        jau::jni::JNICriticalArray<uint8_t, jbyteArray> criticalArray(env); // RAII - release
        uint8_t * out_ptr = criticalArray.get(jout, criticalArray.Mode::UPDATE_AND_RELEASE);
        if( NULL == out_ptr ) {
            throw jau::InternalError("GetPrimitiveArrayCritical(address byte array) is null", E_FILE_LINE);
        }
        const size_t res = ref->peek(out_ptr + joffset, jlength, jpeek_offset);
        return (jint)res;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return 0;
}

jboolean Java_org_jau_nio_ByteInStream_1URL_end_1of_1data(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_URL> ref(env, obj); // hold until done
        return ref->end_of_data() ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_TRUE;
}

jboolean Java_org_jau_nio_ByteInStream_1URL_error(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_URL> ref(env, obj); // hold until done
        return ref->error() ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_TRUE;
}

jstring Java_org_jau_nio_ByteInStream_1URL_id(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_URL> ref(env, obj); // hold until done
        return jau::jni::from_string_to_jstring(env, ref->id());
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

jlong Java_org_jau_nio_ByteInStream_1URL_discard_1next(JNIEnv *env, jobject obj, jlong n) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_URL> ref(env, obj); // hold until done

        const size_t res = ref->discard_next(n);
        return (jlong)res;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return 0;
}

jlong Java_org_jau_nio_ByteInStream_1URL_get_1bytes_1read(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_URL> ref(env, obj); // hold until done
        return static_cast<jlong>( ref->get_bytes_read() );
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return 0;
}

jboolean Java_org_jau_nio_ByteInStream_1URL_has_1content_1size(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_URL> ref(env, obj); // hold until done
        return ref->has_content_size() ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_FALSE;
}

jlong Java_org_jau_nio_ByteInStream_1URL_content_1size(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_URL> ref(env, obj); // hold until done
        return static_cast<jlong>( ref->content_size() );
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return 0;
}

jstring Java_org_jau_nio_ByteInStream_1URL_toString(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_URL> ref(env, obj, false /* throw_on_nullptr */); // hold until done
        std::string str = ref.is_null() ? "null" : ref->to_string();
        return jau::jni::from_string_to_jstring(env, str);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}
