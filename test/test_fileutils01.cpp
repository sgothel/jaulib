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

#include "test_fileutils_copy_r_p.hpp"

extern "C" {
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <sys/wait.h>
    #include <unistd.h>
}

class TestFileUtil01 : TestFileUtilBase {
  public:

    /**
     *
     */
    void test01_cwd() {
        const std::string cwd = jau::fs::get_cwd();
        INFO_STR("\n\ntest01_cwd: cwd "+cwd+"\n");
        REQUIRE( 0 < cwd.size() );
        const size_t idx = cwd.find("/jaulib/");
        REQUIRE( 0 < idx );
        REQUIRE( idx < cwd.size() );
        REQUIRE( idx != std::string::npos );
    }

    /**
     *
     */
    void test02_dirname() {
        {
            const std::string pathname0 = "/";
            const std::string pathname1 = jau::fs::dirname(pathname0);
            INFO_STR("\n\ntest02_dirname: cwd "+pathname0+" -> "+pathname1+"\n");
            REQUIRE( 0 < pathname1.size() );
            REQUIRE( pathname1 == "/" );
        }
        {
            {
                const std::string pathname0 = "lala.txt";
                const std::string pathname1 = jau::fs::dirname(pathname0);
                INFO_STR("\n\ntest02_dirname: cwd "+pathname0+" -> "+pathname1+"\n");
                REQUIRE( 0 < pathname1.size() );
                REQUIRE( pathname1 == "." );
            }
            {
                const std::string pathname0 = "lala";
                const std::string pathname1 = jau::fs::dirname(pathname0);
                INFO_STR("\n\ntest02_dirname: cwd "+pathname0+" -> "+pathname1+"\n");
                REQUIRE( 0 < pathname1.size() );
                REQUIRE( pathname1 == "." );
            }
            {
                const std::string pathname0 = "lala/";
                const std::string pathname1 = jau::fs::dirname(pathname0);
                INFO_STR("\n\ntest02_dirname: cwd "+pathname0+" -> "+pathname1+"\n");
                REQUIRE( 0 < pathname1.size() );
                REQUIRE( pathname1 == "." );
            }
        }
        {
            const std::string pathname0 = "/lala.txt";
            const std::string pathname1 = jau::fs::dirname(pathname0);
            INFO_STR("\n\ntest02_dirname: cwd "+pathname0+" -> "+pathname1+"\n");
            REQUIRE( 0 < pathname1.size() );
            REQUIRE( pathname1 == "/" );
        }
        {
            const std::string pathname0 = "blabla/jaulib/test/sub.txt";
            const std::string pathname1 = jau::fs::dirname(pathname0);
            INFO_STR("\n\ntest02_dirname: cwd "+pathname0+" -> "+pathname1+"\n");
            REQUIRE( 0 < pathname1.size() );
            REQUIRE( pathname1 == "blabla/jaulib/test" );
        }
        {
            const std::string pathname0 = "blabla/jaulib/test/sub";
            const std::string pathname1 = jau::fs::dirname(pathname0);
            INFO_STR("\n\ntest02_dirname: cwd "+pathname0+" -> "+pathname1+"\n");
            REQUIRE( 0 < pathname1.size() );
            REQUIRE( pathname1 == "blabla/jaulib/test" );
        }
        {
            const std::string pathname0 = "blabla/jaulib/test/";
            const std::string pathname1 = jau::fs::dirname(pathname0);
            INFO_STR("\n\ntest02_dirname: cwd "+pathname0+" -> "+pathname1+"\n");
            REQUIRE( 0 < pathname1.size() );
            REQUIRE( pathname1 == "blabla/jaulib" );
        }
        {
            const std::string pathname0 = "blabla/jaulib/test";
            const std::string pathname1 = jau::fs::dirname(pathname0);
            INFO_STR("\n\ntest02_dirname: cwd "+pathname0+" -> "+pathname1+"\n");
            REQUIRE( 0 < pathname1.size() );
            REQUIRE( pathname1 == "blabla/jaulib" );
        }
    }

    void test03_basename() {
        {
            const std::string pathname0 = "/";
            const std::string pathname1 = jau::fs::basename(pathname0);
            INFO_STR("\n\ntest03_basename: cwd "+pathname0+" -> "+pathname1+"\n");
            REQUIRE( 0 < pathname1.size() );
            REQUIRE( pathname1 == "/" );
        }
        {
            {
                const std::string pathname0 = "lala.txt";
                const std::string pathname1 = jau::fs::basename(pathname0);
                INFO_STR("\n\ntest03_basename: cwd "+pathname0+" -> "+pathname1+"\n");
                REQUIRE( 0 < pathname1.size() );
                REQUIRE( pathname1 == "lala.txt" );
            }
            {
                const std::string pathname0 = "lala";
                const std::string pathname1 = jau::fs::basename(pathname0);
                INFO_STR("\n\ntest03_basename: cwd "+pathname0+" -> "+pathname1+"\n");
                REQUIRE( 0 < pathname1.size() );
                REQUIRE( pathname1 == "lala" );
            }
            {
                const std::string pathname0 = "lala/";
                const std::string pathname1 = jau::fs::basename(pathname0);
                INFO_STR("\n\ntest03_basename: cwd "+pathname0+" -> "+pathname1+"\n");
                REQUIRE( 0 < pathname1.size() );
                REQUIRE( pathname1 == "lala" );
            }
        }
        {
            const std::string pathname0 = "/lala.txt";
            const std::string pathname1 = jau::fs::basename(pathname0);
            INFO_STR("\n\ntest03_basename: cwd "+pathname0+" -> "+pathname1+"\n");
            REQUIRE( 0 < pathname1.size() );
            REQUIRE( pathname1 == "lala.txt" );
        }
        {
            const std::string pathname0 = "blabla/jaulib/test/sub.txt";
            const std::string pathname1 = jau::fs::basename(pathname0);
            INFO_STR("\n\ntest03_basename: cwd "+pathname0+" -> "+pathname1+"\n");
            REQUIRE( 0 < pathname1.size() );
            REQUIRE( pathname1 == "sub.txt" );
        }

        {
            const std::string pathname0 = "blabla/jaulib/test/";
            const std::string pathname1 = jau::fs::basename(pathname0);
            INFO_STR("\n\ntest03_basename: cwd "+pathname0+" -> "+pathname1+"\n");
            REQUIRE( 0 < pathname1.size() );
            REQUIRE( pathname1 == "test" );
        }

        {
            const std::string pathname0 = "blabla/jaulib/test";
            const std::string pathname1 = jau::fs::basename(pathname0);
            INFO_STR("\n\ntest03_basename: cwd "+pathname0+" -> "+pathname1+"\n");
            REQUIRE( 0 < pathname1.size() );
            REQUIRE( pathname1 == "test" );
        }
    }

