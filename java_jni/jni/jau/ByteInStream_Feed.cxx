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

#include "org_jau_io_ByteInStream_Feed.h"

#include <jau/debug.hpp>

#include "jau/jni/helper_jni.hpp"

#include "jau/byte_stream.hpp"

jlong Java_org_jau_io_ByteInStream_1Feed_ctorImpl(JNIEnv *env, jobject obj, jstring jid_name, jlong jtimeoutMS) {
    try {
        (void)obj;
        // new instance
        const std::string id_name = jau::jni::from_jstring_to_string(env, jid_name);
        const jau::fraction_i64 timeout = (int64_t)jtimeoutMS * 1_ms;
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref( new jau::io::ByteInStream_Feed(id_name, timeout) );
        return ref.release_to_jlong();
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return (jlong) (intptr_t)nullptr;
}

void Java_org_jau_io_ByteInStream_1Feed_closeStream(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done
        ref->close();
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

void Java_org_jau_io_ByteInStream_1Feed_dtorImpl(JNIEnv *env, jclass clazz, jlong nativeInstance) {
    (void)clazz;
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> sref(nativeInstance, false /* throw_on_nullptr */); // hold copy until done
        if( nullptr != sref.pointer() ) {
            std::shared_ptr<jau::io::ByteInStream_Feed>* sref_ptr = jau::jni::castInstance<jau::io::ByteInStream_Feed>(nativeInstance);
            delete sref_ptr;
        }
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

jboolean Java_org_jau_io_ByteInStream_1Feed_check_1available(JNIEnv *env, jobject obj, jlong n) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done
        return ref->check_available((size_t)n) ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_FALSE;
}

jint Java_org_jau_io_ByteInStream_1Feed_read(JNIEnv *env, jobject obj, jbyteArray jout, jint joffset, jint jlength) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done

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

jint Java_org_jau_io_ByteInStream_1Feed_read2Impl(JNIEnv *env, jobject obj, jobject jout, jint out_offset) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done

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

jint Java_org_jau_io_ByteInStream_1Feed_peek(JNIEnv *env, jobject obj, jbyteArray jout, jint joffset, jint jlength, jlong jpeek_offset) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done

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

jboolean Java_org_jau_io_ByteInStream_1Feed_end_1of_1data(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done
        return ref->end_of_data() ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_TRUE;
}

jboolean Java_org_jau_io_ByteInStream_1Feed_error(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done
        return ref->error() ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_TRUE;
}

jstring Java_org_jau_io_ByteInStream_1Feed_id(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done
        return jau::jni::from_string_to_jstring(env, ref->id());
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

jlong Java_org_jau_io_ByteInStream_1Feed_discard_1next(JNIEnv *env, jobject obj, jlong n) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done

        const size_t res = ref->discard_next(n);
        return (jlong)res;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return 0;
}

jlong Java_org_jau_io_ByteInStream_1Feed_get_1bytes_1read(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done
        return static_cast<jlong>( ref->get_bytes_read() );
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return 0;
}

jboolean Java_org_jau_io_ByteInStream_1Feed_has_1content_1size(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done
        return ref->has_content_size() ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_FALSE;
}

jlong Java_org_jau_io_ByteInStream_1Feed_content_1size(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done
        return static_cast<jlong>( ref->content_size() );
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return 0;
}

void Java_org_jau_io_ByteInStream_1Feed_interruptReader(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done
        ref->interruptReader();
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

void Java_org_jau_io_ByteInStream_1Feed_write(JNIEnv *env, jobject obj, jbyteArray jin, jint joffset, jint jlength) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done

        if( nullptr == jin ) {
            throw jau::IllegalArgumentException("address null", E_FILE_LINE);
        }
        const size_t in_size = env->GetArrayLength(jin);
        if( (size_t)joffset + (size_t)jlength > in_size ) {
            throw jau::IllegalArgumentException("input byte size "+std::to_string(in_size)+" < "+std::to_string(joffset)+" + "+std::to_string(jlength), E_FILE_LINE);
        }
        jau::jni::JNICriticalArray<uint8_t, jbyteArray> criticalArray(env); // RAII - release
        uint8_t * in_ptr = criticalArray.get(jin, criticalArray.Mode::NO_UPDATE_AND_RELEASE);
        if( NULL == in_ptr ) {
            throw jau::InternalError("GetPrimitiveArrayCritical(address byte array) is null", E_FILE_LINE);
        }
        ref->write(in_ptr + joffset, jlength);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

void Java_org_jau_io_ByteInStream_1Feed_write2Impl(JNIEnv *env, jobject obj, jobject jout, jint out_offset, jint out_limit) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done

        if( nullptr == jout ) {
            throw jau::IllegalArgumentException("out buffer null", E_FILE_LINE);
        }
        uint8_t * out_ptr = static_cast<uint8_t *>( env->GetDirectBufferAddress(jout) );
        if( nullptr == out_ptr ) {
            throw jau::IllegalArgumentException("out buffer access failure", E_FILE_LINE);
        }
        ref->write(out_ptr + out_offset, out_limit - out_offset);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

void Java_org_jau_io_ByteInStream_1Feed_set_1content_1size(JNIEnv *env, jobject obj, jlong jcontent_size) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done
        ref->set_content_size( static_cast<uint64_t>( jcontent_size ) );
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

void Java_org_jau_io_ByteInStream_1Feed_set_1eof(JNIEnv *env, jobject obj, jint jresult) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj); // hold until done
        ref->set_eof(static_cast<jau::io::async_io_result_t>(jresult));
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

jstring Java_org_jau_io_ByteInStream_1Feed_toString(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_Feed> ref(env, obj, false /* throw_on_nullptr */); // hold until done
        std::string str = ref.is_null() ? "null" : ref->to_string();
        return jau::jni::from_string_to_jstring(env, str);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}
