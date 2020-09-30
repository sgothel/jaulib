/*
 * Author: Andrei Vasiliu <andrei.vasiliu@intel.com>
 * Copyright (c) 2016 Intel Corporation.
 *
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
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
#include <memory>
#include <stdexcept>
#include <vector>

#include "helper_base.hpp"

#define JAVA_MAIN_PACKAGE "org/tinyb"

jfieldID getField(JNIEnv *env, jobject obj, const char* field_name, const char* field_signature) {
    jclass clazz = env->GetObjectClass(obj);
    java_exception_check_and_throw(env, E_FILE_LINE);
    // J == long
    jfieldID res = env->GetFieldID(clazz, field_name, field_signature);
    java_exception_check_and_throw(env, E_FILE_LINE);
    return res;
}
jclass search_class(JNIEnv *env, const char *clazz_name)
{
    jclass clazz = env->FindClass(clazz_name);
    java_exception_check_and_throw(env, E_FILE_LINE);
    if (!clazz)
    {
        throw direct_bt::InternalError(std::string("no class found: ")+clazz_name, E_FILE_LINE);
    }
    return clazz;
}

jclass search_class(JNIEnv *env, jobject obj)
{
    jclass clazz = env->GetObjectClass(obj);
    java_exception_check_and_throw(env, E_FILE_LINE);
    if (!clazz)
    {
        throw direct_bt::InternalError("no class found", E_FILE_LINE);
    }
    return clazz;
}

jmethodID search_method(JNIEnv *env, jclass clazz, const char *method_name,
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
        throw direct_bt::InternalError(std::string("no method found: ")+method_name, E_FILE_LINE);
    }

    return method;
}

jfieldID search_field(JNIEnv *env, jclass clazz, const char *field_name,
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
        direct_bt::InternalError(std::string("no field found: ")+field_name, E_FILE_LINE);
    }

    return field;
}

bool from_jboolean_to_bool(jboolean val)
{
    bool result;

    if (val == JNI_TRUE)
    {
        result = true;
    }
    else
    {
        if (val == JNI_FALSE)
        {
            result = false;
        }
        else
        {
            throw direct_bt::InternalError("the jboolean value is not true/false", E_FILE_LINE);
        }
    }

    return result;
}

std::string from_jstring_to_string(JNIEnv *env, jstring str)
{
    jboolean is_copy = JNI_TRUE;
    if (!str) {
        throw std::invalid_argument("String should not be null");
    }
    const char *str_chars = (char *)env->GetStringUTFChars(str, &is_copy);
    if (!str_chars) {
        throw std::bad_alloc();
    }
    const std::string string_to_write = std::string(str_chars);

    env->ReleaseStringUTFChars(str, str_chars);

    return string_to_write;
}

jstring from_string_to_jstring(JNIEnv *env, const std::string & str)
{
    return env->NewStringUTF(str.c_str());
}

jobject get_bluetooth_type(JNIEnv *env, const char *field_name)
{
    jclass b_type_enum = search_class(env, JAVA_MAIN_PACKAGE "/BluetoothType");

    jfieldID b_type_field = search_field(env, b_type_enum, field_name, "L" JAVA_MAIN_PACKAGE "/BluetoothType;", true);

    jobject result = env->GetStaticObjectField(b_type_enum, b_type_field);
    env->DeleteLocalRef(b_type_enum);
    return result;
}

jobject get_new_arraylist(JNIEnv *env, unsigned int size, jmethodID *add)
{
    jclass arraylist_class = search_class(env, "java/util/ArrayList");
    jmethodID arraylist_ctor = search_method(env, arraylist_class, "<init>", "(I)V", false);

    jobject result = env->NewObject(arraylist_class, arraylist_ctor, size);
    if (!result)
    {
        throw direct_bt::InternalError("Cannot create instance of class ArrayList", E_FILE_LINE);
    }

    *add = search_method(env, arraylist_class, "add", "(Ljava/lang/Object;)Z", false);

    env->DeleteLocalRef(arraylist_class);
    return result;
}

static void print_native_caught_exception_fwd2java(const std::exception &e, const char* file, int line) {
    fprintf(stderr, "Native exception caught @ %s:%d and forward to Java: %s\n", file, line, e.what()); fflush(stderr);
}
static void print_native_caught_exception_fwd2java(const std::string &msg, const char* file, int line) {
    fprintf(stderr, "Native exception caught @ %s:%d and forward to Java: %s\n", file, line, msg.c_str()); fflush(stderr);
}
static void print_native_caught_exception_fwd2java(const char * cmsg, const char* file, int line) {
    fprintf(stderr, "Native exception caught @ %s:%d and forward to Java: %s\n", file, line, cmsg); fflush(stderr);
}

void raise_java_exception(JNIEnv *env, const std::exception &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/Error"), e.what());
}
void raise_java_exception(JNIEnv *env, const std::runtime_error &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/RuntimeException"), e.what());
}
void raise_java_exception(JNIEnv *env, const direct_bt::RuntimeException &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/RuntimeException"), e.what());
}
void raise_java_exception(JNIEnv *env, const direct_bt::InternalError &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/InternalError"), e.what());
}
void raise_java_exception(JNIEnv *env, const direct_bt::NullPointerException &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/NullPointerException"), e.what());
}
void raise_java_exception(JNIEnv *env, const direct_bt::IllegalArgumentException &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), e.what());
}
void raise_java_exception(JNIEnv *env, const std::invalid_argument &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), e.what());
}
void raise_java_exception(JNIEnv *env, const direct_bt::IllegalStateException &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/IllegalStateException"), e.what());
}
void raise_java_exception(JNIEnv *env, const direct_bt::UnsupportedOperationException &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/UnsupportedOperationException"), e.what());
}
void raise_java_exception(JNIEnv *env, const direct_bt::IndexOutOfBoundsException &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/IndexOutOfBoundsException"), e.what());
}
void raise_java_exception(JNIEnv *env, const std::bad_alloc &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/OutOfMemoryError"), e.what());
}
void raise_java_exception(JNIEnv *env, const direct_bt::OutOfMemoryError &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("java/lang/OutOfMemoryError"), e.what());
}
void raise_java_exception(JNIEnv *env, const direct_bt::BluetoothException &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("org/tinyb/BluetoothException"), e.what());
}
void raise_java_exception(JNIEnv *env, const tinyb::BluetoothException &e, const char* file, int line) {
    print_native_caught_exception_fwd2java(e, file, line);
    env->ThrowNew(env->FindClass("org/tinyb/BluetoothException"), e.what());
}

static std::string _unknown_exception_type_msg("Unknown exception type");

void rethrow_and_raise_java_exception_impl(JNIEnv *env, const char* file, int line) {
    // std::exception_ptr e = std::current_exception();
    try {
        // std::rethrow_exception(e);
        throw; // re-throw current exception
    } catch (const std::bad_alloc &e) {
        raise_java_exception(env, e, file, line);
    } catch (const direct_bt::OutOfMemoryError &e) {
        raise_java_exception(env, e, file, line);
    } catch (const direct_bt::InternalError &e) {
        raise_java_exception(env, e, file, line);
    } catch (const direct_bt::NullPointerException &e) {
        raise_java_exception(env, e, file, line);
    } catch (const direct_bt::IllegalArgumentException &e) {
        raise_java_exception(env, e, file, line);
    } catch (const direct_bt::IllegalStateException &e) {
        raise_java_exception(env, e, file, line);
    } catch (const direct_bt::UnsupportedOperationException &e) {
        raise_java_exception(env, e, file, line);
    } catch (const direct_bt::IndexOutOfBoundsException &e) {
        raise_java_exception(env, e, file, line);
    } catch (const direct_bt::BluetoothException &e) {
        raise_java_exception(env, e, file, line);
    } catch (const tinyb::BluetoothException &e) {
        raise_java_exception(env, e, file, line);
    } catch (const direct_bt::RuntimeException &e) {
        raise_java_exception(env, e, file, line);
    } catch (const std::runtime_error &e) {
        raise_java_exception(env, e, file, line);
    } catch (const std::invalid_argument &e) {
        raise_java_exception(env, e, file, line);
    } catch (const std::exception &e) {
        raise_java_exception(env, e, file, line);
    } catch (const std::string &msg) {
        print_native_caught_exception_fwd2java(msg, file, line);
        env->ThrowNew(env->FindClass("java/lang/Error"), msg.c_str());
    } catch (const char *msg) {
        print_native_caught_exception_fwd2java(msg, file, line);
        env->ThrowNew(env->FindClass("java/lang/Error"), msg);
    } catch (...) {
        print_native_caught_exception_fwd2java(_unknown_exception_type_msg, file, line);
        env->ThrowNew(env->FindClass("java/lang/Error"), _unknown_exception_type_msg.c_str());
    }
}

bool java_exception_check(JNIEnv *env, const char* file, int line)
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

void java_exception_check_and_throw(JNIEnv *env, const char* file, int line)
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

        throw direct_bt::RuntimeException("Java exception occurred @ %s : %d: "+msg, file, line);
    }
}
