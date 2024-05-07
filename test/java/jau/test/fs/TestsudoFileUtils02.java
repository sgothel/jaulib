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

import java.util.List;

import org.jau.fs.CopyOptions;
import org.jau.fs.DirItem;
import org.jau.fs.FMode;
import org.jau.fs.FileStats;
import org.jau.fs.FileUtil;
import org.jau.fs.MountFlags;
import org.jau.fs.TraverseOptions;
import org.jau.fs.UnmountFlags;
import org.jau.io.PrintUtil;
import org.jau.sys.PlatformProps;
import org.jau.sys.PlatformTypes;
import org.junit.Assert;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

import jau.pkg.PlatformRuntime;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestsudoFileUtils02 extends FileUtilBaseTest {
    static final boolean DEBUG = false;

    private static boolean isFolderEmpty(final String path) {
        final List<DirItem> files = FileUtil.get_dir_content(path);
        return null == files || 0 == files.size();
    }

    @Test(timeout = 10000)
    public void test50_mount_copy_r_p() {
        PlatformRuntime.checkInitialized();
        PrintUtil.println(System.err, "test50_mount_copy_r_p\n");

        final FileStats root_orig_stats = getTestDataDirStats();
        Assert.assertTrue( true == root_orig_stats.exists() );
        Assert.assertTrue( true == root_orig_stats.is_dir() );

        {

            final FileStats image_stats = getTestDataImageFile();
            Assert.assertTrue( true == image_stats.exists() );

            final String mount_point = root+"_mount";
            FileUtil.remove(mount_point, TraverseOptions.recursive); // start fresh
            Assert.assertTrue( true == FileUtil.mkdir(mount_point, FMode.def_dir) );

            final MountFlags mount_flags;
            if( PlatformProps.OS == PlatformTypes.OSType.LINUX ) {
                mount_flags = new org.jau.fs.linux.MountFlags();
                mount_flags.set(org.jau.fs.linux.MountFlags.Bit.rdonly);
            } else {
                mount_flags = new MountFlags();
            }
            PrintUtil.println(System.err, "MountFlags for "+PlatformProps.OS+": "+mount_flags);
            final long mctx = FileUtil.mount_image(image_stats.path(), mount_point, "squashfs", mount_flags, "");
            Assert.assertTrue( 0 != mctx );
            Assert.assertFalse(isFolderEmpty(mount_point));

            final CopyOptions copts = new CopyOptions();
            copts.set(CopyOptions.Bit.recursive);
            copts.set(CopyOptions.Bit.preserve_all);
            copts.set(CopyOptions.Bit.sync);
            copts.set(CopyOptions.Bit.verbose);

            final String root_copy = root+"_copy_test50";
            FileUtil.remove(root_copy, TraverseOptions.recursive);
            testxx_copy_r_p("test50_mount_copy_r_p", new FileStats(mount_point), 1 /* source_added_dead_links */, root_copy, copts, false /* dest_is_vfat */);
            Assert.assertTrue( true == FileUtil.remove(root_copy, TraverseOptions.recursive) );

            final UnmountFlags umount_flags;
            if( PlatformProps.OS == PlatformTypes.OSType.LINUX ) {
                umount_flags = new org.jau.fs.linux.UnmountFlags();
                umount_flags.set(org.jau.fs.linux.UnmountFlags.Bit.detach); // lazy
            } else {
                umount_flags = new UnmountFlags();
            }
            PrintUtil.println(System.err, "UnmountFlags for "+PlatformProps.OS+": "+umount_flags);
            final boolean umount_ok = FileUtil.umount(mctx, umount_flags);
            Assert.assertTrue( true == umount_ok );
            Assert.assertTrue(isFolderEmpty(mount_point));

            Assert.assertTrue( true == FileUtil.remove(mount_point, TraverseOptions.recursive) );
        }
    }

    /* pp */ static String test51_devname = null;
    /* pp */ static boolean test51_umount = true;

    @Test(timeout = 10000)
    public void test51_mount_device() {
        PlatformRuntime.checkInitialized();
        PrintUtil.println(System.err, "test51_mount_device\n");
        if( null == test51_devname ) {
            return;
        }
        {
            final String device_file = test51_devname;
            final FileStats device_stats = new FileStats(device_file);
            Assert.assertTrue( true == device_stats.exists() );
            PrintUtil.println(System.err, "Mount "+device_stats);

            final String mount_point = root+"_mount";
            Assert.assertTrue(isFolderEmpty(mount_point));
            FileUtil.remove(mount_point, TraverseOptions.recursive); // start fresh
            Assert.assertTrue( true == FileUtil.mkdir(mount_point, FMode.def_dir) );

            final MountFlags mount_flags;
            if( PlatformProps.OS == PlatformTypes.OSType.LINUX ) {
                mount_flags = new org.jau.fs.linux.MountFlags();
                mount_flags.set(org.jau.fs.linux.MountFlags.Bit.rdonly); // failed: MS_PRIVATE, MS_UNBINDABLE
            } else {
                mount_flags = new MountFlags();
            }
            PrintUtil.println(System.err, "MountFlags for "+PlatformProps.OS+": "+mount_flags);
            final long mctx = FileUtil.mount(device_stats.path(), mount_point, "vfat", mount_flags, "errors=continue,rodir,sys_immutable,usefree");
            Assert.assertTrue( 0 != mctx );
            Assert.assertFalse(isFolderEmpty(mount_point));

            final List<DirItem> files = FileUtil.get_dir_content(mount_point);
            Assert.assertNotNull(files);
            Assert.assertTrue(files.size() > 0 );
            int count=0;
            for(final DirItem item : files) {
                ++count;
                PrintUtil.fprintf_td(System.err, "%,4d: %s\n", count, item);
            }

            if( test51_umount ) {
                final UnmountFlags umount_flags;
                if( PlatformProps.OS == PlatformTypes.OSType.LINUX ) {
                    umount_flags = new org.jau.fs.linux.UnmountFlags();
                    umount_flags.set(org.jau.fs.linux.UnmountFlags.Bit.detach); // lazy
                } else {
                    umount_flags = new UnmountFlags();
                }
                PrintUtil.println(System.err, "UnmountFlags for "+PlatformProps.OS+": "+umount_flags);
                final boolean umount_ok = FileUtil.umount(mount_point, umount_flags);
                Assert.assertTrue( true == umount_ok );
                Assert.assertTrue(isFolderEmpty(mount_point));

                Assert.assertTrue( true == FileUtil.remove(mount_point, TraverseOptions.recursive) );
            }
        }
    }

    public static void main(final String args[]) {
        final int argc = args.length;
        for(int i=0; i< argc; i++) {
            final String arg = args[i];
            if( arg.equals("-mount_device") && args.length > (i+1) ) {
                test51_devname = args[++i];
            } else if( arg.equals("-no_umount") ) {
                test51_umount = false;
            }
        }
        org.junit.runner.JUnitCore.main(TestsudoFileUtils02.class.getName());
    }
}