/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
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
 *************************************************************************
 *
 * Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *        https://www.boost.org/LICENSE_1_0.txt)
 *
 * SPDX-License-Identifier: BSL-1.0
 */

#ifndef CATCH2_EXT_H
#define CATCH2_EXT_H

#include <catch2/catch_amalgamated.hpp>
#include <jau/float_math.hpp>

// namespace Catch {
///////////////////////////////////////////////////////////////////////////////
#define INTERNAL_CATCH_TEST_M( msg, macroName, resultDisposition, ... ) \
    do { \
        /* The expression should not be evaluated, but warnings should hopefully be checked */ \
        CATCH_INTERNAL_IGNORE_BUT_WARN(__VA_ARGS__); \
        std::string s1( macroName##_catch_sr ); \
        s1.append(msg); \
        s1.append(": "); \
        Catch::AssertionHandler catchAssertionHandler( s1, CATCH_INTERNAL_LINEINFO, CATCH_INTERNAL_STRINGIFY(__VA_ARGS__), resultDisposition ); \
        INTERNAL_CATCH_TRY { \
            CATCH_INTERNAL_START_WARNINGS_SUPPRESSION \
            CATCH_INTERNAL_SUPPRESS_PARENTHESES_WARNINGS \
            catchAssertionHandler.handleExpr( Catch::Decomposer() <= __VA_ARGS__ ); \
            CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION \
        } INTERNAL_CATCH_CATCH( catchAssertionHandler ) \
        INTERNAL_CATCH_REACT( catchAssertionHandler ) \
    } while( (void)0, (false) && static_cast<bool>( !!(__VA_ARGS__) ) )

    #define REQUIRE_MSG(MSG, ... ) INTERNAL_CATCH_TEST_M( MSG, "REQUIRE: ", Catch::ResultDisposition::Normal, __VA_ARGS__  )
    #define INFO_STR( msg ) INTERNAL_CATCH_INFO( "INFO", static_cast<std::string>(msg) )

    #define REQUIRE_EPSI(a,b)           INTERNAL_CATCH_TEST( "REQUIRE: ", Catch::ResultDisposition::Normal, jau::machine_equal(a, b))
    #define REQUIRE_EPSI_MSG(m,a,b)     INTERNAL_CATCH_TEST_M( m, "REQUIRE: ", Catch::ResultDisposition::Normal, jau::machine_equal(a, b))

    #define REQUIRE_DIFF(a,b,d)         INTERNAL_CATCH_TEST( "REQUIRE: ", Catch::ResultDisposition::Normal, jau::machine_equal(a, b, 1, d))
    #define REQUIRE_DIFF_MSG(m,a,b,d)   INTERNAL_CATCH_TEST_M( m, "REQUIRE: ", Catch::ResultDisposition::Normal, jau::machine_equal(a, b, 1, d))

// }

#if !defined(CATCH_CONFIG_MAIN)
#include <jau/test/catch2_my_main.cpp>
#endif /* !defined(CATCH_CONFIG_MAIN) */

#endif // CATCH2_EXT_H

