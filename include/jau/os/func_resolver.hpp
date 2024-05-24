/**
 * Author: Sven Gothel <sgothel@jausoft.com>
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
 */
#pragma once

#include <cstdint>
#include <string>

#include <jau/os/dyn_linker.hpp>

namespace jau::os {

    /** \addtogroup OSSup
     *
     *  @{
     */

    /** Interface callers may use ProcAddressHelper's
     * {@link com.jogamp.gluegen.runtime.ProcAddressTable#reset(com.jogamp.common.os.DynamicLookupHelper) reset}
     *  helper method to install function pointers into a
     *  ProcAddressTable. This must typically be written with native
     *  code. */
    class DynamicLookup {
        public:
          virtual ~DynamicLookup() noexcept = default;

          /** Returns the function handle for function `funcName` gathered within the associated native library. */
          virtual DynamicLinker::symhandle_t dynamicLookupFunction(const std::string& funcName) const noexcept = 0;

          /** Returns the function handle for function `funcName` gathered globally within all loaded libraries. */
          virtual DynamicLinker::symhandle_t dynamicLookupFunctionGlobal(const std::string& funcName) const noexcept = 0;

          /**
           * Queries whether function 'funcName' is available.
           */
          bool isFunctionAvailable(const std::string& funcName) const noexcept {
              return nullptr != dynamicLookupFunction(funcName);
          }
    };
    
    class FuncAddrResolver {
        public:
            /**
             * Resolves the name of the function bound to the method and returns the address.
             * <p>
             * Implementation shall ensure {@link SecurityUtil#checkLinkPermission(String)} is performed.
             * </p>
             * @throws SecurityException if user is not granted access for the library set.
             */
            DynamicLinker::symhandle_t resolve(const std::string& name, const DynamicLookup& lookup) const noexcept;
    };

    /**@}*/

}
