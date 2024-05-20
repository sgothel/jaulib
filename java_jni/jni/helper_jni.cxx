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

#include <jni.h>
#include <stdexcept>
#include <vector>

#include <jau/basic_types.hpp>
#include <jau/jni/helper_jni.hpp>


//
// C++ <-> java exceptions
//

bool jau::jni::java_exception_check(JNIEnv *env, const char* file, int line)
{
    jthrowable e = env->ExceptionOccurred();
    if( nullptr != e ) {
#if 1
        // ExceptionDescribe prints an exception and a backtrace of the stack to a system error-reporting channel, such as stderr.
        // The pending exception is cleared as a side-effect of calling this function. This is a convenience routine provided for debugging.
        env->ExceptionDescribe();
#endif
        env->ExceptionClear(); // just be sure, to have same side-effects

        jclass eClazz = search_class(env, e);
        jmethodID toString = search_method(env, eClazz, "toString", "()Ljava/lang/String;", false);
        jstring jmsg = (jstring) env->CallObjectMethod(e, toString);
        std::string msg = from_jstring_to_string(env, jmsg);
        fprintf(stderr, "Java exception occurred @ %s:%d and forward to Java: %s\n", file, line, msg.c_str()); fflush(stderr);

        env->Throw(e); // re-throw the java exception - java side!
        return true;
    }
    return false;
}

void jau::jni::java_exception_check_and_throw(JNIEnv *env, const char* file, int line)
{
    jthrowable e = env->ExceptionOccurred();
    if( nullptr != e ) {
        // ExceptionDescribe prints an exception and a backtrace of the stack to a system error-reporting channel, such as stderr.
        // The pending exception is cleared as a side-effect of calling this function. This is a convenience routine provided for debugging.
        env->ExceptionDescribe();
        env->ExceptionClear(); // just be sure, to have same side-effects

        jclass eClazz = search_class(env, e);
        jmethodID toString = search_method(env, eClazz, "toString", "()Ljava/lang/String;", false);
        jstring jmsg = (jstring) env->CallObjectMethod(e, toString);
        std::string msg = from_jstring_to_string(env, jmsg);
        fprintf(stderr, "Java exception occurred @ %s:%d and forward to Native: %s\n", file, line, msg.c_str()); fflush(stderr);

        throw jau::RuntimeException("Java exception occurred: "+msg, file, line);
    }
}

void jau::jni::print_native_caught_exception_fwd2java(const jau::ExceptionBase &e, const char* file, int line) {
    fprintf(stderr, "Native exception caught @ %s:%d and forward to Java: %s\n", file, line, e.what()); fflush(stderr);
}
void jau::jni::print_native_caught_exception_fwd2java(const std::exception &e, const char* file, int line) {
    fprintf(stderr, "Native exception caught @ %s:%d and forward to Java: %s\n", file, line, e.what()); fflush(stderr);
}
void jau::jni::print_native_caught_exception_fwd2java(const std::string &msg, const char* file, int line) {
    fprintf(stderr, "Native exception caught @ %s:%d and forward to Java: %s\n", file, line, msg.c_str()); fflush(stderr);
}
void jau::jni::print_native_caught_exception_fwd2java(const char * cmsg, const char* file, int line) {
    fprintf(stderr, "Native exception caught @ %s:%d and forward to Java: %s\n", file, line, cmsg); fflush(stderr);
}

void jau::jni::raise_java_exception(JNIEnv *env, const jau::ExceptionBase &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());
}
void jau::jni::raise_java_exception(JNIEnv *env, const jau::RuntimeExceptionBase &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/RuntimeException"), e.what());
}
void jau::jni::raise_java_exception(JNIEnv *env, const jau::InternalError &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(static_cast<const jau::RuntimeExceptionBase&>(e), file, line);
    env->ThrowNew(env->FindClass("java/lang/InternalError"), e.what());
}
void jau::jni::raise_java_exception(JNIEnv *env, const jau::NullPointerException &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(static_cast<const jau::RuntimeExceptionBase&>(e), file, line);
    env->ThrowNew(env->FindClass("java/lang/NullPointerException"), e.what());
}
void jau::jni::raise_java_exception(JNIEnv *env, const jau::IllegalArgumentError &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(static_cast<const jau::LogicErrorBase&>(e), file, line);
    env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), e.what());
}
void jau::jni::raise_java_exception(JNIEnv *env, const jau::IllegalStateError &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(static_cast<const jau::LogicErrorBase&>(e), file, line);
    env->ThrowNew(env->FindClass("java/lang/IllegalStateException"), e.what());
}
void jau::jni::raise_java_exception(JNIEnv *env, const jau::UnsupportedOperationException &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(static_cast<const jau::RuntimeExceptionBase&>(e), file, line);
    env->ThrowNew(env->FindClass("java/lang/UnsupportedOperationException"), e.what());
}
void jau::jni::raise_java_exception(JNIEnv *env, const jau::IndexOutOfBoundsError &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(static_cast<const jau::ExceptionBase&>(e), file, line);
    env->ThrowNew(env->FindClass("java/lang/IndexOutOfBoundsException"), e.what());
}
void jau::jni::raise_java_exception(JNIEnv *env, const std::bad_alloc &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/OutOfMemoryError"), e.what());
}
void jau::jni::raise_java_exception(JNIEnv *env, const jau::OutOfMemoryError &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(static_cast<const jau::ExceptionBase&>(e), file, line);
    env->ThrowNew(env->FindClass("java/lang/OutOfMemoryError"), e.what());
}
void jau::jni::raise_java_exception(JNIEnv *env, const std::exception &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/Error"), e.what());
}
void jau::jni::raise_java_exception(JNIEnv *env, const std::runtime_error &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/RuntimeException"), e.what());
}
void jau::jni::raise_java_exception(JNIEnv *env, const std::invalid_argument &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), e.what());
}

