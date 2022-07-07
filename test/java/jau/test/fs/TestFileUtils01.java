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

import java.time.Instant;
import java.time.ZoneOffset;
import java.time.temporal.ChronoUnit;

import org.jau.fs.CopyOptions;
import org.jau.fs.DirItem;
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
public class TestFileUtils01 extends FileUtilBaseTest {
    static final boolean DEBUG = false;

    @Test(timeout = 10000)
    public final void test01_cwd() {
        PlatformRuntime.checkInitialized();
        final String cwd = FileUtil.get_cwd();
        PrintUtil.println(System.err, "test01_cwd: cwd "+cwd);
        Assert.assertTrue( 0 < cwd.length() );
        final int idx = cwd.indexOf("/jaulib/");
        Assert.assertTrue( 0 < idx );
        Assert.assertTrue( idx < cwd.length() );
        Assert.assertTrue( idx > 0 );
    }

    /**
     *
     */
    @Test(timeout = 10000)
    public final void test02_dirname() {
        PlatformRuntime.checkInitialized();
        {
            final String pathname0 = "/";
            final String pathname1 = FileUtil.dirname(pathname0);
            PrintUtil.println(System.err, "test02_dirname: cwd "+pathname0+" -> "+pathname1);
            Assert.assertTrue( 0 < pathname1.length() );
            Assert.assertTrue( pathname1.equals( "/" ) );
        }
        {
            {
                final String pathname0 = "lala.txt";
                final String pathname1 = FileUtil.dirname(pathname0);
                PrintUtil.println(System.err, "test02_dirname: cwd "+pathname0+" -> "+pathname1);
                Assert.assertTrue( 0 < pathname1.length() );
                Assert.assertTrue( pathname1.equals( "." ) );
            }
            {
                final String pathname0 = "lala";
                final String pathname1 = FileUtil.dirname(pathname0);
                PrintUtil.println(System.err, "test02_dirname: cwd "+pathname0+" -> "+pathname1);
                Assert.assertTrue( 0 < pathname1.length() );
                Assert.assertTrue( pathname1.equals( "." ) );
            }
            {
                final String pathname0 = "lala/";
                final String pathname1 = FileUtil.dirname(pathname0);
                PrintUtil.println(System.err, "test02_dirname: cwd "+pathname0+" -> "+pathname1);
                Assert.assertTrue( 0 < pathname1.length() );
                Assert.assertTrue( pathname1.equals( "." ) );
            }
        }
        {
            final String pathname0 = "/lala.txt";
            final String pathname1 = FileUtil.dirname(pathname0);
            PrintUtil.println(System.err, "test02_dirname: cwd "+pathname0+" -> "+pathname1);
            Assert.assertTrue( 0 < pathname1.length() );
            Assert.assertTrue( pathname1.equals( "/" ) );
        }
        {
            final String pathname0 = "blabla/jaulib/test/sub.txt";
            final String pathname1 = FileUtil.dirname(pathname0);
            PrintUtil.println(System.err, "test02_dirname: cwd "+pathname0+" -> "+pathname1);
            Assert.assertTrue( 0 < pathname1.length() );
            Assert.assertTrue( pathname1.equals( "blabla/jaulib/test" ) );
        }
        {
            final String pathname0 = "blabla/jaulib/test/sub";
            final String pathname1 = FileUtil.dirname(pathname0);
            PrintUtil.println(System.err, "test02_dirname: cwd "+pathname0+" -> "+pathname1);
            Assert.assertTrue( 0 < pathname1.length() );
            Assert.assertTrue( pathname1.equals( "blabla/jaulib/test" ) );
        }
        {
            final String pathname0 = "blabla/jaulib/test/";
            final String pathname1 = FileUtil.dirname(pathname0);
            PrintUtil.println(System.err, "test02_dirname: cwd "+pathname0+" -> "+pathname1);
            Assert.assertTrue( 0 < pathname1.length() );
            Assert.assertTrue( pathname1.equals( "blabla/jaulib" ) );
        }
        {
            final String pathname0 = "blabla/jaulib/test";
            final String pathname1 = FileUtil.dirname(pathname0);
            PrintUtil.println(System.err, "test02_dirname: cwd "+pathname0+" -> "+pathname1);
            Assert.assertTrue( 0 < pathname1.length() );
            Assert.assertTrue( pathname1.equals( "blabla/jaulib" ) );
        }
    }

    @Test(timeout = 10000)
    public final void test03_basename() {
        PlatformRuntime.checkInitialized();
        {
            final String pathname0 = "/";
            final String pathname1 = FileUtil.basename(pathname0);
            PrintUtil.println(System.err, "test03_basename: cwd "+pathname0+" -> "+pathname1);
            Assert.assertTrue( 0 < pathname1.length() );
            Assert.assertTrue( pathname1.equals( "/" ) );
        }
        {
            {
                final String pathname0 = "lala.txt";
                final String pathname1 = FileUtil.basename(pathname0);
                PrintUtil.println(System.err, "test03_basename: cwd "+pathname0+" -> "+pathname1);
                Assert.assertTrue( 0 < pathname1.length() );
                Assert.assertTrue( pathname1.equals( "lala.txt" ) );
            }
            {
                final String pathname0 = "lala";
                final String pathname1 = FileUtil.basename(pathname0);
                PrintUtil.println(System.err, "test03_basename: cwd "+pathname0+" -> "+pathname1);
                Assert.assertTrue( 0 < pathname1.length() );
                Assert.assertTrue( pathname1.equals( "lala" ) );
            }
            {
                final String pathname0 = "lala/";
                final String pathname1 = FileUtil.basename(pathname0);
                PrintUtil.println(System.err, "test03_basename: cwd "+pathname0+" -> "+pathname1);
                Assert.assertTrue( 0 < pathname1.length() );
                Assert.assertTrue( pathname1.equals( "lala" ) );
            }
        }
        {
            final String pathname0 = "/lala.txt";
            final String pathname1 = FileUtil.basename(pathname0);
            PrintUtil.println(System.err, "test03_basename: cwd "+pathname0+" -> "+pathname1);
            Assert.assertTrue( 0 < pathname1.length() );
            Assert.assertTrue( pathname1.equals( "lala.txt" ) );
        }
        {
            final String pathname0 = "blabla/jaulib/test/sub.txt";
            final String pathname1 = FileUtil.basename(pathname0);
            PrintUtil.println(System.err, "test03_basename: cwd "+pathname0+" -> "+pathname1);
            Assert.assertTrue( 0 < pathname1.length() );
            Assert.assertTrue( pathname1.equals( "sub.txt" ) );
        }

        {
            final String pathname0 = "blabla/jaulib/test/";
            final String pathname1 = FileUtil.basename(pathname0);
            PrintUtil.println(System.err, "test03_basename: cwd "+pathname0+" -> "+pathname1);
            Assert.assertTrue( 0 < pathname1.length() );
            Assert.assertTrue( pathname1.equals( "test" ) );
        }

        {
            final String pathname0 = "blabla/jaulib/test";
            final String pathname1 = FileUtil.basename(pathname0);
            PrintUtil.println(System.err, "test03_basename: cwd "+pathname0+" -> "+pathname1);
            Assert.assertTrue( 0 < pathname1.length() );
            Assert.assertTrue( pathname1.equals( "test" ) );
        }
    }

