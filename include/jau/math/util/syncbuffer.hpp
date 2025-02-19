/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2014-2024 Gothel Software e.K.
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
#ifndef JAU_SYNCBUFFER_HPP_
#define JAU_SYNCBUFFER_HPP_

#include <cmath>
#include <cstdarg>
#include <cassert>

#include <jau/float_types.hpp>
#include <jau/functional.hpp>
#include <jau/math/mat4f.hpp>

namespace jau::math::util {

    /** \addtogroup Math
     *
     *  @{
     */

    /**
     * Specific data synchronization action implemented by the data provider
     * to update the buffer with the underlying data before usage, e.g. uploading the `GLUniformData` data to the GPU.
     */
    typedef jau::function<void()> sync_action_t;

    /** Plain function pointer type matching sync_action_t */
    typedef void(*sync_action_fptr)();

    /**
     * Convenient tuple of a sync_action_t and data buffer.
     *
     * sync_action_t is used to update the data buffer in case it is derived
     * or must be otherwise transported, defined by the data provider.
     */
    class SyncBuffer {
      public:
        virtual ~SyncBuffer() noexcept = default;

        /** Returns type signature of implementing class's stored value type. */
        virtual const jau::type_info& valueSignature() const noexcept = 0;

        /** Return the defined sync_action_t */
        virtual sync_action_t& action() noexcept = 0;

        /** Return the underlying data buffer as bytes. */
        virtual const void* data() const noexcept = 0;

        /** Returns element size in bytes */
        virtual size_t elementSize() const noexcept = 0;

        /** Returns element count, total byte size = elementSize() * elementCount() */
        virtual size_t elementCount() const noexcept = 0;

        size_t byteSize() const noexcept { return elementSize() * elementCount(); }

        /**
         * Synchronizes the underlying data before usage.
         * <p>
         * Convenient shortcut for action()() plus chaining.
         * </p>
         */
        SyncBuffer& sync() noexcept { action()(); return *this; }

        std::string toString() const {
            return std::string("SyncBuffer[").append(valueSignature().name())
                   .append(", count ").append(std::to_string(elementCount()))
                   .append(" x ").append(std::to_string(elementSize())).append("]");
        }

        /** Return the underlying data buffer as int8_t if valueSignature() matches, otherwise throw. */
        const int8_t* data_i8() const {
            const jau::type_info& t = valueSignature();
            if( t == jau::int_ctti::i8() ) {
                return reinterpret_cast<const int8_t*>(data());
            } else {
                throw jau::IllegalArgumentError("Storage type `"+t.name()+"`, not matching requested type `"+jau::int_ctti::i8().name()+"`", E_FILE_LINE);
            }
        }
        /** Return the underlying data buffer as int32_t if valueSignature() matches, otherwise throw. */
        const int32_t* data_i32() const {
            const jau::type_info& t = valueSignature();
            if( t == jau::int_ctti::i32() ) {
                return reinterpret_cast<const int32_t*>(data());
            } else {
                throw jau::IllegalArgumentError("Storage type `"+t.name()+"`, not matching requested type `"+jau::int_ctti::i32().name()+"`", E_FILE_LINE);
            }
        }
        /** Return the underlying data buffer as float32_t if valueSignature() matches, otherwise throw. */
        const float32_t* data_f32() const {
            const jau::type_info& t = valueSignature();
            if( t == jau::float_ctti::f32() ) {
                return reinterpret_cast<const float32_t*>(data());
            } else {
                throw jau::IllegalArgumentError("Storage type `"+t.name()+"`, not matching requested type `"+jau::float_ctti::f32().name()+"`", E_FILE_LINE);
            }
        }

    };

    /** SyncBuffer interface with a single underlying Matrix4. */
    template<typename Value_type,
             std::enable_if_t<std::is_floating_point_v<Value_type>, bool> = true>
    class SyncMatrix4 : public SyncBuffer {
      public:
        typedef Value_type               value_type;
        typedef Matrix4<value_type, std::is_floating_point_v<value_type>> Mat4;

        /** Return the underlying Mat4, used to synchronize via action() to the buffer(). */
        virtual const Mat4& matrix() const noexcept = 0;

        /** Return the underlying float data buffer. */
        const value_type* floats() const noexcept { return matrix().cbegin(); }

        const jau::type_info& valueSignature() const noexcept override { return jau::static_ctti<value_type>(); }
        const void* data() const noexcept override { return floats(); }
        size_t elementSize() const noexcept override { return sizeof(float); }
        size_t elementCount() const noexcept override { return 16; }
    };
    typedef SyncMatrix4<float> SyncMat4f;


    /** SyncBuffer interface with multiple underlying Matrix4. */
    template<typename Value_type,
             std::enable_if_t<std::is_floating_point_v<Value_type>, bool> = true>
    class SyncMatrices4 : public SyncBuffer {
      public:
        typedef Value_type               value_type;
        typedef Matrix4<value_type, std::is_floating_point_v<value_type>> Mat4;

        /** Return the underlying Mat4 pointer, used to synchronize via action() to the buffer(). */
        virtual const Mat4* matrices() const noexcept = 0;
        /** Return the number of Mat4 referenced by matrices() */
        virtual size_t matrixCount() const noexcept = 0;

        /** Return the underlying float data buffer. */
        const value_type* floats() const noexcept { return matrices()[0].cbegin(); }

        const jau::type_info& valueSignature() const noexcept override { return jau::static_ctti<value_type>(); }
        const void* data() const noexcept override { return floats(); }
        size_t elementSize() const noexcept override { return sizeof(float); }
        size_t elementCount() const noexcept override { return 16 * matrixCount(); }
    };
    typedef SyncMatrices4<float> SyncMats4f;

 /**@}*/

 } // namespace jau::math::util

 #endif // JAU_SYNCBUFFER_HPP_
