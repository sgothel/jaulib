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

#include <jau/test/catch2_ext.hpp>

#include <jau/debug.hpp>
#include <jau/file_util.hpp>

using namespace jau::fractions_i64_literals;

static constexpr const bool _remove_target_test_dir = true;

struct visitor_stats {
    jau::fs::traverse_options topts;
    int total_real;
    int total_sym_links_existing;
    int total_sym_links_not_existing;
    int total_no_access;
    int total_not_existing;
    size_t total_file_bytes;
    int files_real;
    int files_sym_link;
    int dirs_real;
    int dirs_sym_link;

    visitor_stats(jau::fs::traverse_options topts_)
    : topts(topts_),
      total_real(0),
      total_sym_links_existing(0),
      total_sym_links_not_existing(0),
      total_no_access(0),
      total_not_existing(0),
      total_file_bytes(0),
      files_real(0),
      files_sym_link(0),
      dirs_real(0),
      dirs_sym_link(0)
    {}

    void add(const jau::fs::file_stats& element_stats) {
        if( element_stats.is_link() ) {
            if( element_stats.exists() ) {
                total_sym_links_existing++;
            } else {
                total_sym_links_not_existing++;
            }
        } else {
            total_real++;
        }
        if( !element_stats.has_access() ) {
            total_no_access++;
        }
        if( !element_stats.exists() ) {
            total_not_existing++;
        }
        if( element_stats.is_file() ) {
            if( element_stats.is_link() ) {
                files_sym_link++;
                if( is_set(topts, jau::fs::traverse_options::follow_symlinks) ) {
                    total_file_bytes += element_stats.size();
                }
            } else {
                files_real++;
                total_file_bytes += element_stats.size();
            }
        } else if( element_stats.is_dir() ) {
            if( element_stats.is_link() ) {
                dirs_sym_link++;
            } else {
                dirs_real++;
            }
        }
    }

    std::string to_string() const noexcept {
        std::string res;
        res += "- traverse_options              "+jau::fs::to_string(topts)+"\n";
        res += "- total_real                    "+std::to_string(total_real)+"\n";
        res += "- total_sym_links_existing      "+std::to_string(total_sym_links_existing)+"\n";
        res += "- total_sym_links_not_existing  "+std::to_string(total_sym_links_not_existing)+"\n";
        res += "- total_no_access               "+std::to_string(total_no_access)+"\n";
        res += "- total_not_existing            "+std::to_string(total_not_existing)+"\n";
        res += "- total_file_bytes              "+jau::to_decstring(total_file_bytes)+"\n";
        res += "- files_real                    "+std::to_string(files_real)+"\n";
        res += "- files_sym_link                "+std::to_string(files_sym_link)+"\n";
        res += "- dirs_real                     "+std::to_string(dirs_real)+"\n";
        res += "- dirs_sym_link                 "+std::to_string(dirs_sym_link)+"\n";
        return res;
    }
};

constexpr bool operator ==(const visitor_stats& lhs, const visitor_stats& rhs) noexcept {
    return lhs.total_file_bytes             == rhs.total_file_bytes &&
           lhs.total_real                   == rhs.total_real &&
           lhs.total_sym_links_existing     == rhs.total_sym_links_existing &&
           lhs.total_sym_links_not_existing == rhs.total_sym_links_not_existing &&
           lhs.total_no_access              == rhs.total_no_access &&
           lhs.total_not_existing           == rhs.total_not_existing &&
           lhs.files_real                   == rhs.files_real &&
           lhs.files_sym_link               == rhs.files_sym_link &&
           lhs.dirs_real                    == rhs.dirs_real &&
           lhs.dirs_sym_link                == rhs.dirs_sym_link;
}
constexpr bool operator !=(const visitor_stats& lhs, const visitor_stats& rhs) noexcept {
    return !( lhs == rhs );
}

