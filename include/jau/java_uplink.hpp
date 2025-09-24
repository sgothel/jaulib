/*
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

#ifndef JAU_JAVA_UPLINK_HPP_
#define JAU_JAVA_UPLINK_HPP_

#include <string>
#include <memory>

#include <jau/basic_types.hpp>
#include <jau/string_util.hpp>

namespace jau::jni {

    /** @defgroup JavaVM Java VM Utilities
     *  Java virtual machine support, helping accessing the JVM and converting data types.
     *
     *  @{
     */

    /**
     * Pure virtual JavaAnon, hiding Java JNI details from API,
     * to be implemented by JNI module.
     * <p>
     * One implementation is JavaGlobalObj within the JNI module,
     * wrapping a JNIGlobalRef instance.
     * </p>
     */
    class JavaAnon {
        public:
            virtual ~JavaAnon() noexcept = default;
            virtual std::string toString() const noexcept { return "JavaAnon[???]"; }
    };
    typedef std::shared_ptr<JavaAnon> JavaAnonRef;

    /**
     * Sharing the anonymous Java object (JavaAnon),
     * i.e. exposing the Java object uplink to the C++ implementation.
     */
    class JavaUplink {
        private:
            JavaAnonRef javaObjectRef;

        public:
            virtual std::string toString() const noexcept { return "JavaUplink["+jau::toHexString(this)+"]"; }

            virtual std::string get_java_class() const noexcept = 0;

            std::string javaObjectToString() const noexcept {
                if( nullptr == javaObjectRef ) {
                    return "JavaAnon[null]";
                } else if( 0 == javaObjectRef.use_count() ) { // safe-guard for concurrent dtor
                    return "JavaAnon[empty]";
                }
                return javaObjectRef->toString();
            }

            const JavaAnonRef& getJavaObject() noexcept { return javaObjectRef; }

            /** Assigns a new shared JavaAnon reference, replaced item might be deleted via JNI from dtor */
            void setJavaObject(const JavaAnonRef& objRef) noexcept { javaObjectRef = objRef; }

            /** Resets the shared JavaAnon reference, the replaced item might be deleted via JNI from dtor */
            void setJavaObject() noexcept { javaObjectRef.reset(); }

            /**
             * Throws an IllegalStateException if instance is not valid
             *
             * Default implementation does nothing.
             */
            virtual void checkValidInstance() const {}

            JavaUplink() noexcept = default;
            JavaUplink(const JavaUplink &o) noexcept = default;
            JavaUplink(JavaUplink &&o) noexcept = default;
            JavaUplink& operator=(const JavaUplink &o) noexcept = default;
            JavaUplink& operator=(JavaUplink &&o) noexcept = default;

            virtual ~JavaUplink() noexcept { // NOLINT(modernize-use-equals-default): Intended
                javaObjectRef = nullptr;
            }
    };
    typedef std::shared_ptr<JavaUplink> JavaUplinkRef;

    /**@}*/

} /* namespace jau */


#endif /* JAU_JAVA_UPLINK_HPP_ */
