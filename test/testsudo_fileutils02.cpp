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

class TestFileUtil02 : TestFileUtilBase {
  public:

    void test50_mount_copy_r_p() {
        INFO_STR("\n\ntest50_mount_copy_r_p\n");

        jau::fs::file_stats root_orig_stats(project_root1);
        if( !root_orig_stats.exists() ) {
            root_orig_stats = jau::fs::file_stats(project_root2);
        }
        REQUIRE( true == root_orig_stats.exists() );

        std::string image_file = root_orig_stats.path() + ".sqfs";
        jau::fs::file_stats image_stats(image_file);
        REQUIRE( true == image_stats.exists() );

        const std::string mount_point = root+"_mount";
        jau::fs::remove(mount_point, jau::fs::traverse_options::recursive); // start fresh
        REQUIRE( true == jau::fs::mkdir(mount_point, jau::fs::fmode_t::def_dir_prot) );

        jau::fs::mount_ctx mctx = jau::fs::mount_image(image_stats.path(), mount_point, "squashfs", /* MS_LAZYTIME | MS_NOATIME | */ MS_RDONLY, "");
        REQUIRE( true == mctx.mounted );

        const std::string root_copy = root+"_copy_test50";
        testxx_copy_r_p("test50_mount_copy_r_p", mount_point, 1 /* source_added_dead_links */, root_copy);

        REQUIRE( true == jau::fs::umount(mctx) );

        if constexpr ( _remove_target_test_dir ) {
            REQUIRE( true == jau::fs::remove(mount_point, jau::fs::traverse_options::recursive) );
        }
    }
};

METHOD_AS_TEST_CASE( TestFileUtil02::test50_mount_copy_r_p,     "Test TestFileUtil02 - test50_mount_copy_r_p");
