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

#include "test_fileutils.hpp"

void testxx_copy_r_p(const std::string& title, // NOLINT(misc-definitions-in-headers)
                     const jau::io::fs::file_stats& source, const int source_added_dead_links,
                     const std::string& dest,
                     const jau::io::fs::copy_options copts,
                     const bool dest_is_vfat)
{
    REQUIRE( true == source.exists() );
    REQUIRE( true == source.is_dir() );

    bool dest_is_parent;
    std::string dest_root;
    {
        jau::io::fs::file_stats dest_stats(dest);
        if( dest_stats.exists() ) {
            // If dest_path exists as a directory, source_path dir will be copied below the dest_path directory
            // _if_ copy_options::into_existing_dir is not set. Otherwise its content is copied into the existing dest_path.
            REQUIRE( true == dest_stats.is_dir() );
            if( is_set(copts, jau::io::fs::copy_options::into_existing_dir ) ) {
                dest_is_parent = false;
                dest_root = dest;
            } else {
                dest_is_parent = true;
                dest_root = dest + "/" + source.item().basename();
            }
        } else {
            // If dest_path doesn't exist, source_path dir content is copied into the newly created dest_path.
            dest_is_parent = false;
            dest_root = dest;
        }
    }
    jau::fprintf_td(stdout, "%s: source %s, dest[arg %s, is_parent %d, dest_root %s], copts %s, dest_is_vfat %d\n",
            title.c_str(), source.toString().c_str(),
            dest.c_str(), dest_is_parent, dest_root.c_str(),
            to_string(copts).c_str(), dest_is_vfat);

    const bool opt_follow_links = is_set(copts, jau::io::fs::copy_options::follow_symlinks );
    const bool opt_drop_dest_links = !opt_follow_links && is_set(copts, jau::io::fs::copy_options::ignore_symlink_errors );

    REQUIRE( true == jau::io::fs::copy(source.path(), dest, copts) );

    jau::io::fs::file_stats dest_stats(dest_root);
    REQUIRE( true == dest_stats.exists() );
    REQUIRE( true == dest_stats.ok() );
    REQUIRE( true == dest_stats.is_dir() );

    bool(*pv_capture)(visitor_stats*, jau::io::fs::traverse_event, const jau::io::fs::file_stats&, size_t) =
        ( [](visitor_stats* stats_ptr, jau::io::fs::traverse_event tevt, const jau::io::fs::file_stats& element_stats, size_t depth) -> bool {
            (void)tevt;
            (void)depth;
            stats_ptr->add(element_stats);
            return true;
          } );
    {

        const jau::io::fs::traverse_options topts = jau::io::fs::traverse_options::recursive |
                                                jau::io::fs::traverse_options::dir_entry;
        visitor_stats stats(topts);
        visitor_stats stats_copy(topts);
        const jau::io::fs::path_visitor pv_orig = jau::bind_capref(&stats, pv_capture);
        const jau::io::fs::path_visitor pv_copy = jau::bind_capref(&stats_copy, pv_capture);
        REQUIRE( true == jau::io::fs::visit(source, topts, pv_orig) );
        REQUIRE( true == jau::io::fs::visit(dest_stats, topts, pv_copy) );

        jau::fprintf_td(stdout, "%s: copy %s, traverse %s\n",
                title.c_str(), to_string(copts).c_str(), to_string(topts).c_str());

        jau::fprintf_td(stdout, "%s: source      visitor stats\n%s\n", title.c_str(), stats.toString().c_str());
        jau::fprintf_td(stdout, "%s: destination visitor stats\n%s\n", title.c_str(), stats_copy.toString().c_str());

        REQUIRE(  7 == stats.total_real );
        REQUIRE( 10 - source_added_dead_links == stats.total_sym_links_existing );
        REQUIRE(  4 + source_added_dead_links == stats.total_sym_links_not_existing );
        REQUIRE(  0 == stats.total_no_access );
        REQUIRE(  4 + source_added_dead_links == stats.total_not_existing );
        REQUIRE( 60 == stats.total_file_bytes );
        REQUIRE(  4 == stats.files_real );
        REQUIRE(  9 - source_added_dead_links == stats.files_sym_link );
        REQUIRE(  3 == stats.dirs_real );
        REQUIRE(  1 == stats.dirs_sym_link );

        if( ( !opt_follow_links && !opt_drop_dest_links ) ||
            ( opt_drop_dest_links && 0 < stats_copy.total_sym_links_existing )
          )
        {
            // 1:1 exact copy
            REQUIRE(  7 == stats_copy.total_real );
            REQUIRE(  9 == stats_copy.total_sym_links_existing );
            REQUIRE(  5 == stats_copy.total_sym_links_not_existing ); // symlink ../README.txt + 4 dead_link*
            REQUIRE(  0 == stats_copy.total_no_access );
            REQUIRE(  5 == stats_copy.total_not_existing );           // symlink ../README.txt + 4 dead_link*
            REQUIRE( 60 == stats_copy.total_file_bytes );
            REQUIRE(  4 == stats_copy.files_real );
            REQUIRE(  8 == stats_copy.files_sym_link );
            REQUIRE(  3 == stats_copy.dirs_real );
            REQUIRE(  1 == stats_copy.dirs_sym_link );
        } else if( opt_drop_dest_links ) {
            // destination filesystem has no symlink support, i.e. vfat
            REQUIRE(  7 == stats_copy.total_real );
            REQUIRE(  0 == stats_copy.total_sym_links_existing );
            REQUIRE(  0 == stats_copy.total_sym_links_not_existing ); // symlink ../README.txt + 4 dead_link*
            REQUIRE(  0 == stats_copy.total_no_access );
            REQUIRE(  0 == stats_copy.total_not_existing );           // symlink ../README.txt + 4 dead_link*
            REQUIRE( 60 == stats_copy.total_file_bytes );
            REQUIRE(  4 == stats_copy.files_real );
            REQUIRE(  0 == stats_copy.files_sym_link );
            REQUIRE(  3 == stats_copy.dirs_real );
            REQUIRE(  0 == stats_copy.dirs_sym_link );
        } else if( opt_follow_links ) {
            // followed symlinks
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
    }
    {
        // compare each file in detail O(n*n)
        const jau::io::fs::traverse_options topts = jau::io::fs::traverse_options::recursive |
                                                jau::io::fs::traverse_options::dir_entry;
        struct source_visitor_params {
                std::string title;
                std::string source_folder_path;
                jau::io::fs::file_stats dest;
                bool dest_is_vfat;
                bool opt_drop_dest_links;
        };
        struct dest_visitor_params {
                std::string title;
                std::string source_folder_path;
                std::string dest_folder_path;
                std::string source_basename;
                jau::io::fs::file_stats stats;
                bool dest_is_vfat;
                bool match;
        };
        source_visitor_params svp { .title=title, .source_folder_path=source.path(), .dest=dest_stats, .dest_is_vfat=dest_is_vfat,
                                    .opt_drop_dest_links=opt_drop_dest_links };
        const jau::io::fs::path_visitor pv1 = jau::bind_capref<bool, source_visitor_params, jau::io::fs::traverse_event, const jau::io::fs::file_stats&, size_t>(&svp,
                ( bool(*)(source_visitor_params*, jau::io::fs::traverse_event, const jau::io::fs::file_stats&, size_t) ) /* help template type deduction of function-ptr */
                    ( [](source_visitor_params* _svp, jau::io::fs::traverse_event tevt1, const jau::io::fs::file_stats& element_stats1, size_t depth) -> bool {
                        (void)tevt1;
                        (void)depth;
                        dest_visitor_params dvp { .title=_svp->title, .source_folder_path=_svp->source_folder_path, .dest_folder_path=_svp->dest.path(),
                                                  .source_basename=jau::io::fs::basename( element_stats1.path() ), .stats=element_stats1,
                                                  .dest_is_vfat=_svp->dest_is_vfat, .match=false };
                        const jau::io::fs::path_visitor pv2 = jau::bind_capref<bool, dest_visitor_params, jau::io::fs::traverse_event, const jau::io::fs::file_stats&, size_t>(&dvp,
                                ( bool(*)(dest_visitor_params*, jau::io::fs::traverse_event, const jau::io::fs::file_stats&, size_t depth) ) /* help template type deduction of function-ptr */
                                    ( [](dest_visitor_params* _dvp, jau::io::fs::traverse_event tevt2, const jau::io::fs::file_stats& element_stats2, size_t depth2) -> bool {
                                        (void)tevt2;
                                        (void)depth2;
                                        const std::string path2 = element_stats2.path();
                                        const std::string basename2 = jau::io::fs::basename( path2 );
                                        const std::string source_folder_basename = jau::io::fs::basename( _dvp->source_folder_path );
                                        if( basename2 == _dvp->source_basename ||
                                            ( source_folder_basename == _dvp->source_basename && _dvp->dest_folder_path == path2 )
                                          )
                                        {
                                            bool attr_equal, bit_equal;
                                            if( "README_slink08_relext.txt" == basename2 || basename2.starts_with("dead_link") ) {
                                                // symlink to ../README.txt not existent on target
                                                // dead_link* files intentionally not existant
                                                attr_equal = element_stats2.is_link() &&
                                                             !element_stats2.exists();

                                                bit_equal = true; // pretend
                                            } else {
                                                if( !_dvp->dest_is_vfat ) {
                                                    // full attribute check
                                                    attr_equal =
                                                            element_stats2.mode() == _dvp->stats.mode() &&
                                                            // element_stats2.atime() == _dvp->stats.atime() && // destination access-time may differ due to processing post copy
                                                            element_stats2.mtime() == _dvp->stats.mtime() &&
                                                            element_stats2.uid() == _dvp->stats.uid() &&
                                                            element_stats2.gid() == _dvp->stats.gid() &&
                                                            element_stats2.size() == _dvp->stats.size();
                                                } else {
                                                    // minimal vfat attribute check
                                                    const jau::fraction_timespec td(5_s);

                                                    attr_equal =
                                                            // ( element_stats2.mode() & jau::io::fs::fmode_t::rwx_usr ) == ( _dvp->stats.mode() & jau::io::fs::fmode_t::rwx_usr ) &&
                                                            // element_stats2.atime().tv_sec == _dvp->stats.atime().tv_sec && // destination access-time may differ due to processing post copy
                                                            abs( element_stats2.mtime() - _dvp->stats.mtime() ) <= td &&
                                                            element_stats2.uid() == _dvp->stats.uid() &&
                                                            // element_stats2.gid() == _dvp->stats.gid() &&
                                                            element_stats2.size() == _dvp->stats.size();
                                                }
                                                if( !attr_equal ) {
                                                    jau::fprintf_td(stdout, "%s.check: '%s'\n  mode %s == %s\n  mtime %s == %s, d %s\n  uid %s == %s\n  gid %s == %s\n  size %s == %s\n",
                                                            _dvp->title.c_str(), basename2.c_str(),
                                                            jau::io::fs::to_string(element_stats2.mode()).c_str(), jau::io::fs::to_string(_dvp->stats.mode()).c_str(),
                                                            // element_stats2.atime().toString().c_str(), _dvp->stats.atime().toString().c_str(),
                                                            element_stats2.mtime().toString().c_str(), _dvp->stats.mtime().toString().c_str(),
                                                            abs( element_stats2.mtime() - _dvp->stats.mtime() ).toString().c_str(),
                                                            std::to_string(element_stats2.uid()).c_str(), std::to_string(_dvp->stats.uid()).c_str(),
                                                            std::to_string(element_stats2.gid()).c_str(), std::to_string(_dvp->stats.gid()).c_str(),
                                                            jau::to_decstring(element_stats2.size()).c_str(), jau::to_decstring(_dvp->stats.size()).c_str() );
                                                }

                                                if( _dvp->stats.is_file() ) {
                                                    bit_equal = jau::io::fs::compare(_dvp->stats, element_stats2, true);
                                                } else {
                                                    bit_equal = true; // pretend
                                                }
                                            }
                                            _dvp->match = attr_equal && bit_equal;
                                            jau::fprintf_td(stdout, "%s.check: '%s', match [attr %d, bit %d -> %d]\n\t source %s\n\t dest__ %s\n\n",
                                                    _dvp->title.c_str(), basename2.c_str(), attr_equal, bit_equal, _dvp->match,
                                                    _dvp->stats.toString().c_str(),
                                                    element_stats2.toString().c_str());
                                            return false; // done
                                        } else {
                                            return true; // continue search
                                        }
                                      } ) );
                        if( jau::io::fs::visit(_svp->dest, topts, pv2) ) {
                            // not found
                            const bool ignore = element_stats1.is_link() && _svp->opt_drop_dest_links;
                            jau::fprintf_td(stdout, "%s.check: %s: '%s', not found!\n\t source %s\n\n",
                                    _svp->title.c_str(), ignore ? "Ignored" : "Error", dvp.source_basename.c_str(),
                                    element_stats1.toString().c_str());
                            return ignore;
                        } else {
                            // found
                            if( dvp.match ) {
                                return true; // found and matching, continue
                            } else {
                                return false; // found not matching, abort
                            }
                        }
                      } ) );
        REQUIRE( true == jau::io::fs::visit(source, topts, pv1) );
    }
}