static std::string _unknown_exception_type_msg("Unknown exception type");

void jau::jni::rethrow_and_raise_java_exception_jauimpl(JNIEnv *env, const char* file, int line) {
    // std::exception_ptr e = std::current_exception();
    try {
        // std::rethrow_exception(e);
        throw; // re-throw current exception
    } catch (const jau::OutOfMemoryError &e) {
        jau::jni::raise_java_exception(env, e, file, line);
    } catch (const jau::InternalError &e) {
        jau::jni::raise_java_exception(env, e, file, line);
    } catch (const jau::IndexOutOfBoundsError &e) {
        jau::jni::raise_java_exception(env, e, file, line);
    } catch (const jau::IllegalArgumentError &e) {
        jau::jni::raise_java_exception(env, e, file, line);
    } catch (const jau::IllegalStateError &e) {
        jau::jni::raise_java_exception(env, e, file, line);
    } catch (const jau::UnsupportedOperationException &e) {
        jau::jni::raise_java_exception(env, e, file, line);
    } catch (const jau::NullPointerException &e) {
        jau::jni::raise_java_exception(env, e, file, line);
    } catch (const jau::RuntimeExceptionBase &e) {
        jau::jni::raise_java_exception(env, e, file, line);
    } catch (const jau::ExceptionBase &e) {
        jau::jni::raise_java_exception(env, e, file, line);
    } catch (const std::runtime_error &e) {
        jau::jni::raise_java_exception(env, e, file, line);
    } catch (const std::invalid_argument &e) {
        jau::jni::raise_java_exception(env, e, file, line);
    } catch (const std::bad_alloc &e) {
        jau::jni::raise_java_exception(env, e, file, line);
    } catch (const std::exception &e) {
        jau::jni::raise_java_exception(env, e, file, line);
    } catch (const std::string &msg) {
        jau::jni::print_native_caught_exception_fwd2java(msg, file, line);
        env->ThrowNew(env->FindClass("java/lang/Error"), msg.c_str());
    } catch (const char *msg) {
        jau::jni::print_native_caught_exception_fwd2java(msg, file, line);
        env->ThrowNew(env->FindClass("java/lang/Error"), msg);
    } catch (...) {
        jau::jni::print_native_caught_exception_fwd2java(_unknown_exception_type_msg, file, line);
        env->ThrowNew(env->FindClass("java/lang/Error"), _unknown_exception_type_msg.c_str());
    }
}

//
// Basic
//

jfieldID jau::jni::getField(JNIEnv *env, jobject obj, const char* field_name, const char* field_signature) {
    if( nullptr == obj ) {
        return nullptr;
    }
    jclass clazz = env->GetObjectClass(obj);
    java_exception_check_and_throw(env, E_FILE_LINE);
    // J == long
    jfieldID res = env->GetFieldID(clazz, field_name, field_signature);
    java_exception_check_and_throw(env, E_FILE_LINE);
    return res;
}

jobject jau::jni::getObjectFieldValue(JNIEnv *env, jobject obj, const char* field_name, const char* field_signature) {
    jfieldID f = jau::jni::getField(env, obj, field_name, field_signature);
    if (!f)
    {
        throw jau::InternalError(std::string("no field found: ")+field_signature+" "+field_name, E_FILE_LINE);
    }
    jobject v = env->GetObjectField(obj, f);
    java_exception_check_and_throw(env, E_FILE_LINE);
    if (!v)
    {
        throw jau::InternalError(std::string("no object at field: ")+field_signature+" "+field_name, E_FILE_LINE);
    }
    return v;
}

std::string jau::jni::getStringFieldValue(JNIEnv *env, jobject obj, const char* field_name) {
    return jau::jni::from_jstring_to_string(env, (jstring)jau::jni::getObjectFieldValue(env, obj, field_name, "Ljava/lang/String;"));
}

