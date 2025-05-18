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

#include "jau/os/user_info.hpp"
#include <jau/enum_util.hpp>

extern "C" {
    #include <unistd.h>
    #include <grp.h>
    #include <pwd.h>
    #include <sys/types.h>
    #include <sys/mount.h>
    #include <sys/capability.h>
    #include <sys/prctl.h>
}

class TestFileUtil02 : TestFileUtilBase {
  private:
        static constexpr const bool change_caps = false;

        static void print_creds(const std::string& title) {
            jau::fprintf_td(stderr, "%s: uid %" PRIu32 ", euid %" PRIu32 ", gid %" PRIu32 ", egid %" PRIu32 "\n",
                    title.c_str(), ::getuid(), ::geteuid(), ::getgid(), ::getegid());

            gid_t gid_list[64];
            int count = ::getgroups(sizeof(gid_list)/sizeof(*gid_list), gid_list);
            if( 0 > count ) {
                ERR_PRINT("getgroups() failed");
            } else {
                jau::fprintf_td(stderr, "%s: groups[%d]: ", title.c_str(), count);
                for(int i=0; i<count; ++i) {
                    fprintf(stderr, "%" PRIu32 ", ", gid_list[i]);
                }
                fprintf(stderr, "\n");
            }

        }
        static bool set_groups(size_t size, const gid_t *list) {
            if( 0 > ::setgroups(size, list) ) {
                ERR_PRINT("setgroups failed");
                return false;
            }
            return true;
        }

        static bool set_effective_gid(::gid_t group_id) {
            if( 0 != ::setegid(group_id) ) {
                ERR_PRINT("setegid(%" PRIu32 ") failed", group_id);
                return false;
            }
            return true;
        }

        static bool set_effective_uid(::uid_t user_id) {
            if( 0 != ::seteuid(user_id) ) {
                ERR_PRINT("seteuid(%" PRIu32 ") failed", user_id);
                return false;
            }
            return true;
        }

  public:

    static bool cap_get_flag(cap_t cap_p, cap_value_t cap, cap_flag_t flag, cap_flag_value_t *value_p) noexcept {
        if( 0 != ::cap_get_flag(cap_p, cap, flag, value_p) ) {
            ERR_PRINT("cap_get_flag() failed");
            return false;
        }
        return true;
    }
    static bool cap_set_flag(cap_t cap_p, cap_flag_t flag, int ncap, const cap_value_t *caps, cap_flag_value_t value) noexcept {
        if( 0 != ::cap_set_flag(cap_p, flag, ncap, caps, value) ) {
            ERR_PRINT("cap_set_flag() failed");
            return false;
        }
        return true;
    }

    static bool cap_set_proc_flag(const std::string& title, cap_flag_t flag, int ncap, const cap_value_t *cap_list) noexcept {
        ::cap_t cap_p;
        cap_p = ::cap_get_proc();
        if( nullptr == cap_p ) {
            ERR_PRINT("cap_get_proc() failed");
            return false;
        }
        if( !cap_set_flag(cap_p, flag, ncap, cap_list, CAP_SET) ) { return false; }
        ::cap_set_proc(cap_p);
        {
            char* c_str = ::cap_to_text(cap_p, nullptr);
            jau::fprintf_td(stderr, "%s: set caps %s\n", title.c_str(), c_str);
            ::cap_free(c_str);
        }
        ::cap_free(cap_p);
        return true;
    }

    static void print_caps(const std::string& title) {
        ::cap_t caps;
        caps = ::cap_get_proc();
        if( nullptr == caps ) {
            ERR_PRINT("cap_get_proc() failed");
            return;
        }
        {
            char* c_str = ::cap_to_text(caps, nullptr);
            jau::fprintf_td(stderr, "%s: caps %s\n", title.c_str(), c_str);
            ::cap_free(c_str);
        }
        ::cap_free(caps);
    }

