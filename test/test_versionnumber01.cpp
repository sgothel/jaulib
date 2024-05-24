/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2012-2024 Gothel Software e.K.
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
#include <cstring>
#include <iostream>

#include <jau/test/catch2_ext.hpp>

#include <jau/util/VersionNumber.hpp>

using namespace jau::util;

TEST_CASE( "VersionNumber Test 01a", "[version][util]" ) {
    std::string vs00 = "1.0.16";
    std::string vs01 = "OpenGL ES GLSL ES 1.0.16";
    std::string vs02 = "1.0.16 OpenGL ES GLSL ES";
    
    VersionNumber vn0(1, 0, 16);
    std::cout << "vn0: " << vn0 << std::endl;
    REQUIRE(true == vn0.hasMajor());
    REQUIRE(true == vn0.hasMinor());
    REQUIRE(true == vn0.hasSub());

    VersionNumber vn(vs00);
    std::cout << "vn.00: " << vn << std::endl;    
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs01);
    std::cout << "vn.01: " << vn << std::endl;    
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs02);
    std::cout << "vn.02: " << vn << std::endl;    
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);    
}

TEST_CASE( "VersionNumber Test 01b", "[version][util]" ) {
    const std::string delim = ",";

    const std::string vs00 = "1,0,16";
    const std::string vs01 = "OpenGL ES GLSL ES 1,0,16";
    const std::string vs02 = "1,0,16 OpenGL ES GLSL ES";
    const VersionNumber vn0 = VersionNumber(1, 0, 16);
    std::cout << "vn0: " << vn0 << std::endl;
    REQUIRE(true == vn0.hasMajor());
    REQUIRE(true == vn0.hasMinor());
    REQUIRE(true == vn0.hasSub());

    VersionNumber vn;
    vn = VersionNumber(vs00, delim);
    std::cout << "vn.00: " << vn << std::endl;
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs01, delim);
    std::cout << "vn.01: " << vn << std::endl;
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs02, delim);
    std::cout << "vn.02: " << vn << std::endl;
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);        
}

TEST_CASE( "VersionNumber Test 02a", "[version][util]" ) {
    const std::string vs00 = "4.20";
    const std::string vs01 = "COMPANY via Stupid tool 4.20";
    const std::string vs02 = "4.20 COMPANY via Stupid tool";
    const VersionNumber vn0 = VersionNumber(4, 20, 0);
    std::cout << "vn0: " << vn0 << std::endl;
    REQUIRE(true == vn0.hasMajor());
    REQUIRE(true == vn0.hasMinor());
    REQUIRE(true == vn0.hasSub());

    VersionNumber vn;
    vn = VersionNumber(vs00);
    std::cout << "vn.00: " << vn << std::endl;
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == !vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs01);
    std::cout << "vn.01: " << vn << std::endl;
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == !vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs02);
    std::cout << "vn.02: " << vn << std::endl;    
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == !vn.hasSub());
    REQUIRE(vn0 == vn);
}

TEST_CASE( "VersionNumber Test 02b", "[version][util]" ) {
    const std::string delim = ",";

    const std::string vs00 = "4,20";
    const std::string vs01 = "COMPANY via Stupid tool 4,20";
    const std::string vs02 = "4,20 COMPANY via Stupid tool";
    const VersionNumber vn0 = VersionNumber(4, 20, 0);
    REQUIRE(true == vn0.hasMajor());
    REQUIRE(true == vn0.hasMinor());
    REQUIRE(true == vn0.hasSub());

    VersionNumber vn;

    vn = VersionNumber(vs00, delim);
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == !vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs01, delim);
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == !vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs02, delim);
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == !vn.hasSub());
    REQUIRE(vn0 == vn);
}

TEST_CASE( "VersionNumber Test 03a", "[version][util]" ) {    
    const std::string vs00 = "A10.11.12b";
    const std::string vs01 = "Prelim Text 10.Funny11.Weird12 Something is odd";
    const std::string vs02 = "Prelim Text 10.Funny11l1.Weird12 2 Something is odd";
    const VersionNumber vn0 = VersionNumber(10, 11, 12);
    REQUIRE(true == vn0.hasMajor());
    REQUIRE(true == vn0.hasMinor());
    REQUIRE(true == vn0.hasSub());

    VersionNumber vn;

    vn = VersionNumber(vs00);
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs01);
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs02);
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);
}

TEST_CASE( "VersionNumber Test 03b", "[version][util]" ) {
    const std::string delim = ",";

    const std::string vs00 = "A10,11,12b";
    const std::string vs01 = "Prelim Text 10,Funny11,Weird12 Something is odd";
    const std::string vs02 = "Prelim Text 10,Funny11l1,Weird12 2 Something is odd";
    const VersionNumber vn0 = VersionNumber(10, 11, 12);
    REQUIRE(true == vn0.hasMajor());
    REQUIRE(true == vn0.hasMinor());
    REQUIRE(true == vn0.hasSub());

    VersionNumber vn;

    vn = VersionNumber(vs00, delim);
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs01, delim);
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs02, delim);
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);
}

TEST_CASE( "VersionNumber Test 04a", "[version][util]" ) {
    const std::string vs00 = "A10.11.12b (git-d6c318e)";
    const std::string vs01 = "Prelim Text 10.Funny11.Weird12 Something is odd (git-d6c318e)";
    const std::string vs02 = "Prelim Text 10.Funny11l1.Weird12 2 Something is odd (git-d6c318e)";
    const VersionNumber vn0 = VersionNumber(10, 11, 12);
    REQUIRE(true == vn0.hasMajor());
    REQUIRE(true == vn0.hasMinor());
    REQUIRE(true == vn0.hasSub());

    VersionNumber vn;

    vn = VersionNumber(vs00);
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs01);
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs02);
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);
}

TEST_CASE( "VersionNumber Test 04b", "[version][util]" ) {
    const std::string delim = ",";

    const std::string vs00 = "A10,11,12b (git-d6c318e)";
    const std::string vs01 = "Prelim Text 10,Funny11,Weird12 Something is odd (git-d6c318e)";
    const std::string vs02 = "Prelim Text 10,Funny11l1,Weird12 2 Something is odd (git-d6c318e)";
    const VersionNumber vn0 = VersionNumber(10, 11, 12);
    REQUIRE(true == vn0.hasMajor());
    REQUIRE(true == vn0.hasMinor());
    REQUIRE(true == vn0.hasSub());

    VersionNumber vn;

    vn = VersionNumber(vs00, delim);
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs01, delim);
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);

    vn = VersionNumber(vs02, delim);
    REQUIRE(true == vn.hasMajor());
    REQUIRE(true == vn.hasMinor());
    REQUIRE(true == vn.hasSub());
    REQUIRE(vn0 == vn);
}
