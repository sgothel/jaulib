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

#include "org_jau_fs_FileUtil.h"

#include <jau/debug.hpp>

#include "jau/jni/helper_jni.hpp"

#include "jau/file_util.hpp"

jboolean Java_org_jau_fs_FileUtil_remove_1impl(JNIEnv *env, jclass cls, jstring jpath, jshort jtopts) {
    (void)cls;
    try {
        const std::string path = jau::jni::from_jstring_to_string(env, jpath);
        const jau::fs::traverse_options topts = static_cast<jau::fs::traverse_options>(jtopts);
        return jau::fs::remove(path, topts) ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return false;
}

jboolean Java_org_jau_fs_FileUtil_compare(JNIEnv *env, jclass cls, jstring jsource1, jstring jsource2, jboolean verbose) {
    (void)cls;
    try {
        const std::string source1 = jau::jni::from_jstring_to_string(env, jsource1);
        const std::string source2 = jau::jni::from_jstring_to_string(env, jsource2);
        return jau::fs::compare(source1, source2, JNI_TRUE == verbose) ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return false;
}

jboolean Java_org_jau_fs_FileUtil_copy_1impl(JNIEnv *env, jclass cls, jstring jsource_path, jstring jdest_path, jshort jcopts) {
    (void)cls;
    try {
        const std::string source_path = jau::jni::from_jstring_to_string(env, jsource_path);
        const std::string dest_path = jau::jni::from_jstring_to_string(env, jdest_path);
        const jau::fs::copy_options copts = static_cast<jau::fs::copy_options>(jcopts);
        return jau::fs::copy(source_path, dest_path, copts) ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return false;
}

jlong Java_org_jau_fs_FileUtil_mount_1image(JNIEnv *env, jclass cls,
                                           jstring jimage_path, jstring jmount_point,
                                           jstring jfs_type, jlong jmountflags, jstring jfs_options) {
    (void)cls;
    try {
        const std::string image_path = jau::jni::from_jstring_to_string(env, jimage_path);
        const std::string mount_point = jau::jni::from_jstring_to_string(env, jmount_point);
        const std::string fs_type = jau::jni::from_jstring_to_string(env, jfs_type);
        const long mountflags = static_cast<long>(jmountflags);
        const std::string fs_options = jau::jni::from_jstring_to_string(env, jfs_options);

        jau::fs::mount_ctx res = jau::fs::mount_image(image_path, mount_point, fs_type,
                                                      mountflags, fs_options);

        jau::jni::shared_ptr_ref<jau::fs::mount_ctx> ref( new jau::fs::mount_ctx(res) );
        return ref.release_to_jlong();
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return 0;
}

jboolean Java_org_jau_fs_FileUtil_umount(JNIEnv *env, jclass cls, jlong jcontext) {
    (void)cls;
    try {
        jau::jni::shared_ptr_ref<jau::fs::mount_ctx> sref(jcontext, false /* throw_on_nullptr */); // hold copy until done
        if( nullptr != sref.pointer() ) {
            // delete original
            std::shared_ptr<jau::fs::mount_ctx>* sref_ptr = jau::jni::castInstance<jau::fs::mount_ctx>(jcontext);
            delete sref_ptr;

            // umount using copy if !null
            if( !sref.is_null() ) {
                return jau::fs::umount(*sref) ? JNI_TRUE : JNI_FALSE;
            }
        }
        // dtor copy
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_FALSE;
}