    void test04_dir_item() {
        {
            const std::string dirname_;
            const jau::fs::dir_item di(dirname_);
            INFO_STR("\n\ntest04_dir_item: 01 '"+dirname_+"' -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "." == di.dirname() );
            REQUIRE( "." == di.basename() );
            REQUIRE( "." == di.path() );
        }
        {
            const std::string dirname_(".");
            const jau::fs::dir_item di(dirname_);
            INFO_STR("\n\ntest04_dir_item: 02 '"+dirname_+"' -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "." == di.dirname() );
            REQUIRE( "." == di.basename() );
            REQUIRE( "." == di.path() );
        }
        {
            const std::string dirname_("/");
            const jau::fs::dir_item di(dirname_);
            INFO_STR("\n\ntest04_dir_item: 03 '"+dirname_+"' -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "/" == di.dirname() );
            REQUIRE( "." == di.basename() );
            REQUIRE( "/" == di.path() );
        }

        {
            const std::string path1_ = "lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 10 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "." == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "lala" == di.path() );
        }
        {
            const std::string path1_ = "lala/";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 11 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "." == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "lala" == di.path() );
        }

        {
            const std::string path1_ = "/lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 12 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "/" == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "/lala" == di.path() );
        }

        {
            const std::string path1_ = "dir0/lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 20 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "dir0" == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "dir0/lala" == di.path() );
        }
        {
            const std::string path1_ = "dir0/lala/";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 21 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "dir0" == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "dir0/lala" == di.path() );
        }
        {
            const std::string path1_ = "/dir0/lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 22 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "/dir0" == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "/dir0/lala" == di.path() );
        }
        {
            const std::string path1_ = "/dir0/lala/";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 23 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "/dir0" == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "/dir0/lala" == di.path() );
        }


        {
            const std::string path1_ = "/dir0/../lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 30 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "/" == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "/lala" == di.path() );
        }
        {
            const std::string path1_ = "dir0/../lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 31 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "." == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "lala" == di.path() );
        }
        {
            const std::string path1_ = "../../lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 32 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "../.." == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "../../lala" == di.path() );
        }
        {
            const std::string path1_ = "./../lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 33 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( ".." == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "../lala" == di.path() );
        }
        {
            const std::string path1_ = "dir0/../../lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 34 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( ".." == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "../lala" == di.path() );
        }

        {
            const std::string path1_ = "dir0/dir1/../lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 40 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "dir0" == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "dir0/lala" == di.path() );
        }
        {
            const std::string path1_ = "/dir0/dir1/../lala/";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 41 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "/dir0" == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "/dir0/lala" == di.path() );
        }
        {
            const std::string path1_ = "dir0/dir1/../bbb/ccc/../lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 42 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "dir0/bbb" == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "dir0/bbb/lala" == di.path() );
        }
        {
            const std::string path1_ = "dir0/dir1/bbb/../../lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 43 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "dir0" == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "dir0/lala" == di.path() );
        }
        {
            const std::string path1_ = "dir0/dir1/bbb/../../../lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 44 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "." == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "lala" == di.path() );
        }
        {
            const std::string path1_ = "dir0/dir1/bbb/../../../../lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 45 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( ".." == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "../lala" == di.path() );
        }
        {
            const std::string path1_ = "dir0/dir1/bbb/../../lala/..";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 46 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "." == di.dirname() );
            REQUIRE( "dir0" == di.basename() );
            REQUIRE( "dir0" == di.path() );
        }
        {
            const std::string path1_ = "dir0/./dir1/./bbb/../.././lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 50 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "dir0" == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "dir0/lala" == di.path() );
        }
        {
            const std::string path1_ = "dir0/./dir1/./bbb/../.././lala/.";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 51 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "dir0" == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "dir0/lala" == di.path() );
        }
        {
            const std::string path1_ = "./dir0/./dir1/./bbb/../.././lala/.";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 51 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "dir0" == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "dir0/lala" == di.path() );
        }
        {
            const std::string path1_ = "/./dir0/./dir1/./bbb/../.././lala/.";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 52 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "/dir0" == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "/dir0/lala" == di.path() );
        }

        {
            const std::string path1_ = "../../test_data/file_01_slink09R1.txt";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 60 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "../../test_data" == di.dirname() );
            REQUIRE( "file_01_slink09R1.txt" == di.basename() );
            REQUIRE( path1_ == di.path() );
        }

        {
            const std::string path1_ = "../../../jaulib/test_data";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 61 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "../../../jaulib" == di.dirname() );
            REQUIRE( "test_data" == di.basename() );
            REQUIRE( path1_ == di.path() );
        }

        {
            const std::string path1_ = "../../../../jaulib/test_data";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 62 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "../../../../jaulib" == di.dirname() );
            REQUIRE( "test_data" == di.basename() );
            REQUIRE( path1_ == di.path() );
        }

        {
            const std::string path1_ = "././././jaulib/test_data";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 63 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "jaulib" == di.dirname() );
            REQUIRE( "test_data" == di.basename() );
            REQUIRE( "jaulib/test_data" == di.path() );
        }
        {
            const std::string path1_ = "a/././././jaulib/test_data";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 64 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "a/jaulib" == di.dirname() );
            REQUIRE( "test_data" == di.basename() );
            REQUIRE( "a/jaulib/test_data" == di.path() );
        }

        {
            // Error
            const std::string path1_ = "/../lala";
            const jau::fs::dir_item di(path1_);
            INFO_STR("\n\ntest04_dir_item: 99 '"+path1_+" -> "+di.to_string()+" -> '"+di.path()+"'\n");
            REQUIRE( "/.." == di.dirname() );
            REQUIRE( "lala" == di.basename() );
            REQUIRE( "/../lala" == di.path() );
        }
    }

    void test05_file_stat() {
        errno = 0;
        INFO_STR("\n\ntest05_file_stat\n");

        {
            jau::fs::file_stats stats(project_root_ext+"/file_01.txt");
            jau::fprintf_td(stderr, "test05_file_stat: 01: %s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test05_file_stat: 01: fields %s\n", jau::fs::to_string( stats.fields() ).c_str());
            if( stats.exists() ) {
                REQUIRE(  stats.has_access() );
                REQUIRE( !stats.is_dir() );
                REQUIRE(  stats.is_file() );
                REQUIRE( !stats.is_link() );
                REQUIRE( 15 == stats.size() );
            }
        }

        jau::fs::file_stats proot_stats(project_root1);
        if( !proot_stats.exists() ) {
            proot_stats = jau::fs::file_stats(project_root2);
        }
        jau::fprintf_td(stderr, "test05_file_stat: 11: %s\n", proot_stats.to_string().c_str());
        jau::fprintf_td(stderr, "test05_file_stat: 11: fields %s\n", jau::fs::to_string( proot_stats.fields() ).c_str());
        REQUIRE( true == proot_stats.exists() );
        REQUIRE( true == proot_stats.is_dir() );

        {
            jau::fs::file_stats stats(proot_stats.path()+"/file_01.txt");
            jau::fprintf_td(stderr, "test05_file_stat: 12: %s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test05_file_stat: 12: fields %s\n", jau::fs::to_string( stats.fields() ).c_str());
            REQUIRE(  stats.exists() );
            REQUIRE(  stats.has_access() );
            REQUIRE( !stats.is_dir() );
            REQUIRE(  stats.is_file() );
            REQUIRE( !stats.is_link() );
            REQUIRE( 15 == stats.size() );

            size_t link_count;
            const jau::fs::file_stats* final_target = stats.final_target(&link_count);
            jau::fprintf_td(stderr, "test05_file_stat: 12: final_target (%zu link count): %s\n", link_count, final_target->to_string().c_str());
            REQUIRE( 0 == link_count );
            REQUIRE( final_target == &stats );

            {
                jau::fs::file_stats stats2(proot_stats.path()+"/file_01.txt");
                REQUIRE(  stats2.exists() );
                REQUIRE(  stats2.has_access() );
                REQUIRE( !stats2.is_dir() );
                REQUIRE(  stats2.is_file() );
                REQUIRE( !stats2.is_link() );
                REQUIRE( 15 == stats2.size() );
                REQUIRE( stats == stats2 );
            }
            {
                jau::fs::file_stats stats2(proot_stats.path()+"/dir_01/file_02.txt");
                REQUIRE(  stats2.exists() );
                REQUIRE(  stats2.has_access() );
                REQUIRE( !stats2.is_dir() );
                REQUIRE(  stats2.is_file() );
                REQUIRE( !stats2.is_link() );
                REQUIRE( stats != stats2 );
            }
        }
        {
            jau::fs::file_stats stats(proot_stats.path()+"/dir_01");
            jau::fprintf_td(stderr, "test05_file_stat: 13: %s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test05_file_stat: 13: fields %s\n", jau::fs::to_string( stats.fields() ).c_str());
            REQUIRE(  stats.exists() );
            REQUIRE(  stats.has_access() );
            REQUIRE(  stats.is_dir() );
            REQUIRE( !stats.is_file() );
            REQUIRE( !stats.is_link() );
            REQUIRE( 0 == stats.size() );

            size_t link_count;
            const jau::fs::file_stats* final_target = stats.final_target(&link_count);
            jau::fprintf_td(stderr, "test05_file_stat: 13: final_target (%zu link count): %s\n", link_count, final_target->to_string().c_str());
            REQUIRE( 0 == link_count );
            REQUIRE( final_target == &stats );
        }
        {
            jau::fs::file_stats stats(proot_stats.path()+"/does_not_exist");
            jau::fprintf_td(stderr, "test05_file_stat: 14: %s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test05_file_stat: 14: fields %s\n", jau::fs::to_string( stats.fields() ).c_str());
            REQUIRE( !stats.exists() );
            REQUIRE(  stats.has_access() );
            REQUIRE( !stats.is_dir() );
            REQUIRE( !stats.is_file() );
            REQUIRE( !stats.is_link() );
            REQUIRE( 0 == stats.size() );

            size_t link_count;
            const jau::fs::file_stats* final_target = stats.final_target(&link_count);
            jau::fprintf_td(stderr, "test05_file_stat: 14: final_target (%zu link count): %s\n", link_count, final_target->to_string().c_str());
            REQUIRE( 0 == link_count );
            REQUIRE( final_target == &stats );
        }
    }

    void test06_file_stat_symlinks() {
        errno = 0;
        INFO_STR("\n\ntest06_file_stat_symlinks\n");

        jau::fs::file_stats proot_stats(project_root1);
        if( !proot_stats.exists() ) {
            proot_stats = jau::fs::file_stats(project_root2);
        }
        REQUIRE( true == proot_stats.exists() );
        REQUIRE( true == proot_stats.is_dir() );

        {
            jau::fs::file_stats stats(proot_stats.path()+"/file_01_slink01.txt");
            jau::fprintf_td(stderr, "test06_file_stat_symlinks: 13: %s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test06_file_stat_symlinks: 13: fields %s\n", jau::fs::to_string( stats.fields() ).c_str());
            REQUIRE(  stats.exists() );
            REQUIRE(  stats.has_access() );
            REQUIRE( !stats.is_dir() );
            REQUIRE(  stats.is_file() );
            REQUIRE(  stats.is_link() );
            REQUIRE( 15 == stats.size() );
            REQUIRE( nullptr != stats.link_target_path() );
            REQUIRE( "file_01.txt" == *stats.link_target_path() );

            size_t link_count;
            const jau::fs::file_stats* final_target = stats.final_target(&link_count);
            jau::fprintf_td(stderr, "- final_target (%zu link count): %s\n", link_count, final_target->to_string().c_str());
            REQUIRE( 1 == link_count );
            REQUIRE( final_target != &stats );
            REQUIRE( proot_stats.path()+"/file_01.txt" == final_target->path() );

            REQUIRE( nullptr != stats.link_target() );
            const jau::fs::file_stats* link_target = stats.link_target().get();
            REQUIRE( nullptr != link_target );
            jau::fprintf_td(stderr, "- link_target %s\n", link_target->to_string().c_str());
            REQUIRE( final_target == link_target );
            REQUIRE( !link_target->is_dir() );
            REQUIRE(  link_target->is_file() );
            REQUIRE( !link_target->is_link() );
            REQUIRE( nullptr == link_target->link_target_path() );
            REQUIRE( nullptr == link_target->link_target() );
        }
        {
            jau::fs::file_stats stats(proot_stats.path()+"/fstab_slink07_absolute");
            jau::fprintf_td(stderr, "test06_file_stat_symlinks: 14: %s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test06_file_stat_symlinks: 14: fields %s\n", jau::fs::to_string( stats.fields() ).c_str());
            REQUIRE(  stats.exists() );
            REQUIRE(  stats.has_access() );
            REQUIRE( !stats.is_dir() );
            REQUIRE(  stats.is_file() );
            REQUIRE(  stats.is_link() );
            REQUIRE( 20 < stats.size() ); // greater than basename
            REQUIRE( nullptr != stats.link_target_path() );
            REQUIRE( "/etc/fstab" == *stats.link_target_path() );

            size_t link_count;
            const jau::fs::file_stats* final_target = stats.final_target(&link_count);
            jau::fprintf_td(stderr, "- final_target (%zu link count): %s\n", link_count, final_target->to_string().c_str());
            REQUIRE( 1 == link_count );
            REQUIRE( final_target != &stats );
            REQUIRE( "/etc/fstab" == final_target->path() );

            REQUIRE( nullptr != stats.link_target() );
            const jau::fs::file_stats* link_target = stats.link_target().get();
            REQUIRE( nullptr != link_target );
            jau::fprintf_td(stderr, "- link_target %s\n", link_target->to_string().c_str());
            REQUIRE( final_target == link_target );
            REQUIRE( !link_target->is_dir() );
            REQUIRE(  link_target->is_file() );
            REQUIRE( !link_target->is_link() );
            REQUIRE( nullptr == link_target->link_target_path() );
            REQUIRE( nullptr == link_target->link_target() );
        }
        {
            jau::fs::file_stats stats(proot_stats.path()+"/file_01_slink10R2.txt"); // -> file_01_slink09R1.txt -> file_01_slink01.txt -> file_01.txt
            jau::fprintf_td(stderr, "test06_file_stat_symlinks: 20: %s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test06_file_stat_symlinks: 20: fields %s\n", jau::fs::to_string( stats.fields() ).c_str());
            REQUIRE(  stats.exists() );
            REQUIRE(  stats.has_access() );
            REQUIRE( !stats.is_dir() );
            REQUIRE(  stats.is_file() );
            REQUIRE(  stats.is_link() );
            REQUIRE( 15 == stats.size() );
            REQUIRE( nullptr != stats.link_target_path() );
            REQUIRE( "file_01_slink09R1.txt" == *stats.link_target_path() );

            size_t link_count;
            const jau::fs::file_stats* final_target = stats.final_target(&link_count);
            jau::fprintf_td(stderr, "- final_target (%zu link count): %s\n", link_count, final_target->to_string().c_str());
            REQUIRE( 3 == link_count );
            REQUIRE( final_target != &stats );
            REQUIRE( proot_stats.path()+"/file_01.txt" == final_target->path() );

            REQUIRE( nullptr != stats.link_target() );
            const jau::fs::file_stats* link_target1 = stats.link_target().get();
            jau::fprintf_td(stderr, "- link_target1 %s\n", link_target1->to_string().c_str());
            REQUIRE( final_target != link_target1 );
            REQUIRE( proot_stats.path()+"/file_01_slink09R1.txt" == link_target1->path() );
            REQUIRE( 15 == link_target1->size() );
            REQUIRE( !link_target1->is_dir() );
            REQUIRE(  link_target1->is_file() );
            REQUIRE(  link_target1->is_link() );
            REQUIRE( nullptr != link_target1->link_target_path() );
            REQUIRE( "file_01_slink01.txt" == *link_target1->link_target_path() );
            {
                const jau::fs::file_stats* link_target2 = link_target1->link_target().get();
                REQUIRE( nullptr != link_target2 );
                jau::fprintf_td(stderr, "  - link_target2 %s\n", link_target2->to_string().c_str());
                REQUIRE( final_target != link_target2 );
                REQUIRE( link_target1 != link_target2 );
                REQUIRE( proot_stats.path()+"/file_01_slink01.txt" == link_target2->path() );
                REQUIRE( 15 == link_target2->size() );
                REQUIRE( !link_target2->is_dir() );
                REQUIRE(  link_target2->is_file() );
                REQUIRE(  link_target2->is_link() );
                REQUIRE( nullptr != link_target2->link_target_path() );
                REQUIRE( "file_01.txt" == *link_target2->link_target_path() );

                const jau::fs::file_stats* link_target3 = link_target2->link_target().get();
                REQUIRE( nullptr != link_target3 );
                jau::fprintf_td(stderr, "    - link_target3 %s\n", link_target3->to_string().c_str());
                REQUIRE( final_target == link_target3 );
                REQUIRE( link_target1 != link_target3 );
                REQUIRE( link_target2 != link_target3 );
                REQUIRE( 15 == link_target3->size() );
                REQUIRE( !link_target3->is_dir() );
                REQUIRE(  link_target3->is_file() );
                REQUIRE( !link_target3->is_link() );
                REQUIRE( nullptr == link_target3->link_target_path() );
                REQUIRE( nullptr == link_target3->link_target() );
            }
        }
        {
            jau::fs::file_stats stats(proot_stats.path()+"/dead_link23"); // -> not_existing_file
            jau::fprintf_td(stderr, "test06_file_stat_symlinks: 30: %s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test06_file_stat_symlinks: 30: fields %s\n", jau::fs::to_string( stats.fields() ).c_str());
            REQUIRE( !stats.exists() );
            REQUIRE(  stats.has_access() );
            REQUIRE( !stats.is_dir() );
            REQUIRE( !stats.is_file() );
            REQUIRE(  stats.is_link() );
            REQUIRE( 0 == stats.size() );
            REQUIRE( nullptr != stats.link_target_path() );
            REQUIRE( "not_existing_file" == *stats.link_target_path() );
            REQUIRE( nullptr == stats.link_target() );

            size_t link_count;
            const jau::fs::file_stats* final_target = stats.final_target(&link_count);
            jau::fprintf_td(stderr, "- final_target (%zu link count): %s\n", link_count, final_target->to_string().c_str());
            REQUIRE( 0 == link_count );
            REQUIRE( final_target == &stats );
        }
        {
            jau::fs::file_stats stats(proot_stats.path()+"/dead_link22"); // LOOP: dead_link22 -> dead_link21 -> dead_link20 -> dead_link22 ...
            jau::fprintf_td(stderr, "test06_file_stat_symlinks: 31: %s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test06_file_stat_symlinks: 31: fields %s\n", jau::fs::to_string( stats.fields() ).c_str());
            REQUIRE( !stats.exists() );
            REQUIRE(  stats.has_access() );
            REQUIRE( !stats.is_dir() );
            REQUIRE( !stats.is_file() );
            REQUIRE(  stats.is_link() );
            REQUIRE( 0 == stats.size() );
            REQUIRE( nullptr != stats.link_target_path() );
            REQUIRE( "dead_link21" == *stats.link_target_path() );
            REQUIRE( nullptr == stats.link_target() );

            size_t link_count;
            const jau::fs::file_stats* final_target = stats.final_target(&link_count);
            jau::fprintf_td(stderr, "- final_target (%zu link count): %s\n", link_count, final_target->to_string().c_str());
            REQUIRE( 0 == link_count );
            REQUIRE( *final_target == stats );
            REQUIRE( final_target == &stats );
        }
    }

    static void test_file_stat_fd_item(const jau::fs::fmode_t exp_type, const int fd, const std::string& named_fd1, const std::string& named_fd_link) {
        jau::fprintf_td(stderr, "test_file_stat_fd_item: expect '%s', fd %d, name_fd1 '%s', make_fd_link '%s'\n",
                jau::fs::to_string(exp_type).c_str(), fd, named_fd1.c_str(), named_fd_link.c_str());
        {
            errno = 0;
            jau::fprintf_td(stderr, "test_file_stat_fd_item.1: expect '%s', fd %d\n", jau::fs::to_string(exp_type).c_str(), fd);
            jau::fs::file_stats stats(fd);
            jau::fprintf_td(stderr, "fd: %s\n", stats.to_string().c_str());
            REQUIRE(  stats.exists() );
            REQUIRE(  stats.has_access() );
            REQUIRE( !stats.is_socket() );
            REQUIRE( !stats.is_block() );
            REQUIRE( !stats.is_dir() );
            if( jau::fs::fmode_t::none == ( stats.type_mode() & exp_type ) ) {
                jau::fprintf_td(stderr, "INFO: Not matching expected type '%s': fd: %s\n", jau::fs::to_string(exp_type).c_str(), stats.to_string().c_str());
            }
            REQUIRE(  stats.has_fd() );
            REQUIRE(  fd == stats.fd() );
            if( !stats.is_file() ) {
                REQUIRE( 0 == stats.size() );
            }
            jau::fprintf_td(stderr, "test_file_stat_fd_item.1: X\n\n");
        }
        if( !named_fd1.empty() ) {
            errno = 0;
            jau::fprintf_td(stderr, "test_file_stat_fd_item.2: expect '%s', fd %d, name_fd1 '%s'\n",
                    jau::fs::to_string(exp_type).c_str(), fd, named_fd1.c_str());
            jau::fs::file_stats stats(named_fd1);
            jau::fprintf_td(stderr, "fd_1: %s\n", stats.to_string().c_str());
            REQUIRE(  stats.exists() );
            REQUIRE(  stats.has_access() );
            REQUIRE( !stats.is_socket() );
            REQUIRE( !stats.is_block() );
            REQUIRE( !stats.is_dir() );
            if( jau::fs::fmode_t::none == ( stats.type_mode() & exp_type ) ) {
                jau::fprintf_td(stderr, "INFO: Not matching expected type '%s': fd: %s\n", jau::fs::to_string(exp_type).c_str(), stats.to_string().c_str());
            }
            REQUIRE(  stats.has_fd() );
            REQUIRE(  fd == stats.fd() );
            if( !stats.is_file() ) {
                REQUIRE( 0 == stats.size() );
            }
            jau::fprintf_td(stderr, "test_file_stat_fd_item.2: X\n\n");
        }
        if( !named_fd_link.empty() ) {
            errno = 0;
            jau::fprintf_td(stderr, "test_file_stat_fd_item.3: expect '%s', fd %d, make_fd_link '%s'\n",
                    jau::fs::to_string(exp_type).c_str(), fd, named_fd_link.c_str());
            jau::fs::file_stats stats(named_fd_link);
            jau::fprintf_td(stderr, "fd_link: %s\n", stats.to_string().c_str());
            REQUIRE(  stats.exists() );
            REQUIRE(  stats.has_access() );
            REQUIRE( !stats.is_socket() );
            REQUIRE( !stats.is_block() );
            REQUIRE( !stats.is_dir() );
            if( jau::fs::fmode_t::none == ( stats.type_mode() & exp_type ) ) {
                jau::fprintf_td(stderr, "INFO: Not matching expected type '%s': fd: %s\n", jau::fs::to_string(exp_type).c_str(), stats.to_string().c_str());
            }
            REQUIRE(  stats.has_fd() );
            REQUIRE(  fd == stats.fd() );
            if( !stats.is_file() ) {
                REQUIRE( 0 == stats.size() );
            }

            size_t link_count;
            const jau::fs::file_stats* final_target = stats.final_target(&link_count);
            REQUIRE( nullptr != final_target );
            jau::fprintf_td(stderr, "- final_target (%zu link count): %s\n", link_count, final_target->to_string().c_str());
            REQUIRE( 1 <= link_count );
            REQUIRE( 2 >= link_count );
            jau::fprintf_td(stderr, "test_file_stat_fd_item.3: X\n\n");
        }
    }

    void test07_file_stat_fd() {
        jau::fprintf_td(stderr, "test07_file_stat_fd\n");

        const std::string fd_stdin_1 = "/dev/fd/0";
        const std::string fd_stdout_1 = "/dev/fd/1";
        const std::string fd_stderr_1 = "/dev/fd/2";

        const std::string fd_stdin_l = "/dev/stdin";
        const std::string fd_stdout_l = "/dev/stdout";
        const std::string fd_stderr_l = "/dev/stderr";

        test_file_stat_fd_item(jau::fs::fmode_t::chr, 0, fd_stdin_1, fd_stdin_l);
        test_file_stat_fd_item(jau::fs::fmode_t::chr, 1, fd_stdout_1, fd_stdout_l);
        test_file_stat_fd_item(jau::fs::fmode_t::chr, 2, fd_stderr_1, fd_stderr_l);
        {
            int fd = ::open("test07_file_stat_fd_tmp", O_CREAT|O_WRONLY|O_NOCTTY, S_IRUSR | S_IWUSR | S_IRGRP );
            REQUIRE( 0 <= fd );
            test_file_stat_fd_item(jau::fs::fmode_t::file, fd, jau::fs::to_named_fd(fd), "");
            ::close(fd);
        }
        {
            int pipe_fds[2];
            REQUIRE( 0 == ::pipe(pipe_fds) );
            test_file_stat_fd_item(jau::fs::fmode_t::fifo, pipe_fds[0], jau::fs::to_named_fd(pipe_fds[0]), "");
            test_file_stat_fd_item(jau::fs::fmode_t::fifo, pipe_fds[1], jau::fs::to_named_fd(pipe_fds[1]), "");
            ::close(pipe_fds[0]);
            ::close(pipe_fds[1]);
        }
    }

    // 128 bytes
    static constexpr const char* pipe_msg = "Therefore I say unto you, Take no thought for your life, what ye shall eat, or what ye shall drink; nor yet for your body, what.";
    static const size_t pipe_msg_len = 128;
    static const size_t pipe_msg_count = 10;

    void test08_pipe_01() {
        errno = 0;
        jau::fprintf_td(stderr, "test08_pipe_01\n");

        int pipe_fds[2];
        REQUIRE( 0 == ::pipe(pipe_fds) );
        ::pid_t pid = ::fork();

        if( 0 == pid ) {
            // child process: WRITE
            ::close(pipe_fds[0]); // close unused read end
            const int new_stdout = pipe_fds[1];
            const std::string fd_stdout = jau::fs::to_named_fd(new_stdout);

            jau::fs::file_stats stats_stdout(fd_stdout);
            jau::fprintf_td(stderr, "Child: stats_stdout %s\n", stats_stdout.to_string().c_str());
            if( !stats_stdout.exists() || !stats_stdout.has_fd() || new_stdout != stats_stdout.fd() ) {
                jau::fprintf_td(stderr, "Child: Error: stats_stdout %s\n", stats_stdout.to_string().c_str());
                ::_exit(EXIT_FAILURE);
            }
            jau::io::ByteOutStream_File outfile(fd_stdout);
            if( !outfile.good() || !outfile.is_open() ) {
                jau::fprintf_td(stderr, "Child: Error: outfile bad: %s\n", outfile.to_string().c_str());
                ::_exit(EXIT_FAILURE);
            }

            // throttled w/ 64 bytes per 8_ms, i.e. 1280 / 64 * 8_ms ~ 160_ms (~20 chunks)
            const size_t max_chunck = 64;
            for(size_t count=0; count < pipe_msg_count; ++count) {
                size_t sent=0;
                while( sent < pipe_msg_len && !outfile.fail() ) {
                    const size_t chunck_sz = std::min(max_chunck, pipe_msg_len-sent);
                    if( chunck_sz != outfile.write(pipe_msg+sent, chunck_sz) ) {
                        count = pipe_msg_count;
                        break;
                    }
                    sent += chunck_sz;
                    jau::sleep_for( 8_ms );
                }
            }

            outfile.close();
            ::close(pipe_fds[1]);

            if( outfile.fail() ) {
                jau::fprintf_td(stderr, "Child: Error: outfile failed after write/closure: %s\n", outfile.to_string().c_str());
                ::_exit(EXIT_FAILURE);
            }
            jau::fprintf_td(stderr, "Child: Done\n");
            ::_exit(EXIT_SUCCESS);

        } else if( 0 < pid ) {
            // parent process: READ
            ::close(pipe_fds[1]); // close unused write end
            const int new_stdin = pipe_fds[0]; // dup2(fd[0], 0);
            const std::string fd_stdin = jau::fs::to_named_fd(new_stdin);

            jau::fs::file_stats stats_stdin(fd_stdin);
            jau::fprintf_td(stderr, "Parent: stats_stdin %s\n", stats_stdin.to_string().c_str());
            REQUIRE(  stats_stdin.exists() );
            REQUIRE(  stats_stdin.has_access() );
            REQUIRE( !stats_stdin.is_socket() );
            REQUIRE( !stats_stdin.is_block() );
            REQUIRE( !stats_stdin.is_dir() );
            REQUIRE( !stats_stdin.is_file() );
            const bool fifo_or_char = stats_stdin.is_fifo() || stats_stdin.is_char();
            REQUIRE(  true == fifo_or_char );
            REQUIRE(  stats_stdin.has_fd() );
            REQUIRE(  new_stdin == stats_stdin.fd() );
            REQUIRE( 0 == stats_stdin.size() );

            // capture stdin
            jau::io::ByteInStream_File infile(fd_stdin);
            jau::fprintf_td(stderr, "Parent: infile %s\n", infile.to_string().c_str());
            REQUIRE( !infile.fail() );

            uint8_t buffer[pipe_msg_count * pipe_msg_len + 512];
            ::bzero(buffer, sizeof(buffer));
            size_t total_read = 0;
            {
                while( !infile.end_of_data() && total_read < sizeof(buffer) ) {
                    const size_t got = infile.read(buffer+total_read, sizeof(buffer)-total_read);
                    jau::fprintf_td(stderr, "Parent: infile.a_ %s\n", infile.to_string().c_str());
                    REQUIRE( !infile.fail() );
                    total_read += got;
                    jau::fprintf_td(stderr, "Parent: Got %zu -> %zu\n", got, total_read);
                }
                jau::fprintf_td(stderr, "Parent: infile.a_1 %s\n", infile.to_string().c_str());
                infile.close();
                jau::fprintf_td(stderr, "Parent: infile.a_2 %s\n", infile.to_string().c_str());
                ::close(pipe_fds[0]);
                jau::fprintf_td(stderr, "Parent: infile.a_3 %s\n", infile.to_string().c_str());
                REQUIRE( !infile.fail() );
            }
            // check actual transmitted content
            REQUIRE( total_read == pipe_msg_len*pipe_msg_count);
            for(size_t count=0; count < pipe_msg_count; ++count) {
                REQUIRE( 0 == ::memcmp(pipe_msg, buffer+(count*pipe_msg_len), pipe_msg_len) );
            }

            int pid_status = 0;
            ::pid_t child_pid = ::waitpid(pid, &pid_status, 0);
            if( 0 > child_pid ) {
                jau::fprintf_td(stderr, "Parent: Error: wait(%d) failed: child_pid %d\n", pid, child_pid);
                REQUIRE_MSG("wait for child failed", false);
            } else {
                if( child_pid != pid ) {
                    jau::fprintf_td(stderr, "Parent: Error: wait(%d) terminated child_pid pid %d\n", pid, child_pid);
                    REQUIRE(child_pid == pid);
                }
                if( !WIFEXITED(pid_status) ) {
                    jau::fprintf_td(stderr, "Parent: Error: wait(%d) terminated abnormally child_pid %d, pid_status %d\n", pid, child_pid, pid_status);
                    REQUIRE(true == WIFEXITED(pid_status));
                }
                if( EXIT_SUCCESS != WEXITSTATUS(pid_status) ) {
                    jau::fprintf_td(stderr, "Parent: Error: wait(%d) exit with failure child_pid %d, exit_code %d\n", pid, child_pid, WEXITSTATUS(pid_status));
                    REQUIRE(EXIT_SUCCESS == WEXITSTATUS(pid_status));
                }
            }
        } else {
            // fork failed
            jau::fprintf_td(stderr, "fork failed %d, %s\n", errno, ::strerror(errno));
            REQUIRE_MSG( "fork failed", false );
        }
    }

    /**
     *
     */
    void test10_mkdir() {
        INFO_STR("\n\ntest10_mkdir\n");

        jau::fs::remove(root, jau::fs::traverse_options::recursive); // start fresh
        {
            jau::fs::file_stats root_stats(root);
            INFO_STR("root_stats.pre: "+root_stats.to_string()+"\n");
            REQUIRE( !root_stats.exists() );
            REQUIRE( root_stats.has_access() );
            REQUIRE( !root_stats.is_dir() );
            REQUIRE( !root_stats.is_file() );
            REQUIRE( !root_stats.is_link() );
        }
        REQUIRE( true == jau::fs::mkdir(root, jau::fs::fmode_t::def_dir_prot) );
        {
            jau::fs::file_stats root_stats(root);
            INFO_STR("root_stats.post: "+root_stats.to_string()+"\n");
            REQUIRE( root_stats.exists() );
            REQUIRE( root_stats.has_access() );
            REQUIRE( root_stats.is_dir() );
            REQUIRE( !root_stats.is_file() );
            REQUIRE( !root_stats.is_link() );
        }
        REQUIRE( false == jau::fs::remove(root, jau::fs::traverse_options::none) );
        REQUIRE( true == jau::fs::remove(root, jau::fs::traverse_options::recursive) );
    }

    void test11_touch() {
        const jau::fraction_timespec td_1s(1, 0);

        INFO_STR("\n\ntest11_touch\n");
        const std::string file_01 = root+"/data01.txt";
        const std::string file_02 = root+"/data02.txt";
        REQUIRE( true == jau::fs::mkdir(root, jau::fs::fmode_t::def_dir_prot) );
        {
            jau::fs::file_stats root_stats(root);
            jau::fprintf_td(stderr, "root_stats1.post: %s\n", root_stats.to_string().c_str());
            REQUIRE( root_stats.exists() );
            REQUIRE( root_stats.has_access() );
            REQUIRE( root_stats.is_dir() );
            REQUIRE( !root_stats.is_file() );
            REQUIRE( !root_stats.is_link() );
        }

        REQUIRE( true == jau::fs::touch(file_01, jau::fs::fmode_t::def_file_prot) );
        {
            const jau::fraction_timespec now = jau::getWallClockTime();
            jau::fs::file_stats file_stats(file_01);
            jau::fprintf_td(stderr, "file_stats2.post: %s\n", file_stats.to_string().c_str());
            const jau::fraction_timespec btime = file_stats.btime();
            const jau::fraction_timespec atime = file_stats.atime();
            const jau::fraction_timespec atime_td = jau::abs(now - atime);
            const jau::fraction_timespec mtime = file_stats.mtime();
            const jau::fraction_timespec mtime_td = jau::abs(now - mtime);
            jau::fprintf_td(stderr, "now:   %s, %s\n", now.to_iso8601_string().c_str(), now.to_string().c_str());
            jau::fprintf_td(stderr, "btime: %s, %s\n", btime.to_iso8601_string().c_str(), btime.to_string().c_str());
            jau::fprintf_td(stderr, "atime: %s, %s, td_now %s\n",
                    atime.to_iso8601_string().c_str(), atime.to_string().c_str(), atime_td.to_string().c_str());
            jau::fprintf_td(stderr, "mtime: %s, %s, td_now %s\n",
                    mtime.to_iso8601_string().c_str(), mtime.to_string().c_str(), mtime_td.to_string().c_str());
            REQUIRE( file_stats.exists() );
            REQUIRE( file_stats.has_access() );
            REQUIRE( !file_stats.is_dir() );
            REQUIRE( file_stats.is_file() );
            REQUIRE( !file_stats.is_link() );
            if( file_stats.has( jau::fs::file_stats::field_t::atime ) ) {
                REQUIRE( td_1s >= atime_td );
            }
            if( file_stats.has( jau::fs::file_stats::field_t::mtime ) ) {
                REQUIRE( td_1s >= mtime_td );
            }
        }

        REQUIRE( true == jau::fs::touch(file_02, jau::fs::fmode_t::def_file_prot) );
        {
            const jau::fraction_timespec now = jau::getWallClockTime();
            jau::fs::file_stats file_stats_pre(file_02);
            const jau::fraction_timespec btime_pre = file_stats_pre.btime();
            const jau::fraction_timespec atime_pre = file_stats_pre.atime();
            const jau::fraction_timespec atime_td = jau::abs(now - atime_pre);
            const jau::fraction_timespec mtime_pre = file_stats_pre.mtime();
            const jau::fraction_timespec mtime_td = jau::abs(now - mtime_pre);
            jau::fprintf_td(stderr, "now      : %s, %s\n", now.to_iso8601_string().c_str(), now.to_string().c_str());
            jau::fprintf_td(stderr, "btime.pre: %s, %s\n", btime_pre.to_iso8601_string().c_str(), btime_pre.to_string().c_str());
            jau::fprintf_td(stderr, "atime.pre: %s, %s, td_now %s\n",
                    atime_pre.to_iso8601_string().c_str(), atime_pre.to_string().c_str(), atime_td.to_string().c_str());
            jau::fprintf_td(stderr, "mtime.pre: %s, %s, td_now %s\n",
                    mtime_pre.to_iso8601_string().c_str(), mtime_pre.to_string().c_str(), mtime_td.to_string().c_str());
            if( file_stats_pre.has( jau::fs::file_stats::field_t::atime ) ) {
                REQUIRE( td_1s >= atime_td );
            }
            if( file_stats_pre.has( jau::fs::file_stats::field_t::mtime ) ) {
                REQUIRE( td_1s >= mtime_td );
            }

            const jau::fraction_timespec ts_20200101( 1577836800_s + 0_h); // 2020-01-01 00:00:00
            const jau::fraction_timespec atime_set( ts_20200101 +  1_d + 10_h );
            const jau::fraction_timespec mtime_set( ts_20200101 + 31_d + 10_h );
            INFO_STR("atime.set: "+atime_set.to_iso8601_string()+", "+atime_set.to_string()+"\n");
            INFO_STR("mtime.set: "+mtime_set.to_iso8601_string()+", "+mtime_set.to_string()+"\n");
            REQUIRE( true == jau::fs::touch(file_02, atime_set, mtime_set, jau::fs::fmode_t::def_file_prot) );

            jau::fs::file_stats file_stats_post(file_02);
            const jau::fraction_timespec atime_post = file_stats_post.atime();
            const jau::fraction_timespec mtime_post = file_stats_post.mtime();
            INFO_STR("atime.post: "+atime_post.to_iso8601_string()+", "+atime_post.to_string()+"\n");
            INFO_STR("mtime.post: "+mtime_post.to_iso8601_string()+", "+mtime_post.to_string()+"\n");
            jau::fprintf_td(stderr, "test11_touch: 03: %s\n", file_stats_post.to_string().c_str());
            {
                REQUIRE( file_stats_post.exists() );
                REQUIRE( file_stats_post.has_access() );
                REQUIRE( !file_stats_post.is_dir() );
                REQUIRE( file_stats_post.is_file() );
                REQUIRE( !file_stats_post.is_link() );
                if( file_stats_post.has( jau::fs::file_stats::field_t::atime ) ) {
                    REQUIRE( atime_set == file_stats_post.atime() );
                }
                if( file_stats_post.has( jau::fs::file_stats::field_t::mtime ) ) {
                    REQUIRE( mtime_set == file_stats_post.mtime() );
                }
            }
        }

        REQUIRE( true == jau::fs::remove(root, jau::fs::traverse_options::recursive) );
    }


    void test20_visit() {
        INFO_STR("\n\ntest20_visit\n");

        std::string sub_dir1 = root+"/sub1";
        std::string sub_dir2 = root+"/sub2";
        std::string sub_dir3 = root+"/sub1/sub3";

        REQUIRE( true == jau::fs::mkdir(root, jau::fs::fmode_t::def_dir_prot) );
        REQUIRE( true == jau::fs::touch(root+"/data01.txt") );
        REQUIRE( true == jau::fs::touch(root+"/data02.txt") );
        REQUIRE( true == jau::fs::mkdir(sub_dir1, jau::fs::fmode_t::def_dir_prot) );
        REQUIRE( true == jau::fs::mkdir(sub_dir2, jau::fs::fmode_t::def_dir_prot) );
        REQUIRE( true == jau::fs::mkdir(sub_dir3, jau::fs::fmode_t::def_dir_prot) );
        REQUIRE( true == jau::fs::touch(sub_dir1+"/data03.txt") );
        REQUIRE( true == jau::fs::touch(sub_dir1+"/data04.txt") );
        REQUIRE( true == jau::fs::touch(sub_dir2+"/data05.txt") );
        REQUIRE( true == jau::fs::touch(sub_dir2+"/data06.txt") );
        REQUIRE( true == jau::fs::touch(sub_dir3+"/data07.txt") );
        REQUIRE( true == jau::fs::touch(sub_dir3+"/data08.txt") );

        const jau::fs::traverse_options topts_R_FSL_PDL = jau::fs::traverse_options::recursive |
                                                          jau::fs::traverse_options::follow_symlinks |
                                                          jau::fs::traverse_options::dir_exit;
        visitor_stats stats_R_FSL_PDL(topts_R_FSL_PDL);
        {
            const jau::fs::path_visitor pv = jau::bind_capref(&stats_R_FSL_PDL,
                    ( bool(*)(visitor_stats*, jau::fs::traverse_event, const jau::fs::file_stats&) ) /* help template type deduction of function-ptr */
                        ( [](visitor_stats* stats_ptr, jau::fs::traverse_event tevt, const jau::fs::file_stats& element_stats) -> bool {
                            (void)tevt;
                            stats_ptr->add(element_stats);
                            return true;
                          } ) );
            REQUIRE( true == jau::fs::visit(root, topts_R_FSL_PDL, pv) );
            jau::fprintf_td(stderr, "test20_visit[R, FSL, PDL]: %s\n%s\n", to_string(topts_R_FSL_PDL).c_str(), stats_R_FSL_PDL.to_string().c_str());
            REQUIRE( 12 == stats_R_FSL_PDL.total_real );
            REQUIRE(  0 == stats_R_FSL_PDL.total_sym_links_existing );
            REQUIRE(  0 == stats_R_FSL_PDL.total_sym_links_not_existing );
            REQUIRE(  0 == stats_R_FSL_PDL.total_no_access );
            REQUIRE(  0 == stats_R_FSL_PDL.total_not_existing );
            REQUIRE(  0 == stats_R_FSL_PDL.total_file_bytes );
            REQUIRE(  8 == stats_R_FSL_PDL.files_real );
            REQUIRE(  0 == stats_R_FSL_PDL.files_sym_link );
            REQUIRE(  4 == stats_R_FSL_PDL.dirs_real );
            REQUIRE(  0 == stats_R_FSL_PDL.dirs_sym_link );
        }
        const jau::fs::traverse_options topts_R_FSL = jau::fs::traverse_options::recursive |
                                                      jau::fs::traverse_options::follow_symlinks |
                                                      jau::fs::traverse_options::dir_entry;
        visitor_stats stats_R_FSL(topts_R_FSL);
        {
            const jau::fs::path_visitor pv = jau::bind_capref(&stats_R_FSL,
                    ( bool(*)(visitor_stats*, jau::fs::traverse_event, const jau::fs::file_stats&) ) /* help template type deduction of function-ptr */
                        ( [](visitor_stats* stats_ptr, jau::fs::traverse_event tevt, const jau::fs::file_stats& element_stats) -> bool {
                            (void)tevt;
                            stats_ptr->add(element_stats);
                            return true;
                          } ) );
            REQUIRE( true == jau::fs::visit(root, topts_R_FSL, pv) );
            jau::fprintf_td(stderr, "test20_visit[R, FSL]: %s\n%s\n", to_string(topts_R_FSL).c_str(), stats_R_FSL.to_string().c_str());
            REQUIRE( stats_R_FSL_PDL == stats_R_FSL );
        }

        REQUIRE( true == jau::fs::remove(root, jau::fs::traverse_options::recursive) );
    }

    void test22_visit_symlinks() {
        INFO_STR("\n\ntest22_visit_symlinks\n");

        jau::fs::file_stats proot_stats(project_root1);
        if( !proot_stats.exists() ) {
            proot_stats = jau::fs::file_stats(project_root2);
        }
        REQUIRE( true == proot_stats.exists() );

        {
            const jau::fs::traverse_options topts = jau::fs::traverse_options::recursive |
                                                    jau::fs::traverse_options::dir_entry;
            visitor_stats stats(topts);
            const jau::fs::path_visitor pv = jau::bind_capref(&stats,
                    ( bool(*)(visitor_stats*, jau::fs::traverse_event, const jau::fs::file_stats&) ) /* help template type deduction of function-ptr */
                        ( [](visitor_stats* stats_ptr, jau::fs::traverse_event tevt, const jau::fs::file_stats& element_stats) -> bool {
                            (void)tevt;
                            stats_ptr->add(element_stats);
                            return true;
                          } ) );
            REQUIRE( true == jau::fs::visit(proot_stats, topts, pv) );
            jau::fprintf_td(stderr, "test22_visit[R]: %s\n%s\n", to_string(topts).c_str(), stats.to_string().c_str());
            REQUIRE(  7 == stats.total_real );
            REQUIRE( 10 == stats.total_sym_links_existing );
            REQUIRE(  4 == stats.total_sym_links_not_existing );
            REQUIRE(  0 == stats.total_no_access );
            REQUIRE(  4 == stats.total_not_existing );
            REQUIRE( 60 == stats.total_file_bytes );
            REQUIRE(  4 == stats.files_real );
            REQUIRE(  9 == stats.files_sym_link );
            REQUIRE(  3 == stats.dirs_real );
            REQUIRE(  1 == stats.dirs_sym_link );
        }
        {
            const jau::fs::traverse_options topts = jau::fs::traverse_options::recursive |
                                                    jau::fs::traverse_options::dir_entry |
                                                    jau::fs::traverse_options::follow_symlinks;
            visitor_stats stats(topts);
            const jau::fs::path_visitor pv = jau::bind_capref(&stats,
                    ( bool(*)(visitor_stats*, jau::fs::traverse_event, const jau::fs::file_stats&) ) /* help template type deduction of function-ptr */
                        ( [](visitor_stats* stats_ptr, jau::fs::traverse_event tevt, const jau::fs::file_stats& element_stats) -> bool {
                            (void)tevt;
                            stats_ptr->add(element_stats);
                            return true;
                          } ) );
            REQUIRE( true == jau::fs::visit(proot_stats, topts, pv) );
            jau::fprintf_td(stderr, "test22_visit[R, FSL]: %s\n%s\n", to_string(topts).c_str(), stats.to_string().c_str());
            REQUIRE(  9 == stats.total_real );
            REQUIRE( 11 == stats.total_sym_links_existing );
            REQUIRE(  4 == stats.total_sym_links_not_existing );
            REQUIRE(  0 == stats.total_no_access );
            REQUIRE(  4 == stats.total_not_existing );
            REQUIRE( 60 <  stats.total_file_bytes ); // some followed symlink files are of unknown size, e.g. /etc/fstab
            REQUIRE(  6 == stats.files_real );
            REQUIRE( 10 == stats.files_sym_link );
            REQUIRE(  3 == stats.dirs_real );
            REQUIRE(  1 == stats.dirs_sym_link );
        }
    }

    void test30_copy_file2dir() {
        INFO_STR("\n\ntest30_copy_file2dir\n");

        jau::fs::file_stats root_orig_stats(project_root1);
        if( !root_orig_stats.exists() ) {
            root_orig_stats = jau::fs::file_stats(project_root2);
        }
        REQUIRE( true == root_orig_stats.exists() );

        const std::string root_copy = root+"_copy_test30";
        {
            // Fresh target folder
            jau::fs::remove(root_copy, jau::fs::traverse_options::recursive);

            REQUIRE( true == jau::fs::mkdir(root_copy, jau::fs::fmode_t::def_dir_prot) );
            {
                jau::fs::file_stats stats(root_copy);
                REQUIRE( true == stats.exists() );
                REQUIRE( true == stats.ok() );
                REQUIRE( true == stats.is_dir() );
            }
        }
        jau::fs::file_stats source1_stats(root_orig_stats.path()+"/file_01.txt");
        jau::fprintf_td(stderr, "test30_copy_file2dir: source1: %s\n", source1_stats.to_string().c_str());
        {
            REQUIRE( true == source1_stats.exists() );
            REQUIRE( true == source1_stats.ok() );
            REQUIRE( true == source1_stats.is_file() );
        }
        {
            // Copy file to folder
            const jau::fs::copy_options copts = jau::fs::copy_options::preserve_all |
                                                jau::fs::copy_options::verbose;
            {
                jau::fs::file_stats dest_stats(root_copy+"/file_01.txt");
                jau::fprintf_td(stderr, "test30_copy_file2dir: 01: dest.pre: %s\n", dest_stats.to_string().c_str());
                REQUIRE( false == dest_stats.exists() );
            }
            REQUIRE( true == jau::fs::copy(source1_stats.path(), root_copy, copts) );
            {
                jau::fs::file_stats dest_stats(root_copy+"/file_01.txt");
                jau::fprintf_td(stderr, "test30_copy_file2dir: 01: dest.post: %s\n", dest_stats.to_string().c_str());
                REQUIRE( true == dest_stats.exists() );
                REQUIRE( true == dest_stats.ok() );
                REQUIRE( true == dest_stats.is_file() );
                REQUIRE( source1_stats.size() == dest_stats.size() );
                REQUIRE( source1_stats.mode() == dest_stats.mode() );
            }
        }
        {
            // Error: already exists of 'Copy file to folder'
            const jau::fs::copy_options copts = jau::fs::copy_options::preserve_all |
                                                jau::fs::copy_options::verbose;
            {
                jau::fs::file_stats dest_stats(root_copy+"/file_01.txt");
                jau::fprintf_td(stderr, "test30_copy_file2dir: 02: dest.pre: %s\n", dest_stats.to_string().c_str());
                REQUIRE( true == dest_stats.exists() );
                REQUIRE( true == dest_stats.ok() );
                REQUIRE( true == dest_stats.is_file() );
            }
            REQUIRE( false == jau::fs::copy(source1_stats.path(), root_copy, copts) );
        }
        {
            // Overwrite copy file to folder
            const jau::fs::copy_options copts = jau::fs::copy_options::preserve_all |
                                                jau::fs::copy_options::overwrite |
                                                jau::fs::copy_options::verbose;

            jau::fprintf_td(stderr, "test30_copy_file2dir: 03: source: %s\n", source1_stats.to_string().c_str());
            {
                jau::fs::file_stats dest_stats(root_copy+"/file_01.txt");
                jau::fprintf_td(stderr, "test30_copy_file2dir: 03: dest.pre: %s\n", dest_stats.to_string().c_str());
                REQUIRE( true == dest_stats.exists() );
                REQUIRE( true == dest_stats.ok() );
                REQUIRE( true == dest_stats.is_file() );
                REQUIRE( source1_stats.size() == dest_stats.size() );
                REQUIRE( source1_stats.mode() == dest_stats.mode() );
            }
            REQUIRE( true == jau::fs::copy(source1_stats.path(), root_copy, copts) );
            {
                jau::fs::file_stats dest_stats(root_copy+"/file_01.txt");
                jau::fprintf_td(stderr, "test30_copy_file2dir: 03: dest.post: %s\n", dest_stats.to_string().c_str());
                REQUIRE( true == dest_stats.exists() );
                REQUIRE( true == dest_stats.ok() );
                REQUIRE( true == dest_stats.is_file() );
                REQUIRE( source1_stats.size() == dest_stats.size() );
                REQUIRE( source1_stats.mode() == dest_stats.mode() );
            }
        }
        if constexpr ( _remove_target_test_dir ) {
            REQUIRE( true == jau::fs::remove(root_copy, jau::fs::traverse_options::recursive) );
        }
    }

    void test31_copy_file2file() {
        INFO_STR("\n\ntest31_copy_file2file\n");

        jau::fs::file_stats root_orig_stats(project_root1);
        if( !root_orig_stats.exists() ) {
            root_orig_stats = jau::fs::file_stats(project_root2);
        }
        REQUIRE( true == root_orig_stats.exists() );

        const std::string root_copy = root+"_copy_test31";
        {
            // Fresh target folder
            jau::fs::remove(root_copy, jau::fs::traverse_options::recursive);

            REQUIRE( true == jau::fs::mkdir(root_copy, jau::fs::fmode_t::def_dir_prot) );
            {
                jau::fs::file_stats stats(root_copy);
                REQUIRE( true == stats.exists() );
                REQUIRE( true == stats.ok() );
                REQUIRE( true == stats.is_dir() );
            }
        }
        jau::fs::file_stats source1_stats(root_orig_stats.path()+"/file_01.txt");
        jau::fprintf_td(stderr, "test31_copy_file2file: source1: %s\n", source1_stats.to_string().c_str());
        {
            REQUIRE( true == source1_stats.exists() );
            REQUIRE( true == source1_stats.ok() );
            REQUIRE( true == source1_stats.is_file() );
        }
        jau::fs::file_stats source2_stats(root_orig_stats.path()+"/README_slink08_relext.txt");
        jau::fprintf_td(stderr, "test31_copy_file2file: source2: %s\n", source2_stats.to_string().c_str());
        {
            REQUIRE( true == source2_stats.exists() );
            REQUIRE( true == source2_stats.ok() );
            REQUIRE( true == source2_stats.is_file() );
            REQUIRE( true == source2_stats.is_link() );
        }
        {
            // Copy file to new file-name
            const jau::fs::copy_options copts = jau::fs::copy_options::preserve_all |
                                                jau::fs::copy_options::verbose;
            {
                jau::fs::file_stats dest_stats(root_copy+"/file_10.txt");
                jau::fprintf_td(stderr, "test31_copy_file2file: 10: dest.pre: %s\n", dest_stats.to_string().c_str());
                REQUIRE( false == dest_stats.exists() );
            }
            REQUIRE( true == jau::fs::copy(source1_stats.path(), root_copy+"/file_10.txt", copts) );
            jau::fs::sync(); // just check API
            {
                jau::fs::file_stats dest_stats(root_copy+"/file_10.txt");
                jau::fprintf_td(stderr, "test31_copy_file2file: 10: dest.post: %s\n", dest_stats.to_string().c_str());
                REQUIRE( true == dest_stats.exists() );
                REQUIRE( true == dest_stats.ok() );
                REQUIRE( true == dest_stats.is_file() );
                REQUIRE( source1_stats.size() == dest_stats.size() );
                REQUIRE( source1_stats.mode() == dest_stats.mode() );
            }
        }
        {
            // Error: already exists of 'Copy file to file'
            const jau::fs::copy_options copts = jau::fs::copy_options::preserve_all |
                                                jau::fs::copy_options::verbose;
            {
                jau::fs::file_stats dest_stats(root_copy+"/file_10.txt");
                jau::fprintf_td(stderr, "test31_copy_file2file: 11: dest.pre: %s\n", dest_stats.to_string().c_str());
                REQUIRE( true == dest_stats.exists() );
                REQUIRE( true == dest_stats.ok() );
                REQUIRE( true == dest_stats.is_file() );
            }
            REQUIRE( false == jau::fs::copy(source1_stats.path(), root_copy+"/file_10.txt", copts) );
            jau::fs::sync(); // just check API
        }
        {
            // Overwrite copy file to file
            const jau::fs::copy_options copts = jau::fs::copy_options::preserve_all |
                                                jau::fs::copy_options::overwrite |
                                                jau::fs::copy_options::follow_symlinks |
                                                jau::fs::copy_options::verbose;

            {
                jau::fs::file_stats dest_stats(root_copy+"/file_10.txt");
                jau::fprintf_td(stderr, "test31_copy_file2file: 12: dest.pre: %s\n", dest_stats.to_string().c_str());
                REQUIRE( true == dest_stats.exists() );
                REQUIRE( true == dest_stats.ok() );
                REQUIRE( true == dest_stats.is_file() );
                REQUIRE( source1_stats.size() == dest_stats.size() );
                REQUIRE( source1_stats.mode() == dest_stats.mode() );
            }
            REQUIRE( true == jau::fs::copy(source2_stats.path(), root_copy+"/file_10.txt", copts) );
            jau::fs::sync(); // just check API
            {
                jau::fs::file_stats dest_stats(root_copy+"/file_10.txt");
                jau::fprintf_td(stderr, "test31_copy_file2file: 12: dest.post: %s\n", dest_stats.to_string().c_str());
                REQUIRE( true  == dest_stats.exists() );
                REQUIRE( true  == dest_stats.ok() );
                REQUIRE( true  == dest_stats.is_file() );
                REQUIRE( false == dest_stats.is_link() );
                REQUIRE( source2_stats.size() == dest_stats.size() );
                REQUIRE( source2_stats.link_target()->prot_mode() == dest_stats.prot_mode() );
            }
        }
        if constexpr ( _remove_target_test_dir ) {
            REQUIRE( true == jau::fs::remove(root_copy, jau::fs::traverse_options::recursive) );
        }
    }

    void test40_copy_ext_r_p() {
        INFO_STR("\n\ntest40_copy_ext_r_p\n");

        jau::fs::file_stats root_orig_stats(project_root1);
        if( !root_orig_stats.exists() ) {
            root_orig_stats = jau::fs::file_stats(project_root2);
        }
        REQUIRE( true == root_orig_stats.exists() );

        const jau::fs::copy_options copts = jau::fs::copy_options::recursive |
                                            jau::fs::copy_options::preserve_all |
                                            jau::fs::copy_options::sync |
                                            jau::fs::copy_options::verbose;
        const std::string root_copy = root+"_copy_test40";
        jau::fs::remove(root_copy, jau::fs::traverse_options::recursive);
        testxx_copy_r_p("test40_copy_ext_r_p", root_orig_stats, 0 /* source_added_dead_links */, root_copy, copts, false /* dest_is_vfat */);
        REQUIRE( true == jau::fs::remove(root_copy, jau::fs::traverse_options::recursive) );
    }

    void test41_copy_ext_r_p_below() {
        INFO_STR("\n\ntest41_copy_ext_r_p_below\n");

        jau::fs::file_stats root_orig_stats(project_root1);
        if( !root_orig_stats.exists() ) {
            root_orig_stats = jau::fs::file_stats(project_root2);
        }
        REQUIRE( true == root_orig_stats.exists() );

        const jau::fs::copy_options copts = jau::fs::copy_options::recursive |
                                            jau::fs::copy_options::preserve_all |
                                            jau::fs::copy_options::sync |
                                            jau::fs::copy_options::verbose;
        const std::string root_copy_parent = root+"_copy_test41_parent";
        jau::fs::remove(root_copy_parent, jau::fs::traverse_options::recursive);
        REQUIRE( true == jau::fs::mkdir(root_copy_parent, jau::fs::fmode_t::def_dir_prot) );
        testxx_copy_r_p("test41_copy_ext_r_p_below", root_orig_stats, 0 /* source_added_dead_links */, root_copy_parent, copts, false /* dest_is_vfat */);
        REQUIRE( true == jau::fs::remove(root_copy_parent, jau::fs::traverse_options::recursive) );
    }

    void test42_copy_ext_r_p_into() {
        INFO_STR("\n\ntest41_copy_ext_r_p_into\n");

        jau::fs::file_stats root_orig_stats(project_root1);
        if( !root_orig_stats.exists() ) {
            root_orig_stats = jau::fs::file_stats(project_root2);
        }
        REQUIRE( true == root_orig_stats.exists() );

        const jau::fs::copy_options copts = jau::fs::copy_options::recursive |
                                            jau::fs::copy_options::into_existing_dir |
                                            jau::fs::copy_options::preserve_all |
                                            jau::fs::copy_options::sync |
                                            jau::fs::copy_options::verbose;
        const std::string root_copy = root+"_copy_test42_into";
        jau::fs::remove(root_copy, jau::fs::traverse_options::recursive);
        REQUIRE( true == jau::fs::mkdir(root_copy, jau::fs::fmode_t::def_dir_prot) );
        testxx_copy_r_p("test42_copy_ext_r_p_into", root_orig_stats, 0 /* source_added_dead_links */, root_copy, copts, false /* dest_is_vfat */);
        REQUIRE( true == jau::fs::remove(root_copy, jau::fs::traverse_options::recursive) );
    }

    void test43_copy_ext_r_p_over() {
        INFO_STR("\n\ntest43_copy_ext_r_p_over\n");

        jau::fs::file_stats root_orig_stats(project_root1);
        if( !root_orig_stats.exists() ) {
            root_orig_stats = jau::fs::file_stats(project_root2);
        }
        REQUIRE( true == root_orig_stats.exists() );

        const jau::fs::copy_options copts = jau::fs::copy_options::recursive |
                                            jau::fs::copy_options::preserve_all |
                                            jau::fs::copy_options::sync |
                                            jau::fs::copy_options::verbose;
        const std::string root_copy = root+"_copy_test43_over";
        jau::fs::remove(root_copy, jau::fs::traverse_options::recursive);
        REQUIRE( true == jau::fs::mkdir(root_copy, jau::fs::fmode_t::def_dir_prot) );
        const std::string root_copy_sub = root_copy+"/"+root_orig_stats.item().basename();
        REQUIRE( true == jau::fs::mkdir(root_copy_sub, jau::fs::fmode_t::def_dir_prot) );
        testxx_copy_r_p("test43_copy_ext_r_p_over", root_orig_stats, 0 /* source_added_dead_links */, root_copy, copts, false /* dest_is_vfat */);
        REQUIRE( true == jau::fs::remove(root_copy, jau::fs::traverse_options::recursive) );
    }

    void test49_copy_ext_r_p_vfat() {
        INFO_STR("\n\ntest49_copy_ext_r_p_vfat\n");

        // Query and prepare vfat data sink
        jau::fs::file_stats dest_fs_vfat_stats(dest_fs_vfat);
        if( !dest_fs_vfat_stats.is_dir() ) {
            jau::fprintf_td(stderr, "test49_copy_ext_r_p_vfat: Skipped, no vfat dest-dir %s\n", dest_fs_vfat_stats.to_string().c_str());
            return;
        }
        const std::string dest_vfat_parent = dest_fs_vfat+"/test49_data_sink";
        jau::fs::remove(dest_vfat_parent, jau::fs::traverse_options::recursive);
        if( !jau::fs::mkdir(dest_vfat_parent, jau::fs::fmode_t::def_dir_prot) ) {
            jau::fprintf_td(stderr, "test49_copy_ext_r_p_vfat: Skipped, couldn't create vfat dest folder %s\n", dest_vfat_parent.c_str());
            return;
        }

        // Source
        jau::fs::file_stats root_orig_stats(project_root1);
        if( !root_orig_stats.exists() ) {
            root_orig_stats = jau::fs::file_stats(project_root2);
        }
        REQUIRE( true == root_orig_stats.exists() );

        const jau::fs::copy_options copts = jau::fs::copy_options::recursive |
                                            jau::fs::copy_options::preserve_all |
                                            jau::fs::copy_options::ignore_symlink_errors |
                                            jau::fs::copy_options::sync |
                                            jau::fs::copy_options::verbose;
        const std::string dest_vfat_dir = dest_vfat_parent+"/"+root;
        testxx_copy_r_p("test49_copy_ext_r_p_vfat", root_orig_stats, 0 /* source_added_dead_links */, dest_vfat_dir, copts, true /* dest_is_vfat */);

        REQUIRE( true == jau::fs::remove(dest_vfat_parent, jau::fs::traverse_options::recursive) );
    }

    void test50_copy_ext_r_p_fsl() {
        INFO_STR("\n\ntest50_copy_ext_r_p_fsl\n");

        jau::fs::file_stats root_orig_stats(project_root1);
        if( !root_orig_stats.exists() ) {
            root_orig_stats = jau::fs::file_stats(project_root2);
        }
        REQUIRE( true == root_orig_stats.exists() );

        const std::string root_copy = root+"_copy_test50";
        const jau::fs::copy_options copts = jau::fs::copy_options::recursive |
                                            jau::fs::copy_options::preserve_all |
                                            jau::fs::copy_options::follow_symlinks |
                                            jau::fs::copy_options::ignore_symlink_errors |
                                            jau::fs::copy_options::verbose;
        {
            jau::fs::remove(root_copy, jau::fs::traverse_options::recursive);

            REQUIRE( true == jau::fs::copy(root_orig_stats.path(), root_copy, copts) );
        }
        jau::fs::file_stats root_copy_stats(root_copy);
        REQUIRE( true == root_copy_stats.exists() );
        REQUIRE( true == root_copy_stats.ok() );
        REQUIRE( true == root_copy_stats.is_dir() );

        bool(*pv_capture)(visitor_stats*, jau::fs::traverse_event, const jau::fs::file_stats&) =
            ( [](visitor_stats* stats_ptr, jau::fs::traverse_event tevt, const jau::fs::file_stats& element_stats) -> bool {
                (void)tevt;
                stats_ptr->add(element_stats);
                return true;
              } );
        {
            const jau::fs::traverse_options topts_orig = jau::fs::traverse_options::recursive |
                                                         jau::fs::traverse_options::dir_entry |
                                                         jau::fs::traverse_options::follow_symlinks;

            const jau::fs::traverse_options topts_copy = jau::fs::traverse_options::recursive |
                                                         jau::fs::traverse_options::dir_entry;
            visitor_stats stats(topts_orig);
            visitor_stats stats_copy(topts_copy);
            const jau::fs::path_visitor pv_orig = jau::bind_capref(&stats, pv_capture);
            const jau::fs::path_visitor pv_copy = jau::bind_capref(&stats_copy, pv_capture);
            REQUIRE( true == jau::fs::visit(root_orig_stats, topts_orig, pv_orig) );
            REQUIRE( true == jau::fs::visit(root_copy_stats, topts_copy, pv_copy) );

            jau::fprintf_td(stderr, "test50_copy_ext_r_p_fsl: copy %s, traverse_orig %s, traverse_copy %s\n",
                    to_string(copts).c_str(), to_string(topts_orig).c_str(), to_string(topts_copy).c_str());

            jau::fprintf_td(stderr, "test50_copy_ext_r_p_fsl: source      visitor stats\n%s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test50_copy_ext_r_p_fsl: destination visitor stats\n%s\n", stats_copy.to_string().c_str());

            REQUIRE(  9 == stats.total_real );
            REQUIRE( 11 == stats.total_sym_links_existing );
            REQUIRE(  4 == stats.total_sym_links_not_existing );
            REQUIRE(  0 == stats.total_no_access );
            REQUIRE(  4 == stats.total_not_existing );
            REQUIRE( 60 <  stats.total_file_bytes );                  // some followed symlink files are of unknown size, e.g. /etc/fstab
            REQUIRE(  6 == stats.files_real );
            REQUIRE( 10 == stats.files_sym_link );
            REQUIRE(  3 == stats.dirs_real );
            REQUIRE(  1 == stats.dirs_sym_link );

            REQUIRE( 20 == stats_copy.total_real );
            REQUIRE(  0 == stats_copy.total_sym_links_existing );
            REQUIRE(  0 == stats_copy.total_sym_links_not_existing );
            REQUIRE(  0 == stats_copy.total_no_access );
            REQUIRE(  0 == stats_copy.total_not_existing );
            REQUIRE( 60 <  stats_copy.total_file_bytes );             // some followed symlink files are of unknown size, e.g. /etc/fstab
            REQUIRE( 16 == stats_copy.files_real );
            REQUIRE(  0 == stats_copy.files_sym_link );
            REQUIRE(  4 == stats_copy.dirs_real );
            REQUIRE(  0 == stats_copy.dirs_sym_link );
        }

        const std::string root_copy_renamed = root+"_copy_test50_renamed";
        {
            REQUIRE( true == jau::fs::rename(root_copy, root_copy_renamed) );
        }
        jau::fs::file_stats root_copy_stats2(root_copy);
        REQUIRE( false == root_copy_stats2.exists() );

        jau::fs::file_stats root_copy_renamed_stats(root_copy_renamed);
        REQUIRE( true == root_copy_renamed_stats.exists() );
        REQUIRE( true == root_copy_renamed_stats.ok() );
        REQUIRE( true == root_copy_renamed_stats.is_dir() );

        {
            const jau::fs::traverse_options topts_copy = jau::fs::traverse_options::recursive |
                                                         jau::fs::traverse_options::dir_entry;
            visitor_stats stats_copy(topts_copy);
            const jau::fs::path_visitor pv_copy = jau::bind_capref(&stats_copy, pv_capture);
            REQUIRE( true == jau::fs::visit(root_copy_renamed_stats, topts_copy, pv_copy) );

            jau::fprintf_td(stderr, "test50_copy_ext_r_p_fsl: renamed: traverse_copy %s\n", to_string(topts_copy).c_str());

            jau::fprintf_td(stderr, "test50_copy_ext_r_p_fsl: renamed: visitor stats\n%s\n", stats_copy.to_string().c_str());

            REQUIRE( 20 == stats_copy.total_real );
            REQUIRE(  0 == stats_copy.total_sym_links_existing );
            REQUIRE(  0 == stats_copy.total_sym_links_not_existing );
            REQUIRE(  0 == stats_copy.total_no_access );
            REQUIRE(  0 == stats_copy.total_not_existing );
            REQUIRE( 60 <  stats_copy.total_file_bytes );             // some followed symlink files are of unknown size, e.g. /etc/fstab
            REQUIRE( 16 == stats_copy.files_real );
            REQUIRE(  0 == stats_copy.files_sym_link );
            REQUIRE(  4 == stats_copy.dirs_real );
            REQUIRE(  0 == stats_copy.dirs_sym_link );
        }

        if constexpr ( _remove_target_test_dir ) {
            REQUIRE( true == jau::fs::remove(root_copy_renamed, jau::fs::traverse_options::recursive) );
        }
    }
};

