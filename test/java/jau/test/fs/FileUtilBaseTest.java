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

import java.time.temporal.ChronoUnit;

import org.jau.fs.CopyOptions;
import org.jau.fs.FileStats;
import org.jau.fs.FileUtil;
import org.jau.fs.TraverseEvent;
import org.jau.fs.TraverseOptions;
import org.jau.io.PrintUtil;
import org.junit.Assert;

import jau.test.junit.util.JunitTracer;

public class FileUtilBaseTest extends JunitTracer {
    public static final String root = "test_data";
    // normal location with jaulib as sole project
    public static final String project_root1 = "../../../test_data";
    // submodule location with jaulib directly hosted below main project
    public static final String project_root2 = "../../../../jaulib/test_data";
    // external filesystem to test ...
    public static final String project_root_ext = "/mnt/ssd0/data/test_data";
    // external vfat filesystem destination to test ...
    public static final String dest_fs_vfat = "/mnt/vfat";

    public static class VisitorStats {
        public TraverseOptions topts;
        public int total_real;
        public int total_sym_links_existing;
        public int total_sym_links_not_existing;
        public int total_no_access;
        public int total_not_existing;
        public long total_file_bytes;
        public int files_real;
        public int files_sym_link;
        public int dirs_real;
        public int dirs_sym_link;

        public VisitorStats(final TraverseOptions topts_) {
            topts = topts_;
            total_real = 0;
            total_sym_links_existing = 0;
            total_sym_links_not_existing = 0;
            total_no_access = 0;
            total_not_existing = 0;
            total_file_bytes = 0;
            files_real = 0;
            files_sym_link = 0;
            dirs_real = 0;
            dirs_sym_link = 0;
        }

