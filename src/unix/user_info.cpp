/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2024 Gothel Software e.K.
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
#if !defined(_WIN32)

    #include "jau/debug.hpp"
    #include "jau/os/user_info.hpp"

    extern "C" {
        #include <unistd.h>
        #include <grp.h>
        #include <pwd.h>
        #include <sys/types.h>
    }

    using namespace jau;
    using namespace jau::os;

    bool UserInfo::get_groups(std::vector<id_t>& list) noexcept {
        // jau::fprintf_td(stderr, "%s: uid %" PRIu32 ", euid %" PRIu32 ", gid %" PRIu32 ", egid %" PRIu32 "\n",
        //        title.c_str(), ::getuid(), ::geteuid(), ::getgid(), ::getegid());
        list.clear();
        ::gid_t gid_list[64];
        int count = ::getgroups(sizeof(gid_list)/sizeof(*gid_list), gid_list);
        if( 0 > count ) {
            DBG_PRINT("getgroups() failed");
            return false;
        } else {
            for(int i=0; i<count; ++i) {
                list.push_back((id_t)gid_list[i]);
            }
            DBG_PRINT("getgroups(): %s", jau::to_string(list).c_str());
            return true;
        }
    }

    bool UserInfo::set_groups(const std::vector<id_t>& list) noexcept {
        std::vector<::gid_t> n_list;
        n_list.reserve(list.size()+1);
        for(const id_t& gid : list) {
            n_list.push_back( (::gid_t)gid );
        }
        if( 0 > ::setgroups(n_list.size(), n_list.data()) ) {
            ERR_PRINT("setgroups failed");
            return false;
        }
        return true;
    }

    bool UserInfo::set_effective_gid(id_t group_id) noexcept {
        ::gid_t n_group_id = (::gid_t) group_id;
        if( 0 != ::setegid(n_group_id) ) {
            ERR_PRINT("setegid(%" PRIu64 ") failed", group_id);
            return false;
        }
        return true;
    }

    bool UserInfo::set_effective_uid(id_t user_id) noexcept {
        ::uid_t n_user_id = (::uid_t)user_id;
        if( 0 != ::seteuid(n_user_id) ) {
            ERR_PRINT("seteuid(%" PRIu64 ") failed", user_id);
            return false;
        }
        return true;
    }

    static bool UserInfo_get_env_uid(::uid_t& res_uid, const bool try_sudo) noexcept {
        char *env_str = nullptr;
        long long env_val = 0;
        if( try_sudo ) {
            env_str = ::getenv("SUDO_UID");
            if( nullptr != env_str ) {
                if( jau::to_integer(env_val, env_str, strlen(env_str)) ) {
                    res_uid = (::uid_t) env_val;
                    return true;
                }
            }
        }
        env_str = ::getenv("UID");
        if( nullptr != env_str ) {
            if( jau::to_integer(env_val, env_str, strlen(env_str)) ) {
                res_uid = (::uid_t) env_val;
                return true;
            }
        }
        return false;
    }
    bool UserInfo::get_env_uid(id_t &res_uid, const bool try_sudo) noexcept {
        ::uid_t n_res_uid = 0;
        if ( UserInfo_get_env_uid(n_res_uid, try_sudo) ) {
            res_uid = (id_t)n_res_uid;
            return true;
        }
        return false;
    }

    bool UserInfo::get_env_username(std::string &username, const bool try_sudo) noexcept {
        char *env_str = nullptr;
        if ( try_sudo ) {
            env_str = ::getenv("SUDO_USER");
            if ( nullptr != env_str ) {
                username = std::string(env_str);
                return true;
            }
        }
        env_str = ::getenv("USER");
        if ( nullptr != env_str ) {
            username = std::string(env_str);
            return true;
        }
        return false;
    }

    bool UserInfo::get_creds(id_t &res_uid, id_t &res_gid, std::string &username, std::string &homedir, std::string &shell) noexcept {
        ::uid_t n_res_uid = (::uid_t)res_uid;
        const bool is_root = 0 == n_res_uid;
        struct passwd pwd;
        char buffer[1024];

        if ( !is_root || UserInfo_get_env_uid(n_res_uid, is_root) ) {
            struct passwd *pwd_res = nullptr;
            if ( 0 != ::getpwuid_r(n_res_uid, &pwd, buffer, sizeof(buffer), &pwd_res) || nullptr == pwd_res ) {
                DBG_PRINT("getpwuid(%" PRIu32 ") failed", n_res_uid);
                return false;
            }
            DBG_PRINT("getpwuid(%" PRIu32 "): name '%s', uid %" PRIu32 ", gid %" PRIu32 "\n", n_res_uid, pwd_res->pw_name, pwd_res->pw_uid, pwd_res->pw_gid);
            res_uid = (id_t)n_res_uid;
            res_gid = (id_t)(::gid_t)(pwd_res->pw_gid);
            username = std::string(pwd_res->pw_name);
            homedir = std::string(pwd_res->pw_dir);
            shell = std::string(pwd_res->pw_shell);
            return true;
        } else {
            std::string tmp_username;
            if ( get_env_username(tmp_username, is_root) ) {
                struct passwd *pwd_res = nullptr;
                if ( 0 != ::getpwnam_r(tmp_username.c_str(), &pwd, buffer, sizeof(buffer), &pwd_res) || nullptr == pwd_res ) {
                    DBG_PRINT("getpwnam(%s) failed\n", tmp_username.c_str());
                    return false;
                }
                DBG_PRINT("getpwnam(%s): name '%s', uid %" PRIu32 ", gid %" PRIu32 "\n", tmp_username.c_str(), pwd_res->pw_name, pwd_res->pw_uid, pwd_res->pw_gid);
                res_uid = (id_t)n_res_uid;
                res_gid = (id_t)(::gid_t)(pwd_res->pw_gid);
                username = std::string(pwd_res->pw_name);
                homedir = std::string(pwd_res->pw_dir);
                shell = std::string(pwd_res->pw_shell);
                return true;
            }
        }
        return false;
    }

    bool UserInfo::get_creds(const std::string &username_lookup, id_t &res_uid, id_t &res_gid, std::string &username, std::string &homedir, std::string &shell) noexcept {
        struct passwd pwd;
        char buffer[1024];
        struct passwd *pwd_res = nullptr;
        if ( 0 != ::getpwnam_r(username_lookup.c_str(), &pwd, buffer, sizeof(buffer), &pwd_res) || nullptr == pwd_res ) {
            DBG_PRINT("getpwnam(%s) failed\n", username_lookup.c_str());
            return false;
        }
        DBG_PRINT("getpwnam(%s): name '%s', uid %" PRIu32 ", gid %" PRIu32 "\n", username_lookup.c_str(), pwd_res->pw_name, pwd_res->pw_uid, pwd_res->pw_gid);
        res_uid = (id_t)(::uid_t)(pwd_res->pw_uid);
        res_gid = (id_t)(::gid_t)(pwd_res->pw_gid);
        username = std::string(pwd_res->pw_name);
        homedir = std::string(pwd_res->pw_dir);
        shell = std::string(pwd_res->pw_shell);
        return true;
    }

    UserInfo::UserInfo() noexcept {
        m_uid = (id_t)::getuid();
        m_valid = get_creds(m_uid, m_gid, m_username, m_homedir, m_shell);
        if ( m_valid ) {
            get_groups(m_gid_list);
        }
    }
    UserInfo::UserInfo(id_t uid) noexcept {
        m_uid = uid;
        m_valid = get_creds(m_uid, m_gid, m_username, m_homedir, m_shell);
        if ( m_valid ) {
            get_groups(m_gid_list);
        }
    }

    UserInfo::UserInfo(const std::string &username) noexcept {
        m_valid = get_creds(username, m_uid, m_gid, m_username, m_homedir, m_shell);
        if ( m_valid ) {
            get_groups(m_gid_list);
        }
    }

#endif  // !_WIN32
