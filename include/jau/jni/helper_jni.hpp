/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020, 2022 Gothel Software e.K.
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

#ifndef JAU_HELPER_JNI_HPP_
#define JAU_HELPER_JNI_HPP_

#include <limits>
#include <vector>
#include <memory>
#include <jni.h>

#include <jau/java_uplink.hpp>
#include <jau/basic_algos.hpp>
#include <jau/basic_types.hpp>
#include <jau/darray.hpp>
#include <jau/functional.hpp>
#include <jau/io/io_util.hpp>

#include <jau/jni/jni_mem.hpp>

namespace jau::jni {

    /** \addtogroup JavaVM
     *
     *  @{
     */

    //
    // C++ <-> java exceptions
    //

    /**
     * Return true if a java exception occurred, otherwise false.
     * <p>
     * In case of an exception, the information might be logged to stderr.
     * </p>
     * <p>
     * In case of an exception, user shall release resourced in their JNI code
     * and leave immediately.
     * </p>
     */
    bool java_exception_check(JNIEnv *env, const char* file, int line);

    /**
     * Throws a C++ exception if a java exception occurred, otherwise do nothing.
     * <p>
     * In case of an exception, the information might be logged to stderr.
     * </p>
     * <p>
     * In case of an exception and hence thrown C++ exception,
     * might want to catch all and handle it via {@link #rethrow_and_raise_java_exception(JNIEnv*)}.
     * </p>
     */
    void java_exception_check_and_throw(JNIEnv *env, const char* file, int line);

    void print_native_caught_exception_fwd2java(const jau::ExceptionBase &e, const char* file, int line);
    void print_native_caught_exception_fwd2java(const std::exception &e, const char* file, int line);
    void print_native_caught_exception_fwd2java(const std::string &msg, const char* file, int line);
    void print_native_caught_exception_fwd2java(const char * cmsg, const char* file, int line);

    void raise_java_exception(JNIEnv *env, const jau::ExceptionBase &e, const char* file, int line);
    void raise_java_exception(JNIEnv *env, const jau::RuntimeExceptionBase &e, const char* file, int line);
    void raise_java_exception(JNIEnv *env, const jau::InternalError &e, const char* file, int line);
    void raise_java_exception(JNIEnv *env, const jau::NullPointerException &e, const char* file, int line);
    void raise_java_exception(JNIEnv *env, const jau::IllegalArgumentError &e, const char* file, int line);
    void raise_java_exception(JNIEnv *env, const jau::IllegalStateError &e, const char* file, int line);
    void raise_java_exception(JNIEnv *env, const jau::UnsupportedOperationException &e, const char* file, int line);
    void raise_java_exception(JNIEnv *env, const jau::IndexOutOfBoundsError &e, const char* file, int line);
    void raise_java_exception(JNIEnv *env, const jau::OutOfMemoryError &e, const char* file, int line);
    void raise_java_exception(JNIEnv *env, const std::exception &e, const char* file, int line);
    void raise_java_exception(JNIEnv *env, const std::runtime_error &e, const char* file, int line);
    void raise_java_exception(JNIEnv *env, const std::invalid_argument &e, const char* file, int line);
    void raise_java_exception(JNIEnv *env, const std::bad_alloc &e, const char* file, int line);

    /**
     * Re-throw current exception and raise respective java exception
     * using any matching function above.
     */
    void rethrow_and_raise_java_exception_jauimpl(JNIEnv *env, const char* file, int line);

    /**
     * Re-throw current exception and raise respective java exception
     * using any matching function above.
     */
    #define rethrow_and_raise_java_exception_jau(E) jau::jni::rethrow_and_raise_java_exception_jauimpl((E), __FILE__, __LINE__)
    // inline void rethrow_and_raise_java_exception_jau(JNIEnv *env) { rethrow_and_raise_java_exception_jauimpl(env, __FILE__, __LINE__); }

    //
    // Basic
    //

