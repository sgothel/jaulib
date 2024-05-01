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
#pragma once

#include <cstdint>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <string>

namespace jau::os {

    /** \addtogroup OSSup
     *
     *  @{
     */

    /**
     * User account information of the underlying OS.
     */
    class UserInfo {
        public:
            typedef uint64_t id_t;
        private:
            bool m_valid;
            id_t m_uid;
            id_t m_gid;
            std::string m_username;
            std::string m_homedir;
            std::string m_shell;
            std::vector<id_t> m_gid_list;

            static bool set_groups(const std::vector<id_t>& list) noexcept;
            static bool set_effective_gid(id_t group_id) noexcept;
            static bool set_effective_uid(id_t user_id) noexcept;

            static bool get_groups(std::vector<id_t>& list) noexcept;
            static bool get_env_uid(id_t& res_uid, const bool try_sudo) noexcept;
            static bool get_env_username(std::string& username, const bool try_sudo) noexcept;

            /** get creds by passed res_uid (updated) */
            static bool get_creds(id_t& res_uid, id_t& res_gid, std::string& username, std::string& homedir, std::string& shell) noexcept;
            /** get creds by passed username */
            static bool get_creds(const std::string& username_lookup, id_t& res_uid, id_t& res_gid, std::string& username, std::string& homedir, std::string& shell) noexcept;

        public:
            /** Create instance of the user executing this application. */
            UserInfo() noexcept;

            /** Create instance of the given user id. */
            UserInfo(id_t uid) noexcept;

            /** Create instance of the given user name. */
            UserInfo(const std::string& username) noexcept;

            bool isValid() const noexcept { return m_valid; }
            id_t uid() const noexcept { return m_uid; }
            id_t gid() const noexcept { return m_gid; }
            const std::string& username() const noexcept { return m_username; }
            const std::string& homedir() const noexcept { return m_homedir; }
            const std::string& shell() const noexcept { return m_shell; }
            const std::vector<id_t>& groups() const noexcept { return m_gid_list; }

            std::string toString() const noexcept {
                std::string s = "UserInfo['";
                if( m_valid ) {
                    s.append(username()).append("', uid ").append(std::to_string(uid())).append(", gid ").append(std::to_string(gid()))
                     .append(", home '").append(homedir()).append("', shell '").append(shell())
                     .append("', groups [").append(jau::to_string(groups())).append("]]");
                } else {
                    s.append("undef]");
                }
                return s;
            }
    };

    /**@}*/

}
