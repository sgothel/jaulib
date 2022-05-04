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

#ifndef JAU_JNIMEM__HPP_
#define JAU_JNIMEM__HPP_

#include <jni.h>
#include <stdexcept>
#include <mutex>

#include <jau/basic_types.hpp>

/** \addtogroup JavaVM
 *
 *  @{
 */

extern JavaVM* vm;


/* 
 * This class provides a lifetime-managed JNIEnv object, which attaches or
 * detaches the current thread from the JVM automatically
 */
class JNIEnvContainer {
private:    
    JNIEnv *env = nullptr;
    bool needsDetach = false;
    
public:
    /* Attaches this thread to the JVM if it is not already attached */
    JNIEnvContainer();
    /* Detaches this thread to the JVM if it is attached */
    ~JNIEnvContainer();

    /* Provides access to the local thread's JNIEnv object */
    JNIEnv *operator*();
    /* Provides access to the local thread's JNIEnv object's methods */
    JNIEnv *operator->();

    /* Attaches this thread to the JVM if it is not already attached */
    void attach();
    /* Detaches this thread to the JVM if it is attached */
    void detach();
};

/* Each thread has a local jni_env variable of JNIEnvContainer type */
extern thread_local JNIEnvContainer jni_env;

/*
 * This class provides a lifetime-managed GlobalRef variable,
 * which is automatically deleted when it goes out of scope.
 *
 * RAII-style acquire and relinquish via destructor
 */
class JNIGlobalRef {
private:
    mutable std::mutex mtx;
    jobject object;

public:
    static inline void check(jobject object, const char* file, int line) {
        if( nullptr == object ) {
            throw jau::RuntimeException("JNIGlobalRef::check: Null jobject", file, line);
        }
    }

    /* Creates a GlobalRef using a nullptr for API convenience, lazy assignment. */
    JNIGlobalRef() noexcept;

    /* Creates a GlobalRef from an object passed to it */
    JNIGlobalRef(jobject object);

    JNIGlobalRef(const JNIGlobalRef &o);
    JNIGlobalRef(JNIGlobalRef &&o) noexcept;

    JNIGlobalRef& operator=(const JNIGlobalRef &o);
    JNIGlobalRef& operator=(JNIGlobalRef &&o) noexcept;

    /* Deletes the stored GlobalRef */
    ~JNIGlobalRef() noexcept;

    /**
     * Should return JNIGlobalRefType if the `object` is valid or JNIInvalidRefType if the `object` is nullptr.
     */
    jobjectRefType getObjectRefType() const noexcept;
    bool isValidReference() const noexcept { return getObjectRefType() != JNIInvalidRefType; }

    /* Provides access to the stored GlobalRef as an jobject. */
    jobject operator*() noexcept;

    /* Provides access to the stored GlobalRef as an jobject. */
    jobject getObject() const noexcept;

    /* Provides access to the stored GlobalRef as a jclass. */
    jclass getClass() const noexcept { return (jclass)getObject(); }

    bool operator==(const JNIGlobalRef& rhs) const noexcept;

    bool operator!=(const JNIGlobalRef& rhs) const noexcept
    { return !( *this == rhs ); }
};

/*
 * This class provides a lifetime-managed 'PrimitiveArrayCritical' pinned heap,
 * which is automatically released when it goes out of scope.
 * <p>
 * RAII-style acquire and relinquish via destructor
 * </p>
 */
template <typename T, typename U>
class JNICriticalArray {
public:
    enum Mode : jint {
        /** Like default 0: If 'isCopy': Update the java array data with the copy and free the copy. */
        UPDATE_AND_RELEASE = 0,

        /** Like JNI_COMMIT: If 'isCopy': Update the java array data with the copy, but do not free the copy. */
        UPDATE_NO_RELEASE = JNI_COMMIT,

        /** Like default JNI_ABORT: If 'isCopy': Do not update the java array data with the copy, but free the copy. */
        NO_UPDATE_AND_RELEASE = JNI_ABORT,
    };

private:
    JNIEnv *env;
    Mode mode = UPDATE_AND_RELEASE;
    U jarray = nullptr;
    T* narray = nullptr;
    jboolean isCopy = false;

public:
    JNICriticalArray(JNIEnv *env_val) : env(env_val) {}

    JNICriticalArray(const JNICriticalArray &o) = delete;
    JNICriticalArray(JNICriticalArray &&o) = delete;
    JNICriticalArray& operator=(const JNICriticalArray &o) = delete;
    JNICriticalArray& operator=(JNICriticalArray &&o) = delete;

    /**
     * Release the acquired primitive array, RAII style.
     */
    ~JNICriticalArray() {
        release();
    }

    /**
     * Manual release of the acquired primitive array,
     * usually one likes to simply do this via the destructor, RAII style.
     */
    void release() {
        if( nullptr != narray ) {
            env->ReleasePrimitiveArrayCritical(jarray, narray, mode);
            this->jarray = nullptr;
            this->narray = nullptr;
            this->env = nullptr;
        }
    }

    /**
     * Acquired the primitive array.
     */
    T* get(U jarray_val, Mode mode_val=UPDATE_AND_RELEASE) {
        if( nullptr == jarray_val ) {
            return nullptr;
        }
        T* _narray = static_cast<T*>( env->GetPrimitiveArrayCritical(jarray_val, &isCopy) );
        if( nullptr != _narray ) {
            this->mode = mode_val;
            this->jarray = jarray_val;
            this->narray = _narray;
            return _narray;
        }
        return nullptr;
    }

    /**
     * Returns true if the primitive array had been acquired
     * and the JVM utilizes a copy of the underlying java array.
     */
    bool getIsCopy() const { return isCopy; }
};

/**@}*/

#endif /* JAU_JNIMEM__HPP_ */

