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

#include "jau/byte_stream.hpp"

#include <jau/debug.hpp>

#include "jau/jni/helper_jni.hpp"

// #include "org_jau_io_IOState.h"
#include "org_jau_io_ByteInStream_File.h"


extern "C" {
    #include <fcntl.h>
}

//
// IOState
//

jstring Java_org_jau_io_IOState_to_1string(JNIEnv *env, jclass cls, jint mask) {
    (void)cls;
    try {
        const std::string s = jau::io::to_string(static_cast<jau::io::iostate>(mask));
        return jau::jni::from_string_to_jstring(env, s);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

//
// ByteInStream_File
//

jlong Java_org_jau_io_ByteInStream_1File_ctorImpl1(JNIEnv *env, jclass cls, jstring jpath) {
    try {
        (void)cls;
        // new instance
        const std::string _path = jau::jni::from_jstring_to_string(env, jpath);
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref( new jau::io::ByteInStream_File( _path ) );
        return ref.release_to_jlong();
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return (jlong) (intptr_t)nullptr;
}

jlong Java_org_jau_io_ByteInStream_1File_ctorImpl2(JNIEnv *env, jclass cls, jint dirfd, jstring jpath) {
    try {
        (void)cls;
        // new instance
        const std::string path = jau::jni::from_jstring_to_string(env, jpath);
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref( new jau::io::ByteInStream_File(dirfd, path) );
        return ref.release_to_jlong();
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return (jlong) (intptr_t)nullptr;
}

jlong Java_org_jau_io_ByteInStream_1File_ctorImpl3(JNIEnv *env, jclass cls, jint fd) {
    try {
        (void)cls;
        // new instance
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref( new jau::io::ByteInStream_File(fd) );
        return ref.release_to_jlong();
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return (jlong) (intptr_t)nullptr;
}

void Java_org_jau_io_ByteInStream_1File_closeStream(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done
        ref->close();
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

void Java_org_jau_io_ByteInStream_1File_dtorImpl(JNIEnv *env, jclass clazz, jlong nativeInstance) {
    (void)clazz;
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> sref(nativeInstance, false /* throw_on_nullptr */); // hold copy until done
        if( nullptr != sref.pointer() ) {
            std::shared_ptr<jau::io::ByteInStream_File>* sref_ptr = jau::jni::castInstance<jau::io::ByteInStream_File>(nativeInstance);
            delete sref_ptr;
        }
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

void Java_org_jau_io_ByteInStream_1File_clearImpl(JNIEnv *env, jobject obj, jint mask) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done
        ref->clear( static_cast<jau::io::iostate>(mask) );
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

jint Java_org_jau_io_ByteInStream_1File_fd(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done
        return ref->fd();
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return -1;
}

jint Java_org_jau_io_ByteInStream_1File_rdStateImpl(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done
        return static_cast<jint>( ref->rdstate() );
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return static_cast<jint>(jau::io::iostate::failbit);
}

void Java_org_jau_io_ByteInStream_1File_setStateImpl(JNIEnv *env, jobject obj, jint mask) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done
        ref->setstate( static_cast<jau::io::iostate>(mask) );
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

jboolean Java_org_jau_io_ByteInStream_1File_good(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done
        return ref->good() ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_FALSE;
}

jboolean Java_org_jau_io_ByteInStream_1File_eof(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done
        return ref->eof() ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_TRUE;
}

jboolean Java_org_jau_io_ByteInStream_1File_fail(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done
        return ref->fail() ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_TRUE;
}

jboolean Java_org_jau_io_ByteInStream_1File_bad(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done
        return ref->bad() ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_FALSE;
}

jboolean Java_org_jau_io_ByteInStream_1File_available(JNIEnv *env, jobject obj, jlong n) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done
        return ref->available((size_t)n) ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_FALSE;
}

jint Java_org_jau_io_ByteInStream_1File_read(JNIEnv *env, jobject obj, jbyteArray jout, jint joffset, jint jlength) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done

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

jint Java_org_jau_io_ByteInStream_1File_read2Impl(JNIEnv *env, jobject obj, jobject jout, jint out_offset) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done

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

jint Java_org_jau_io_ByteInStream_1File_peek(JNIEnv *env, jobject obj, jbyteArray jout, jint joffset, jint jlength, jlong jpeek_offset) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done

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

jstring Java_org_jau_io_ByteInStream_1File_id(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done
        return jau::jni::from_string_to_jstring(env, ref->id());
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

jlong Java_org_jau_io_ByteInStream_1File_discard_1next(JNIEnv *env, jobject obj, jlong n) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done

        const size_t res = ref->discard(n);
        return (jlong)res;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return 0;
}

jlong Java_org_jau_io_ByteInStream_1File_tellg(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done
        return static_cast<jlong>( ref->tellg() );
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return 0;
}

jboolean Java_org_jau_io_ByteInStream_1File_has_1content_1size(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done
        return ref->has_content_size() ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_FALSE;
}

jlong Java_org_jau_io_ByteInStream_1File_content_1size(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj); // hold until done
        return static_cast<jlong>( ref->content_size() );
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return 0;
}

jstring Java_org_jau_io_ByteInStream_1File_toString(JNIEnv *env, jobject obj) {
    try {
        jau::jni::shared_ptr_ref<jau::io::ByteInStream_File> ref(env, obj, false /* throw_on_nullptr */); // hold until done
        std::string str = ref.is_null() ? "null" : ref->to_string();
        return jau::jni::from_string_to_jstring(env, str);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}
