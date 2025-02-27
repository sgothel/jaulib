/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */

#ifndef JAU_MATH_UTIL_SYNCBUFFER_HPP_
#define JAU_MATH_UTIL_SYNCBUFFER_HPP_

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

        /** Return the defined sync_action_t */
        virtual sync_action_t& action() noexcept = 0;

        /** Return the underlying data buffer as bytes. */
        virtual const void* data() const noexcept = 0;

        /** Returns type signature of implementing class's stored component value type. */
        virtual const jau::type_info& compSignature() const noexcept = 0;

        /** The component's size in bytes */
        // virtual size_t bytesPerComp() const noexcept = 0;

        /** The number of components per element */
        // virtual size_t compsPerElem() const noexcept = 0;

        /** Returns element count. One element consist compsPerElem() components. */
        // virtual size_t elementCount() const noexcept = 0;

        /** Returns the byte size of all elements, i.e. elementCount() * compsPerElem() * bytesPerComp(). */
        // virtual size_t byteCount() const noexcept = 0;

        /**
         * Synchronizes the underlying data before usage.
         * <p>
         * Convenient shortcut for action()() plus chaining.
         * </p>
         */
        SyncBuffer& sync() noexcept { action()(); return *this; }

        virtual std::string toString() const = 0;
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

        const void* data() const noexcept override { return floats(); }

        const jau::type_info& compSignature() const noexcept override { return jau::static_ctti<value_type>(); }
        /** The component's size in bytes */
        size_t bytesPerComp() const noexcept { return sizeof(value_type); }
        /** The number of components per element */
        size_t compsPerElem() const noexcept { return 16; }
        /** Returns element count. One element consist compsPerElem() components. */
        size_t elementCount() const noexcept { return 1; }
        /** Returns the byte size of all elements, i.e. elementCount() * compsPerElem() * bytesPerComp(). */
        size_t byteCount() const noexcept { return 1 * 16 * bytesPerComp(); }

        std::string toString() const override {
            return std::string("SyncMatrix4[").append(compSignature().name())
                   .append(", count ").append(std::to_string(elementCount()))
                   .append(" elem x ").append(std::to_string(compsPerElem()))
                   .append(" comp x ").append(std::to_string(bytesPerComp()))
                   .append(" = ").append(std::to_string(byteCount())).append(" bytes]");
        }
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

        const void* data() const noexcept override { return floats(); }

        const jau::type_info& compSignature() const noexcept override { return jau::static_ctti<value_type>(); }
        /** The component's size in bytes */
        size_t bytesPerComp() const noexcept { return sizeof(value_type); }
        /** The number of components per element */
        size_t compsPerElem() const noexcept { return 16; }
        /** Returns element count. One element consist compsPerElem() components. */
        size_t elementCount() const noexcept { return matrixCount(); }
        /** Returns the byte size of all elements, i.e. elementCount() * compsPerElem() * bytesPerComp(). */
        size_t byteCount() const noexcept { return matrixCount() * 16 * bytesPerComp(); }

        std::string toString() const override {
            return std::string("SyncMatrices4[").append(compSignature().name())
                   .append(", count ").append(std::to_string(elementCount()))
                   .append(" elem x ").append(std::to_string(compsPerElem()))
                   .append(" comp x ").append(std::to_string(bytesPerComp()))
                   .append(" = ").append(std::to_string(byteCount())).append(" bytes]");
        }

    };
    typedef SyncMatrices4<float> SyncMats4f;

 /**@}*/

 } // namespace jau::math::util

 #endif // JAU_MATH_UTIL_SYNCBUFFER_HPP_