class TestFileUtil01 {
  public:
    const std::string root = "test_data";
    // normal location with jaulib as sole project
    const std::string project_root1 = "../../test_data";
    // submodule location with jaulib directly hosted below main project
    const std::string project_root2 = "../../../jaulib/test_data";
    // external filesystem to test ...
    const std::string project_root_ext = "/mnt/ssd0/data/test_data";

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
            REQUIRE( "../../test_data/file_01_slink09R1.txt" == di.path() );
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
        INFO_STR("\n\ntest05_file_stat\n");

        {
            jau::fs::file_stats stats(project_root_ext+"/file_01.txt");
            jau::fprintf_td(stderr, "test04_file_stat: 01: %s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test04_file_stat: 01: fields %s\n", jau::fs::to_string( stats.fields() ).c_str());
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
        jau::fprintf_td(stderr, "test04_file_stat: 11: %s\n", proot_stats.to_string().c_str());
        jau::fprintf_td(stderr, "test04_file_stat: 11: fields %s\n", jau::fs::to_string( proot_stats.fields() ).c_str());
        REQUIRE( true == proot_stats.exists() );
        REQUIRE( true == proot_stats.is_dir() );

        {
            jau::fs::file_stats stats(proot_stats.path()+"/file_01.txt");
            jau::fprintf_td(stderr, "test04_file_stat: 12: %s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test04_file_stat: 12: fields %s\n", jau::fs::to_string( stats.fields() ).c_str());
            REQUIRE(  stats.exists() );
            REQUIRE(  stats.has_access() );
            REQUIRE( !stats.is_dir() );
            REQUIRE(  stats.is_file() );
            REQUIRE( !stats.is_link() );
            REQUIRE( 15 == stats.size() );
        }
        {
            jau::fs::file_stats stats(proot_stats.path()+"/file_01_slink01.txt");
            jau::fprintf_td(stderr, "test04_file_stat: 13: %s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test04_file_stat: 13: fields %s\n", jau::fs::to_string( stats.fields() ).c_str());
            if( stats.is_link() ) {
                jau::fprintf_td(stderr, "test04_file_stat: 13: link_target %s\n", stats.link_target()->to_string().c_str());
            }
            REQUIRE(  stats.exists() );
            REQUIRE(  stats.has_access() );
            REQUIRE( !stats.is_dir() );
            REQUIRE(  stats.is_file() );
            REQUIRE(  stats.is_link() );
            REQUIRE( 15 == stats.size() );
            REQUIRE( nullptr != stats.link_target() );
            const jau::fs::file_stats* link_target = stats.link_target().get();
            REQUIRE( nullptr != link_target );
            REQUIRE( "file_01.txt" == link_target->path() );
        }
        {
            jau::fs::file_stats stats(proot_stats.path()+"/fstab_slink07_absolute");
            jau::fprintf_td(stderr, "test04_file_stat: 14: %s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test04_file_stat: 14: fields %s\n", jau::fs::to_string( stats.fields() ).c_str());
            if( stats.is_link() ) {
                jau::fprintf_td(stderr, "test04_file_stat: 14: link_target %s\n", stats.link_target()->to_string().c_str());
            }
            REQUIRE(  stats.exists() );
            REQUIRE(  stats.has_access() );
            REQUIRE( !stats.is_dir() );
            REQUIRE(  stats.is_file() );
            REQUIRE(  stats.is_link() );
            REQUIRE( 20 < stats.size() ); // greater than basename
            REQUIRE( nullptr != stats.link_target() );
            const jau::fs::file_stats* link_target = stats.link_target().get();
            REQUIRE( nullptr != link_target );
            REQUIRE( "/etc/fstab" == link_target->path() );
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
            INFO_STR("root_stats.pre: "+root_stats.to_string(true)+"\n");
            REQUIRE( !root_stats.exists() );
            REQUIRE( root_stats.has_access() );
            REQUIRE( !root_stats.is_dir() );
            REQUIRE( !root_stats.is_file() );
            REQUIRE( !root_stats.is_link() );
        }
        REQUIRE( true == jau::fs::mkdir(root, jau::fs::fmode_t::def_dir_prot) );
        {
            jau::fs::file_stats root_stats(root);
            INFO_STR("root_stats.post: "+root_stats.to_string(true)+"\n");
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
        INFO_STR("\n\ntest11_touch\n");
        const std::string file_01 = root+"/data01.txt";
        const std::string file_02 = root+"/data02.txt";
        REQUIRE( true == jau::fs::mkdir(root, jau::fs::fmode_t::def_dir_prot) );
        {
            jau::fs::file_stats root_stats(root);
            INFO_STR("root_stats1.post: "+root_stats.to_string(true)+"\n");
            REQUIRE( root_stats.exists() );
            REQUIRE( root_stats.has_access() );
            REQUIRE( root_stats.is_dir() );
            REQUIRE( !root_stats.is_file() );
            REQUIRE( !root_stats.is_link() );
        }

        REQUIRE( true == jau::fs::touch(file_01, jau::fs::fmode_t::def_file_prot) );
        {
            jau::fs::file_stats file_stats(file_01);
            INFO_STR("file_stats2.post: "+file_stats.to_string(true)+"\n");
            REQUIRE( file_stats.exists() );
            REQUIRE( file_stats.has_access() );
            REQUIRE( !file_stats.is_dir() );
            REQUIRE( file_stats.is_file() );
            REQUIRE( !file_stats.is_link() );
        }

        {
            REQUIRE( true == jau::fs::touch(file_02, jau::fs::fmode_t::def_file_prot) );
            jau::fs::file_stats file_stats_pre(file_02);
            const jau::fraction_timespec btime_pre = file_stats_pre.btime();
            const jau::fraction_timespec atime_pre = file_stats_pre.atime();
            const jau::fraction_timespec mtime_pre = file_stats_pre.mtime();
            INFO_STR("btime.pre: "+btime_pre.to_iso8601_string(true)+", "+btime_pre.to_string()+"\n");
            INFO_STR("atime.pre: "+atime_pre.to_iso8601_string(true)+", "+atime_pre.to_string()+"\n");
            INFO_STR("mtime.pre: "+mtime_pre.to_iso8601_string(true)+", "+mtime_pre.to_string()+"\n");

            const jau::fraction_timespec ts_20200101( 1577836800_s + 0_h); // 2020-01-01 00:00:00
            const jau::fraction_timespec atime_set( ts_20200101 +  1_d + 10_h );
            const jau::fraction_timespec mtime_set( ts_20200101 + 31_d + 10_h );
            INFO_STR("atime.set: "+atime_set.to_iso8601_string(true)+", "+atime_set.to_string()+"\n");
            INFO_STR("mtime.set: "+mtime_set.to_iso8601_string(true)+", "+mtime_set.to_string()+"\n");
            REQUIRE( true == jau::fs::touch(file_02, atime_set, mtime_set, jau::fs::fmode_t::def_file_prot) );

            jau::fs::file_stats file_stats_post(file_02);
            const jau::fraction_timespec atime_post = file_stats_post.atime();
            const jau::fraction_timespec mtime_post = file_stats_post.mtime();
            INFO_STR("atime.post: "+atime_post.to_iso8601_string(true)+", "+atime_post.to_string()+"\n");
            INFO_STR("mtime.post: "+mtime_post.to_iso8601_string(true)+", "+mtime_post.to_string()+"\n");
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
            const jau::fs::path_visitor pv = jau::bindCaptureRefFunc(&stats_R_FSL_PDL,
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
            const jau::fs::path_visitor pv = jau::bindCaptureRefFunc(&stats_R_FSL,
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

    void test21_symlink_file() {
        INFO_STR("\n\ntest21_symlink_file\n");

        jau::fs::file_stats proot_stats(project_root1);
        if( !proot_stats.exists() ) {
            proot_stats = jau::fs::file_stats(project_root2);
        }
        REQUIRE( true == proot_stats.exists() );

        INFO_STR("project_root "+proot_stats.to_string(true)+"\n");
        REQUIRE( true == proot_stats.is_dir() );

        jau::fs::file_stats file_01_link_stats(proot_stats.path()+"/file_01_slink01.txt");
        INFO_STR("project_root "+file_01_link_stats.to_string(true)+"\n");
        REQUIRE( true == file_01_link_stats.is_link() );
        REQUIRE( true == file_01_link_stats.is_file() );
        REQUIRE( 15 == file_01_link_stats.size() );
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
            const jau::fs::path_visitor pv = jau::bindCaptureRefFunc(&stats,
                    ( bool(*)(visitor_stats*, jau::fs::traverse_event, const jau::fs::file_stats&) ) /* help template type deduction of function-ptr */
                        ( [](visitor_stats* stats_ptr, jau::fs::traverse_event tevt, const jau::fs::file_stats& element_stats) -> bool {
                            (void)tevt;
                            stats_ptr->add(element_stats);
                            return true;
                          } ) );
            REQUIRE( true == jau::fs::visit(proot_stats, topts, pv) );
            jau::fprintf_td(stderr, "test22_visit[R]: %s\n%s\n", to_string(topts).c_str(), stats.to_string().c_str());
            REQUIRE(  7 == stats.total_real );
            REQUIRE(  8 == stats.total_sym_links_existing );
            REQUIRE(  0 == stats.total_sym_links_not_existing );
            REQUIRE(  0 == stats.total_no_access );
            REQUIRE(  0 == stats.total_not_existing );
            REQUIRE( 60 == stats.total_file_bytes );
            REQUIRE(  4 == stats.files_real );
            REQUIRE(  7 == stats.files_sym_link );
            REQUIRE(  3 == stats.dirs_real );
            REQUIRE(  1 == stats.dirs_sym_link );
        }
        {
            const jau::fs::traverse_options topts = jau::fs::traverse_options::recursive |
                                                    jau::fs::traverse_options::dir_entry |
                                                    jau::fs::traverse_options::follow_symlinks;
            visitor_stats stats(topts);
            const jau::fs::path_visitor pv = jau::bindCaptureRefFunc(&stats,
                    ( bool(*)(visitor_stats*, jau::fs::traverse_event, const jau::fs::file_stats&) ) /* help template type deduction of function-ptr */
                        ( [](visitor_stats* stats_ptr, jau::fs::traverse_event tevt, const jau::fs::file_stats& element_stats) -> bool {
                            (void)tevt;
                            stats_ptr->add(element_stats);
                            return true;
                          } ) );
            REQUIRE( true == jau::fs::visit(proot_stats, topts, pv) );
            jau::fprintf_td(stderr, "test22_visit[R, FSL]: %s\n%s\n", to_string(topts).c_str(), stats.to_string().c_str());
            REQUIRE(  9 == stats.total_real );
            REQUIRE(  9 == stats.total_sym_links_existing );
            REQUIRE(  0 == stats.total_sym_links_not_existing );
            REQUIRE(  0 == stats.total_no_access );
            REQUIRE(  0 == stats.total_not_existing );
            REQUIRE( 60 <  stats.total_file_bytes ); // some followed symlink files are of unknown size, e.g. /etc/fstab
            REQUIRE(  6 == stats.files_real );
            REQUIRE(  8 == stats.files_sym_link );
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
        jau::fprintf_td(stderr, "test30_copy_file2dir: source2: %s\n", source2_stats.to_string().c_str());
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
                jau::fprintf_td(stderr, "test30_copy_file2dir: 11: dest.pre: %s\n", dest_stats.to_string().c_str());
                REQUIRE( true == dest_stats.exists() );
                REQUIRE( true == dest_stats.ok() );
                REQUIRE( true == dest_stats.is_file() );
            }
            REQUIRE( false == jau::fs::copy(source1_stats.path(), root_copy+"/file_10.txt", copts) );
        }
        {
            // Overwrite copy file to file
            const jau::fs::copy_options copts = jau::fs::copy_options::preserve_all |
                                                jau::fs::copy_options::overwrite |
                                                jau::fs::copy_options::follow_symlinks |
                                                jau::fs::copy_options::verbose;

            {
                jau::fs::file_stats dest_stats(root_copy+"/file_10.txt");
                jau::fprintf_td(stderr, "test30_copy_file2dir: 12: dest.pre: %s\n", dest_stats.to_string().c_str());
                REQUIRE( true == dest_stats.exists() );
                REQUIRE( true == dest_stats.ok() );
                REQUIRE( true == dest_stats.is_file() );
                REQUIRE( source1_stats.size() == dest_stats.size() );
                REQUIRE( source1_stats.mode() == dest_stats.mode() );
            }
            REQUIRE( true == jau::fs::copy(source2_stats.path(), root_copy+"/file_10.txt", copts) );
            {
                jau::fs::file_stats dest_stats(root_copy+"/file_10.txt");
                jau::fprintf_td(stderr, "test30_copy_file2dir: 12: dest.post: %s\n", dest_stats.to_string().c_str());
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

        const std::string root_copy = root+"_copy_test40";
        const jau::fs::copy_options copts = jau::fs::copy_options::recursive |
                                            jau::fs::copy_options::preserve_all |
                                            jau::fs::copy_options::sync |
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

            const jau::fs::traverse_options topts = jau::fs::traverse_options::recursive |
                                                    jau::fs::traverse_options::dir_entry;
            visitor_stats stats(topts);
            visitor_stats stats_copy(topts);
            const jau::fs::path_visitor pv_orig = jau::bindCaptureRefFunc(&stats, pv_capture);
            const jau::fs::path_visitor pv_copy = jau::bindCaptureRefFunc(&stats_copy, pv_capture);
            REQUIRE( true == jau::fs::visit(root_orig_stats, topts, pv_orig) );
            REQUIRE( true == jau::fs::visit(root_copy_stats, topts, pv_copy) );

            jau::fprintf_td(stderr, "test40_copy_ext_r_p: copy %s, traverse %s\n",
                    to_string(copts).c_str(), to_string(topts).c_str());

            jau::fprintf_td(stderr, "test40_copy_ext_r_p: source      visitor stats\n%s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test40_copy_ext_r_p: destination visitor stats\n%s\n", stats_copy.to_string().c_str());

            REQUIRE(  7 == stats.total_real );
            REQUIRE(  8 == stats.total_sym_links_existing );
            REQUIRE(  0 == stats.total_sym_links_not_existing );
            REQUIRE(  0 == stats.total_no_access );
            REQUIRE(  0 == stats.total_not_existing );
            REQUIRE( 60 == stats.total_file_bytes );
            REQUIRE(  4 == stats.files_real );
            REQUIRE(  7 == stats.files_sym_link );
            REQUIRE(  3 == stats.dirs_real );
            REQUIRE(  1 == stats.dirs_sym_link );

            REQUIRE(  7 == stats_copy.total_real );
            REQUIRE(  7 == stats_copy.total_sym_links_existing );
            REQUIRE(  1 == stats_copy.total_sym_links_not_existing ); // symlink ../README.txt
            REQUIRE(  0 == stats_copy.total_no_access );
            REQUIRE(  1 == stats_copy.total_not_existing );           // symlink ../README.txt
            REQUIRE( 60 == stats_copy.total_file_bytes );
            REQUIRE(  4 == stats_copy.files_real );
            REQUIRE(  6 == stats_copy.files_sym_link );
            REQUIRE(  3 == stats_copy.dirs_real );
            REQUIRE(  1 == stats_copy.dirs_sym_link );
        }
        {
            // compare each file in detail O(n*n)
            const jau::fs::traverse_options topts = jau::fs::traverse_options::recursive |
                                                    jau::fs::traverse_options::dir_entry;
            struct search_and_result {
                    std::string basename;
                    jau::fs::file_stats stats;
                    bool match;
            };
            const jau::fs::path_visitor pv1 = jau::bindCaptureRefFunc<bool, const jau::fs::file_stats, jau::fs::traverse_event, const jau::fs::file_stats&>(&root_copy_stats,
                    ( bool(*)(const jau::fs::file_stats*, jau::fs::traverse_event, const jau::fs::file_stats&) ) /* help template type deduction of function-ptr */
                        ( [](const jau::fs::file_stats* _root_copy_stats, jau::fs::traverse_event tevt1, const jau::fs::file_stats& element_stats1) -> bool {
                            (void)tevt1;
                            search_and_result sar { jau::fs::basename( element_stats1.path() ), element_stats1, false };
                            const jau::fs::path_visitor pv2 = jau::bindCaptureRefFunc<bool, search_and_result, jau::fs::traverse_event, const jau::fs::file_stats&>(&sar,
                                    ( bool(*)(search_and_result*, jau::fs::traverse_event, const jau::fs::file_stats&) ) /* help template type deduction of function-ptr */
                                        ( [](search_and_result* _sar, jau::fs::traverse_event tevt2, const jau::fs::file_stats& element_stats2) -> bool {
                                            (void)tevt2;
                                            const std::string basename2 = jau::fs::basename( element_stats2.path() );
                                            if( basename2 == _sar->basename ||
                                                ( "test_data" == _sar->basename && "test_data_copy_test40" == basename2 )
                                              )
                                            {
                                                if( "README_slink08_relext.txt" == basename2 ) {
                                                    // symlink to ../README.txt not existent on target
                                                    _sar->match = element_stats2.is_link() &&
                                                                  !element_stats2.exists();
                                                } else {
                                                    _sar->match =
                                                            element_stats2.mode() == _sar->stats.mode() &&
                                                            element_stats2.atime() == _sar->stats.atime() &&
                                                            element_stats2.mtime() == _sar->stats.mtime() &&
                                                            element_stats2.uid() == _sar->stats.uid() &&
                                                            element_stats2.gid() == _sar->stats.gid() &&
                                                            element_stats2.size() == _sar->stats.size();
                                                }
                                                jau::fprintf_td(stderr, "test40_copy_ext_r_p.check: '%s', match %d\n\t source %s\n\t dest__ %s\n\n",
                                                        basename2.c_str(), _sar->match,
                                                        _sar->stats.to_string().c_str(),
                                                        element_stats2.to_string().c_str());
                                                return false; // done
                                            } else {
                                                return true; // continue search
                                            }
                                          } ) );
                            if( jau::fs::visit(*_root_copy_stats, topts, pv2) ) {
                                jau::fprintf_td(stderr, "test40_copy_ext_r_p.check: '%s', not found!\n\t source %s\n\n",
                                        sar.basename.c_str(),
                                        element_stats1.to_string().c_str());
                                return false; // not found, abort
                            } else {
                                // found
                                if( sar.match ) {
                                    return true; // found and matching, continue
                                } else {
                                    return false; // found not matching, abort
                                }
                            }
                          } ) );
            REQUIRE( true == jau::fs::visit(root_orig_stats, topts, pv1) );
        }
        if constexpr ( _remove_target_test_dir ) {
            REQUIRE( true == jau::fs::remove(root_copy, jau::fs::traverse_options::recursive) );
        }
    }

    void test41_copy_ext_r_p_fsl() {
        INFO_STR("\n\ntest41_copy_ext_r_p_fsl\n");

        jau::fs::file_stats root_orig_stats(project_root1);
        if( !root_orig_stats.exists() ) {
            root_orig_stats = jau::fs::file_stats(project_root2);
        }
        REQUIRE( true == root_orig_stats.exists() );

        const std::string root_copy = root+"_copy_test41";
        const jau::fs::copy_options copts = jau::fs::copy_options::recursive |
                                            jau::fs::copy_options::preserve_all |
                                            jau::fs::copy_options::follow_symlinks |
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
            const jau::fs::path_visitor pv_orig = jau::bindCaptureRefFunc(&stats, pv_capture);
            const jau::fs::path_visitor pv_copy = jau::bindCaptureRefFunc(&stats_copy, pv_capture);
            REQUIRE( true == jau::fs::visit(root_orig_stats, topts_orig, pv_orig) );
            REQUIRE( true == jau::fs::visit(root_copy_stats, topts_copy, pv_copy) );

            jau::fprintf_td(stderr, "test41_copy_ext_r_p_fsl: copy %s, traverse_orig %s, traverse_copy %s\n",
                    to_string(copts).c_str(), to_string(topts_orig).c_str(), to_string(topts_copy).c_str());

            jau::fprintf_td(stderr, "test41_copy_ext_r_p_fsl: source      visitor stats\n%s\n", stats.to_string().c_str());
            jau::fprintf_td(stderr, "test41_copy_ext_r_p_fsl: destination visitor stats\n%s\n", stats_copy.to_string().c_str());

            REQUIRE(  9 == stats.total_real );
            REQUIRE(  9 == stats.total_sym_links_existing );
            REQUIRE(  0 == stats.total_sym_links_not_existing );
            REQUIRE(  0 == stats.total_no_access );
            REQUIRE(  0 == stats.total_not_existing );
            REQUIRE( 60 <  stats.total_file_bytes );                  // some followed symlink files are of unknown size, e.g. /etc/fstab
            REQUIRE(  6 == stats.files_real );
            REQUIRE(  8 == stats.files_sym_link );
            REQUIRE(  3 == stats.dirs_real );
            REQUIRE(  1 == stats.dirs_sym_link );

            REQUIRE( 18 == stats_copy.total_real );
            REQUIRE(  0 == stats_copy.total_sym_links_existing );
            REQUIRE(  0 == stats_copy.total_sym_links_not_existing );
            REQUIRE(  0 == stats_copy.total_no_access );
            REQUIRE(  0 == stats_copy.total_not_existing );
            REQUIRE( 60 <  stats_copy.total_file_bytes );             // some followed symlink files are of unknown size, e.g. /etc/fstab
            REQUIRE( 14 == stats_copy.files_real );
            REQUIRE(  0 == stats_copy.files_sym_link );
            REQUIRE(  4 == stats_copy.dirs_real );
            REQUIRE(  0 == stats_copy.dirs_sym_link );
        }
        if constexpr ( _remove_target_test_dir ) {
            REQUIRE( true == jau::fs::remove(root_copy, jau::fs::traverse_options::recursive) );
        }
    }

};

METHOD_AS_TEST_CASE( TestFileUtil01::test01_cwd,                "Test TestFileUtil01 - test01_cwd");
METHOD_AS_TEST_CASE( TestFileUtil01::test02_dirname,            "Test TestFileUtil01 - test02_dirname");
METHOD_AS_TEST_CASE( TestFileUtil01::test03_basename,           "Test TestFileUtil01 - test03_basename");
METHOD_AS_TEST_CASE( TestFileUtil01::test04_dir_item,           "Test TestFileUtil01 - test04_dir_item");
METHOD_AS_TEST_CASE( TestFileUtil01::test05_file_stat,          "Test TestFileUtil01 - test05_file_stat");

METHOD_AS_TEST_CASE( TestFileUtil01::test10_mkdir,              "Test TestFileUtil01 - test10_mkdir");
METHOD_AS_TEST_CASE( TestFileUtil01::test11_touch,              "Test TestFileUtil01 - test11_touch");

METHOD_AS_TEST_CASE( TestFileUtil01::test20_visit,              "Test TestFileUtil01 - test20_visit");
METHOD_AS_TEST_CASE( TestFileUtil01::test21_symlink_file,       "Test TestFileUtil01 - test21_symlink_file");
METHOD_AS_TEST_CASE( TestFileUtil01::test22_visit_symlinks,     "Test TestFileUtil01 - test22_visit_symlinks");

METHOD_AS_TEST_CASE( TestFileUtil01::test30_copy_file2dir,      "Test TestFileUtil01 - test30_copy_file2dir");
METHOD_AS_TEST_CASE( TestFileUtil01::test31_copy_file2file,     "Test TestFileUtil01 - test31_copy_file2file");

METHOD_AS_TEST_CASE( TestFileUtil01::test40_copy_ext_r_p,       "Test TestFileUtil01 - test40_copy_ext_r_p");
METHOD_AS_TEST_CASE( TestFileUtil01::test41_copy_ext_r_p_fsl,   "Test TestFileUtil01 - test41_copy_ext_r_p_fsl");