METHOD_AS_TEST_CASE( TestFileUtil01::test01_cwd,                "Test TestFileUtil01 - test01_cwd");
METHOD_AS_TEST_CASE( TestFileUtil01::test02_dirname,            "Test TestFileUtil01 - test02_dirname");
METHOD_AS_TEST_CASE( TestFileUtil01::test03_basename,           "Test TestFileUtil01 - test03_basename");
METHOD_AS_TEST_CASE( TestFileUtil01::test04_dir_item,           "Test TestFileUtil01 - test04_dir_item");
METHOD_AS_TEST_CASE( TestFileUtil01::test05_file_stat,          "Test TestFileUtil01 - test05_file_stat");
METHOD_AS_TEST_CASE( TestFileUtil01::test06_file_stat_symlinks, "Test TestFileUtil01 - test06_file_stat_symlinks");
METHOD_AS_TEST_CASE( TestFileUtil01::test07_file_stat_fd,       "Test TestFileUtil01 - test07_file_stat_fd");
METHOD_AS_TEST_CASE( TestFileUtil01::test08_pipe_01,            "Test TestFileUtil01 - test08_pipe_01");

METHOD_AS_TEST_CASE( TestFileUtil01::test10_mkdir,              "Test TestFileUtil01 - test10_mkdir");
METHOD_AS_TEST_CASE( TestFileUtil01::test11_touch,              "Test TestFileUtil01 - test11_touch");

