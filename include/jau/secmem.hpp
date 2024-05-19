/*
 * Copyright (c) 2024 Gothel Software e.K.
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

#ifndef JAU_SECMEM_HPP_
#define JAU_SECMEM_HPP_

#include <jau/cpp_lang_util.hpp>
#include <cstring>

namespace jau {

    /** @defgroup SecMem Secure Memory
     *  Secure Memory utilities
     *
     *  @{
     */
     
    /** 
     * Wrapper to ::explicit_bzero(), ::bzero() or ::memset(), whichever is available in that order.
     *
     * Implementation shall not be optimized away for security reasons. 
     */
    void zero_bytes_sec(void *s, size_t n) noexcept __attribute__((nonnull (1))) __attrdecl_no_optimize__;    

    /**@}*/

} // namespace jau

#endif /* JAU_SECMEM_HPP_ */
