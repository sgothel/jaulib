/*
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
 *
 */

#ifndef PACKED_ATTRIBUTE_HPP_
#define PACKED_ATTRIBUTE_HPP_

/** \addtogroup CppLang
 *
 *  @{
 */

/** packed__: lead in macro, requires __packed lead out as well. Consider using __pack(...). */
#ifndef packed__
    #ifdef _MSC_VER
        #define packed__ __pragma( pack(push, 1) )
    #else
        #define packed__
    #endif
#endif

/** __packed: lead out macro, requires packed__ lead in as well. Consider using __pack(...). */
#ifndef __packed
    #ifdef _MSC_VER
        #define __packed __pragma( pack(pop))
    #else
        #define __packed __attribute__ ((packed))
    #endif
#endif

/** __pack(...): Produces MSVC, clang and gcc compatible lead-in and -out macros. */
#ifndef __pack
    #ifdef _MSC_VER
        #define __pack(...) __pragma( pack(push, 1) ) __VA_ARGS__ __pragma( pack(pop))
    #else
        #define __pack(...) __VA_ARGS__ __attribute__ ((packed))
    #endif
#endif

/**@}*/

namespace jau {

    /** \addtogroup CppLang
     *
     *  @{
     */

    /**
     * Support aligned memory transfer from and to potentially unaligned memory.
     *
     * This template causes little to no runtime costs.
     *
     * A casted data pointer to `packed_t<T>*` is similar to 'T * p = (T *) buffer', having `uint8_t * buffer`.
     * However, `packed_t<T>*` doesn't have any intrinsic alignment corrections due to
     * its used `__attribute__((__packed__))`.
     *
     * packed_t is used in \ref ByteUtils.
     *
     * @anchor packed_t_alignment_cast
     * Background for using packed_t:
     *
     * Due to the potentially unaligned memory address of `buffer`,
     * we can't just directly use pointer arithmetic like:
     * <pre>
     *   // return uint16_t from memory
     *   return *( (uint16_t *) ( buffer ) );
     *
     *   // store uint16_t to memory
     *   *( (uint16_t *) ( buffer ) ) = v;
     * </pre>
     *
     * The universal alternative using `memcpy()` is costly:
     * <pre>
     *   // return uint16_t from memory
     *   memcpy(&v, buffer, sizeof(v));
     *   return v;
     *
     *   // store uint16_t to memory
     *   memcpy(buffer, &v, sizeof(v));
     * </pre>
     *
     * Solution is to use the *free of costs* high performance *compiler magic* 'struct __attribute__((__packed__))'.<br />
     * The offset memory is pointer_cast() into the desired packed_t type and its packed_t::store accessed thereafter:
     * <pre>
     *   // return uint16_t from memory
     *   return pointer_cast<const packed_t<uint16_t>*>( buffer )->store;
     *
     *   // store uint16_t to memory
     *   pointer_cast<packed_t<uint16_t>*>( buffer )->store = v;
     * </pre>
     */
    template<typename T> __pack ( struct packed_t {
        T store;
    } ) ;

    /**@}*/

} // namespace jau

/**@}*/

#endif /* PACKED_ATTRIBUTE_HPP_ */
