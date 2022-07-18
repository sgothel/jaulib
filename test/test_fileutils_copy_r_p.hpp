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

void testxx_copy_r_p(const std::string& title, const jau::fs::file_stats& source, const int source_added_dead_links, const std::string& dest) {
    REQUIRE( true == source.exists() );
    REQUIRE( true == source.is_dir() );

    bool dest_is_parent;
    std::string dest_root;
    {
        jau::fs::file_stats dest_stats(dest);
        if( dest_stats.exists() ) {
            // If dest_path exists as a directory, source_path dir will be copied below the dest_path directory.
            REQUIRE( true == dest_stats.is_dir() );
            dest_is_parent = true;
            dest_root = dest + "/" + source.item().basename();
        } else {
            // If dest_path doesn't exist, source_path dir content is copied into the newly created dest_path.
            dest_is_parent = false;
            dest_root = dest;
        }
    }
    jau::fprintf_td(stderr, "%s: source %s, dest[arg %s, is_parent %d, dest_root %s]\n",
            title.c_str(), source.to_string().c_str(), dest.c_str(), dest_is_parent, dest_root.c_str());

    const jau::fs::copy_options copts = jau::fs::copy_options::recursive |
                                        jau::fs::copy_options::preserve_all |
                                        jau::fs::copy_options::sync |
                                        jau::fs::copy_options::verbose;
    REQUIRE( true == jau::fs::copy(source.path(), dest, copts) );

    jau::fs::file_stats dest_stats(dest_root);
    REQUIRE( true == dest_stats.exists() );
    REQUIRE( true == dest_stats.ok() );
    REQUIRE( true == dest_stats.is_dir() );

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
        REQUIRE( true == jau::fs::visit(source, topts, pv_orig) );
        REQUIRE( true == jau::fs::visit(dest_stats, topts, pv_copy) );

        jau::fprintf_td(stderr, "%s: copy %s, traverse %s\n",
                title.c_str(), to_string(copts).c_str(), to_string(topts).c_str());

        jau::fprintf_td(stderr, "%s: source      visitor stats\n%s\n", title.c_str(), stats.to_string().c_str());
        jau::fprintf_td(stderr, "%s: destination visitor stats\n%s\n", title.c_str(), stats_copy.to_string().c_str());

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
    }
    {
        // compare each file in detail O(n*n)
        const jau::fs::traverse_options topts = jau::fs::traverse_options::recursive |
                                                jau::fs::traverse_options::dir_entry;
        struct source_visitor_params {
                std::string title;
                std::string source_folder_path;
                jau::fs::file_stats dest;
        };
        struct dest_visitor_params {
                std::string title;
                std::string source_folder_path;
                std::string dest_folder_path;
                std::string source_basename;
                jau::fs::file_stats stats;
                bool match;
        };
        source_visitor_params svp { title, source.path(), dest_stats };
        const jau::fs::path_visitor pv1 = jau::bindCaptureRefFunc<bool, source_visitor_params, jau::fs::traverse_event, const jau::fs::file_stats&>(&svp,
                ( bool(*)(source_visitor_params*, jau::fs::traverse_event, const jau::fs::file_stats&) ) /* help template type deduction of function-ptr */
                    ( [](source_visitor_params* _svp, jau::fs::traverse_event tevt1, const jau::fs::file_stats& element_stats1) -> bool {
                        (void)tevt1;
                        dest_visitor_params dvp { _svp->title, _svp->source_folder_path, _svp->dest.path(), jau::fs::basename( element_stats1.path() ), element_stats1, false };
                        const jau::fs::path_visitor pv2 = jau::bindCaptureRefFunc<bool, dest_visitor_params, jau::fs::traverse_event, const jau::fs::file_stats&>(&dvp,
                                ( bool(*)(dest_visitor_params*, jau::fs::traverse_event, const jau::fs::file_stats&) ) /* help template type deduction of function-ptr */
                                    ( [](dest_visitor_params* _dvp, jau::fs::traverse_event tevt2, const jau::fs::file_stats& element_stats2) -> bool {
                                        (void)tevt2;
                                        const std::string path2 = element_stats2.path();
                                        const std::string basename2 = jau::fs::basename( path2 );
                                        const std::string source_folder_basename = jau::fs::basename( _dvp->source_folder_path );
                                        if( basename2 == _dvp->source_basename ||
                                            ( source_folder_basename == _dvp->source_basename && _dvp->dest_folder_path == path2 )
                                          )
                                        {
                                            bool attr_equal, bit_equal;
                                            if( "README_slink08_relext.txt" == basename2 || 0 == basename2.find("dead_link") ) {
                                                // symlink to ../README.txt not existent on target
                                                // dead_link* files intentionally not existant
                                                attr_equal = element_stats2.is_link() &&
                                                             !element_stats2.exists();

                                                bit_equal = true; // pretend
                                            } else {
                                                attr_equal =
                                                        element_stats2.mode() == _dvp->stats.mode() &&
                                                        // element_stats2.atime() == _dvp->stats.atime() && // destination access-time may differ due to processing post copy
                                                        element_stats2.mtime() == _dvp->stats.mtime() &&
                                                        element_stats2.uid() == _dvp->stats.uid() &&
                                                        element_stats2.gid() == _dvp->stats.gid() &&
                                                        element_stats2.size() == _dvp->stats.size();

                                                if( _dvp->stats.is_file() ) {
                                                    bit_equal = jau::fs::compare(_dvp->stats, element_stats2, true);
                                                } else {
                                                    bit_equal = true; // pretend
                                                }
                                            }
                                            _dvp->match = attr_equal && bit_equal;
                                            jau::fprintf_td(stderr, "%s.check: '%s', match [attr %d, bit %d -> %d]\n\t source %s\n\t dest__ %s\n\n",
                                                    _dvp->title.c_str(), basename2.c_str(), attr_equal, bit_equal, _dvp->match,
                                                    _dvp->stats.to_string().c_str(),
                                                    element_stats2.to_string().c_str());
                                            return false; // done
                                        } else {
                                            return true; // continue search
                                        }
                                      } ) );
                        if( jau::fs::visit(_svp->dest, topts, pv2) ) {
                            jau::fprintf_td(stderr, "%s.check: '%s', not found!\n\t source %s\n\n",
                                    _svp->title.c_str(), dvp.source_basename.c_str(),
                                    element_stats1.to_string().c_str());
                            return false; // not found, abort
                        } else {
                            // found
                            if( dvp.match ) {
                                return true; // found and matching, continue
                            } else {
                                return false; // found not matching, abort
                            }
                        }
                      } ) );
        REQUIRE( true == jau::fs::visit(source, topts, pv1) );
    }
}