    /**
     * Get group-id by groupname using system commands `getent` and `cut`.
     *
     * Known group-ids are
     * - Ubuntu, Debian group 24: cdrom
     * - FreeBSD, Ubuntu, Debian group 44: video
     * - Alpine/Linux group 27: video
     */
    static ::gid_t get_gid(const std::string& groupname) {
        static const ::gid_t default_group = 44;
        std::string cmd("getent group "+groupname+" | cut -d: -f3");
        FILE* fp = ::popen(cmd.c_str(), "r");
        if (fp == nullptr) {
            fprintf(stderr, "Command failed (1) '%s'\n", cmd.c_str() );
            return default_group;
        }
        char result[100];
        ::gid_t result_int = default_group;
        if( nullptr != ::fgets(result, sizeof(result), fp) ) {
            result_int = static_cast<::gid_t>( ::atoi(result) );
            jau::PLAIN_PRINT(true, "get_gid(%s) -> %s (%d)", groupname.c_str(), result, (int)result_int);
        } else {
            fprintf(stderr, "Command failed (2) '%s'\n", cmd.c_str() );
        }
        ::pclose(fp);
        return result_int;
    }

    void test50_mount_copy_r_p() {
        INFO_STR("\n\ntest50_mount_copy_r_p\n");
        // ::cap_value_t cap_list[] = { CAP_SYS_ADMIN, CAP_SETUID, CAP_SETGID, CAP_CHOWN, CAP_FOWNER };
        ::cap_value_t cap_list[] = { CAP_SYS_ADMIN, CAP_SETUID, CAP_SETGID };
        const size_t cap_list_size = sizeof(cap_list) / sizeof(*cap_list);

        const ::uid_t super_uid = 0;
        ::uid_t caller_uid = ::getuid();

        ::uid_t user_id = caller_uid;
        jau::os::UserInfo user_info(user_id);
        if( !user_info.isValid() ) {
            ERR_PRINT("couldn't fetch [SUDO_]UID");
            return;
        }
        ::gid_t group_id = (::gid_t)user_info.gid();
        ::gid_t group_list[] = { user_id, group_id, get_gid("video") };

        const bool setuid_user_to_root = super_uid != caller_uid;
        if( setuid_user_to_root ) {
            print_creds("user level - setuid user -> root");

            ::cap_t caps;
            caps = ::cap_get_proc();
            if( nullptr == caps ) {
                ERR_PRINT("cap_get_proc() failed");
                return;
            }
            {
                char* c_str = ::cap_to_text(caps, nullptr);
                jau::fprintf_td(stderr, "user level: caps %s\n", c_str);
                ::cap_free(c_str);
            }
            cap_flag_value_t cap_sys_admin, cap_setuid, cap_setgid;
            if( !cap_get_flag(caps, CAP_SYS_ADMIN,  ::CAP_EFFECTIVE, &cap_sys_admin) ) { return; }
            if( !cap_get_flag(caps, CAP_SETUID,     ::CAP_EFFECTIVE, &cap_setuid) ) { return; }
            if( !cap_get_flag(caps, CAP_SETGID,     ::CAP_EFFECTIVE, &cap_setgid) ) { return; }
            jau::fprintf_td(stderr, "Caps: sys_admin %d, setuid %d, setgid %d\n",
                    cap_sys_admin, cap_setuid, cap_setgid);

            // Not required as mount/umount uses fork(), then seteuid(0)
            if( 0 > ::prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) ) { ERR_PRINT("prctl() failed"); }

            ::cap_free(caps);

            if( !cap_sys_admin || !cap_setuid || !cap_setgid ) {
                ERR_PRINT("capabilities incomplete, needs: cap_sys_admin, cap_setuid, cap_setgid, uid is % " PRIu32 "", caller_uid);
                return;
            }

            if( !set_groups(sizeof(group_list)/sizeof(*group_list), group_list) ) {
                return;
            }

        } else {
            print_creds("root level - setuid root -> user");

            if constexpr ( change_caps ) {
                ::cap_t caps;
                caps = ::cap_get_proc();
                if( nullptr == caps ) {
                    ERR_PRINT("cap_get_proc() failed");
                    return;
                }

                if( !cap_set_flag(caps, ::CAP_EFFECTIVE, cap_list_size, cap_list, ::CAP_SET) ) { return; }
                if( !cap_set_flag(caps, ::CAP_INHERITABLE, cap_list_size, cap_list, ::CAP_SET) ) { return; }
                if( !cap_set_flag(caps, ::CAP_PERMITTED, cap_list_size, cap_list, ::CAP_SET) ) { return; }
                if( !cap_set_proc(caps) ) { return; }
                {
                    char* c_str = ::cap_to_text(caps, nullptr);
                    jau::fprintf_td(stderr, "root level: caps %s\n", c_str);
                    ::cap_free(c_str);
                }
                if( 0 > ::prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) ) {
                    ERR_PRINT("prctl() failed");
                }
                ::cap_free(caps);
            } else {
                jau::fprintf_td(stderr, "using: changing caps disabled\n");
            }