        public void add(final FileStats element_stats) {
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
                    if( topts.isSet(TraverseOptions.Bit.follow_symlinks) ) {
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

        @Override
        public boolean equals(final Object other) {
            if( this == other ) {
                return true;
            }
            if( !( other instanceof VisitorStats ) ) {
                return false;
            }
            final VisitorStats o = (VisitorStats)other;
            return  total_file_bytes             == o.total_file_bytes &&
                    total_real                   == o.total_real &&
                    total_sym_links_existing     == o.total_sym_links_existing &&
                    total_sym_links_not_existing == o.total_sym_links_not_existing &&
                    total_no_access              == o.total_no_access &&
                    total_not_existing           == o.total_not_existing &&
                    files_real                   == o.files_real &&
                    files_sym_link               == o.files_sym_link &&
                    dirs_real                    == o.dirs_real &&
                    dirs_sym_link                == o.dirs_sym_link;
        }

        @Override
        public String toString() {
            final StringBuilder res = new StringBuilder();
            res.append( "- traverse_options              ").append(topts).append("\n");
            res.append( "- total_real                    ").append(total_real).append("\n");
            res.append( "- total_sym_links_existing      ").append(total_sym_links_existing).append("\n");
            res.append( "- total_sym_links_not_existing  ").append(total_sym_links_not_existing).append("\n");
            res.append( "- total_no_access               ").append(total_no_access).append("\n");
            res.append( "- total_not_existing            ").append(total_not_existing).append("\n");
            res.append( "- total_file_bytes              ").append(String.format("%,d", total_file_bytes)).append("\n");
            res.append( "- files_real                    ").append(files_real).append("\n");
            res.append( "- files_sym_link                ").append(files_sym_link).append("\n");
            res.append( "- dirs_real                     ").append(dirs_real).append("\n");
            res.append( "- dirs_sym_link                 ").append(dirs_sym_link).append("\n");
            return res.toString();
        }
    }

    public static class PathStatsVisitor implements FileUtil.PathVisitor {
        private final VisitorStats stats;

        public PathStatsVisitor(final VisitorStats stats_) {
            stats = stats_;
        }

        @Override
        public boolean visit(final TraverseEvent tevt, final FileStats item_stats) {
            // PrintUtil.fprintf_td(System.err, "add: item_stats "+item_stats+", tevt "+tevt+"\n");
            stats.add(item_stats);
            return true;
        }
    }

    static class source_visitor_params {
        public String title;
        public String source_folder_path;
        public FileStats dest;
        public boolean dest_is_vfat;
        public boolean opt_drop_dest_links;

        public source_visitor_params(final String t, final String sfp, final FileStats d, final boolean dest_is_vfat_, final boolean opt_drop_dest_links_) {
            title = t;
            source_folder_path = sfp;
            dest = d;
            dest_is_vfat = dest_is_vfat_;
            opt_drop_dest_links = opt_drop_dest_links_;
        }
    };

    static class dest_visitor_params {
        public String title;
        public String source_folder_path;
        public String dest_folder_path;
        public String source_basename;
        public FileStats stats;
        public boolean dest_is_vfat;
        public boolean match;
        public dest_visitor_params(final String t, final String sfp, final String dfp, final String sb, final FileStats s, final boolean dest_is_vfat_) {
            title = t;
            source_folder_path = sfp;
            dest_folder_path = dfp;
            source_basename = sb;
            stats = s;
            dest_is_vfat = dest_is_vfat_;
            match = false;
        }
    };

    public void testxx_copy_r_p(final String title, final FileStats source, final int source_added_dead_links,
                                final String dest,
                                final CopyOptions copts,
                                final boolean dest_is_vfat) {
        Assert.assertTrue( source.exists() );
        Assert.assertTrue( source.is_dir() );

        final boolean dest_is_parent;
        final String dest_root;
        {
            final FileStats dest_stats = new FileStats(dest);
            if( dest_stats.exists() ) {
                // If dest_path exists as a directory, source_path dir will be copied below the dest_path directory.
                Assert.assertTrue( dest_stats.is_dir() );
                dest_is_parent = true;
                dest_root = dest + "/" + source.item().basename();
            } else {
                // If dest_path doesn't exist, source_path dir content is copied into the newly created dest_path.
                dest_is_parent = false;
                dest_root = dest;
            }
        }
        PrintUtil.fprintf_td(System.err, "%s: source %s, dest[arg %s, is_parent %b, dest_root %s], copts %s, dest_is_vfat %b\n",
                title, source, dest, dest_is_parent, dest_root, copts, dest_is_vfat);

        final boolean opt_follow_links = copts.isSet(CopyOptions.Bit.follow_symlinks);
        final boolean opt_drop_dest_links = !opt_follow_links && copts.isSet(CopyOptions.Bit.ignore_symlink_errors);

        Assert.assertTrue( true == FileUtil.copy(source.path(), dest, copts) );

        final FileStats dest_stats = new FileStats(dest_root);
        Assert.assertTrue( true == dest_stats.exists() );
        Assert.assertTrue( true == dest_stats.ok() );
        Assert.assertTrue( true == dest_stats.is_dir() );

        {
            final TraverseOptions topts = new TraverseOptions();
            topts.set(TraverseOptions.Bit.recursive);
            topts.set(TraverseOptions.Bit.dir_entry);

            final VisitorStats stats = new VisitorStats(topts);
            final VisitorStats stats_copy = new VisitorStats(topts);

            final PathStatsVisitor pv_orig = new PathStatsVisitor(stats);
            final PathStatsVisitor pv_copy = new PathStatsVisitor(stats_copy);

            Assert.assertTrue( true == FileUtil.visit(source, topts, pv_orig) );
            Assert.assertTrue( true == FileUtil.visit(dest_stats, topts, pv_copy) );

            PrintUtil.fprintf_td(System.err, "%s: copy %s, traverse %s\n", title, copts, topts);
            PrintUtil.fprintf_td(System.err, "%s: source      visitor stats\n%s\n", title, stats);
            PrintUtil.fprintf_td(System.err, "%s: destination visitor stats\n%s\n", title, stats_copy);

            Assert.assertTrue(  7 == stats.total_real );
            Assert.assertTrue( 10 - source_added_dead_links == stats.total_sym_links_existing );
            Assert.assertTrue(  4 + source_added_dead_links == stats.total_sym_links_not_existing );
            Assert.assertTrue(  0 == stats.total_no_access );
            Assert.assertTrue(  4 + source_added_dead_links == stats.total_not_existing );
            Assert.assertTrue( 60 == stats.total_file_bytes );
            Assert.assertTrue(  4 == stats.files_real );
            Assert.assertTrue(  9 - source_added_dead_links == stats.files_sym_link );
            Assert.assertTrue(  3 == stats.dirs_real );
            Assert.assertTrue(  1 == stats.dirs_sym_link );

            if( ( !opt_follow_links && !opt_drop_dest_links ) ||
                ( opt_drop_dest_links && 0 < stats_copy.total_sym_links_existing )
              )
            {
                // 1:1 exact copy
                Assert.assertTrue(  7 == stats_copy.total_real );
                Assert.assertTrue(  9 == stats_copy.total_sym_links_existing );
                Assert.assertTrue(  5 == stats_copy.total_sym_links_not_existing ); // symlink ../README.txt + 4 dead_link*
                Assert.assertTrue(  0 == stats_copy.total_no_access );
                Assert.assertTrue(  5 == stats_copy.total_not_existing );           // symlink ../README.txt + 4 dead_link*
                Assert.assertTrue( 60 == stats_copy.total_file_bytes );
                Assert.assertTrue(  4 == stats_copy.files_real );
                Assert.assertTrue(  8 == stats_copy.files_sym_link );
                Assert.assertTrue(  3 == stats_copy.dirs_real );
                Assert.assertTrue(  1 == stats_copy.dirs_sym_link );
            } else if( opt_drop_dest_links ) {
                // destination filesystem has no symlink support, i.e. vfat
                Assert.assertTrue(  7 == stats_copy.total_real );
                Assert.assertTrue(  0 == stats_copy.total_sym_links_existing );
                Assert.assertTrue(  0 == stats_copy.total_sym_links_not_existing ); // symlink ../README.txt + 4 dead_link*
                Assert.assertTrue(  0 == stats_copy.total_no_access );
                Assert.assertTrue(  0 == stats_copy.total_not_existing );           // symlink ../README.txt + 4 dead_link*
                Assert.assertTrue( 60 == stats_copy.total_file_bytes );
                Assert.assertTrue(  4 == stats_copy.files_real );
                Assert.assertTrue(  0 == stats_copy.files_sym_link );
                Assert.assertTrue(  3 == stats_copy.dirs_real );
                Assert.assertTrue(  0 == stats_copy.dirs_sym_link );
            } else if( opt_follow_links ) {
                // followed symlinks
                Assert.assertTrue( 20 == stats_copy.total_real );
                Assert.assertTrue(  0 == stats_copy.total_sym_links_existing );
                Assert.assertTrue(  0 == stats_copy.total_sym_links_not_existing ); // symlink ../README.txt + 4 dead_link*
                Assert.assertTrue(  0 == stats_copy.total_no_access );
                Assert.assertTrue(  0 == stats_copy.total_not_existing );           // symlink ../README.txt + 4 dead_link*
                Assert.assertTrue( 60 <  stats_copy.total_file_bytes );
                Assert.assertTrue( 16 == stats_copy.files_real );
                Assert.assertTrue(  0 == stats_copy.files_sym_link );
                Assert.assertTrue(  4 == stats_copy.dirs_real );
                Assert.assertTrue(  0 == stats_copy.dirs_sym_link );
            }
        }
        {
            // compare each file in detail O(n*n)
            final TraverseOptions topts = new TraverseOptions();
            topts.set(TraverseOptions.Bit.recursive);
            topts.set(TraverseOptions.Bit.dir_entry);

            final source_visitor_params svp = new source_visitor_params(title, source.path(), dest_stats, dest_is_vfat, opt_drop_dest_links);
            final FileUtil.PathVisitor pv1 = new FileUtil.PathVisitor() {
                @Override
                public boolean visit(final TraverseEvent tevt1, final FileStats element_stats1) {
                    final dest_visitor_params dvp = new dest_visitor_params(svp.title, svp.source_folder_path, svp.dest.path(), FileUtil.basename(element_stats1.path() ), element_stats1, svp.dest_is_vfat);
                    final FileUtil.PathVisitor pv2 = new FileUtil.PathVisitor() {
                        @Override
                        public boolean visit(final TraverseEvent tevt2, final FileStats element_stats2) {
                            final String path2 = element_stats2.path();
                            final String basename2 = FileUtil.basename( path2 );
                            final String source_folder_basename = FileUtil.basename( dvp.source_folder_path );
                            if( basename2.equals( dvp.source_basename ) ||
                                ( source_folder_basename.equals( dvp.source_basename ) && dvp.dest_folder_path.equals( path2 ) )
                              )
                            {
                                boolean attr_equal, bit_equal;
                                if( "README_slink08_relext.txt".equals(basename2) || 0 == basename2.indexOf("dead_link") ) {
                                    // symlink to ../README.txt not existent on target
                                    // dead_link* files intentionally not existant
                                    attr_equal = element_stats2.is_link() &&
                                                 !element_stats2.exists();

                                    bit_equal = true; // pretend
                                } else {
                                    if( !dvp.dest_is_vfat ) {
                                        // full attribute check
                                        attr_equal =
                                                element_stats2.mode().equals( dvp.stats.mode() ) &&
                                                // element_stats2.atime().equals( dvp.stats.atime() ) && // destination access-time may differ due to processing post copy
                                                element_stats2.mtime().equals( dvp.stats.mtime() ) &&
                                                element_stats2.uid() == dvp.stats.uid() &&
                                                element_stats2.gid() == dvp.stats.gid() &&
                                                element_stats2.size() == dvp.stats.size();
                                    } else {
                                        // minimal vfat attribute check
                                        // const jau::fraction_timespec td(5_s);
                                        final long td_ms = 5000;

                                        attr_equal =
                                                // ( element_stats2.mode() & jau::fs::fmode_t::rwx_usr ) == ( dvp.stats.mode() & jau::fs::fmode_t::rwx_usr ) &&
                                                // element_stats2.atime().equals( dvp.stats.atime() ) && // destination access-time may differ due to processing post copy
                                                Math.abs( element_stats2.mtime().toEpochMilli() - dvp.stats.mtime().toEpochMilli() ) <= td_ms &&
                                                element_stats2.uid() == dvp.stats.uid() &&
                                                // element_stats2.gid() == dvp.stats.gid() &&
                                                element_stats2.size() == dvp.stats.size();
                                    }
                                    if( dvp.stats.is_file() ) {
                                        bit_equal = FileUtil.compare(dvp.stats.path(), element_stats2.path(), true);
                                    } else {
                                        bit_equal = true; // pretend
                                    }
                                }
                                dvp.match = attr_equal && bit_equal;
                                PrintUtil.fprintf_td(System.err, "%s.check: '%s', match [attr %b, bit %b -> %b]\n\t source %s\n\t dest__ %s\n\n",
                                        dvp.title, basename2, attr_equal, bit_equal, dvp.match,
                                        dvp.stats,
                                        element_stats2);
                                return false; // done
                            } else {
                                return true; // continue search
                            }
                        } };
                    if( FileUtil.visit(svp.dest, topts, pv2) ) {
                        // not found
                        final boolean ignore = element_stats1.is_link() && svp.opt_drop_dest_links;
                        PrintUtil.fprintf_td(System.err, "%s.check: %s: '%s', not found!\n\t source %s\n\n",
                                svp.title, ignore ? "Ignored" : "Error", dvp.source_basename, element_stats1);
                        return ignore;
                    } else {
                        // found
                        if( dvp.match ) {
                            return true; // found and matching, continue
                        } else {
                            return false; // found not matching, abort
                        }
                    }
                } };
            Assert.assertTrue( true == FileUtil.visit(source, topts, pv1) );
        }
    }

}