jlong jau::jni::getLongFieldValue(JNIEnv *env, jobject obj, const char* field_name) {
    jfieldID f = jau::jni::getField(env, obj, field_name, "J");
    if (!f)
    {
        throw jau::InternalError(std::string("no field found: J ")+field_name, E_FILE_LINE);
    }
    jlong v = env->GetLongField(obj, f);
    java_exception_check_and_throw(env, E_FILE_LINE);
    return v;
}

jint jau::jni::getIntFieldValue(JNIEnv *env, jobject obj, const char* field_name) {
    jfieldID f = jau::jni::getField(env, obj, field_name, "I");
    if (!f)
    {
        throw jau::InternalError(std::string("no field found: I ")+field_name, E_FILE_LINE);
    }
    jint v = env->GetIntField(obj, f);
    java_exception_check_and_throw(env, E_FILE_LINE);
    return v;
}

jclass jau::jni::search_class(JNIEnv *env, const char *clazz_name)
{
    jclass clazz = env->FindClass(clazz_name);
    java_exception_check_and_throw(env, E_FILE_LINE);
    if (!clazz)
    {
        throw jau::InternalError(std::string("no class found: ")+clazz_name, E_FILE_LINE);
    }
    return clazz;
}

jclass jau::jni::search_class(JNIEnv *env, jobject obj)
{
    jclass clazz = env->GetObjectClass(obj);
    java_exception_check_and_throw(env, E_FILE_LINE);
    if (!clazz)
    {
        throw jau::InternalError("no class found", E_FILE_LINE);
    }
    return clazz;
}

jclass jau::jni::search_class(JNIEnv *env, JavaUplink &object)
{
    return search_class(env, object.get_java_class().c_str());
}

jmethodID jau::jni::search_method(JNIEnv *env, jclass clazz, const char *method_name,
                             const char *prototype, bool is_static)
{
    jmethodID method;
    if (is_static)
    {
        method = env->GetStaticMethodID(clazz, method_name, prototype);
    }
    else
    {
        method = env->GetMethodID(clazz, method_name, prototype);
    }
    java_exception_check_and_throw(env, E_FILE_LINE);

    if (!method)
    {
        throw jau::InternalError(std::string("no method found: ")+method_name, E_FILE_LINE);
    }

    return method;
}

jfieldID jau::jni::search_field(JNIEnv *env, jclass clazz, const char *field_name,
                           const char *type, bool is_static)
{
    jfieldID field;
    if (is_static)
    {
        field = env->GetStaticFieldID(clazz, field_name, type);
    }
    else
    {
        field = env->GetFieldID(clazz, field_name, type);
    }
    java_exception_check_and_throw(env, E_FILE_LINE);

    if (!field)
    {
        throw jau::InternalError(std::string("no field found: ")+field_name, E_FILE_LINE);
    }

    return field;
}

bool jau::jni::from_jboolean_to_bool(jboolean val)
{
    if( JNI_TRUE == val ) {
        return true;
    } else if( JNI_FALSE == val ) {
        return false;
    } else {
        throw jau::InternalError("the jboolean value is not true/false", E_FILE_LINE);
    }
}

std::string jau::jni::from_jstring_to_string(JNIEnv *env, jstring jstr) {
    if (!jstr) {
        throw jau::IllegalArgumentError("String argument should not be null", E_FILE_LINE);
    }
    const char *str_chars = (char *)env->GetStringUTFChars(jstr, nullptr);
    if (!str_chars) {
            throw jau::OutOfMemoryError("GetStringUTFChars returned null", E_FILE_LINE);
    }
    std::string str_cpy(str_chars);
    env->ReleaseStringUTFChars(jstr, str_chars);
    return str_cpy;
}

jstring jau::jni::from_string_to_jstring(JNIEnv *env, const std::string & str) {
    return env->NewStringUTF(str.c_str());
}

jau::io::secure_string jau::jni::from_jbytebuffer_to_sstring(JNIEnv *env, jobject jbytebuffer) {
    if (!jbytebuffer) {
        throw jau::IllegalArgumentError("ByteBuffer argument should not be null", E_FILE_LINE);
    }
    const char* address = (const char*)env->GetDirectBufferAddress(jbytebuffer);
    size_t capacity = (size_t)env->GetDirectBufferCapacity(jbytebuffer);
    if( nullptr == address || 0 == capacity ) {
        return jau::io::secure_string(); // empty
    }
    jclass buffer_class = search_class(env, "java/nio/Buffer");
    jmethodID buffer_limit = search_method(env, buffer_class, "limit", "()I", false);
    jint jbytebuffer_limit = env->CallIntMethod(jbytebuffer, buffer_limit);
    java_exception_check_and_throw(env, E_FILE_LINE);
    size_t max_len = std::min<size_t>(capacity, jbytebuffer_limit);
    if( 0 == max_len ) {
        return jau::io::secure_string(); // empty
    }
    return jau::io::secure_string(address, ::strnlen(address, max_len));
}