            if( !set_groups(sizeof(group_list)/sizeof(*group_list), group_list) ) {
                return;
            }
            if( !set_effective_gid(group_id) ) {
                return;
            }
            if( !set_effective_uid(user_id) ) {
                return;
            }

            if constexpr ( change_caps ) {
                if( !cap_set_proc_flag("user level", CAP_EFFECTIVE, cap_list_size, cap_list) ) { return; }
            }
        }
        print_creds("user level");
        REQUIRE( user_id == ::geteuid() );

        {
            jau::io::fs::file_stats image_stats = getTestDataImageFile(executable_path);
            REQUIRE( true == image_stats.exists() );

            const std::string mount_point = temp_root+"_mount";
            jau::io::fs::remove(mount_point, jau::io::fs::traverse_options::recursive);  // start fresh
            REQUIRE(true == jau::io::fs::mkdir(mount_point, jau::io::fs::fmode_t::def_dir_prot));

            jau::io::fs::mount_ctx mctx;
            {
                REQUIRE( user_id == ::geteuid() );
                print_creds("pre-mount");
                print_caps("pre-mount");

                jau::io::fs::mountflags_t flags = 0;
#ifdef __linux__
                flags |= number(jau::io::fs::mountflags_linux::rdonly);
#endif
                jau::fprintf_td(stderr, "MountFlags %" PRIu64 "\n", flags);
                mctx = jau::io::fs::mount_image(image_stats.path(), mount_point, "squashfs", flags, "");

                print_creds("post-mount");
                print_caps("post-mount");
                REQUIRE( user_id == ::geteuid() );
            }
            REQUIRE( true == mctx.mounted );

            const jau::io::fs::copy_options copts = jau::io::fs::copy_options::recursive |
                                                    jau::io::fs::copy_options::preserve_all |
                                                    jau::io::fs::copy_options::sync |
                                                    jau::io::fs::copy_options::verbose;
            const std::string root_copy = temp_root+"_copy_test50";
            jau::io::fs::remove(root_copy, jau::io::fs::traverse_options::recursive);
            testxx_copy_r_p("test50_mount_copy_r_p", mount_point, 1 /* source_added_dead_links */, root_copy, copts, false /* dest_is_vfat */);
            REQUIRE(true == jau::io::fs::remove(root_copy, jau::io::fs::traverse_options::recursive));

            bool umount_ok;
            {
                REQUIRE( user_id == ::geteuid() );
                print_creds("pre-umount");
                print_caps("pre-umount");

                jau::io::fs::umountflags_t flags = 0;
#ifdef __linux__
                flags |= number(jau::io::fs::umountflags_linux::detach);  // lazy
#endif
                jau::fprintf_td(stderr, "UnmountFlags %d\n", flags);
                umount_ok = jau::io::fs::umount(mctx, flags);

                print_creds("post-umount");
                print_caps("post-umount");
                REQUIRE( user_id == ::geteuid() );
            }
            REQUIRE( true == umount_ok );

            if constexpr ( _remove_target_test_dir ) {
                REQUIRE(true == jau::io::fs::remove(mount_point, jau::io::fs::traverse_options::recursive));
            }
        }
    }
};

METHOD_AS_TEST_CASE( TestFileUtil02::test50_mount_copy_r_p,     "Test TestFileUtil02 - test50_mount_copy_r_p");
