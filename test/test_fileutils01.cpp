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
#include <iostream>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <memory>
#include <thread>
#include <pthread.h>

#define CATCH_CONFIG_RUNNER
// #define CATCH_CONFIG_MAIN
#include <catch2/catch_amalgamated.hpp>
#include <jau/test/catch2_ext.hpp>

#include <jau/debug.hpp>
#include <jau/file_util.hpp>

using namespace jau::fractions_i64_literals;

class TestFileUtil01 {
  public:
    const std::string root = "test_data";

    /**
     *
     */
    void test01_mkdir() {
        INFO_STR("\n\ntest01_mkdir\n");
        jau::fs::remove(root, true); // start fresh
        {
            jau::fs::file_stats root_stats(root);
            INFO_STR("root_stats.pre: "+root_stats.to_string(true)+"\n");
            REQUIRE( !root_stats.exists() );
            REQUIRE( root_stats.has_access() );
            REQUIRE( !root_stats.is_dir() );
            REQUIRE( !root_stats.is_file() );
            REQUIRE( !root_stats.is_link() );
        }
        REQUIRE( true == jau::fs::mkdir(root) );
        {
            jau::fs::file_stats root_stats(root);
            INFO_STR("root_stats.post: "+root_stats.to_string(true)+"\n");
            REQUIRE( root_stats.exists() );
            REQUIRE( root_stats.has_access() );
            REQUIRE( root_stats.is_dir() );
            REQUIRE( !root_stats.is_file() );
            REQUIRE( !root_stats.is_link() );
        }
        REQUIRE( false == jau::fs::remove(root, false) );
        REQUIRE( true == jau::fs::remove(root, true) );
    }

    void test02_touch() {
        INFO_STR("\n\ntest02_touch\n");
        const std::string file_01 = root+"/data01.txt";
        const std::string file_02 = root+"/data02.txt";
        REQUIRE( true == jau::fs::mkdir(root) );
        {
            jau::fs::file_stats root_stats(root);
            INFO_STR("root_stats1.post: "+root_stats.to_string(true)+"\n");
            REQUIRE( root_stats.exists() );
            REQUIRE( root_stats.has_access() );
            REQUIRE( root_stats.is_dir() );
            REQUIRE( !root_stats.is_file() );
            REQUIRE( !root_stats.is_link() );
        }

        REQUIRE( true == jau::fs::touch(file_01) );
        {
            jau::fs::file_stats file_stats(file_01);
            INFO_STR("file_stats2.post: "+file_stats.to_string(true)+"\n");
            REQUIRE( file_stats.exists() );
            REQUIRE( file_stats.has_access() );
            REQUIRE( !file_stats.is_dir() );
            REQUIRE( file_stats.is_file() );
            REQUIRE( !file_stats.is_link() );
        }

        const jau::fraction_timespec atime = jau::getWallClockTime();
        const jau::fraction_timespec ts_20200101( 1577836800_s + 0_h); // 2020-01-01 00:00:00
        const jau::fraction_timespec mtime( ts_20200101 + 31_d + 10_h );
        INFO_STR("atime.pre: "+atime.to_iso8601_string(true)+", "+atime.to_string()+"\n");
        INFO_STR("mtime.pre: "+mtime.to_iso8601_string(true)+", "+mtime.to_string()+"\n");
        REQUIRE( true == jau::fs::touch(file_02, atime, mtime) );
        {
            jau::fs::file_stats file_stats(file_02);
            INFO_STR("file_stats3.post: "+file_stats.to_string(true)+"\n");
            REQUIRE( file_stats.exists() );
            REQUIRE( file_stats.has_access() );
            REQUIRE( !file_stats.is_dir() );
            REQUIRE( file_stats.is_file() );
            REQUIRE( !file_stats.is_link() );
            REQUIRE( atime == file_stats.atime() );
            REQUIRE( mtime == file_stats.mtime() );
        }

        REQUIRE( true == jau::fs::remove(root, true) );
    }

    void test03_visit() {
        INFO_STR("\n\ntest03_visit\n");

        std::string sub_dir1 = root+"/sub1";
        std::string sub_dir2 = root+"/sub2";
        std::string sub_dir3 = root+"/sub1/sub3";

        REQUIRE( true == jau::fs::mkdir(root) );
        REQUIRE( true == jau::fs::touch(root+"/data01.txt") );
        REQUIRE( true == jau::fs::touch(root+"/data02.txt") );
        REQUIRE( true == jau::fs::mkdir(sub_dir1) );
        REQUIRE( true == jau::fs::mkdir(sub_dir2) );
        REQUIRE( true == jau::fs::mkdir(sub_dir3) );
        REQUIRE( true == jau::fs::touch(sub_dir1+"/data03.txt") );
        REQUIRE( true == jau::fs::touch(sub_dir1+"/data04.txt") );
        REQUIRE( true == jau::fs::touch(sub_dir2+"/data05.txt") );
        REQUIRE( true == jau::fs::touch(sub_dir2+"/data06.txt") );
        REQUIRE( true == jau::fs::touch(sub_dir3+"/data07.txt") );
        REQUIRE( true == jau::fs::touch(sub_dir3+"/data08.txt") );

        struct visitor_stats {
            int total;
            int files;
            int dirs;
        };
        visitor_stats stats { 0, 0, 0 };
        const jau::fs::path_visitor pv = jau::bindCaptureRefFunc(&stats,
                ( bool(*)(visitor_stats*, const jau::fs::file_stats&) ) /* help template type deduction of function-ptr */
                    ( [](visitor_stats* stats_ptr, const jau::fs::file_stats& element_stats) -> bool {
                        stats_ptr->total++;
                        jau::fprintf_td(stderr, "test_03_visit: %d: %s\n", stats_ptr->total, element_stats.to_string(true).c_str());
                        if( !element_stats.has_access() ) {
                            return false;
                        }
                        if( element_stats.is_file() ) {
                            stats_ptr->files++;
                        } else if( element_stats.is_dir() ) {
                            stats_ptr->dirs++;
                        }
                        return true;
                      } ) );
        REQUIRE( true == jau::fs::visit(root, pv) );
        REQUIRE( 12 == stats.total );
        REQUIRE(  8 == stats.files );
        REQUIRE(  4 == stats.dirs );

        REQUIRE( true == jau::fs::remove(root, true) );
    }
};

METHOD_AS_TEST_CASE( TestFileUtil01::test01_mkdir, "Test TestFileUtil01 - test01_mkdir");
METHOD_AS_TEST_CASE( TestFileUtil01::test02_touch, "Test TestFileUtil01 - test02_touch");
METHOD_AS_TEST_CASE( TestFileUtil01::test03_visit, "Test TestFileUtil01 - test03_visit");