jobject jau::jni::get_new_arraylist(JNIEnv *env, jsize size, jmethodID *add)
{
    jclass arraylist_class = search_class(env, "java/util/ArrayList");
    jmethodID arraylist_ctor = search_method(env, arraylist_class, "<init>", "(I)V", false);

    jobject result = env->NewObject(arraylist_class, arraylist_ctor, size);
    if (!result)
    {
        throw jau::InternalError("Cannot create instance of class ArrayList with size "+std::to_string(size), E_FILE_LINE);
    }

    *add = search_method(env, arraylist_class, "add", "(Ljava/lang/Object;)Z", false);

    env->DeleteLocalRef(arraylist_class);
    return result;
}

jobject jau::jni::convert_vector_bytes_to_jarraylist(JNIEnv *env, const std::vector<std::vector<uint8_t>>& array)
{
    nsize_t array_size = array.size();

    jmethodID arraylist_add;
    jobject result = get_new_arraylist(env, (jsize)array_size, &arraylist_add);

    if (0 == array_size) {
        return result;
    }

    jau::for_each(array.begin(), array.end(), [&](const std::vector<uint8_t>& elem){
        jbyteArray jelem = jau::jni::convert_bytes_to_jbytearray(env, elem);
        env->CallBooleanMethod(result, arraylist_add, jelem);
        jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);
        env->DeleteLocalRef(jelem);
    });
    return result;
}

jobject jau::jni::convert_vector_string_to_jarraylist(JNIEnv *env, const std::vector<std::string>& array)
{
    nsize_t array_size = array.size();

    jmethodID arraylist_add;
    jobject result = get_new_arraylist(env, (jsize)array_size, &arraylist_add);

    if (0 == array_size) {
        return result;
    }

    jau::for_each(array.begin(), array.end(), [&](const std::string& elem){
        jstring jelem = from_string_to_jstring(env, elem);
        env->CallBooleanMethod(result, arraylist_add, jelem);
        jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);
        env->DeleteLocalRef(jelem);
    });
    return result;
}

jobject jau::jni::convert_vector_stringview_to_jarraylist(JNIEnv *env, const std::vector<std::string_view>& array)
{
    nsize_t array_size = array.size();

    jmethodID arraylist_add;
    jobject result = get_new_arraylist(env, (jsize)array_size, &arraylist_add);

    if (0 == array_size) {
        return result;
    }

    jau::for_each(array.begin(), array.end(), [&](const std::string_view& elem_view){
        const std::string elem(elem_view);
        jstring jelem = from_string_to_jstring(env, elem);
        env->CallBooleanMethod(result, arraylist_add, jelem);
        jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);
        env->DeleteLocalRef(jelem);
    });
    return result;
}

std::vector<std::string> jau::jni::convert_jlist_string_to_vector(JNIEnv *env, jobject jlist)
{
    std::vector<std::string> result;

    jclass list_class = search_class(env, "java/util/List");
    jmethodID list_size = search_method(env, list_class, "size", "()I", false);
    nsize_t array_size = env->CallIntMethod(jlist, list_size);
    jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);

    if (0 == array_size) {
        return result;
    }
    jmethodID list_get = search_method(env, list_class, "get", "(I)Ljava/lang/Object;", false);

    for(nsize_t i=0; i<array_size; ++i) {
        jobject jstr = env->CallObjectMethod(jlist, list_get, i);
        jau::jni::java_exception_check_and_throw(env, E_FILE_LINE);
        result.push_back( jau::jni::from_jstring_to_string(env, (jstring)jstr) );
        env->DeleteLocalRef(jstr);
    }
    return result;
}

//
// C++ java_anon implementation
//

jau::jni::JavaGlobalObj::~JavaGlobalObj() noexcept { // NOLINT(bugprone-exception-escape): handled
    try {
        JNIEnv *env = *jni_env;
        if( nullptr == env ) {
            ABORT("JavaGlobalObj::dtor null JNIEnv");
        }
        {
            std::unique_lock<std::mutex> lock(javaObjectRef.mtx);
            jobject obj = javaObjectRef.object;
            if( nullptr == obj || nullptr == mNotifyDeleted ) {
                return;
            }
            env->CallVoidMethod(obj, mNotifyDeleted);
        }
        java_exception_check_and_throw(env, E_FILE_LINE); // caught below
    } catch (std::exception &e) {
        fprintf(stderr, "JavaGlobalObj::dtor: Caught %s\n", e.what());
    }
}

//
// C++ java_anon <-> java access, assuming field "long nativeInstance" and native method 'void checkValid()'
//

//
// C++ java_anon <-> java access, all generic
//

