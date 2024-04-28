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
#include <cstdint>
#include <cassert>
#include <limits>
#include <string>
#include <vector>
#include <initializer_list>
#include <iostream>

#include <jau/functional.hpp>
#include <jau/math/mat4f.hpp>

namespace jau::math::util {

    /** \addtogroup Math
     *
     *  @{
     */

    /**
     * Specific data synchronization action implemented by the data provider
     * to update the buffer with the underlying data before usage, e.g. uploading the {@link com.jogamp.opengl.GLUniformData GLUniformData} data to the GPU.
     * <p>
     * Example: Invoked before delivering {@link com.jogamp.opengl.GLUniformData GLUniformData}'s data via {@link com.jogamp.opengl.GLUniformData#getObject() getObject()}
     * or {@link com.jogamp.opengl.GLUniformData#getBuffer() getBuffer()}.
     * </p>
     */
    typedef jau::function<void()> sync_action_t;

    /** Plain function pointer type matching sync_action_t */
    typedef void(*sync_action_fptr)();

    /**
     * Convenient tuple of a {@link SyncAction} and {@link Buffer}.
     * <p>
     * {@link SyncAction#sync()} is used to update the {@link Buffer} with the underlying data
     * known to the data provider.
     * </p>
     * @see SyncAction
     */
    class SyncBuffer {
      public:
        virtual ~SyncBuffer() noexcept = default;

        /**
         * Return the {@link SyncAction}.
         * @see SyncAction
         */
        virtual sync_action_t& action() noexcept = 0;

        /** Return the underlying data buffer. */
        virtual const void* buffer() const noexcept = 0;

        /** Returns element size in bytes */
        virtual size_t elementSize() const noexcept = 0;

        /** Returns element count, total byte size = elementSize() * elementCount() */
        virtual size_t elementCount() const noexcept = 0;

        /**
         * Synchronizes the underlying data before usage.
         * <p>
         * Convenient shortcut for action()() plus chaining.
         * </p>
         */
        SyncBuffer& sync() noexcept { action()(); return *this; }
    };

    /** {@link SyncBuffer} interface with a single underlying {@link Matrix4f}, used in {@link SyncMatrix4f16} and {@link com.jogamp.math.util.PMVMatrix4f PMVMatrix4f}. */
    class SyncMat4f : public SyncBuffer {
      public:
        virtual ~SyncMat4f() noexcept = default;

        /** Return the underlying Mat4f, used to synchronize via action() to the buffer(). */
        virtual const Mat4f& matrix() const noexcept = 0;

        /** Return the underlying float data buffer. */
        const float* floats() const noexcept { return matrix().cbegin(); }

        virtual const void* buffer() const noexcept override { return floats(); }
        virtual size_t elementSize() const noexcept override { return sizeof(float); }
        virtual size_t elementCount() const noexcept override { return 16; }
    };

    /** {@link SyncBuffer} with a multiple underlying {@link Matrix4f}, used in {@link SyncMatrices4f16} and {@link com.jogamp.math.util.PMVMatrix4f PMVMatrix4f} */
    class SyncMats4f : public SyncBuffer {
      public:
        virtual ~SyncMats4f() noexcept = default;

        /** Return the underlying Mat4f pointer, used to synchronize via action() to the buffer(). */
        virtual const Mat4f* matrices() const noexcept = 0;
        /** Return the number of Mat4f referenced by matrices() */
        virtual size_t matrixCount() const noexcept = 0;

        const float* floats() const noexcept { return matrices()[0].cbegin(); }
        virtual const void* buffer() const noexcept override { return floats(); }
        virtual size_t elementSize() const noexcept override { return sizeof(float); }
        virtual size_t elementCount() const noexcept override { return 16 * matrixCount(); }
    };

 /**@}*/

 } // namespace jau::math::util

 #endif // JAU_SYNCBUFFER_HPP_