    jfieldID getField(JNIEnv *env, jobject obj, const char* field_name, const char* field_signature);
    inline jfieldID getInstanceField(JNIEnv *env, jobject obj) {
        return getField(env, obj, "nativeInstance", "J");
    }
    jobject getObjectFieldValue(JNIEnv *env, jobject obj, const char* field_name, const char* field_signature);
    std::string getStringFieldValue(JNIEnv *env, jobject obj, const char* field_name);
    jlong getLongFieldValue(JNIEnv *env, jobject obj, const char* field_name);
    jint getIntFieldValue(JNIEnv *env, jobject obj, const char* field_name);

    jclass search_class(JNIEnv *env, const char *clazz_name);
    jclass search_class(JNIEnv *env, jobject obj);
    jclass search_class(JNIEnv *env, JavaUplink &object);

    jmethodID search_method(JNIEnv *env, jclass clazz, const char *method_name,
                            const char *prototype, bool is_static);
    jfieldID search_field(JNIEnv *env, jclass clazz, const char *field_name,
                            const char *type, bool is_static);

    bool from_jboolean_to_bool(const jboolean val);

    std::string from_jstring_to_string(JNIEnv *env, jstring str);
    jstring from_string_to_jstring(JNIEnv *env, const std::string & str);

    jau::io::secure_string from_jbytebuffer_to_sstring(JNIEnv *env, jobject jbytebuffer);

    jobject get_new_arraylist(JNIEnv *env, jsize size, jmethodID *add);

    jobject convert_vector_bytes_to_jarraylist(JNIEnv *env, const std::vector<std::vector<uint8_t>>& array);
    jobject convert_vector_string_to_jarraylist(JNIEnv *env, const std::vector<std::string>& array);
    jobject convert_vector_stringview_to_jarraylist(JNIEnv *env, const std::vector<std::string_view>& array);
    std::vector<std::string> convert_jlist_string_to_vector(JNIEnv *env, jobject jlist);

