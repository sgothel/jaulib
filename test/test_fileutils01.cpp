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
    const std::string project_root = "../../test_data";

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
            int sym_links;
        };
        visitor_stats stats { 0, 0, 0, 0 };
        const jau::fs::path_visitor pv = jau::bindCaptureRefFunc(&stats,
                ( bool(*)(visitor_stats*, const jau::fs::file_stats&) ) /* help template type deduction of function-ptr */
                    ( [](visitor_stats* stats_ptr, const jau::fs::file_stats& element_stats) -> bool {
                        stats_ptr->total++;
                        jau::fprintf_td(stderr, "test_03_visit: %d: %s\n", stats_ptr->total, element_stats.to_string(true).c_str());
                        if( !element_stats.has_access() ) {
                            return false;
                        }
                        if( element_stats.is_link() ) {
                            stats_ptr->sym_links++;
                        }
                        if( element_stats.is_file() ) {
                            stats_ptr->files++;
                        } else if( element_stats.is_dir() ) {
                            stats_ptr->dirs++;
                        }
                        return true;
                      } ) );
        REQUIRE( true == jau::fs::visit(root, true /* follow_sym_link_dirs */, pv) );
        REQUIRE( 12 == stats.total );
        REQUIRE(  8 == stats.files );
        REQUIRE(  4 == stats.dirs );
        REQUIRE(  0 == stats.sym_links );

        REQUIRE( true == jau::fs::remove(root, true) );
    }

    /**
     *
     */
    void test04_cwd() {
        const std::string cwd = jau::fs::get_cwd();
        INFO_STR("\n\ntest04_cwd: cwd "+cwd+"\n");
        REQUIRE( 0 < cwd.size() );
        const size_t idx = cwd.find("/jaulib/");
        REQUIRE( 0 < idx );
        REQUIRE( idx < cwd.size() );
        REQUIRE( idx != std::string::npos );
    }

    void test04_symlink_file() {
        INFO_STR("\n\ntest04_symlink_file\n");
        jau::fs::file_stats proot_stats(project_root);
        INFO_STR("project_root "+proot_stats.to_string(true)+"\n");
        REQUIRE( true == proot_stats.is_dir() );

        jau::fs::file_stats file_01_link_stats(proot_stats.path()+"/file_01_slink.txt");
        INFO_STR("project_root "+file_01_link_stats.to_string(true)+"\n");
        REQUIRE( true == file_01_link_stats.is_link() );
        REQUIRE( true == file_01_link_stats.is_file() );
        REQUIRE( 15 == file_01_link_stats.size() );
    }

    void test05_visit_symlinks() {
        INFO_STR("\n\ntest05_visit_symlinks\n");

        struct visitor_stats {
            int total_real;
            int total_sym_link;
            int files_real;
            int files_sym_link;
            int dirs_real;
            int dirs_sym_link;
        };
        {
            visitor_stats stats { 0, 0, 0, 0, 0, 0 };
            const jau::fs::path_visitor pv = jau::bindCaptureRefFunc(&stats,
                    ( bool(*)(visitor_stats*, const jau::fs::file_stats&) ) /* help template type deduction of function-ptr */
                        ( [](visitor_stats* stats_ptr, const jau::fs::file_stats& element_stats) -> bool {
                            if( element_stats.is_link() ) {
                                stats_ptr->total_sym_link++;
                            } else {
                                stats_ptr->total_real++;
                            }
                            jau::fprintf_td(stderr, "test05_visit_symlinks: total[real %d, symlink %d]: %s\n",
                                    stats_ptr->total_real, stats_ptr->total_sym_link, element_stats.to_string(true).c_str());
                            if( !element_stats.has_access() ) {
                                return false;
                            }
                            if( element_stats.is_file() ) {
                                if( element_stats.is_link() ) {
                                    stats_ptr->files_sym_link++;
                                } else {
                                    stats_ptr->files_real++;
                                }
                            } else if( element_stats.is_dir() ) {
                                if( element_stats.is_link() ) {
                                    stats_ptr->dirs_sym_link++;
                                } else {
                                    stats_ptr->dirs_real++;
                                }
                            }
                            return true;
                          } ) );
            REQUIRE( true == jau::fs::visit(project_root, false /* follow_sym_link_dirs */, pv) );
            REQUIRE(  7 == stats.total_real );
            REQUIRE(  3 == stats.total_sym_link );
            REQUIRE(  4 == stats.files_real );
            REQUIRE(  3 == stats.files_sym_link );
            REQUIRE(  3 == stats.dirs_real );
            REQUIRE(  0 == stats.dirs_sym_link );
        }
        {
            visitor_stats stats { 0, 0, 0, 0, 0, 0 };
            const jau::fs::path_visitor pv = jau::bindCaptureRefFunc(&stats,
                    ( bool(*)(visitor_stats*, const jau::fs::file_stats&) ) /* help template type deduction of function-ptr */
                        ( [](visitor_stats* stats_ptr, const jau::fs::file_stats& element_stats) -> bool {
                            if( element_stats.is_link() ) {
                                stats_ptr->total_sym_link++;
                            } else {
                                stats_ptr->total_real++;
                            }
                            jau::fprintf_td(stderr, "test05_visit_symlinks: total[real %d, symlink %d]: %s\n",
                                    stats_ptr->total_real, stats_ptr->total_sym_link, element_stats.to_string(true).c_str());
                            if( !element_stats.has_access() ) {
                                return false;
                            }
                            if( element_stats.is_file() ) {
                                if( element_stats.is_link() ) {
                                    stats_ptr->files_sym_link++;
                                } else {
                                    stats_ptr->files_real++;
                                }
                            } else if( element_stats.is_dir() ) {
                                if( element_stats.is_link() ) {
                                    stats_ptr->dirs_sym_link++;
                                } else {
                                    stats_ptr->dirs_real++;
                                }
                            }
                            return true;
                          } ) );
            REQUIRE( true == jau::fs::visit(project_root, true /* follow_sym_link_dirs */, pv) );
            REQUIRE(  9 == stats.total_real );
            REQUIRE(  5 == stats.total_sym_link );
            REQUIRE(  6 == stats.files_real );
            REQUIRE(  4 == stats.files_sym_link );
            REQUIRE(  3 == stats.dirs_real );
            REQUIRE(  1 == stats.dirs_sym_link );
        }
    }


};

METHOD_AS_TEST_CASE( TestFileUtil01::test01_mkdir,          "Test TestFileUtil01 - test01_mkdir");
METHOD_AS_TEST_CASE( TestFileUtil01::test02_touch,          "Test TestFileUtil01 - test02_touch");
METHOD_AS_TEST_CASE( TestFileUtil01::test03_visit,          "Test TestFileUtil01 - test03_visit");
METHOD_AS_TEST_CASE( TestFileUtil01::test04_cwd,            "Test TestFileUtil01 - test04_cwd");
METHOD_AS_TEST_CASE( TestFileUtil01::test04_symlink_file,   "Test TestFileUtil01 - test04_symlink_file");
METHOD_AS_TEST_CASE( TestFileUtil01::test05_visit_symlinks, "Test TestFileUtil01 - test05_visit_symlinks");

