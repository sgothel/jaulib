/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2020 ZAFENA AB
 *
 * Author: Petre Eftime <petre.p.eftime@intel.com>
 * Copyright (c) 2016 Intel Corporation.
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

#include <cstdio>

#include <jau/debug.hpp>
#include <jau/jni/jni_mem.hpp>

using namespace jau::jni;

JavaVM* jau::jni::vm;
thread_local JNIEnvContainer jau::jni::jni_env;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *initVM, void *reserved) {
    (void)reserved;
    vm = initVM;
    return JNI_VERSION_1_8;
}

JNIEXPORT jint JNICALL JNI_OnLoad_jaulib_jni_jni(JavaVM *initVM, void *reserved) {
    (void)reserved;
    vm = initVM;
    return JNI_VERSION_1_8;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
    (void)vm;
    (void)reserved;
}

JNIEXPORT void JNICALL JNI_OnUnload_jaulib_jni_jni(JavaVM *vm, void *reserved) {
    (void)vm;
    (void)reserved;
}

JNIEnv *JNIEnvContainer::operator*() {
    attach();
    return env;
}

JNIEnv *JNIEnvContainer::operator->() {
    attach();
    return env;
}

JNIEnvContainer::JNIEnvContainer() = default;

JNIEnvContainer::~JNIEnvContainer() {
    detach();
}

void JNIEnvContainer::attach() {
    if (env != nullptr) {
        return;
    }
    JNIEnv *newEnv = nullptr;
    int envRes;

    envRes = vm->GetEnv((void **) &env, JNI_VERSION_1_8) ;
    if( JNI_EDETACHED == envRes ) {
        envRes = vm->AttachCurrentThreadAsDaemon((void**) &newEnv, nullptr);
        if( JNI_OK != envRes ) {
            throw jau::RuntimeException("Attach to VM failed, error "+std::to_string(envRes), E_FILE_LINE);
        }
        env = newEnv;
    } else if( JNI_OK != envRes ) {
        throw jau::RuntimeException("GetEnv of VM failed, error "+std::to_string(envRes), E_FILE_LINE);
    }
    if (env==nullptr) {
        throw jau::RuntimeException("GetEnv of VM is NULL", E_FILE_LINE);
    }
    needsDetach = nullptr != newEnv;
}

void JNIEnvContainer::detach() {
    if (env == nullptr) {
        return;
    }
    if( needsDetach ) {
        vm->DetachCurrentThread();
    }
    env = nullptr;
    needsDetach = false;
}

JNIGlobalRef::JNIGlobalRef() noexcept {
    this->object = nullptr;
    DBG_JNI_PRINT("JNIGlobalRef::def_ctor0 nullptr");
}

JNIGlobalRef::JNIGlobalRef(jobject _object) {
    if( nullptr == _object ) {
        throw jau::RuntimeException("JNIGlobalRef ctor1 null jobject", E_FILE_LINE);
    }
    JNIEnv * env = *jni_env;
#if 0
    // Notably 'JNIInvalidRefType' is returned on a valid jobject via its constructor call, e.g. Java_jau_direct_1bt_DBTManager_initImpl()
    const jobjectRefType ref_type = env->GetObjectRefType(object);
    if( JNIInvalidRefType == ref_type ) {
        throw jau::RuntimeException("JavaGlobalObj::ctor1: Invalid non-null jobject", E_FILE_LINE);
    }
#endif
    this->object = env->NewGlobalRef(_object);
    DBG_JNI_PRINT("JNIGlobalRef::def_ctor1 %p -> %p", _object, this->object);
}

JNIGlobalRef::JNIGlobalRef(const JNIGlobalRef &o) {
    JNIEnv * env = *jni_env;
    std::unique_lock<std::mutex> lock(o.mtx);

    if( nullptr == o.object ) {
        throw jau::RuntimeException("Other JNIGlobalRef jobject is null", E_FILE_LINE);
    }
    const jobjectRefType ref_type = env->GetObjectRefType(o.object);
    if( JNIInvalidRefType == ref_type ) {
        throw jau::RuntimeException("JavaGlobalObj::ctor2: Invalid non-null jobject", E_FILE_LINE);
    }
    object = env->NewGlobalRef(o.object);
    DBG_JNI_PRINT("JNIGlobalRef::copy_ctor %p -> %p", o.object, object);
}

JNIGlobalRef::JNIGlobalRef(JNIGlobalRef &&o) noexcept {
    std::unique_lock<std::mutex> lock(o.mtx);

    object = o.object;
    DBG_JNI_PRINT("JNIGlobalRef::move_ctor %p (nulled) -> %p", o.object, object);
    o.object = nullptr;
}