    template< class byte_container_type,
              std::enable_if_t<std::is_integral_v<typename byte_container_type::value_type> &&
                               std::is_convertible_v<typename byte_container_type::value_type, jbyte>,
                               bool> = true>
    jbyteArray convert_bytes_to_jbytearray(JNIEnv *env, const byte_container_type& data) {
        const size_t data_size = data.size();
        jbyteArray jdata = env->NewByteArray((jsize)data_size);
        env->SetByteArrayRegion(jdata, 0, (jsize)data_size, (const jbyte *)data.data());
        jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);
        return jdata;
    }

    //
    // C++ JavaAnon implementation
    //

    /**
     * Implementation for JavaAnon,
     * by simply wrapping a JNIGlobalRef instance.
     */
    class JavaGlobalObj : public JavaAnon {
        private:
            JNIGlobalRef javaObjectRef;
            jmethodID  mNotifyDeleted;

        public:
            static inline void check(const JavaAnonRef& shref, const char* file, int line) {
                if( nullptr == shref ) {
                    throw RuntimeException("JavaGlobalObj::check: Null shared-JavaAnonObj", file, line);
                }
                if( 0 == shref.use_count() ) { // safe-guard for concurrent dtor
                    throw RuntimeException("JavaGlobalObj::check: Empty shared-JavaAnonObj", file, line);
                }
                const jobject obj = static_cast<const JavaGlobalObj*>(shref.get())->getObject();
                if( nullptr == obj ) {
                    throw RuntimeException("JavaGlobalObj::check: Null object", file, line);
                }
            }
            static inline jobject checkAndGetObject(const JavaAnonRef& shref, const char* file, int line) {
                if( nullptr == shref ) {
                    throw RuntimeException("JavaGlobalObj::check: Null shared-JavaAnonObj", file, line);
                }
                if( 0 == shref.use_count() ) { // safe-guard for concurrent dtor
                    throw RuntimeException("JavaGlobalObj::check: Empty shared-JavaAnonObj", file, line);
                }
                const jobject obj = static_cast<const JavaGlobalObj*>(shref.get())->getObject();
                if( nullptr == obj ) {
                    throw RuntimeException("JavaGlobalObj::check: Null object", file, line);
                }
                return obj;
            }
            static bool isValid(const JavaAnonRef& shref) noexcept {
                if( nullptr == shref || 0 == shref.use_count() ) {
                    return false;
                }
                const jobject obj = static_cast<const JavaGlobalObj*>(shref.get())->getObject();
                if( nullptr == obj ) {
                    return false;
                }
                return true;
            }
            JavaGlobalObj(jobject obj, jmethodID mNotifyDeleted_) noexcept
            : javaObjectRef(obj), mNotifyDeleted(mNotifyDeleted_) { }

            JavaGlobalObj(const JNIGlobalRef& obj, jmethodID mNotifyDeleted_) noexcept
            : javaObjectRef(obj), mNotifyDeleted(mNotifyDeleted_) { }

            JavaGlobalObj(JNIGlobalRef && obj, jmethodID mNotifyDeleted_) noexcept
            : javaObjectRef(std::move(obj)), mNotifyDeleted(mNotifyDeleted_) { }

            JavaGlobalObj(const JavaGlobalObj &o) noexcept = default;
            JavaGlobalObj(JavaGlobalObj &&o) noexcept = default;
            JavaGlobalObj& operator=(const JavaGlobalObj &o) noexcept = default;
            JavaGlobalObj& operator=(JavaGlobalObj &&o) noexcept = default;

            ~JavaGlobalObj() noexcept override;

            std::string toString() const noexcept override {
                return "JavaGlobalObj["+jau::toHexString(javaObjectRef.getObject())+"]";
            }

            const JNIGlobalRef & getJavaObject() const noexcept { return javaObjectRef; }
            JNIGlobalRef getJavaObject() noexcept { return javaObjectRef; }

            /* Provides access to the stored GlobalRef as an jobject. */
            jobject getObject() const noexcept { return javaObjectRef.getObject(); }
            /* Provides access to the stored GlobalRef as a jclass. */
            jclass getClass() const noexcept { return javaObjectRef.getClass(); }

            /* Provides access to the stored GlobalRef as an getJavaObject. */
            static JNIGlobalRef GetJavaObject(const JavaAnonRef & shref) noexcept {
                return static_cast<JavaGlobalObj*>(shref.get())->getJavaObject();
            }
            /* Provides access to the stored GlobalRef as an jobject. */
            static jobject GetObject(const JavaAnonRef & shref) noexcept {
                return static_cast<JavaGlobalObj*>(shref.get())->getObject();
            }

            /* Provides access to the stored GlobalRef as a jclass. */
            static jclass GetClass(const JavaAnonRef & shref) noexcept {
                return static_cast<JavaGlobalObj*>(shref.get())->getClass();
            }
    };
    typedef std::shared_ptr<JavaGlobalObj> JavaGlobalObjRef;

    //
    // C++ JavaAnon <-> java access, all generic
    //
    // We prefer using `std::shared_ptr<T>` instead of a `naked pointer`,
    // this way we automatically preserve the native instance lifecycle while within a JNI method.
    //

    template <typename T>
    T *getObjectRef(JNIEnv *env, jobject obj, const char* field_name)
    {
        jlong jobj = env->GetLongField(obj, getField(env, obj, field_name, "J"));
        java_exception_check_and_throw(env, E_FILE_LINE);
        return reinterpret_cast<T *>(jobj);
    }

    template <typename T>
    void setObjectRef(JNIEnv *env, jobject obj, T *t, const char* field_name)
    {
        jlong jobj = reinterpret_cast<jlong>(t);
        env->SetLongField(obj, getField(env, obj, field_name, "J"), jobj);
        java_exception_check_and_throw(env, E_FILE_LINE);
    }

    /**
     * Returns the cast `shared_ptr<T>` pointer from the java object's `long nativeInstance` field.
     *
     * If `throw_on_nullptr` is true, throws an exception if the shared_ptr<T> pointer is nullptr.
     *
     * @tparam T
     * @param instance
     * @param throw_on_nullptr
     * @return
     */
    template <typename T>
    std::shared_ptr<T> * castInstance(jlong instance, const bool throw_on_nullptr=true)
    {
        std::shared_ptr<T> * ref_ptr = reinterpret_cast<std::shared_ptr<T> *>(instance); // NOLINT(performance-no-int-to-ptr): Required and intended
        if( throw_on_nullptr ) {
            if (nullptr == ref_ptr) {
                throw jau::RuntimeException("null reference store", E_FILE_LINE);
            }
        }
        return ref_ptr;
    }

    /**
     * Returns the cast `shared_ptr<T>` pointer from the java object's `long nativeInstance` field.
     *
     * If `throw_on_nullptr` is true, throws an exception if either the shared_ptr<T> pointer or
     * its managed object reference is nullptr.
     *
     * @tparam T
     * @param env
     * @param obj
     * @param throw_on_nullptr if true, throws exception if instance reference is nullptr (default). Otherwise not.
     */
    template <typename T>
    std::shared_ptr<T>* getInstance(JNIEnv *env, jobject obj, const bool throw_on_nullptr=true) {
        const jlong nativeInstance = env->GetLongField(obj, getInstanceField(env, obj));
        java_exception_check_and_throw(env, E_FILE_LINE);
        std::shared_ptr<T>* ref_ptr = reinterpret_cast<std::shared_ptr<T> *>(nativeInstance); // NOLINT(performance-no-int-to-ptr): Required and intended
        if( throw_on_nullptr ) {
            if (nullptr == ref_ptr) {
                throw jau::RuntimeException("null reference store", E_FILE_LINE);
            }
            if (nullptr == *ref_ptr) {
                throw jau::RuntimeException("null reference", E_FILE_LINE);
            }
        }
        return ref_ptr;
    }

    /**
     * Deletes the `std::shared_ptr<T>` storage of the java object if exists first
     * and writes the given `std::shared_ptr<T>` storage pointer into its `long nativeInstance` field.
     *
     * @tparam T
     * @param env
     * @param obj
     * @param t
     */
    template <typename T>
    void setInstance(JNIEnv *env, jobject obj, const std::shared_ptr<T>& t)
    {
         if (t == nullptr) {
             throw jau::RuntimeException("Trying to create null object", E_FILE_LINE);
         }
         const jlong instance = (jlong) (intptr_t) &t;

         jfieldID instance_field = getInstanceField(env, obj);
         java_exception_check_and_throw(env, E_FILE_LINE);
         {
             const jlong nativeInstance = env->GetLongField(obj, instance_field);
             java_exception_check_and_throw(env, E_FILE_LINE);
             std::shared_ptr<T>* other = reinterpret_cast<std::shared_ptr<T> *>(nativeInstance); // NOLINT(performance-no-int-to-ptr): Required and intended
             if( nullptr != other ) {
                 delete other;
             }
         }
         env->SetLongField(obj, instance_field, instance);
         java_exception_check_and_throw(env, E_FILE_LINE);
    }


    /**
     * Deletes the `std::shared_ptr<T>` storage of the java object if exists
     * and write `nullptr` into its `long nativeInstance` field.
     *
     * @tparam T
     * @param env
     * @param obj
     */
    template <typename T>
    void clearInstance(JNIEnv *env, jobject obj) {
        jfieldID instance_field = getInstanceField(env, obj);
        java_exception_check_and_throw(env, E_FILE_LINE);
        {
            const jlong nativeInstance = env->GetLongField(obj, instance_field);
            java_exception_check_and_throw(env, E_FILE_LINE);
            std::shared_ptr<T>* other = reinterpret_cast<std::shared_ptr<T> *>(nativeInstance); // NOLINT(performance-no-int-to-ptr): Required and intended
            if( nullptr != other ) {
                delete other;
            }
        }
        env->SetLongField(obj, instance_field, 0);
        java_exception_check_and_throw(env, E_FILE_LINE);
    }

    /**
     * A `std::shared_ptr<T>` storage instance to be copied from and released into a java object's `long nativeInstance` field.
     *
     * An instance holds a shared_ptr<T> storage pointer for a managed object T.
     *
     * Using a `shared_ptr<T>` copy increments its reference counter and prohibits its destruction while in use.
     *
     * We prefer using `std::shared_ptr<T>` instead of a `naked pointer`,
     * this way we automatically preserve the native instance lifecycle while within a JNI method.
     *
     * @tparam T the managed object type
     */
    template <typename T>
    class shared_ptr_ref {
        private:
            std::shared_ptr<T>* ref_ptr;

            void safe_delete() {
                std::shared_ptr<T>* ref_ptr_ = ref_ptr;
                ref_ptr = nullptr;
                delete ref_ptr_;
            }

            static jlong get_instance(JNIEnv *env, jobject obj) {
                if( nullptr != obj ) {
                    const jlong res = env->GetLongField(obj, getInstanceField(env, obj));
                    java_exception_check_and_throw(env, E_FILE_LINE);
                    return res;
                } else {
                    return 0;
                }
            }
            static jlong get_instance(JNIEnv *env, jobject obj, jfieldID instance_field) {
                if( nullptr != obj ) {
                    const jlong res = env->GetLongField(obj, instance_field);
                    java_exception_check_and_throw(env, E_FILE_LINE);
                    return res;
                } else {
                    return 0;
                }
            }

        public:
            /** Default constructor, nullptr */
            shared_ptr_ref() noexcept
            : ref_ptr( new std::shared_ptr<T>() )
            { }

            /** Copy constructor */
            shared_ptr_ref(const shared_ptr_ref& o)
            : ref_ptr( new std::shared_ptr<T>( o.shared_ptr() ) )
            { }

            /** Move constructor. */
            shared_ptr_ref(shared_ptr_ref&& o) noexcept
            : ref_ptr( o.ref_ptr )
            {
                o.ref_ptr = nullptr;
            }

            /** Assignment operator. */
            shared_ptr_ref& operator=(const shared_ptr_ref& o) {
                if( this != &o ) {
                    if( nullptr != ref_ptr ) {
                        *ref_ptr = o.shared_ptr();
                    } else {
                        ref_ptr = new std::shared_ptr<T>( o.shared_ptr() );
                    }
                }
                return *this;
            }

            /** Move assignment operator. */
            shared_ptr_ref& operator=(shared_ptr_ref&& o)  noexcept {
                if( nullptr != ref_ptr ) {
                    safe_delete();
                }
                ref_ptr = o.ref_ptr;
                o.ref_ptr = nullptr;
                return *this;
            }

            ~shared_ptr_ref() {
                if( nullptr != ref_ptr ) {
                    safe_delete();
                }
            }

            /** Constructs a new instance, taking ownership of the given T pointer. */
            shared_ptr_ref(T * ptr)
            : ref_ptr( new std::shared_ptr<T>( ptr ) )
            { }

            /** Constructs a new instance, copying the given std::shared_ptr<T>. */
            shared_ptr_ref(const std::shared_ptr<T>& ref)
            : ref_ptr( new std::shared_ptr<T>( ref ) )
            { }

            /** Constructs a new instance, moving the given std::shared_ptr<T>. */
            shared_ptr_ref(std::shared_ptr<T>&& ref) noexcept
            : ref_ptr( new std::shared_ptr<T>( std::move(ref) ) )
            { }

            /** Assignment operator. */
            shared_ptr_ref& operator=(const std::shared_ptr<T>& o) {
                if( nullptr != ref_ptr ) {
                    *ref_ptr = o;
                } else {
                    ref_ptr = new std::shared_ptr<T>( o );
                }
                return *this;
            }

            /**
             * Throws an exception if this instances shared_ptr<T> storage is nullptr.
             *
             * The managed object reference may be nullptr.
             */
            void null_check1() const {
                if (nullptr == ref_ptr) {
                    throw jau::RuntimeException("null reference store", E_FILE_LINE);
                }
            }

            /**
             * Throws an exception if either this instances shared_ptr<T> storage or
             * the managed object reference is nullptr.
             */
            void null_check2() const {
                if (nullptr == ref_ptr) {
                    throw jau::RuntimeException("null reference store", E_FILE_LINE);
                }
                if (nullptr == *ref_ptr) {
                    throw jau::RuntimeException("null reference", E_FILE_LINE);
                }
            }

            /**
             * Constructs a new instance, copying the instance from the given java `long nativeInstance` value,
             * representing a java object's shared_ptr<T> storage
             *
             * Using a `shared_ptr<T>` copy increments its reference counter and prohibits its destruction while in use.
             *
             * If `throw_on_nullptr` is true, throws an exception if either this instances shared_ptr<T> storage or
             * the managed object reference is nullptr.
             *
             * @param nativeInstance the jlong representation of another shared_ptr<T> storage
             * @param throw_on_nullptr if true, throws an exception if either this instances shared_ptr<T> storage or the managed object reference is nullptr.
             */
            shared_ptr_ref(jlong nativeInstance, const bool throw_on_nullptr=true)
            : ref_ptr( new std::shared_ptr<T>() )
            {
                std::shared_ptr<T> * other = reinterpret_cast<std::shared_ptr<T> *>(nativeInstance); // NOLINT(performance-no-int-to-ptr): Required and intended
                if( nullptr != other  && nullptr != *other ) {
                    *ref_ptr = *other;
                }
                if( throw_on_nullptr ) {
                    null_check2(); // exception if nullptr, even if other shared_ptr instance becomes nullptr @ copy-ctor
                }
            }

            /**
             * Constructs a new instance, copying the instance from the java object's `long nativeInstance` field.
             * representing its shared_ptr<T> storage
             *
             * Using a `shared_ptr<T>` copy increments its reference counter and prohibits its destruction while in use.
             *
             * If `throw_on_nullptr` is true, throws an exception if either this instances shared_ptr<T> storage or
             * the managed object reference is nullptr.
             *
             * @param env denoting the JVM
             * @param obj denoting the java object holding the `long nativeInstance` field, representing its shared_ptr<T> storage. Maybe `nullptr`, see `throw_on_nullptr`.
             * @param throw_on_nullptr if true, throws an exception if either this instances shared_ptr<T> storage or the managed object reference is nullptr.
             */
            shared_ptr_ref(JNIEnv *env, jobject obj, const bool throw_on_nullptr=true)
            : shared_ptr_ref( get_instance(env, obj), throw_on_nullptr )
            { }

            /**
             * Release ownership and returns the shared_ptr<T> storage.
             *
             * This instance shall not be used anymore.
             */
            std::shared_ptr<T>* release() noexcept {
                const std::shared_ptr<T>* res = ref_ptr;
                ref_ptr = nullptr;
                return res;
            }

            /**
             * Release ownership and return the jlong representation of the shared_ptr<T> storage.
             *
             * This instance shall not be used anymore.
             */
            jlong release_to_jlong() noexcept {
                const jlong res = (jlong) (intptr_t)ref_ptr;
                ref_ptr = nullptr;
                return res;
            }

            /**
             * Deletes the `std::shared_ptr<T>` storage of the target java object if exists first
             * and writes this instance's `std::shared_ptr<T>` storage pointer into its `long nativeInstance` field,
             * then releases ownership, see release_to_jlong().
             *
             * This instance shall not be used anymore.
             *
             * Throws an exception if either this instances shared_ptr<T> storage or
             * the managed object reference is nullptr.
             *
             * @param env
             * @param obj the target java object
             */
            void release_into_object(JNIEnv *env, jobject obj) {
                null_check2();
                if( nullptr == obj ) {
                    throw jau::RuntimeException("null target object", E_FILE_LINE);
                }
                jfieldID instance_field = getInstanceField(env, obj);
                java_exception_check_and_throw(env, E_FILE_LINE);
                {
                    std::shared_ptr<T> * other = reinterpret_cast<std::shared_ptr<T> *>( get_instance(env, obj, instance_field) ); // NOLINT(performance-no-int-to-ptr): Required and intended
                    if( nullptr != other ) {
                        delete other;
                    }
                }
                env->SetLongField(obj, instance_field, release_to_jlong());
                java_exception_check_and_throw(env, E_FILE_LINE);
            }

            /**
             * Returns true if either this instances shared_ptr<T> storage or
             * the managed object reference is nullptr.
             */
            bool is_null() const noexcept {
                return nullptr == ref_ptr || nullptr == *ref_ptr;
            }

            /**
             * Provides access to the shared_ptr<T> pointer, l-value of storage.
             */
            std::shared_ptr<T>* pointer() noexcept {
                return ref_ptr;
            }

            /**
             * Provides access to const reference of shared_ptr<T>, r-value.
             *
             * Throws an exception if this instances shared_ptr<T> storage is nullptr.
             */
            const std::shared_ptr<T>& shared_ptr() const {
                null_check1();
                return *ref_ptr;
            }

            /**
             * Provides access to reference of stored T.
             *
             * Throws an exception if either this instances shared_ptr<T> storage or
             * the managed object reference is nullptr.
             */
            T& operator*() {
                null_check2();
                return *(ref_ptr->get());
            }

            /**
             * Provides access to pointer of stored T.
             *
             * Throws an exception if either this instances shared_ptr<T> storage or
             * the managed object reference is nullptr.
             */
            T* operator->() {
                null_check2();
                return ref_ptr->get();
            }

            std::string toString() const noexcept {
                return "shared_ptr_ref[ ptr "+jau::toHexString(ref_ptr)+
                                     ", obj "+ ( nullptr != ref_ptr ? jau::toHexString(ref_ptr->get()) : "null" ) + "]";
            }
    };

    //
    // C++ <-> java type mapping
    //

    template <typename T>
    jobject convert_instance_to_jobject(JNIEnv *env, const std::shared_ptr<T>& elem,
            const char *ctor_prototype, jau::function<jobject(JNIEnv*, jclass, jmethodID, const std::shared_ptr<T>&)> ctor)
    {
        jclass clazz = search_class(env, T::java_class().c_str());
        jmethodID clazz_ctor = search_method(env, clazz, "<init>", ctor_prototype, false);

        jobject object = ctor(env, clazz, clazz_ctor, elem);
        if (!object)
        {
            throw jau::RuntimeException("Cannot create instance of class", E_FILE_LINE);
        }
        java_exception_check_and_throw(env, E_FILE_LINE);

        return object;
    }

    template <typename T>
    jobject convert_instance_to_jobject(JNIEnv *env, jclass clazz,
            const char *ctor_prototype, jau::function<jobject(JNIEnv*, jclass, jmethodID, const std::shared_ptr<T>&)> ctor,
            const std::shared_ptr<T>& elem)
    {
        jmethodID clazz_ctor = search_method(env, clazz, "<init>", ctor_prototype, false);

        jobject object = ctor(env, clazz, clazz_ctor, elem);
        if (!object)
        {
            throw jau::RuntimeException("Cannot create instance of class", E_FILE_LINE);
        }
        java_exception_check_and_throw(env, E_FILE_LINE);

        return object;
    }

    template <typename T>
    jobject convert_vector_sharedptr_to_jarraylist(JNIEnv *env, T& array)
    {
        nsize_t array_size = array.size();

        jmethodID arraylist_add;
        jobject result = get_new_arraylist(env, (jsize)array_size, &arraylist_add);

        if (0 == array_size) {
            return result;
        }

        jau::for_each(array.begin(), array.end(), [&](typename T::value_type & elem){
            JavaAnonRef objref = elem->getJavaObject();
            if ( nullptr == objref ) {
                throw InternalError("JavaUplink element of array has no valid java-object: "+elem->toString(), E_FILE_LINE);
            }
            env->CallBooleanMethod(result, arraylist_add, JavaGlobalObj::GetObject(objref));
        });
        return result;
    }

    template <typename T, typename U>
    jobject convert_vector_sharedptr_to_jarraylist(JNIEnv *env, T& array,
            const char *ctor_prototype, jau::function<jobject(JNIEnv*, jclass, jmethodID, const std::shared_ptr<U>&)> ctor)
    {
        const size_t array_size = array.size();
        if( array_size > std::numeric_limits<jsize>::max() ) {
            throw jau::RuntimeException("Native array size "+std::to_string(array_size)+
                " exceeds max jsize "+std::to_string(std::numeric_limits<jsize>::max()), E_FILE_LINE);
        }
        const jsize jarray_size = array_size;

        jmethodID arraylist_add;
        jobject result = get_new_arraylist(env, jarray_size, &arraylist_add);

        if (jarray_size == 0)
        {
            return result;
        }

        jclass clazz = search_class(env, U::java_class().c_str());
        jmethodID clazz_ctor = search_method(env, clazz, "<init>", ctor_prototype, false);

        for (jsize i = 0; i < jarray_size; ++i)
        {
            jobject object = ctor(env, clazz, clazz_ctor, array[i] /* const std::shared_ptr<U>& */);
            if (!object)
            {
                throw jau::RuntimeException("Cannot create instance of class", E_FILE_LINE);
            }
            env->CallBooleanMethod(result, arraylist_add, object);
            java_exception_check_and_throw(env, E_FILE_LINE);
        }
        return result;
    }

    template <typename T, typename U>
    jobject convert_vector_sharedptr_to_jarraylist(JNIEnv *env, T& array, jau::function<jobject(JNIEnv*, const std::shared_ptr<U>&)> ctor)
    {
        const size_t array_size = array.size();
        if( array_size > std::numeric_limits<jsize>::max() ) {
            throw jau::RuntimeException("Native array size "+std::to_string(array_size)+
                " exceeds max jsize "+std::to_string(std::numeric_limits<jsize>::max()), E_FILE_LINE);
        }
        const jsize jarray_size = array_size;

        jmethodID arraylist_add;
        jobject result = get_new_arraylist(env, jarray_size, &arraylist_add);

        if (jarray_size == 0)
        {
            return result;
        }

        for (jsize i = 0; i < jarray_size; ++i)
        {
            jobject object = ctor(env, array[i] /* const std::shared_ptr<U>& */);
            if (!object)
            {
                throw jau::RuntimeException("Cannot create instance of class", E_FILE_LINE);
            }
            env->CallBooleanMethod(result, arraylist_add, object);
            java_exception_check_and_throw(env, E_FILE_LINE);
        }
        return result;
    }

    template <typename T, typename U>
    jobject convert_vector_to_jarraylist(JNIEnv *env, T& array, jau::function<jobject(JNIEnv*, const U&)> ctor)
    {
        const size_t array_size = array.size();
        if( array_size > std::numeric_limits<jsize>::max() ) {
            throw jau::RuntimeException("Native array size "+std::to_string(array_size)+
                " exceeds max jsize "+std::to_string(std::numeric_limits<jsize>::max()), E_FILE_LINE);
        }
        const jsize jarray_size = array_size;

        jmethodID arraylist_add;
        jobject result = get_new_arraylist(env, jarray_size, &arraylist_add);

        if (jarray_size == 0)
        {
            return result;
        }

        for (jsize i = 0; i < jarray_size; ++i)
        {
            jobject object = ctor(env, array[i] /* const U& */);
            if (!object)
            {
                throw jau::RuntimeException("Cannot create instance of class", E_FILE_LINE);
            }
            env->CallBooleanMethod(result, arraylist_add, object);
            java_exception_check_and_throw(env, E_FILE_LINE);
        }
        return result;
    }

    /**@}*/


} // namespace jau::jni

#endif /* JAU_HELPER_JNI_HPP_ */
