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

#include "org_jau_fs_FMode.h"
#include "org_jau_fs_FileStats.h"
#include "org_jau_fs_FileUtil.h"
#include "org_jau_fs_DirItem.h"

#include <jau/debug.hpp>

#include "jau/jni/helper_jni.hpp"

#include "jau/file_util.hpp"

extern "C" {
    #include <sys/stat.h>
}

//
// FMode
//

jstring Java_org_jau_fs_FMode_to_1string(JNIEnv *env, jclass cls, jint mask, jboolean show_rwx) {
    (void)cls;
    try {
        const std::string s = jau::fs::to_string(static_cast<jau::fs::fmode_t>(mask), JNI_TRUE == show_rwx);
        return jau::jni::from_string_to_jstring(env, s);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

//
// FileStats
//

jlong Java_org_jau_fs_FileStats_ctorImpl1(JNIEnv *env, jclass clazz, jstring jpath) {
    (void)clazz;
    try {
        if( nullptr == jpath ) {
            throw jau::IllegalArgumentException("path null", E_FILE_LINE);
        }
        std::string path = jau::jni::from_jstring_to_string(env, jpath);

        // new instance
        jau::jni::shared_ptr_ref<jau::fs::file_stats> ref( new jau::fs::file_stats( path ) );

        return ref.release_to_jlong();
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return (jlong) (intptr_t) nullptr;
}

jlong Java_org_jau_fs_FileStats_ctorImpl2(JNIEnv *env, jclass clazz, jstring jdirname, jstring jbasename) {
    (void)clazz;
    try {
        if( nullptr == jdirname || nullptr == jbasename ) {
            throw jau::IllegalArgumentException("path null", E_FILE_LINE);
        }
        std::string dirname = jau::jni::from_jstring_to_string(env, jdirname);
        std::string basename = jau::jni::from_jstring_to_string(env, jbasename);
        jau::fs::dir_item item(dirname, basename);

        // new instance
        jau::jni::shared_ptr_ref<jau::fs::file_stats> ref( new jau::fs::file_stats( item ) );

        return ref.release_to_jlong();
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return (jlong) (intptr_t) nullptr;
}

jlong Java_org_jau_fs_FileStats_ctorImpl3(JNIEnv *env, jclass clazz, jint fd) {
    (void)clazz;
    try {
        // new instance
        jau::jni::shared_ptr_ref<jau::fs::file_stats> ref( new jau::fs::file_stats( fd ) );

        return ref.release_to_jlong();
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return (jlong) (intptr_t) nullptr;
}

jlong Java_org_jau_fs_FileStats_ctorLinkTargetImpl(JNIEnv *env, jclass clazz, jlong nativeInstance) {
    (void)clazz;
    try {
        jau::jni::shared_ptr_ref<jau::fs::file_stats> sref(nativeInstance, true /* throw_on_nullptr */); // hold copy until done

        const std::shared_ptr<jau::fs::file_stats>& lt = sref->link_target();
        if( nullptr != lt ) {
            jau::jni::shared_ptr_ref<jau::fs::file_stats> ref( lt );
            return ref.release_to_jlong();
        }
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return (jlong) (intptr_t) nullptr;
}

void Java_org_jau_fs_FileStats_dtorImpl(JNIEnv *env, jclass clazz, jlong nativeInstance) {
    (void)clazz;
    try {
        jau::jni::shared_ptr_ref<jau::fs::file_stats> sref(nativeInstance, false /* throw_on_nullptr */); // hold copy until done
        if( nullptr != sref.pointer() ) {
            std::shared_ptr<jau::fs::file_stats>* sref_ptr = jau::jni::castInstance<jau::fs::file_stats>(nativeInstance);
            delete sref_ptr;
        }
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
}

jintArray Java_org_jau_fs_FileStats_getInt6FieldsFModeFdUidGidErrno(JNIEnv *env, jclass clazz, jlong nativeInstance) {
    (void)clazz;
    try {
        jau::jni::shared_ptr_ref<jau::fs::file_stats> sref(nativeInstance, true /* throw_on_nullptr */); // hold copy until done

        const size_t array_size = 6;
        const jint values[] = { (jint)sref->fields(), (jint)sref->mode(), (jint)sref->fd(), (jint)sref->uid(), (jint)sref->gid(), (jint)sref->errno_res() };
        jintArray jres = env->NewIntArray((jsize)array_size);
        if (nullptr == jres) {
            throw jau::InternalError("Cannot create instance of jintArray", E_FILE_LINE);
        }
        env->SetIntArrayRegion(jres, 0, (jsize)array_size, values);
        jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);
        return jres;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

jobjectArray Java_org_jau_fs_DirItem_getString2DirItem(JNIEnv *env, jclass clazz, jstring jpath) {
    (void)clazz;
    try {
        const std::string path = jau::jni::from_jstring_to_string(env, jpath);

        const size_t array_size = 2;
        jclass strclz = jau::jni::search_class(env, "java/lang/String");
        jobjectArray jres = env->NewObjectArray(array_size, strclz, nullptr);
        if (nullptr == jres) {
            throw jau::InternalError("Cannot create instance of jobjectArray", E_FILE_LINE);
        }
        jau::fs::dir_item di(path);
        {
            jstring jstr = jau::jni::from_string_to_jstring(env, di.dirname());
            env->SetObjectArrayElement(jres, 0, jstr);
            env->DeleteLocalRef(jstr);
            jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);
        }
        {
            jstring jstr = jau::jni::from_string_to_jstring(env, di.basename());
            env->SetObjectArrayElement(jres, 1, jstr);
            env->DeleteLocalRef(jstr);
            jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);
        }
        return jres;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

jobjectArray Java_org_jau_fs_FileStats_getString3DirItemLinkTargetPath(JNIEnv *env, jclass clazz, jlong nativeInstance) {
    (void)clazz;
    try {
        jau::jni::shared_ptr_ref<jau::fs::file_stats> sref(nativeInstance, true /* throw_on_nullptr */); // hold copy until done

        const size_t array_size = 3;
        jclass strclz = jau::jni::search_class(env, "java/lang/String");
        jobjectArray jres = env->NewObjectArray(array_size, strclz, nullptr);
        if (nullptr == jres) {
            throw jau::InternalError("Cannot create instance of jobjectArray", E_FILE_LINE);
        }
        {
            jstring jstr = jau::jni::from_string_to_jstring(env, sref->item().dirname());
            env->SetObjectArrayElement(jres, 0, jstr);
            env->DeleteLocalRef(jstr);
            jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);
        }
        {
            jstring jstr = jau::jni::from_string_to_jstring(env, sref->item().basename());
            env->SetObjectArrayElement(jres, 1, jstr);
            env->DeleteLocalRef(jstr);
            jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);
        }
        {
            const std::shared_ptr<std::string>& str_ref = sref->link_target_path();
            jstring jstr = nullptr != str_ref ? jau::jni::from_string_to_jstring(env, *str_ref) : nullptr;
            env->SetObjectArrayElement(jres, 2, jstr);
            if( nullptr != jstr ) {
                env->DeleteLocalRef(jstr);
            }
            jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);
        }
        return jres;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

jlongArray Java_org_jau_fs_FileStats_getLong9SizeTimes(JNIEnv *env, jclass clazz, jlong nativeInstance) {
    (void)clazz;
    try {
        jau::jni::shared_ptr_ref<jau::fs::file_stats> sref(nativeInstance, true /* throw_on_nullptr */); // hold copy until done

        const size_t array_size = 9;
        const jlong values[] = { (jlong)sref->size(),
                                 (jlong)sref->btime().tv_sec, (jlong)sref->btime().tv_nsec,
                                 (jlong)sref->atime().tv_sec, (jlong)sref->atime().tv_nsec,
                                 (jlong)sref->ctime().tv_sec, (jlong)sref->ctime().tv_nsec,
                                 (jlong)sref->mtime().tv_sec, (jlong)sref->mtime().tv_nsec };
        jlongArray jres = env->NewLongArray((jsize)array_size);
        if (nullptr == jres) {
            throw jau::InternalError("Cannot create instance of jlongArray", E_FILE_LINE);
        }
        env->SetLongArrayRegion(jres, 0, (jsize)array_size, values);
        jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);
        return jres;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

//
// FileUtil
//

jstring Java_org_jau_fs_FileUtil_get_1cwd(JNIEnv *env, jclass cls) {
    (void)cls;
    try {
        const std::string cwd = jau::fs::get_cwd();
        return jau::jni::from_string_to_jstring(env, cwd);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

jstring Java_org_jau_fs_FileUtil_dirname(JNIEnv *env, jclass cls, jstring jpath) {
    (void)cls;
    try {
        const std::string path = jau::jni::from_jstring_to_string(env, jpath);
        const std::string dir = jau::fs::dirname(path);
        return jau::jni::from_string_to_jstring(env, dir);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

jstring Java_org_jau_fs_FileUtil_basename(JNIEnv *env, jclass cls, jstring jpath) {
    (void)cls;
    try {
        const std::string path = jau::jni::from_jstring_to_string(env, jpath);
        const std::string bname = jau::fs::basename(path);
        return jau::jni::from_string_to_jstring(env, bname);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

jstring Java_org_jau_fs_FileUtil_to_1named_1fd(JNIEnv *env, jclass cls, jint fd) {
    (void)cls;
    try {
        const std::string named_fd = jau::fs::to_named_fd(fd);
        return jau::jni::from_string_to_jstring(env, named_fd);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

jint Java_org_jau_fs_FileUtil_from_1named_1fd(JNIEnv *env, jclass cls, jstring jnamed_fd) {
    (void)cls;
    try {
        const std::string named_fd = jau::jni::from_jstring_to_string(env, jnamed_fd);
        return jau::fs::from_named_fd(named_fd);
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return -1;
}

jint Java_org_jau_fs_FileUtil_from_1java_1fd(JNIEnv *env, jclass cls, jobject jfd) {
    (void)cls;
    try {
        return jau::jni::getIntFieldValue(env, jfd, "fd");
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return -1;
}

jboolean Java_org_jau_fs_FileUtil_mkdirImpl(JNIEnv *env, jclass cls, jstring jpath, jint jmode) {
    (void)cls;
    try {
        const std::string path = jau::jni::from_jstring_to_string(env, jpath);
        const jau::fs::fmode_t mode = static_cast<jau::fs::fmode_t>(jmode);
        return jau::fs::mkdir(path, mode) ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_FALSE;
}

constexpr const long MY_UTIME_NOW  = ((1l << 30) - 1l);

jboolean Java_org_jau_fs_FileUtil_touchImpl(JNIEnv *env, jclass cls, jstring jpath,
                                            jlong atime_s, jlong atime_ns,
                                            jlong mtime_s, jlong mtime_ns,
                                            jint jmode)
{
    (void)cls;
    try {
        const std::string path = jau::jni::from_jstring_to_string(env, jpath);
        const jau::fs::fmode_t mode = static_cast<jau::fs::fmode_t>(jmode);
        if( MY_UTIME_NOW == atime_ns || MY_UTIME_NOW == mtime_ns ) {
            return jau::fs::touch(path, mode) ? JNI_TRUE : JNI_FALSE;
        } else {
            const jau::fraction_timespec atime((int64_t)atime_s, (int64_t)atime_ns);
            const jau::fraction_timespec mtime((int64_t)mtime_s, (int64_t)mtime_ns);
            return jau::fs::touch(path, atime, mtime, mode) ? JNI_TRUE : JNI_FALSE;
        }
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return JNI_FALSE;
}

jobject Java_org_jau_fs_FileUtil_get_1dir_1content(JNIEnv *env, jclass cls, jstring jpath) {
    (void)cls;
    try {
        const std::string path = jau::jni::from_jstring_to_string(env, jpath);
        std::vector<jau::fs::dir_item> content;
        const jau::fs::consume_dir_item cs = jau::bindCaptureRefFunc<void, std::vector<jau::fs::dir_item>, const jau::fs::dir_item&>(&content,
            ( void(*)(std::vector<jau::fs::dir_item>*, const jau::fs::dir_item&) ) /* help template type deduction of function-ptr */
                ( [](std::vector<jau::fs::dir_item>* receiver, const jau::fs::dir_item& item) -> void { receiver->push_back( item ); } )
        );
        if( get_dir_content(path, cs) ) {
            static const std::string _dirItemClassName("org/jau/fs/DirItem");
            static const std::string _dirItemClazzCtorArgs("(Ljava/lang/String;Ljava/lang/String;)V");
            jclass dirItemClazz = jau::jni::search_class(env, _dirItemClassName.c_str());
            jmethodID dirItemClazzCtor = jau::jni::search_method(env, dirItemClazz, "<init>", _dirItemClazzCtorArgs.c_str(), false);
            std::function<jobject(JNIEnv*, const jau::fs::dir_item&)> ctor_diritem =
                    [&](JNIEnv *env_, const jau::fs::dir_item& di)->jobject {
                        jstring dname = jau::jni::from_string_to_jstring(env_, di.dirname());
                        jstring bname = jau::jni::from_string_to_jstring(env_, di.basename());
                        jobject jdirItem = env->NewObject(dirItemClazz, dirItemClazzCtor, dname, bname);
                        jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);
                        return jdirItem;
                    };
            return jau::jni::convert_vector_to_jarraylist<std::vector<jau::fs::dir_item>, jau::fs::dir_item>(env, content, ctor_diritem);
        }
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return nullptr;
}

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

jboolean Java_org_jau_fs_FileUtil_rename(JNIEnv *env, jclass cls, jstring joldpath, jstring jnewpath) {
    (void)cls;
    try {
        const std::string oldpath = jau::jni::from_jstring_to_string(env, joldpath);
        const std::string newpath = jau::jni::from_jstring_to_string(env, jnewpath);
        return jau::fs::rename(oldpath, newpath) ? JNI_TRUE : JNI_FALSE;
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
    return false;
}

void Java_org_jau_fs_FileUtil_sync(JNIEnv *env, jclass cls) {
    (void)cls;
    try {
        jau::fs::sync();
    } catch(...) {
        rethrow_and_raise_java_exception_jau(env);
    }
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

        if( res.mounted ) {
            jau::jni::shared_ptr_ref<jau::fs::mount_ctx> ref( new jau::fs::mount_ctx(res) );
            return ref.release_to_jlong();
        }
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