JNIGlobalRef& JNIGlobalRef::operator=(const JNIGlobalRef &o) {
    if( &o == this ) {
        return *this;
    }
    JNIEnv * env = *jni_env;
    std::unique_lock<std::mutex> lockThis(mtx, std::defer_lock);    // utilize std::lock(r, w), allowing mixed order w/o deadlock
    std::unique_lock<std::mutex> lockThat(o.mtx, std::defer_lock);  // otherwise RAII-style relinquish via destructor
    std::lock(lockThis, lockThat);

    if( nullptr != object ) { // always
        const jobjectRefType ref_type = env->GetObjectRefType(object);
        if( JNIInvalidRefType == ref_type ) {
            object = nullptr;
            throw jau::RuntimeException("JavaGlobalObj::assignment: Invalid non-null jobject", E_FILE_LINE);
        }
        env->DeleteGlobalRef(object);
        object = nullptr;
    }
    if( nullptr == o.object ) {
        throw jau::RuntimeException("Other JNIGlobalRef jobject is null", E_FILE_LINE);
    }
    object = env->NewGlobalRef(o.object);
    DBG_JNI_PRINT("JNIGlobalRef::copy_assign %p -> %p", o.object, object);
    return *this;
}

JNIGlobalRef& JNIGlobalRef::operator=(JNIGlobalRef &&o) noexcept {
    std::unique_lock<std::mutex> lockThis(mtx, std::defer_lock);    // utilize std::lock(r, w), allowing mixed order w/o deadlock
    std::unique_lock<std::mutex> lockThat(o.mtx, std::defer_lock);  // otherwise RAII-style relinquish via destructor
    std::lock(lockThis, lockThat);

    object = o.object;
    DBG_JNI_PRINT("JNIGlobalRef::move_assign %p (nulled) -> %p", o.object, object);
    o.object = nullptr;
    return *this;
}

JNIGlobalRef::~JNIGlobalRef() noexcept {
    try {
        JNIEnv * env = *jni_env; // exception if null
        std::unique_lock<std::mutex> lock(mtx);
        DBG_JNI_PRINT("JNIGlobalRef::dtor %p", object);
        if( nullptr != object ) {
            // due to move ctor and assignment, we accept nullptr object
            const jobjectRefType ref_type = env->GetObjectRefType(object);
            if( JNIInvalidRefType == ref_type ) {
                ERR_PRINT("Invalid non-null jobject"); // noexcept
            } else {
                env->DeleteGlobalRef(object);
            }
            object = nullptr;
        }
    } catch (jau::ExceptionBase &e0) {
        if( root_environment::is_terminating() ) {
            if( jau::environment::get().debug ) {
                fprintf(stderr, "JNIGlobalRef::dtor: Caught at exit %s\n", e0.whole_message().c_str());
            } else {
                // Be brief @ exit, as its expected at JVM shutdown
                fprintf(stderr, "JNIGlobalRef::dtor: Caught at exit %s\n", e0.brief_message().c_str());
            }
        } else {
            fprintf(stderr, "JNIGlobalRef::dtor: Caught %s\n", e0.whole_message().c_str());
        }
    } catch (std::exception &e) {
        if( root_environment::is_terminating() ) {
            if( jau::environment::get().debug ) {
                fprintf(stderr, "JNIGlobalRef::dtor: Caught at exit %s\n", e.what());
            } else {
                // Be brief @ exit, as its expected at JVM shutdown
                fprintf(stderr, "JNIGlobalRef::dtor: Caught at exit ...\n");
            }
        } else {
            fprintf(stderr, "JNIGlobalRef::dtor: Caught %s\n", e.what());
        }
    }
}

jobjectRefType JNIGlobalRef::getObjectRefType() const noexcept {
    try {
        JNIEnv * env = *jni_env;
        std::unique_lock<std::mutex> lock(mtx);
        return env->GetObjectRefType(object);
    } catch (const jau::ExceptionBase &e) {
        ERR_PRINT("%s", e.brief_message().c_str());
    } catch (...) {
        ERR_PRINT("Unknown exception");
    }
    return jobjectRefType::JNIInvalidRefType;
}

jobject JNIGlobalRef::operator*() noexcept {
    std::unique_lock<std::mutex> lock(mtx);
    return object;
}

jobject JNIGlobalRef::getObject() const noexcept {
    std::unique_lock<std::mutex> lock(mtx);
    return object;
}


bool JNIGlobalRef::operator==(const JNIGlobalRef& rhs) const noexcept {
    if( &rhs == this ) {
        DBG_JNI_PRINT("JNIGlobalRef::== true: %p == %p (ptr)", object, rhs.object);
        return true;
    }
    bool res = false;
    try {
        JNIEnv * env = *jni_env;
        std::unique_lock<std::mutex> lockThis(mtx, std::defer_lock);     // utilize std::lock(r, w), allowing mixed order w/o deadlock
        std::unique_lock<std::mutex> lockThat(rhs.mtx, std::defer_lock); // otherwise RAII-style relinquish via destructor
        std::lock(lockThis, lockThat);

        res = JNI_TRUE == env->IsSameObject(object, rhs.object);
        DBG_JNI_PRINT("JNIGlobalRef::== %d: %p == %p (IsSameObject)", res, object, rhs.object);
    } catch (const jau::ExceptionBase &e) {
        ERR_PRINT("%s", e.brief_message().c_str());
    } catch (...) {
        ERR_PRINT("Unknown exception");
    }
    return res;
}
