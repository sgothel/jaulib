/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
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
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <memory>
#include <thread>
#include <pthread.h>

#include <jau/test/catch2_ext.hpp>

#include <jau/file_util.hpp>
#include <jau/cpuid.hpp>
#include <jau/os/os_support.hpp>
#include <jau/os/user_info.hpp>

TEST_CASE( "Test 00 Platform Info - os_and_arch", "[endian][abi][cpu][os]" ) {
    std::cout << jau::os::get_platform_info() << std::endl;
}

TEST_CASE( "Test 01 OS CPU ABI ENDIAN - os_and_arch", "[endian][abi][cpu][os]" ) {
    const jau::os::os_type os = jau::os::os_type::native;
    const jau::cpu::cpu_family cpu = jau::cpu::get_cpu_family();
    const jau::os::abi_type abi = jau::os::get_abi_type();
    const jau::endian byte_order = jau::endian::native;

    std::string os_and_arch = jau::os::get_os_and_arch(os, cpu, abi, byte_order);
    std::cout << "- os_type:    " << jau::os::to_string(os) << std::endl;
    std::cout << "- cpu_family: " << jau::cpu::to_string(cpu) << std::endl;
    std::cout << "- abi_type:   " << jau::os::to_string(abi) << std::endl;
    std::cout << "- endian:     " << jau::to_string(byte_order) << std::endl;
    std::cout << "- ptr-bits:   " << std::to_string(jau::cpu::get_arch_psize()) << std::endl;
    std::cout << "- os_and_arch " << os_and_arch << std::endl << std::endl;

    std::cout << jau::cpu::get_cpu_info() << std::endl << std::endl;

    REQUIRE( true == jau::os::is_defined_os_type(jau::os::os_type::native) );
    REQUIRE( std::string::npos == os_and_arch.find("undef") );

    REQUIRE( true == jau::is_defined_endian(jau::endian::native) );
    REQUIRE( true == jau::is_little_or_big_endian() );
}

TEST_CASE( "Test 10 User Info", "[user][os]" ) {
    {
        jau::os::UserInfo uinfo;
        std::cout << "User-Current: " << uinfo.toString() << std::endl;
        REQUIRE( true == uinfo.isValid() );
    }
    {
        jau::os::UserInfo uinfo("root");
        std::cout << "User 'root':  " << uinfo.toString() << std::endl;
        // REQUIRE( true == uinfo.isValid() );
    }

}