    @Test(timeout = 10000)
    public final void test04_dir_item() {
        PlatformRuntime.checkInitialized();
        {
            final String dirname_ = "";
            final DirItem di = new DirItem(dirname_);
            PrintUtil.println(System.err, "test04_dir_item: 01 '"+dirname_+"' -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( ".".equals( di.dirname() ) );
            Assert.assertTrue( ".".equals( di.basename() ) );
            Assert.assertTrue( ".".equals( di.path() ) );
        }
        {
            final String dirname_ =".";
            final DirItem di = new DirItem(dirname_);
            PrintUtil.println(System.err, "test04_dir_item: 02 '"+dirname_+"' -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( ".".equals( di.dirname() ) );
            Assert.assertTrue( ".".equals( di.basename() ) );
            Assert.assertTrue( ".".equals( di.path() ) );
        }
        {
            final String dirname_ ="/";
            final DirItem di = new DirItem(dirname_);
            PrintUtil.println(System.err, "test04_dir_item: 03 '"+dirname_+"' -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "/".equals( di.dirname() ) );
            Assert.assertTrue( ".".equals( di.basename() ) );
            Assert.assertTrue( "/".equals( di.path() ) );
        }

        {
            final String path1_ = "lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 10 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( ".".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "lala".equals( di.path() ) );
        }
        {
            final String path1_ = "lala/";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 11 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( ".".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "lala".equals( di.path() ) );
        }

        {
            final String path1_ = "/lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 12 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "/".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "/lala".equals( di.path() ) );
        }

        {
            final String path1_ = "dir0/lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 20 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "dir0".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "dir0/lala".equals( di.path() ) );
        }
        {
            final String path1_ = "dir0/lala/";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 21 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "dir0".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "dir0/lala".equals( di.path() ) );
        }
        {
            final String path1_ = "/dir0/lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 22 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "/dir0".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "/dir0/lala".equals( di.path() ) );
        }
        {
            final String path1_ = "/dir0/lala/";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 23 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "/dir0".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "/dir0/lala".equals( di.path() ) );
        }


        {
            final String path1_ = "/dir0/../lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 30 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "/".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "/lala".equals( di.path() ) );
        }
        {
            final String path1_ = "dir0/../lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 31 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( ".".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "lala".equals( di.path() ) );
        }
        {
            final String path1_ = "../../lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 32 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "../..".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "../../lala".equals( di.path() ) );
        }
        {
            final String path1_ = "./../lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 33 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "..".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "../lala".equals( di.path() ) );
        }
        {
            final String path1_ = "dir0/../../lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 34 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "..".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "../lala".equals( di.path() ) );
        }

        {
            final String path1_ = "dir0/dir1/../lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 40 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "dir0".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "dir0/lala".equals( di.path() ) );
        }
        {
            final String path1_ = "/dir0/dir1/../lala/";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 41 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "/dir0".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "/dir0/lala".equals( di.path() ) );
        }
        {
            final String path1_ = "dir0/dir1/../bbb/ccc/../lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 42 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "dir0/bbb".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "dir0/bbb/lala".equals( di.path() ) );
        }
        {
            final String path1_ = "dir0/dir1/bbb/../../lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 43 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "dir0".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "dir0/lala".equals( di.path() ) );
        }
        {
            final String path1_ = "dir0/dir1/bbb/../../../lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 44 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( ".".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "lala".equals( di.path() ) );
        }
        {
            final String path1_ = "dir0/dir1/bbb/../../../../lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 45 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "..".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "../lala".equals( di.path() ) );
        }
        {
            final String path1_ = "dir0/dir1/bbb/../../lala/..";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 46 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( ".".equals( di.dirname() ) );
            Assert.assertTrue( "dir0".equals( di.basename() ) );
            Assert.assertTrue( "dir0".equals( di.path() ) );
        }
        {
            final String path1_ = "dir0/./dir1/./bbb/../.././lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 50 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "dir0".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "dir0/lala".equals( di.path() ) );
        }
        {
            final String path1_ = "dir0/./dir1/./bbb/../.././lala/.";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 51 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "dir0".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "dir0/lala".equals( di.path() ) );
        }
        {
            final String path1_ = "./dir0/./dir1/./bbb/../.././lala/.";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 51 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "dir0".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "dir0/lala".equals( di.path() ) );
        }
        {
            final String path1_ = "/./dir0/./dir1/./bbb/../.././lala/.";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 52 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "/dir0".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "/dir0/lala".equals( di.path() ) );
        }

        {
            final String path1_ = "../../test_data/file_01_slink09R1.txt";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 60 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "../../test_data".equals( di.dirname() ) );
            Assert.assertTrue( "file_01_slink09R1.txt".equals( di.basename() ) );
            Assert.assertTrue( path1_.equals( di.path() ) );
        }

        {
            final String path1_ = "../../../jaulib/test_data";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 61 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "../../../jaulib".equals( di.dirname() ) );
            Assert.assertTrue( "test_data".equals( di.basename() ) );
            Assert.assertTrue( path1_.equals( di.path() ) );
        }

        {
            final String path1_ = "../../../../jaulib/test_data";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 62 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "../../../../jaulib".equals( di.dirname() ) );
            Assert.assertTrue( "test_data".equals( di.basename() ) );
            Assert.assertTrue( path1_.equals( di.path() ) );
        }

        {
            final String path1_ = "././././jaulib/test_data";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 63 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "jaulib".equals( di.dirname() ) );
            Assert.assertTrue( "test_data".equals( di.basename() ) );
            Assert.assertTrue( "jaulib/test_data".equals( di.path() ) );
        }

        {
            final String path1_ = "a/././././jaulib/test_data";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 64 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "a/jaulib".equals( di.dirname() ) );
            Assert.assertTrue( "test_data".equals( di.basename() ) );
            Assert.assertTrue( "a/jaulib/test_data".equals( di.path() ) );
        }

        {
            // Error
            final String path1_ = "/../lala";
            final DirItem di = new DirItem(path1_);
            PrintUtil.println(System.err, "test04_dir_item: 99 '"+path1_+" -> "+di.toString()+" -> '"+di.path()+"'\n");
            Assert.assertTrue( "/..".equals( di.dirname() ) );
            Assert.assertTrue( "lala".equals( di.basename() ) );
            Assert.assertTrue( "/../lala".equals( di.path() ) );
        }
    }

    @Test(timeout = 10000)
    public final void test05_file_stat() {
        PlatformRuntime.checkInitialized();
        PrintUtil.println(System.err, "test05_file_stat\n");

        {
            final FileStats stats = new FileStats(project_root_ext+"/file_01.txt");
            PrintUtil.fprintf_td(System.err, "test05_file_stat: 01: %s\n", stats);
            PrintUtil.fprintf_td(System.err, "test05_file_stat: 01: fields %s\n", stats.fields());
            if( stats.exists() ) {
                Assert.assertTrue(  stats.has_access() );
                Assert.assertTrue( !stats.is_dir() );
                Assert.assertTrue(  stats.is_file() );
                Assert.assertTrue( !stats.is_link() );
                Assert.assertTrue( 15 == stats.size() );
            }
        }

        FileStats proot_stats = new FileStats(project_root1);
        if( !proot_stats.exists() ) {
            proot_stats = new FileStats(project_root2);
        }
        PrintUtil.fprintf_td(System.err, "test05_file_stat: 11: %s\n", proot_stats);
        PrintUtil.fprintf_td(System.err, "test05_file_stat: 11: fields %s\n", proot_stats.fields());
        Assert.assertTrue( true == proot_stats.exists() );
        Assert.assertTrue( true == proot_stats.is_dir() );

        {
            final FileStats stats = new FileStats(proot_stats.path()+"/file_01.txt");
            PrintUtil.fprintf_td(System.err, "test05_file_stat: 12: %s\n", stats);
            PrintUtil.fprintf_td(System.err, "test05_file_stat: 12: fields %s\n", stats.fields());
            Assert.assertTrue(  stats.exists() );
            Assert.assertTrue(  stats.has_access() );
            Assert.assertTrue( !stats.is_dir() );
            Assert.assertTrue(  stats.is_file() );
            Assert.assertTrue( !stats.is_link() );
            Assert.assertTrue( 15 == stats.size() );

            final long link_count[] = { 0 };
            final FileStats final_target = stats.final_target(link_count);
            PrintUtil.fprintf_td(System.err, "test05_file_stat: 12: final_target (%d link count): %s\n", link_count[0], final_target);
            Assert.assertTrue( 0 == link_count[0] );
            Assert.assertTrue( final_target.equals( stats ) );

            {
                final FileStats stats2 = new FileStats(proot_stats.path()+"/file_01.txt");
                Assert.assertTrue(  stats2.exists() );
                Assert.assertTrue(  stats2.has_access() );
                Assert.assertTrue( !stats2.is_dir() );
                Assert.assertTrue(  stats2.is_file() );
                Assert.assertTrue( !stats2.is_link() );
                Assert.assertTrue( 15 == stats2.size() );
                Assert.assertEquals( stats, stats2 );
            }
            {
                final FileStats stats2 = new FileStats(proot_stats.path()+"/dir_01/file_02.txt");
                Assert.assertTrue(  stats2.exists() );
                Assert.assertTrue(  stats2.has_access() );
                Assert.assertTrue( !stats2.is_dir() );
                Assert.assertTrue(  stats2.is_file() );
                Assert.assertTrue( !stats2.is_link() );
                Assert.assertNotEquals( stats, stats2 );
            }
        }
        {
            final FileStats stats = new FileStats(proot_stats.path()+"/dir_01");
            PrintUtil.fprintf_td(System.err, "test05_file_stat: 13: %s\n", stats);
            PrintUtil.fprintf_td(System.err, "test05_file_stat: 13: fields %s\n", stats.fields());
            Assert.assertTrue(  stats.exists() );
            Assert.assertTrue(  stats.has_access() );
            Assert.assertTrue(  stats.is_dir() );
            Assert.assertTrue( !stats.is_file() );
            Assert.assertTrue( !stats.is_link() );
            Assert.assertTrue( 0 == stats.size() );

            final long link_count[] = { 0 };
            final FileStats final_target = stats.final_target(link_count);
            PrintUtil.fprintf_td(System.err, "test05_file_stat: 13: final_target (%d link count): %s\n", link_count[0], final_target);
            Assert.assertTrue( 0 == link_count[0] );
            Assert.assertTrue( final_target.equals( stats ) );
        }
        {
            final FileStats stats = new FileStats(proot_stats.path()+"/does_not_exist");
            PrintUtil.fprintf_td(System.err, "test05_file_stat: 14: %s\n", stats);
            PrintUtil.fprintf_td(System.err, "test05_file_stat: 14: fields %s\n", stats.fields());
            Assert.assertTrue( !stats.exists() );
            Assert.assertTrue(  stats.has_access() );
            Assert.assertTrue( !stats.is_dir() );
            Assert.assertTrue( !stats.is_file() );
            Assert.assertTrue( !stats.is_link() );
            Assert.assertTrue( 0 == stats.size() );

            final long link_count[] = { 0 };
            final FileStats final_target = stats.final_target(link_count);
            PrintUtil.fprintf_td(System.err, "test05_file_stat: 14: final_target (%d link count): %s\n", link_count[0], final_target);
            Assert.assertTrue( 0 == link_count[0] );
            Assert.assertTrue( final_target.equals( stats ) );
        }
    }

    @Test(timeout = 10000)
    public final void test06_file_stat_symlinks() {
        PlatformRuntime.checkInitialized();
        PrintUtil.println(System.err, "test06_file_stat_symlinks\n");

        FileStats proot_stats = new FileStats(project_root1);
        if( !proot_stats.exists() ) {
            proot_stats = new FileStats(project_root2);
        }
        Assert.assertTrue( true == proot_stats.exists() );
        Assert.assertTrue( true == proot_stats.is_dir() );

        {
            final FileStats stats = new FileStats(proot_stats.path()+"/file_01_slink01.txt");
            PrintUtil.fprintf_td(System.err, "test06_file_stat_symlinks: 13: %s\n", stats);
            PrintUtil.fprintf_td(System.err, "test06_file_stat_symlinks: 13: fields %s\n", stats.fields());
            Assert.assertTrue(  stats.exists() );
            Assert.assertTrue(  stats.has_access() );
            Assert.assertTrue( !stats.is_dir() );
            Assert.assertTrue(  stats.is_file() );
            Assert.assertTrue(  stats.is_link() );
            Assert.assertTrue( 15 == stats.size() );
            Assert.assertTrue( null != stats.link_target_path() );
            Assert.assertTrue( "file_01.txt".equals( stats.link_target_path() ) );

            final long link_count[] = { 0 };
            final FileStats final_target = stats.final_target(link_count);
            PrintUtil.fprintf_td(System.err, "- final_target (%d link count): %s\n", link_count[0], final_target);
            Assert.assertTrue( 1 == link_count[0] );
            Assert.assertTrue( final_target != stats );
            Assert.assertTrue( (proot_stats.path()+"/file_01.txt").equals( final_target.path() ) );

            Assert.assertTrue( null != stats.link_target() );
            final FileStats link_target = stats.link_target();
            Assert.assertTrue( null != link_target );
            PrintUtil.fprintf_td(System.err, "- link_target %s\n", link_target);
            Assert.assertTrue( final_target.equals( link_target ) );
            Assert.assertTrue( !link_target.is_dir() );
            Assert.assertTrue(  link_target.is_file() );
            Assert.assertTrue( !link_target.is_link() );
            Assert.assertTrue( null == link_target.link_target_path() );
            Assert.assertTrue( null == link_target.link_target() );
        }
        {
            final FileStats stats = new FileStats(proot_stats.path()+"/fstab_slink07_absolute");
            PrintUtil.fprintf_td(System.err, "test06_file_stat_symlinks: 14: %s\n", stats);
            PrintUtil.fprintf_td(System.err, "test06_file_stat_symlinks: 14: fields %s\n", stats.fields());
            Assert.assertTrue(  stats.exists() );
            Assert.assertTrue(  stats.has_access() );
            Assert.assertTrue( !stats.is_dir() );
            Assert.assertTrue(  stats.is_file() );
            Assert.assertTrue(  stats.is_link() );
            Assert.assertTrue( 20 < stats.size() ); // greater than basename
            Assert.assertTrue( null != stats.link_target_path() );
            Assert.assertTrue( "/etc/fstab".equals( stats.link_target_path() ) );

            final long link_count[] = { 0 };
            final FileStats final_target = stats.final_target(link_count);
            PrintUtil.fprintf_td(System.err, "- final_target (%d link count): %s\n", link_count[0], final_target);
            Assert.assertTrue( 1 == link_count[0] );
            Assert.assertTrue( final_target != stats );
            Assert.assertTrue( "/etc/fstab".equals( final_target.path() ) );

            Assert.assertTrue( null != stats.link_target() );
            final FileStats link_target = stats.link_target();
            Assert.assertTrue( null != link_target );
            PrintUtil.fprintf_td(System.err, "- link_target %s\n", link_target);
            Assert.assertTrue( final_target.equals( link_target ) );
            Assert.assertTrue( !link_target.is_dir() );
            Assert.assertTrue(  link_target.is_file() );
            Assert.assertTrue( !link_target.is_link() );
            Assert.assertTrue( null == link_target.link_target_path() );
            Assert.assertTrue( null == link_target.link_target() );
        }
        {
            final FileStats stats = new FileStats(proot_stats.path()+"/file_01_slink10R2.txt"); // -> file_01_slink09R1.txt -> file_01_slink01.txt -> file_01.txt
            PrintUtil.fprintf_td(System.err, "test06_file_stat_symlinks: 20: %s\n", stats);
            PrintUtil.fprintf_td(System.err, "test06_file_stat_symlinks: 20: fields %s\n", stats.fields());
            Assert.assertTrue(  stats.exists() );
            Assert.assertTrue(  stats.has_access() );
            Assert.assertTrue( !stats.is_dir() );
            Assert.assertTrue(  stats.is_file() );
            Assert.assertTrue(  stats.is_link() );
            Assert.assertTrue( 15 == stats.size() );
            Assert.assertTrue( null != stats.link_target_path() );
            Assert.assertTrue( "file_01_slink09R1.txt".equals( stats.link_target_path() ) );

            final long link_count[] = { 0 };
            final FileStats final_target = stats.final_target(link_count);
            PrintUtil.fprintf_td(System.err, "- final_target (%d link count): %s\n", link_count[0], final_target);
            Assert.assertTrue( 3 == link_count[0] );
            Assert.assertTrue( final_target != stats );
            Assert.assertTrue( (proot_stats.path()+"/file_01.txt").equals( final_target.path() ) );

            Assert.assertTrue( null != stats.link_target() );
            final FileStats link_target1 = stats.link_target();
            PrintUtil.fprintf_td(System.err, "- link_target1 %s\n", link_target1);
            Assert.assertTrue( final_target != link_target1 );
            Assert.assertTrue( (proot_stats.path()+"/file_01_slink09R1.txt").equals( link_target1.path() ) );
            Assert.assertTrue( 15 == link_target1.size() );
            Assert.assertTrue( !link_target1.is_dir() );
            Assert.assertTrue(  link_target1.is_file() );
            Assert.assertTrue(  link_target1.is_link() );
            Assert.assertTrue( null != link_target1.link_target_path() );
            Assert.assertTrue( "file_01_slink01.txt".equals( link_target1.link_target_path() ) );
            {
                final FileStats link_target2 = link_target1.link_target();
                Assert.assertTrue( null != link_target2 );
                PrintUtil.fprintf_td(System.err, "  - link_target2 %s\n", link_target2);
                Assert.assertTrue( final_target != link_target2 );
                Assert.assertTrue( link_target1 != link_target2 );
                Assert.assertTrue( (proot_stats.path()+"/file_01_slink01.txt").equals( link_target2.path() ) );
                Assert.assertTrue( 15 == link_target2.size() );
                Assert.assertTrue( !link_target2.is_dir() );
                Assert.assertTrue(  link_target2.is_file() );
                Assert.assertTrue(  link_target2.is_link() );
                Assert.assertTrue( null != link_target2.link_target_path() );
                Assert.assertTrue( "file_01.txt".equals( link_target2.link_target_path() ) );

                final FileStats link_target3 = link_target2.link_target();
                Assert.assertTrue( null != link_target3 );
                PrintUtil.fprintf_td(System.err, "    - link_target3 %s\n", link_target3);
                Assert.assertTrue( final_target.equals( link_target3 ) );
                Assert.assertTrue( link_target1 != link_target3 );
                Assert.assertTrue( link_target2 != link_target3 );
                Assert.assertTrue( 15 == link_target3.size() );
                Assert.assertTrue( !link_target3.is_dir() );
                Assert.assertTrue(  link_target3.is_file() );
                Assert.assertTrue( !link_target3.is_link() );
                Assert.assertTrue( null == link_target3.link_target_path() );
                Assert.assertTrue( null == link_target3.link_target() );
            }
        }
        {
            final FileStats stats = new FileStats(proot_stats.path()+"/dead_link23"); // -> not_existing_file
            PrintUtil.fprintf_td(System.err, "test06_file_stat_symlinks: 30: %s\n", stats);
            PrintUtil.fprintf_td(System.err, "test06_file_stat_symlinks: 30: fields %s\n", stats.fields());
            Assert.assertTrue( !stats.exists() );
            Assert.assertTrue(  stats.has_access() );
            Assert.assertTrue( !stats.is_dir() );
            Assert.assertTrue( !stats.is_file() );
            Assert.assertTrue(  stats.is_link() );
            Assert.assertTrue( 0 == stats.size() );
            Assert.assertTrue( null != stats.link_target_path() );
            Assert.assertTrue( "not_existing_file".equals( stats.link_target_path() ) );
            Assert.assertTrue( null == stats.link_target() );

            final long link_count[] = { 0 };
            final FileStats final_target = stats.final_target(link_count);
            PrintUtil.fprintf_td(System.err, "- final_target (%d link count): %s\n", link_count[0], final_target);
            Assert.assertTrue( 0 == link_count[0] );
            Assert.assertTrue( final_target.equals( stats ) );
        }
        {
            final FileStats stats = new FileStats(proot_stats.path()+"/dead_link22"); // LOOP: dead_link22 -> dead_link21 -> dead_link20 -> dead_link22 ...
            PrintUtil.fprintf_td(System.err, "test06_file_stat_symlinks: 31: %s\n", stats);
            PrintUtil.fprintf_td(System.err, "test06_file_stat_symlinks: 31: fields %s\n", stats.fields());
            Assert.assertTrue( !stats.exists() );
            Assert.assertTrue(  stats.has_access() );
            Assert.assertTrue( !stats.is_dir() );
            Assert.assertTrue( !stats.is_file() );
            Assert.assertTrue(  stats.is_link() );
            Assert.assertTrue( 0 == stats.size() );
            Assert.assertTrue( null != stats.link_target_path() );
            Assert.assertTrue( "dead_link21".equals( stats.link_target_path() ) );
            Assert.assertTrue( null == stats.link_target() );

            final long link_count[] = { 0 };
            final FileStats final_target = stats.final_target(link_count);
            PrintUtil.fprintf_td(System.err, "- final_target (%d link count): %s\n", link_count[0], final_target);
            Assert.assertTrue( 0 == link_count[0] );
            Assert.assertTrue( final_target.equals( stats ) );
        }
    }

    @Test(timeout = 10000)
    public final void test10_mkdir() {
        PlatformRuntime.checkInitialized();
        PrintUtil.println(System.err, "test10_mkdir\n");

        FileUtil.remove(root, topts_rec); // start fresh
        {
            final FileStats root_stats = new FileStats(root);
            PrintUtil.println(System.err, "root_stats.pre: "+root_stats);
            Assert.assertTrue( !root_stats.exists() );
            Assert.assertTrue( root_stats.has_access() );
            Assert.assertTrue( !root_stats.is_dir() );
            Assert.assertTrue( !root_stats.is_file() );
            Assert.assertTrue( !root_stats.is_link() );
        }
        final FMode mode_def_dir = new FMode(FMode.Bit.def_dir_prot.value);
        Assert.assertTrue( true == FileUtil.mkdir(root, mode_def_dir) );
        {
            final FileStats root_stats = new FileStats(root);
            PrintUtil.println(System.err, "root_stats.post: "+root_stats);
            Assert.assertTrue( root_stats.exists() );
            Assert.assertTrue( root_stats.has_access() );
            Assert.assertTrue( root_stats.is_dir() );
            Assert.assertTrue( !root_stats.is_file() );
            Assert.assertTrue( !root_stats.is_link() );
        }
        Assert.assertTrue( false == FileUtil.remove(root, topts_none) );
        Assert.assertTrue( true == FileUtil.remove(root, topts_rec) );
    }

    @Test(timeout = 10000)
    public void test11_touch() {
        PlatformRuntime.checkInitialized();
        PrintUtil.println(System.err, "test11_touch\n");
        final String file_01 = root+"/data01.txt";
        final String file_02 = root+"/data02.txt";
        Assert.assertTrue( true == FileUtil.mkdir(root, FMode.def_dir) );
        {
            final FileStats root_stats = new FileStats(root);
            PrintUtil.println(System.err, "root_stats1.post: "+root_stats);
            Assert.assertTrue( root_stats.exists() );
            Assert.assertTrue( root_stats.has_access() );
            Assert.assertTrue( root_stats.is_dir() );
            Assert.assertTrue( !root_stats.is_file() );
            Assert.assertTrue( !root_stats.is_link() );
        }

        Assert.assertTrue( true == FileUtil.touch(file_01, FMode.def_dir) );
        {
            final Instant now = Instant.now();
            final FileStats file_stats = new FileStats(file_01);
            PrintUtil.println(System.err, "file_stats2.post: "+file_stats);
            final Instant btime = file_stats.btime();
            final Instant atime = file_stats.atime();
            final long atime_td_ms = atime.until(now, ChronoUnit.MILLIS);
            final Instant mtime = file_stats.mtime();
            final long mtime_td_ms = mtime.until(now, ChronoUnit.MILLIS);
            PrintUtil.println(System.err, "now:   "+now.atZone(ZoneOffset.UTC));
            PrintUtil.println(System.err, "btime: "+btime.atZone(ZoneOffset.UTC));
            PrintUtil.println(System.err, "atime: "+atime.atZone(ZoneOffset.UTC)+", td_now "+atime_td_ms+"ms");
            PrintUtil.println(System.err, "mtime: "+mtime.atZone(ZoneOffset.UTC)+", td_now "+mtime_td_ms+"ms");
            Assert.assertTrue( file_stats.exists() );
            Assert.assertTrue( file_stats.has_access() );
            Assert.assertTrue( !file_stats.is_dir() );
            Assert.assertTrue( file_stats.is_file() );
            Assert.assertTrue( !file_stats.is_link() );
            if( file_stats.has( FileStats.Field.Type.atime ) ) {
                Assert.assertTrue( 1000 >= atime_td_ms );
            }
            if( file_stats.has( FileStats.Field.Type.mtime ) ) {
                Assert.assertTrue( 1000 >= mtime_td_ms );
            }
        }

        Assert.assertTrue( true == FileUtil.touch(file_02, FMode.def_dir ) );
        {
            final Instant now = Instant.now();
            final FileStats file_stats_pre = new FileStats(file_02);
            final Instant btime_pre = file_stats_pre.btime();
            final Instant atime_pre = file_stats_pre.atime();
            final long atime_td_ms = atime_pre.until(now, ChronoUnit.MILLIS);
            final Instant mtime_pre = file_stats_pre.mtime();
            final long mtime_td_ms = mtime_pre.until(now, ChronoUnit.MILLIS);
            PrintUtil.println(System.err, "now      : "+now.atZone(ZoneOffset.UTC));
            PrintUtil.println(System.err, "btime.pre: "+btime_pre.atZone(ZoneOffset.UTC));
            PrintUtil.println(System.err, "atime.pre: "+atime_pre.atZone(ZoneOffset.UTC)+", td_now "+atime_td_ms+"ms");
            PrintUtil.println(System.err, "mtime.pre: "+mtime_pre.atZone(ZoneOffset.UTC)+", td_now "+mtime_td_ms+"ms");
            if( file_stats_pre.has( FileStats.Field.Type.atime ) ) {
                Assert.assertTrue( 1000 >= atime_td_ms );
            }
            if( file_stats_pre.has( FileStats.Field.Type.mtime ) ) {
                Assert.assertTrue( 1000 >= mtime_td_ms );
            }

            // final jau::fraction_timespec ts_20200101( 1577836800_s + 0_h); // 2020-01-01 00:00:00
            final Instant ts_20200101 = Instant.ofEpochSecond(1577836800); // 2020-01-01 00:00:00
            final Instant atime_set = ts_20200101.plus( 1, ChronoUnit.DAYS).plus(10, ChronoUnit.HOURS);
            final Instant mtime_set = ts_20200101.plus( 31, ChronoUnit.DAYS).plus(10, ChronoUnit.HOURS);
            PrintUtil.println(System.err, "atime.set: "+atime_set.atZone(ZoneOffset.UTC)+", "+atime_set.atZone(ZoneOffset.UTC));
            PrintUtil.println(System.err, "mtime.set: "+mtime_set.atZone(ZoneOffset.UTC)+", "+mtime_set.atZone(ZoneOffset.UTC));
            Assert.assertTrue( true == FileUtil.touch(file_02, atime_set, mtime_set, FMode.def_file) );

            final FileStats file_stats_post = new FileStats(file_02);
            final Instant atime_post = file_stats_post.atime();
            final Instant mtime_post = file_stats_post.mtime();
            PrintUtil.println(System.err, "atime.post: "+atime_post.atZone(ZoneOffset.UTC)+", "+atime_post.atZone(ZoneOffset.UTC));
            PrintUtil.println(System.err, "mtime.post: "+mtime_post.atZone(ZoneOffset.UTC)+", "+mtime_post.atZone(ZoneOffset.UTC));
            PrintUtil.fprintf_td(System.err, "test11_touch: 03: %s\n", file_stats_post);
            {
                Assert.assertTrue( file_stats_post.exists() );
                Assert.assertTrue( file_stats_post.has_access() );
                Assert.assertTrue( !file_stats_post.is_dir() );
                Assert.assertTrue( file_stats_post.is_file() );
                Assert.assertTrue( !file_stats_post.is_link() );
                if( file_stats_post.has( FileStats.Field.Type.atime ) ) {
                    Assert.assertTrue( atime_set.equals( file_stats_post.atime() ) );
                }
                if( file_stats_post.has( FileStats.Field.Type.mtime ) ) {
                    Assert.assertTrue( mtime_set.equals( file_stats_post.mtime() ) );
                }
            }
        }

        Assert.assertTrue( true == FileUtil.remove(root, topts_rec) );
    }

    @Test(timeout = 10000)
    public void test20_visit() {
        PlatformRuntime.checkInitialized();
        PrintUtil.println(System.err, "test20_visit\n");

        final String sub_dir1 = root+"/sub1";
        final String sub_dir2 = root+"/sub2";
        final String sub_dir3 = root+"/sub1/sub3";

        Assert.assertTrue( true == FileUtil.mkdir(root, FMode.def_dir) );
        Assert.assertTrue( true == FileUtil.touch(root+"/data01.txt", FMode.def_file) );
        Assert.assertTrue( true == FileUtil.touch(root+"/data02.txt", FMode.def_file) );
        Assert.assertTrue( true == FileUtil.mkdir(sub_dir1, FMode.def_dir) );
        Assert.assertTrue( true == FileUtil.mkdir(sub_dir2, FMode.def_dir) );
        Assert.assertTrue( true == FileUtil.mkdir(sub_dir3, FMode.def_dir) );
        Assert.assertTrue( true == FileUtil.touch(sub_dir1+"/data03.txt", FMode.def_file) );
        Assert.assertTrue( true == FileUtil.touch(sub_dir1+"/data04.txt", FMode.def_file) );
        Assert.assertTrue( true == FileUtil.touch(sub_dir2+"/data05.txt", FMode.def_file) );
        Assert.assertTrue( true == FileUtil.touch(sub_dir2+"/data06.txt", FMode.def_file) );
        Assert.assertTrue( true == FileUtil.touch(sub_dir3+"/data07.txt", FMode.def_file) );
        Assert.assertTrue( true == FileUtil.touch(sub_dir3+"/data08.txt", FMode.def_file) );

        final TraverseOptions topts_R_FSL_PDL = new TraverseOptions();
        topts_R_FSL_PDL.set(TraverseOptions.Bit.recursive);
        topts_R_FSL_PDL.set(TraverseOptions.Bit.follow_symlinks);
        topts_R_FSL_PDL.set(TraverseOptions.Bit.dir_exit);
        final VisitorStats stats_R_FSL_PDL = new VisitorStats(topts_R_FSL_PDL);
        {
            final PathStatsVisitor pv = new PathStatsVisitor(stats_R_FSL_PDL);
            Assert.assertTrue( true == FileUtil.visit(root, topts_R_FSL_PDL, pv) );
            PrintUtil.fprintf_td(System.err, "test20_visit[R, FSL, PDL]: %s\n%s\n", topts_R_FSL_PDL, stats_R_FSL_PDL);
            Assert.assertTrue( 12 == stats_R_FSL_PDL.total_real );
            Assert.assertTrue(  0 == stats_R_FSL_PDL.total_sym_links_existing );
            Assert.assertTrue(  0 == stats_R_FSL_PDL.total_sym_links_not_existing );
            Assert.assertTrue(  0 == stats_R_FSL_PDL.total_no_access );
            Assert.assertTrue(  0 == stats_R_FSL_PDL.total_not_existing );
            Assert.assertTrue(  0 == stats_R_FSL_PDL.total_file_bytes );
            Assert.assertTrue(  8 == stats_R_FSL_PDL.files_real );
            Assert.assertTrue(  0 == stats_R_FSL_PDL.files_sym_link );
            Assert.assertTrue(  4 == stats_R_FSL_PDL.dirs_real );
            Assert.assertTrue(  0 == stats_R_FSL_PDL.dirs_sym_link );
        }
        final TraverseOptions topts_R_FSL = new TraverseOptions();
        topts_R_FSL.set(TraverseOptions.Bit.recursive);
        topts_R_FSL.set(TraverseOptions.Bit.follow_symlinks);
        topts_R_FSL.set(TraverseOptions.Bit.dir_entry);
        final VisitorStats stats_R_FSL = new VisitorStats(topts_R_FSL);
        {
            final PathStatsVisitor pv = new PathStatsVisitor(stats_R_FSL);
            Assert.assertTrue( true == FileUtil.visit(root, topts_R_FSL, pv) );
            PrintUtil.fprintf_td(System.err, "test20_visit[R, FSL]: %s\n%s\n", topts_R_FSL, stats_R_FSL);
            Assert.assertTrue( stats_R_FSL_PDL.equals( stats_R_FSL ) );
        }
        Assert.assertTrue( true == FileUtil.remove(root, topts_rec) );
    }

    @Test(timeout = 10000)
    public void test22_visit_symlinks() {
        PlatformRuntime.checkInitialized();
        PrintUtil.println(System.err, "test22_visit_symlinks\n");

        FileStats proot_stats = new FileStats(project_root1);
        if( !proot_stats.exists() ) {
            proot_stats = new FileStats(project_root2);
        }
        Assert.assertTrue( true == proot_stats.exists() );
        Assert.assertTrue( true == proot_stats.is_dir() );

        {
            final TraverseOptions topts = new TraverseOptions();
            topts.set(TraverseOptions.Bit.recursive);
            topts.set(TraverseOptions.Bit.dir_exit);
            final VisitorStats stats = new VisitorStats(topts);

            final PathStatsVisitor pv = new PathStatsVisitor(stats);
            Assert.assertTrue( true == FileUtil.visit(proot_stats, topts, pv) );
            PrintUtil.fprintf_td(System.err, "test22_visit[R]: %s\n%s\n", topts, stats);
            Assert.assertTrue(  7 == stats.total_real );
            Assert.assertTrue( 10 == stats.total_sym_links_existing );
            Assert.assertTrue(  4 == stats.total_sym_links_not_existing );
            Assert.assertTrue(  0 == stats.total_no_access );
            Assert.assertTrue(  4 == stats.total_not_existing );
            Assert.assertTrue( 60 == stats.total_file_bytes );
            Assert.assertTrue(  4 == stats.files_real );
            Assert.assertTrue(  9 == stats.files_sym_link );
            Assert.assertTrue(  3 == stats.dirs_real );
            Assert.assertTrue(  1 == stats.dirs_sym_link );
        }

        {
            final TraverseOptions topts = new TraverseOptions();
            topts.set(TraverseOptions.Bit.recursive);
            topts.set(TraverseOptions.Bit.dir_entry);
            topts.set(TraverseOptions.Bit.follow_symlinks);
            final VisitorStats stats = new VisitorStats(topts);

            final PathStatsVisitor pv = new PathStatsVisitor(stats);
            Assert.assertTrue( true == FileUtil.visit(proot_stats, topts, pv) );
            PrintUtil.fprintf_td(System.err, "test22_visit[R, FSL]: %s\n%s\n", topts, stats);
            Assert.assertTrue(  9 == stats.total_real );
            Assert.assertTrue( 11 == stats.total_sym_links_existing );
            Assert.assertTrue(  4 == stats.total_sym_links_not_existing );
            Assert.assertTrue(  0 == stats.total_no_access );
            Assert.assertTrue(  4 == stats.total_not_existing );
            Assert.assertTrue( 60 <  stats.total_file_bytes ); // some followed symlink files are of unknown size, e.g. /etc/fstab
            Assert.assertTrue(  6 == stats.files_real );
            Assert.assertTrue( 10 == stats.files_sym_link );
            Assert.assertTrue(  3 == stats.dirs_real );
            Assert.assertTrue(  1 == stats.dirs_sym_link );
        }
    }

    @Test(timeout = 10000)
    public void test30_copy_file2dir() {
        PlatformRuntime.checkInitialized();
        PrintUtil.println(System.err, "test30_copy_file2dir\n");

        FileStats root_orig_stats = new FileStats(project_root1);
        if( !root_orig_stats.exists() ) {
            root_orig_stats = new FileStats(project_root2);
        }
        Assert.assertTrue( true == root_orig_stats.exists() );
        Assert.assertTrue( true == root_orig_stats.is_dir() );

        final String root_copy = root+"_copy_test30";
        {
            // Fresh target folder
            FileUtil.remove(root_copy, topts_rec);

            Assert.assertTrue( true == FileUtil.mkdir(root_copy, FMode.def_dir) );
            {
                final FileStats stats = new FileStats(root_copy);
                Assert.assertTrue( true == stats.exists() );
                Assert.assertTrue( true == stats.ok() );
                Assert.assertTrue( true == stats.is_dir() );
            }
        }
        final FileStats source1_stats = new FileStats(root_orig_stats.path()+"/file_01.txt");
        PrintUtil.fprintf_td(System.err, "test30_copy_file2dir: source1: %s\n", source1_stats);
        {
            Assert.assertTrue( true == source1_stats.exists() );
            Assert.assertTrue( true == source1_stats.ok() );
            Assert.assertTrue( true == source1_stats.is_file() );
        }
        {
            // Copy file to folder
            final CopyOptions copts = new CopyOptions();
            copts.set(CopyOptions.Bit.preserve_all);
            copts.set(CopyOptions.Bit.verbose);
            {
                final FileStats dest_stats = new FileStats(root_copy+"/file_01.txt");
                PrintUtil.fprintf_td(System.err, "test30_copy_file2dir: 01: dest.pre: %s\n", dest_stats);
                Assert.assertTrue( false == dest_stats.exists() );
            }
            Assert.assertTrue( true == FileUtil.copy(source1_stats.path(), root_copy, copts) );
            {
                final FileStats dest_stats = new FileStats(root_copy+"/file_01.txt");
                PrintUtil.fprintf_td(System.err, "test30_copy_file2dir: 01: dest.post: %s\n", dest_stats);
                Assert.assertTrue( true == dest_stats.exists() );
                Assert.assertTrue( true == dest_stats.ok() );
                Assert.assertTrue( true == dest_stats.is_file() );
                Assert.assertTrue( source1_stats.size() == dest_stats.size() );
                Assert.assertTrue( source1_stats.mode().equals( dest_stats.mode() ) );
            }
        }
        {
            // Error: already exists of 'Copy file to folder'
            final CopyOptions copts = new CopyOptions();
            copts.set(CopyOptions.Bit.preserve_all);
            copts.set(CopyOptions.Bit.verbose);
            {
                final FileStats dest_stats = new FileStats(root_copy+"/file_01.txt");
                PrintUtil.fprintf_td(System.err, "test30_copy_file2dir: 02: dest.pre: %s\n", dest_stats);
                Assert.assertTrue( true == dest_stats.exists() );
                Assert.assertTrue( true == dest_stats.ok() );
                Assert.assertTrue( true == dest_stats.is_file() );
            }
            Assert.assertTrue( false == FileUtil.copy(source1_stats.path(), root_copy, copts) );
        }
        {
            // Overwrite copy file to folder
            final CopyOptions copts = new CopyOptions();
            copts.set(CopyOptions.Bit.preserve_all);
            copts.set(CopyOptions.Bit.overwrite);
            copts.set(CopyOptions.Bit.verbose);

            PrintUtil.fprintf_td(System.err, "test30_copy_file2dir: 03: source: %s\n", source1_stats);
            {
                final FileStats dest_stats = new FileStats(root_copy+"/file_01.txt");
                PrintUtil.fprintf_td(System.err, "test30_copy_file2dir: 03: dest.pre: %s\n", dest_stats);
                Assert.assertTrue( true == dest_stats.exists() );
                Assert.assertTrue( true == dest_stats.ok() );
                Assert.assertTrue( true == dest_stats.is_file() );
                Assert.assertTrue( source1_stats.size() == dest_stats.size() );
                Assert.assertTrue( source1_stats.mode().equals( dest_stats.mode() ) );
            }
            Assert.assertTrue( true == FileUtil.copy(source1_stats.path(), root_copy, copts) );
            {
                final FileStats dest_stats = new FileStats(root_copy+"/file_01.txt");
                PrintUtil.fprintf_td(System.err, "test30_copy_file2dir: 03: dest.post: %s\n", dest_stats);
                Assert.assertTrue( true == dest_stats.exists() );
                Assert.assertTrue( true == dest_stats.ok() );
                Assert.assertTrue( true == dest_stats.is_file() );
                Assert.assertTrue( source1_stats.size() == dest_stats.size() );
                Assert.assertTrue( source1_stats.mode().equals( dest_stats.mode() ) );
            }
        }
        Assert.assertTrue( true == FileUtil.remove(root_copy, topts_rec) );
    }

    @Test(timeout = 10000)
    public void test31_copy_file2file() {
        PlatformRuntime.checkInitialized();
        PrintUtil.println(System.err, "test31_copy_file2file\n");

        FileStats root_orig_stats = new FileStats(project_root1);
        if( !root_orig_stats.exists() ) {
            root_orig_stats = new FileStats(project_root2);
        }
        Assert.assertTrue( true == root_orig_stats.exists() );
        Assert.assertTrue( true == root_orig_stats.is_dir() );

        final String root_copy = root+"_copy_test31";
        {
            // Fresh target folder
            FileUtil.remove(root_copy, topts_rec);

            Assert.assertTrue( true == FileUtil.mkdir(root_copy, FMode.def_dir) );
            {
                final FileStats stats = new FileStats(root_copy);
                Assert.assertTrue( true == stats.exists() );
                Assert.assertTrue( true == stats.ok() );
                Assert.assertTrue( true == stats.is_dir() );
            }
        }
        final FileStats source1_stats = new FileStats(root_orig_stats.path()+"/file_01.txt");
        PrintUtil.fprintf_td(System.err, "test31_copy_file2file: source1: %s\n", source1_stats);
        {
            Assert.assertTrue( true == source1_stats.exists() );
            Assert.assertTrue( true == source1_stats.ok() );
            Assert.assertTrue( true == source1_stats.is_file() );
        }
        final FileStats source2_stats = new FileStats(root_orig_stats.path()+"/README_slink08_relext.txt");
        PrintUtil.fprintf_td(System.err, "test31_copy_file2file: source2: %s\n", source2_stats);
        {
            Assert.assertTrue( true == source2_stats.exists() );
            Assert.assertTrue( true == source2_stats.ok() );
            Assert.assertTrue( true == source2_stats.is_file() );
            Assert.assertTrue( true == source2_stats.is_link() );
        }
        {
            // Copy file to new file-name
            final CopyOptions copts = new CopyOptions();
            copts.set(CopyOptions.Bit.preserve_all);
            copts.set(CopyOptions.Bit.verbose);
            {
                final FileStats dest_stats = new FileStats(root_copy+"/file_10.txt");
                PrintUtil.fprintf_td(System.err, "test31_copy_file2file: 10: dest.pre: %s\n", dest_stats);
                Assert.assertTrue( false == dest_stats.exists() );
            }
            Assert.assertTrue( true == FileUtil.copy(source1_stats.path(), root_copy+"/file_10.txt", copts) );
            {
                final FileStats dest_stats = new FileStats(root_copy+"/file_10.txt");
                PrintUtil.fprintf_td(System.err, "test31_copy_file2file: 10: dest.post: %s\n", dest_stats);
                Assert.assertTrue( true == dest_stats.exists() );
                Assert.assertTrue( true == dest_stats.ok() );
                Assert.assertTrue( true == dest_stats.is_file() );
                Assert.assertTrue( source1_stats.size() == dest_stats.size() );
                Assert.assertTrue( source1_stats.mode().equals( dest_stats.mode() ) );
            }
        }
        {
            // Error: already exists of 'Copy file to file'
            final CopyOptions copts = new CopyOptions();
            copts.set(CopyOptions.Bit.preserve_all);
            copts.set(CopyOptions.Bit.verbose);
            {
                final FileStats dest_stats = new FileStats(root_copy+"/file_10.txt");
                PrintUtil.fprintf_td(System.err, "test31_copy_file2file: 11: dest.pre: %s\n", dest_stats);
                Assert.assertTrue( true == dest_stats.exists() );
                Assert.assertTrue( true == dest_stats.ok() );
                Assert.assertTrue( true == dest_stats.is_file() );
            }
            Assert.assertTrue( false == FileUtil.copy(source1_stats.path(), root_copy+"/file_10.txt", copts) );
        }
        {
            // Overwrite copy file to file
            final CopyOptions copts = new CopyOptions();
            copts.set(CopyOptions.Bit.preserve_all);
            copts.set(CopyOptions.Bit.overwrite);
            copts.set(CopyOptions.Bit.follow_symlinks);
            copts.set(CopyOptions.Bit.verbose);
            {
                final FileStats dest_stats = new FileStats(root_copy+"/file_10.txt");
                PrintUtil.fprintf_td(System.err, "test31_copy_file2file: 12: dest.pre: %s\n", dest_stats);
                Assert.assertTrue( true == dest_stats.exists() );
                Assert.assertTrue( true == dest_stats.ok() );
                Assert.assertTrue( true == dest_stats.is_file() );
                Assert.assertTrue( source1_stats.size() == dest_stats.size() );
                Assert.assertTrue( source1_stats.mode().equals( dest_stats.mode() ) );
            }
            Assert.assertTrue( true == FileUtil.copy(source2_stats.path(), root_copy+"/file_10.txt", copts) );
            {
                final FileStats dest_stats = new FileStats(root_copy+"/file_10.txt");
                PrintUtil.fprintf_td(System.err, "test31_copy_file2file: 12: dest.post: %s\n", dest_stats);
                Assert.assertTrue( true  == dest_stats.exists() );
                Assert.assertTrue( true  == dest_stats.ok() );
                Assert.assertTrue( true  == dest_stats.is_file() );
                Assert.assertTrue( false == dest_stats.is_link() );
                Assert.assertTrue( source2_stats.size() == dest_stats.size() );
                Assert.assertTrue( source2_stats.link_target().prot_mode().equals( dest_stats.prot_mode() ) );
            }
        }
        Assert.assertTrue( true == FileUtil.remove(root_copy, topts_rec) );
    }

    @Test(timeout = 10000)
    public void test40_copy_ext_r_p() {
        PlatformRuntime.checkInitialized();
        PrintUtil.println(System.err, "test40_copy_ext_r_p\n");

        FileStats root_orig_stats = new FileStats(project_root1);
        if( !root_orig_stats.exists() ) {
            root_orig_stats = new FileStats(project_root2);
        }
        Assert.assertTrue( true == root_orig_stats.exists() );
        Assert.assertTrue( true == root_orig_stats.is_dir() );

        final String root_copy = root+"_copy_test40";
        testxx_copy_r_p("test40_copy_ext_r_p", root_orig_stats, 0 /* source_added_dead_links */, root_copy);
        Assert.assertTrue( true == FileUtil.remove(root_copy, topts_rec) );
    }

    @Test(timeout = 10000)
    public void test41_copy_ext_r_p_fsl() {
        PlatformRuntime.checkInitialized();
        PrintUtil.println(System.err, "test41_copy_ext_r_p_fsl\n");

        FileStats root_orig_stats = new FileStats(project_root1);
        if( !root_orig_stats.exists() ) {
            root_orig_stats = new FileStats(project_root2);
        }
        Assert.assertTrue( true == root_orig_stats.exists() );
        Assert.assertTrue( true == root_orig_stats.is_dir() );

        final String root_copy = root+"_copy_test41";
        final CopyOptions copts = new CopyOptions();
        copts.set(CopyOptions.Bit.recursive);
        copts.set(CopyOptions.Bit.preserve_all);
        copts.set(CopyOptions.Bit.follow_symlinks);
        copts.set(CopyOptions.Bit.ignore_symlink_errors);
        copts.set(CopyOptions.Bit.verbose);
        {
            FileUtil.remove(root_copy, topts_rec);

            Assert.assertTrue( true == FileUtil.copy(root_orig_stats.path(), root_copy, copts) );
        }
        final FileStats root_copy_stats = new FileStats(root_copy);
        Assert.assertTrue( true == root_copy_stats.exists() );
        Assert.assertTrue( true == root_copy_stats.ok() );
        Assert.assertTrue( true == root_copy_stats.is_dir() );

        {
            final TraverseOptions topts_orig = new TraverseOptions();
            topts_orig.set(TraverseOptions.Bit.recursive);
            topts_orig.set(TraverseOptions.Bit.dir_entry);
            topts_orig.set(TraverseOptions.Bit.follow_symlinks);

            final TraverseOptions topts_copy = new TraverseOptions();
            topts_copy.set(TraverseOptions.Bit.recursive);
            topts_copy.set(TraverseOptions.Bit.dir_entry);

            final VisitorStats stats = new VisitorStats(topts_orig);
            final VisitorStats stats_copy = new VisitorStats(topts_copy);

            final PathStatsVisitor pv_orig = new PathStatsVisitor(stats);
            final PathStatsVisitor pv_copy = new PathStatsVisitor(stats_copy);

            Assert.assertTrue( true == FileUtil.visit(root_orig_stats, topts_orig, pv_orig) );
            Assert.assertTrue( true == FileUtil.visit(root_copy_stats, topts_copy, pv_copy) );

            PrintUtil.fprintf_td(System.err, "test41_copy_ext_r_p_fsl: copy %s, traverse_orig %s, traverse_copy %s\n", copts, topts_orig, topts_copy);

            PrintUtil.fprintf_td(System.err, "test41_copy_ext_r_p_fsl: source      visitor stats\n%s\n", stats);
            PrintUtil.fprintf_td(System.err, "test41_copy_ext_r_p_fsl: destination visitor stats\n%s\n", stats_copy);

            Assert.assertTrue(  9 == stats.total_real );
            Assert.assertTrue( 11 == stats.total_sym_links_existing );
            Assert.assertTrue(  4 == stats.total_sym_links_not_existing );
            Assert.assertTrue(  0 == stats.total_no_access );
            Assert.assertTrue(  4 == stats.total_not_existing );
            Assert.assertTrue( 60 <  stats.total_file_bytes );                  // some followed symlink files are of unknown size, e.g. /etc/fstab
            Assert.assertTrue(  6 == stats.files_real );
            Assert.assertTrue( 10 == stats.files_sym_link );
            Assert.assertTrue(  3 == stats.dirs_real );
            Assert.assertTrue(  1 == stats.dirs_sym_link );

            Assert.assertTrue( 20 == stats_copy.total_real );
            Assert.assertTrue(  0 == stats_copy.total_sym_links_existing );
            Assert.assertTrue(  0 == stats_copy.total_sym_links_not_existing );
            Assert.assertTrue(  0 == stats_copy.total_no_access );
            Assert.assertTrue(  0 == stats_copy.total_not_existing );
            Assert.assertTrue( 60 <  stats_copy.total_file_bytes );             // some followed symlink files are of unknown size, e.g. /etc/fstab
            Assert.assertTrue( 16 == stats_copy.files_real );
            Assert.assertTrue(  0 == stats_copy.files_sym_link );
            Assert.assertTrue(  4 == stats_copy.dirs_real );
            Assert.assertTrue(  0 == stats_copy.dirs_sym_link );
        }
        Assert.assertTrue( true == FileUtil.remove(root_copy, topts_rec) );
    }

    public static void main(final String args[]) {
        org.junit.runner.JUnitCore.main(TestFileUtils01.class.getName());
    }

}