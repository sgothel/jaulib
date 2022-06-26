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

#include "org_jau_nio_Uri.h"

#include <jau/debug.hpp>

#include "jau/jni/helper_jni.hpp"

#include "jau/io_util.hpp"

jobject Java_org_jau_nio_Uri_supported_1protocols(JNIEnv *env, jclass cls) {
    (void)cls;
    try {
        std::vector<std::string_view> protos = jau::io::uri::supported_protocols();
        return jau::jni::convert_vector_stringview_to_jarraylist(env, protos);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

jstring Java_org_jau_nio_Uri_get_1scheme(JNIEnv *env, jclass cls, jstring juri) {
    (void)cls;
    try {
        const std::string uri = jau::jni::from_jstring_to_string(env, juri);
        const std::string res(jau::io::uri::get_scheme(uri));
        return jau::jni::from_string_to_jstring(env, res);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

jboolean Java_org_jau_nio_Uri_protocol_1supported(JNIEnv *env, jclass cls, jstring juri) {
    (void)cls;
    try {
        const std::string uri = jau::jni::from_jstring_to_string(env, juri);
        return jau::io::uri::protocol_supported(uri) ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_FALSE;
}

jboolean Java_org_jau_nio_Uri_is_1local_1file_1protocol(JNIEnv *env, jclass cls, jstring juri) {
    (void)cls;
    try {
        const std::string uri = jau::jni::from_jstring_to_string(env, juri);
        return jau::io::uri::is_local_file_protocol(uri) ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_FALSE;
}

