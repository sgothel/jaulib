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

package jau.test.fs;

import org.jau.fs.CopyOptions;
import org.jau.fs.FMode;
import org.jau.fs.FileStats;
import org.jau.fs.FileUtil;
import org.jau.fs.TraverseOptions;
import org.jau.io.PrintUtil;
import org.junit.Assert;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

import jau.pkg.PlatformRuntime;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestsudoFileUtils02 extends FileUtilBaseTest {
    static final boolean DEBUG = false;

    @Test(timeout = 10000)
    public void test50_mount_copy_r_p() {
        PlatformRuntime.checkInitialized();
        PrintUtil.println(System.err, "test50_mount_copy_r_p\n");

        FileStats root_orig_stats = new FileStats(project_root1);
        if( !root_orig_stats.exists() ) {
            root_orig_stats = new FileStats(project_root2);
        }
        Assert.assertTrue( true == root_orig_stats.exists() );
        Assert.assertTrue( true == root_orig_stats.is_dir() );

        {
            final String image_file = "../" + root + ".sqfs";
            final FileStats image_stats = new FileStats(image_file);
            Assert.assertTrue( true == image_stats.exists() );

            final String mount_point = root+"_mount";
            FileUtil.remove(mount_point, TraverseOptions.recursive); // start fresh
            Assert.assertTrue( true == FileUtil.mkdir(mount_point, FMode.def_dir) );

            final long mctx = FileUtil.mount_image(image_stats.path(), mount_point, "squashfs", 0, "");
            Assert.assertTrue( 0 != mctx );

            final CopyOptions copts = new CopyOptions();
            copts.set(CopyOptions.Bit.recursive);
            copts.set(CopyOptions.Bit.preserve_all);
            copts.set(CopyOptions.Bit.sync);
            copts.set(CopyOptions.Bit.verbose);

            final String root_copy = root+"_copy_test50";
            FileUtil.remove(root_copy, TraverseOptions.recursive);
            testxx_copy_r_p("test50_mount_copy_r_p", new FileStats(mount_point), 1 /* source_added_dead_links */, root_copy, copts, false /* dest_is_vfat */);
            Assert.assertTrue( true == FileUtil.remove(root_copy, TraverseOptions.recursive) );

            final boolean umount_ok = FileUtil.umount(mctx);
            Assert.assertTrue( true == umount_ok );

            Assert.assertTrue( true == FileUtil.remove(root_copy, TraverseOptions.recursive) );
            Assert.assertTrue( true == FileUtil.remove(mount_point, TraverseOptions.recursive) );
        }
    }

    public static void main(final String args[]) {
        org.junit.runner.JUnitCore.main(TestsudoFileUtils02.class.getName());
    }
}