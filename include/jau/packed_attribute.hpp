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

namespace jau {

    /**
     * Safe access to a pointer cast from unaligned memory via __packed__ attribute,
     * i.e. utilizing compiler generated safe load and store operations.
     * <p>
     * This template shall cause no costs, the cast data pointer is identical to 'T & p = &store'.
     * </p>
     */
    template<typename T> __pack ( struct packed_t {
        T store;
    } ) ;

} // namespace jau

/**@}*/

#endif /* PACKED_ATTRIBUTE_HPP_ */