METHOD_AS_TEST_CASE( TestFileUtil01::test20_visit,              "Test TestFileUtil01 - test20_visit");
METHOD_AS_TEST_CASE( TestFileUtil01::test22_visit_symlinks,     "Test TestFileUtil01 - test22_visit_symlinks");

METHOD_AS_TEST_CASE( TestFileUtil01::test30_copy_file2dir,      "Test TestFileUtil01 - test30_copy_file2dir");
METHOD_AS_TEST_CASE( TestFileUtil01::test31_copy_file2file,     "Test TestFileUtil01 - test31_copy_file2file");

METHOD_AS_TEST_CASE( TestFileUtil01::test40_copy_ext_r_p,       "Test TestFileUtil01 - test40_copy_ext_r_p");
METHOD_AS_TEST_CASE( TestFileUtil01::test41_copy_ext_r_p_below, "Test TestFileUtil01 - test41_copy_ext_r_p_below");
METHOD_AS_TEST_CASE( TestFileUtil01::test42_copy_ext_r_p_into,  "Test TestFileUtil01 - test42_copy_ext_r_p_into");
METHOD_AS_TEST_CASE( TestFileUtil01::test43_copy_ext_r_p_over,  "Test TestFileUtil01 - test43_copy_ext_r_p_over");

METHOD_AS_TEST_CASE( TestFileUtil01::test49_copy_ext_r_p_vfat,  "Test TestFileUtil01 - test49_copy_ext_r_p_vfat");

METHOD_AS_TEST_CASE( TestFileUtil01::test50_copy_ext_r_p_fsl,   "Test TestFileUtil01 - test50_copy_ext_r_p_fsl");
