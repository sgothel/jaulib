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
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <memory>
#include <thread>
#include <pthread.h>

#include <jau/test/catch2_ext.hpp>

#include <jau/debug.hpp>
#include <jau/file_util.hpp>
#include <jau/byte_stream.hpp>

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

class TestFileUtilBase {
  private:
    const std::string image_file = "test_data.sqfs";
    // normal location with jaulib as sole project (a)
    const std::string project_root1a = "../../test_data";
    // normal location with jaulib as sole project (b)
    const std::string project_root1b = "../../../test_data";
    // submodule location with jaulib directly hosted below main project (a)
    const std::string project_root2a = "../../../jaulib/test_data";
    // submodule location with jaulib directly hosted below main project (b)
    const std::string project_root2b = "../../../../jaulib/test_data";

  public:
    const std::string temp_root = "test_data_temp";

    jau::fs::file_stats getTestDataDirStats(const std::string& test_exe_path) noexcept {
        const std::string test_exe_dir = jau::fs::dirname(test_exe_path);
        std::string path = test_exe_dir + "/" + project_root1a;
        jau::fs::file_stats path_stats(path);
        if( path_stats.exists() ) {
            return path_stats;
        }
        path = test_exe_dir + "/" + project_root1b;
        path_stats = jau::fs::file_stats(path);
        if( path_stats.exists() ) {
            return path_stats;
        }
        path = test_exe_dir + "/" + project_root2a;
        path_stats = jau::fs::file_stats(path);
        if( path_stats.exists() ) {
            return path_stats;
        }
        path = test_exe_dir + "/" + project_root2b;
        path_stats = jau::fs::file_stats(path);
        if( path_stats.exists() ) {
            return path_stats;
        }
        return jau::fs::file_stats();
    }
    std::string getTestDataRelDir(const std::string& test_exe_path) noexcept {
        const std::string test_exe_dir = jau::fs::dirname(test_exe_path);
        std::string path = test_exe_dir + "/" + project_root1a;
        jau::fs::file_stats path_stats(path);
        if( path_stats.exists() ) {
            return project_root1a;
        }
        path = test_exe_dir + "/" + project_root1b;
        path_stats = jau::fs::file_stats(path);
        if( path_stats.exists() ) {
            return project_root1b;
        }
        path = test_exe_dir + "/" + project_root2a;
        path_stats = jau::fs::file_stats(path);
        if( path_stats.exists() ) {
            return project_root2a;
        }
        path = test_exe_dir + "/" + project_root2b;
        path_stats = jau::fs::file_stats(path);
        if( path_stats.exists() ) {
            return project_root2b;
        }
        return "";
    }
    jau::fs::file_stats getTestDataImageFile(const std::string& test_exe_path) noexcept {
        const std::string test_exe_dir = jau::fs::dirname(test_exe_path);
        std::string path = test_exe_dir + "/" + image_file;
        jau::fs::file_stats path_stats(path);
        if( path_stats.exists() ) {
            return path_stats;
        }
        return jau::fs::file_stats();
    }

    // external filesystem source to test ...
    const std::string project_root_ext = "/mnt/ssd0/data/test_data";
    // external vfat filesystem destination to test ...
    const std::string dest_fs_vfat = "/mnt/vfat";
};
