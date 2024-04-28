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
#ifndef JAU_SSTACK_HPP_
#define JAU_SSTACK_HPP_

#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cassert>
#include <limits>
#include <vector>
#include <type_traits>
#include <algorithm>

#include <jau/cpp_lang_util.hpp>
#include <jau/math/mat4f.hpp>

namespace jau::math::util {

    /** \addtogroup Math
     *
     *  @{
     */

    /**
     * A simple stack of compounds, each consisting of `element_size` * `T`
     * @tparam T type of one element used in each compound
     * @tparam element_size number of T elements making up one compound
     */
    template<typename Value_type, size_t Element_size,
             std::enable_if_t<std::is_floating_point_v<Value_type> ||
                              std::is_integral_v<Value_type>,
                              bool> = true>
    class SimpleStack {
        public:
            typedef Value_type  value_type;
            constexpr static const size_t element_size = Element_size;

        private:
            int growSize;
            std::vector<value_type> buffer;

        public:
            /**
             * Start w/ zero size and growSize is 16, half GL-min size (32)
             */
            constexpr_cxx20 SimpleStack() noexcept
            : growSize(16*element_size), buffer(0) {}

            /**
             * @param initialSize initial size
             * @param growSize grow size if {@link #position()} is reached, maybe <code>0</code>
             */
            constexpr_cxx20 SimpleStack(int initialSize, int growSize_) noexcept
            : growSize(growSize_), buffer(initialSize) {}

            constexpr_cxx20 size_t growIfNecessary(int length) noexcept {
                const size_t p = buffer.size();
                const size_t nsz = buffer.size() + length;
                if( nsz > buffer.capacity() ) {
                    buffer.reserve(buffer.size() + std::max(length, growSize));
                }
                buffer.resize(nsz);
                return p;
            }

            constexpr_cxx20 void push(const value_type* src) noexcept {
                size_t p = growIfNecessary(element_size);
                for(size_t i=0; i<element_size; ++i) {
                    buffer[p+i] = src[i];
                }
            }

            constexpr_cxx20 void pop(value_type* dest) noexcept {
                const size_t sz = buffer.size();
                assert( sz >= element_size );
                const size_t p = sz - element_size;
                for(size_t i=0; i<element_size; ++i) {
                    dest[i] = buffer[p+i];
                }
                buffer.resize(p);
            }
    };

    /**
     * 4x4 float matrix stack based on single float elements
     */
    typedef SimpleStack<float, 16 /* Element_size */> Stack16f;

    /**
     * A Matrix stack of compounds, each consisting of 16 * `T`
     * @tparam T type of one element used in each compound
     */
    template<typename Value_type,
             std::enable_if_t<std::is_floating_point_v<Value_type> ||
                              std::is_integral_v<Value_type>,
                              bool> = true>
    class MatrixStack {
        public:
            typedef Value_type  value_type;
            typedef Matrix4<value_type> matrix_t;

        private:
            int growSize;
            std::vector<matrix_t> buffer;

        public:
            /**
             * Start w/ zero size and growSize is 16, half GL-min size (32)
             */
            constexpr_cxx20 MatrixStack() noexcept
            : growSize(16), buffer(0) {}

            /**
             * @param initialSize initial size
             * @param growSize grow size if {@link #position()} is reached, maybe <code>0</code>
             */
            constexpr_cxx20 MatrixStack(int initialSize, int growSize_) noexcept
            : growSize(growSize_), buffer(initialSize) {}

            constexpr_cxx20 void growIfNecessary(int length) noexcept {
                const size_t nsz = buffer.size() + length;
                if( nsz > buffer.capacity() ) {
                    buffer.reserve(buffer.size() + std::max(length, growSize));
                }
            }

            constexpr_cxx20 void push(const matrix_t& src) noexcept {
                growIfNecessary(1);
                buffer.push_back(src);
            }

            constexpr_cxx20 void pop(matrix_t& dest) noexcept {
                const size_t sz = buffer.size();
                assert( sz >= 1 );
                const size_t p = sz - 1;
                dest.load( buffer[p] );
                buffer.resize(p);
            }
    };

    /**
     * 4x4 float matrix stack
     */
    typedef MatrixStack<float> Mat4fStack;

    /**@}*/

} // namespace jau::math

#endif // JAU_SSTACK_HPP_
