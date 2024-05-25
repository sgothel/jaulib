/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2010-2024 Gothel Software e.K.
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

#ifndef JAU_VERSIONNUMBER_HPP_
#define JAU_VERSIONNUMBER_HPP_

#include <compare>
#include <cstddef>
#include <iostream>
#include <ostream>
#include <regex>
#include <string>

#include <jau/int_types.hpp>
#include <jau/string_util.hpp>

namespace jau::util {

    /**
     * Simple version number class containing a version number
     * either being {@link #VersionNumber(int, int, int) defined explicit}
     * or {@link #VersionNumber(String, String) derived from a string}.
     *
     * For the latter case, you can query whether a component has been defined explicitly by the given <code>versionString</code>,
     * via {@link #hasMajor()}, {@link #hasMinor()} and {@link #hasSub()}.
     *     
     * The state whether a component is defined explicitly <i>is not considered</i>
     * in the {@link #hashCode()}, {@link #equals(Object)} or {@link #compareTo(Object)} methods,
     * since the version number itself is treated regardless.
     */
    class VersionNumber {
      private:
        int m_major, m_minor, m_sub;
        int m_git_commits;
        uint64_t m_git_ssha;
        bool m_git_dirty;
        ssize_t m_strEnd;
        std::string m_version_str;
        
        uint16_t state;

        constexpr static const uint16_t HAS_MAJOR       = 1U << 0;
        constexpr static const uint16_t HAS_MINOR       = 1U << 1;
        constexpr static const uint16_t HAS_SUB         = 1U << 2;
        constexpr static const uint16_t HAS_GIT_INFO    = 1U << 3;
        constexpr static const bool DBG_OUT             = false;

      protected:
        VersionNumber(int majorRev, int minorRev, int subMinorRev, 
                      int gitCommits, uint64_t gitSSHA, bool gitDirty,
                      ssize_t strEnd, uint16_t _state) noexcept 
        : m_major(majorRev), m_minor(minorRev), m_sub(subMinorRev),
          m_git_commits(gitCommits), m_git_ssha(gitSSHA), m_git_dirty(gitDirty),
          m_strEnd(strEnd), m_version_str(), state(_state) {} 
    
      public:
        static std::regex getPattern(const std::string& delim) {
            // v0.0.1-3-gd55f8a3-dirty
            // return std::regex( R"(\D*(\d+)[^\.\s]*(?:\.\D*(\d+)[^\.\s]*(?:\.\D*(\d+)(?:\-(\d+)\-g([0-9a-f]+)(\-dirty)?)?)?)?)");
            return std::regex( R"(\D*(\d+)[^\)" + delim + R"(\s]*(?:\)" + delim + R"(\D*(\d+)[^\)" + delim + R"(\s]*(?:\)" + delim + R"(\D*(\d+)(?:\-(\d+)\-g([0-9a-f]+)(\-dirty)?)?)?)?)");
        }

        static const std::regex& getDefaultPattern() noexcept { // NOLINT(bugprone-exception-escape)
            static std::regex defPattern = getPattern(".");
            return defPattern;
        }

        /**
         * Explicit version number instantiation, with all components defined explicitly including git.
         * @see #hasMajor()
         * @see #hasMinor()
         * @see #hasSub()
         * @see #hasGitInfo()
         */
        VersionNumber(int majorRev, int minorRev, int subMinorRev, int gitCommits, uint64_t gitSSHA, bool gitDirty) noexcept 
        : VersionNumber(majorRev, minorRev, subMinorRev, 
                        gitCommits, gitSSHA, gitDirty, 
                        -1, HAS_MAJOR | HAS_MINOR | HAS_SUB | HAS_GIT_INFO)
        {}
        
        /**
         * Explicit version number instantiation, with all components defined explicitly excluding git.
         * @see #hasMajor()
         * @see #hasMinor()
         * @see #hasSub()
         */
        VersionNumber(int majorRev, int minorRev, int subMinorRev) noexcept 
        : VersionNumber(majorRev, minorRev, subMinorRev, 0, 0, false, -1, HAS_MAJOR | HAS_MINOR | HAS_SUB)
        {}
        
        /**
         * Default ctor for zero version.
         * @see #hasMajor()
         * @see #hasMinor()
         * @see #hasSub()
         */
        VersionNumber() noexcept 
        : VersionNumber(0, 0, 0, 0, 0, false, -1, HAS_MAJOR | HAS_MINOR | HAS_SUB)
        {}
        
        VersionNumber(const VersionNumber&)            noexcept = default;
        VersionNumber(VersionNumber&&)                 noexcept = default;
        VersionNumber& operator=(const VersionNumber&) noexcept = default;
        VersionNumber& operator=(VersionNumber&&)      noexcept = default;

        /**
         * String derived version number instantiation.
         * <p>
         * You can query whether a component has been defined explicitly by the given <code>versionString</code>,
         * via {@link #hasMajor()}, {@link #hasMinor()} and {@link #hasSub()}.
         * </p>
         * @param versionString should be given as [MAJOR[.MINOR[.SUB]]]
         * @param versionPattern the {@link java.util.regex.Pattern pattern} parser, must be compatible w/ {@link #getVersionNumberPattern(String)}
         *
         * @see #hasMajor()
         * @see #hasMinor()
         * @see #hasSub()
         */
        VersionNumber(const std::string& versionString, const std::regex& versionPattern) noexcept
        : m_major(0), m_minor(0), m_sub(0), 
          m_git_commits(0), m_git_ssha(0), m_git_dirty(false),
          m_strEnd(0), m_version_str(versionString), state(0) 
        {
            // group1: \d* == digits major
            // group2: \d* == digits minor
            // group3: \d* == digits sub
            // group4: \d* == digits commit-count
            // group5: hex == hex short-sha
            // group6: str == dirty
            std::smatch match;
            if( std::regex_search(versionString, match, versionPattern) ) {
                m_strEnd = match.position() + match.length();
                if constexpr ( DBG_OUT ) {
                    std::cout << "XXX: " << versionString << std::endl;
                    std::cout << "XXX: match pos " << match.position() << ", len " << match.length() << ", sz " << match.size() << std::endl;
                    for(size_t i=0; i<match.size(); ++i) {
                        const std::string& s = match[i];
                        std::cout << "- [" << i << "]: '" << s << "', len " << s.length() << std::endl;
                    }
                }
                if( match.size() >= 2 && match[1].length() > 0 ) {
                    m_major  = std::stoi(match[1]);
                    state |= HAS_MAJOR;
                    if( match.size() >= 3 && match[2].length() > 0 ) {
                        m_minor  = std::stoi(match[2]);
                        state |= HAS_MINOR;
                        if( match.size() >= 4 && match[3].length() > 0 ) {
                            m_sub    = std::stoi(match[3]);
                            state |= HAS_SUB;                            
                            if( match.size() >= 5 && match[4].length() > 0 ) {
                                m_git_commits = std::stoi(match[4]);
                                state |= HAS_GIT_INFO;
                                if constexpr ( DBG_OUT ) {
                                    std::cout << "XXX: git commits " << m_git_commits << std::endl;
                                }
                                if( match.size() >= 6 && match[5].length() > 0 ) {
                                    m_git_ssha = jau::from_hexstring(match[5]);
                                    if constexpr ( DBG_OUT ) {
                                        std::cout << "XXX: git ssha '" << match[5] << "', hex " << jau::to_hexstring(m_git_ssha) << ", dec " << m_git_ssha << std::endl;
                                    }
                                    if( match.size() >= 7 && match[6].length() > 0 ) {
                                        m_git_dirty = true;
                                        if constexpr ( DBG_OUT ) {
                                            std::cout << "XXX: git dirty " << m_git_dirty << std::endl;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        /**
         * String derived version number instantiation.
         * <p>
         * Utilizing the default {@link java.util.regex.Pattern pattern} parser with delimiter "<b>.</b>", see {@link #getDefaultVersionNumberPattern()}.
         * </p>
         * <p>
         * You can query whether a component has been defined explicitly by the given <code>versionString</code>,
         * via {@link #hasMajor()}, {@link #hasMinor()} and {@link #hasSub()}.
         * </p>
         * @param versionString should be given as [MAJOR[.MINOR[.SUB]]]
         *
         * @see #hasMajor()
         * @see #hasMinor()
         * @see #hasSub()
         */
        VersionNumber(const std::string& versionString) noexcept
        : VersionNumber(versionString, getDefaultPattern())
        { }
    
        /**
         * String derived version number instantiation.
         * <p>
         * Utilizing {@link java.util.regex.Pattern pattern} parser created via {@link #getVersionNumberPattern(String)}.
         * </p>
         * <p>
         * You can query whether a component has been defined explicitly by the given <code>versionString</code>,
         * via {@link #hasMajor()}, {@link #hasMinor()} and {@link #hasSub()}.
         * </p>
         * @param versionString should be given as [MAJOR[.MINOR[.SUB]]]
         * @param delim the delimiter, e.g. "."
         *
         * @see #hasMajor()
         * @see #hasMinor()
         * @see #hasSub()
         */
        VersionNumber(const std::string& versionString, const std::string& delim) noexcept 
        : VersionNumber(versionString, getPattern(delim))
        { }
    

        /** Returns <code>true</code>, if all version components are zero, otherwise <code>false</code>. */
        bool isZero() const noexcept {
            return m_major == 0 && m_minor == 0 && m_sub == 0;
        }
    
        /** Returns <code>true</code>, if the major component is defined explicitly, otherwise <code>false</code>. Undefined components has the value <code>0</code>. */
        constexpr bool hasMajor() const noexcept { return 0 != ( HAS_MAJOR & state ); }
        /** Returns <code>true</code>, if the optional minor component is defined explicitly, otherwise <code>false</code>. Undefined components has the value <code>0</code>. */
        constexpr bool hasMinor() const noexcept { return 0 != ( HAS_MINOR & state ); }
        /** Returns <code>true</code>, if the optional sub component is defined explicitly, otherwise <code>false</code>. Undefined components has the value <code>0</code>. */
        constexpr bool hasSub()   const noexcept { return 0 != ( HAS_SUB & state ); }
        /** Returns <code>true</code>, if the optional git information is defined explicitly, otherwise <code>false</code>. */
        constexpr bool hasGitInfo()  const noexcept { return 0 != ( HAS_GIT_INFO & state ); }
    
        /** Returns true if constructed with a `version-string`, otherwise false. */
        constexpr bool hasString() const noexcept { return m_version_str.length() > 0; }
        /** Returns the used `version-string`, empty if not constructed with such. */
        constexpr const std::string& versionString() const noexcept { return m_version_str; }
        
        /**
         * If constructed with `version-string`, returns the string offset <i>after</i> the last matching character,
         * or <code>0</code> if none matched, or <code>-1</code> if not constructed with a string.
         */
        constexpr ssize_t endOfStringMatch() const noexcept { return m_strEnd; }

        constexpr int major() const noexcept { return m_major; }
        constexpr int minor() const noexcept { return m_minor; }
        constexpr int sub() const noexcept { return m_sub; }
        constexpr int git_commits() const noexcept { return m_git_commits; }
        constexpr uint64_t git_ssha() const noexcept { return m_git_ssha; }
        constexpr bool git_dirty() const noexcept { return m_git_dirty; }
                
        /** Two way comparison operator */
        constexpr bool operator==(const VersionNumber& vo) const noexcept {
            return m_major == vo.m_major && m_minor == vo.m_minor && m_sub == vo.m_sub && 
                   m_git_commits == vo.m_git_commits && m_git_ssha == vo.m_git_ssha && m_git_dirty == vo.m_git_dirty; 
        }
        
        /** Three way std::strong_ordering comparison operator */
        constexpr std::strong_ordering operator<=>(const VersionNumber& vo) const noexcept {
            if( m_major > vo.m_major ) {
                return std::strong_ordering::greater;
            } else if( m_major < vo.m_major ) {
                return std::strong_ordering::less;
            } else if( m_minor > vo.m_minor ) {
                return std::strong_ordering::greater;
            } else if( m_minor < vo.m_minor ) {
                return std::strong_ordering::less;
            } else if( m_sub > vo.m_sub ) {
                return std::strong_ordering::greater;
            } else if( m_sub < vo.m_sub ) {
                return std::strong_ordering::less;
            }
            if( hasGitInfo() ) {                
                if( m_git_commits > vo.m_git_commits ) {
                    return std::strong_ordering::greater;
                } else if( m_git_commits < vo.m_git_commits ) {
                    return std::strong_ordering::less;
                } else if( !m_git_dirty && vo.m_git_dirty ) {
                    return std::strong_ordering::greater;
                } else if( m_git_dirty && !vo.m_git_dirty ) {
                    return std::strong_ordering::less;
                } else if( m_git_ssha > vo.m_git_ssha ) {  // no sane interpretation of m_git_ssha
                    return std::strong_ordering::greater;
                } else if( m_git_ssha < vo.m_git_ssha ) {  // no sane interpretation of m_git_ssha
                    return std::strong_ordering::less;
                }
            }
            return std::strong_ordering::equal;
        }
        
        constexpr std::size_t hash() const noexcept {
            // 31 * x == (x << 5) - x
            std::size_t h = 31 + major();
            h = ((h << 5) - h) + minor();
            h = ((h << 5) - h) + sub();
            h =  (h << 15)     + git_ssha();  // 32-bit aligned
            return (h << 1) + ( git_dirty() ? 1 : 0 );
        }
        
        std::string toString() const noexcept {
            std::string res = std::to_string(m_major) + "." + std::to_string(m_minor) + "." + std::to_string(m_sub);
            if( hasGitInfo() ) {
                res.append(", git[post ").append(std::to_string(m_git_commits))
                   .append(", tip ").append(jau::to_hexstring(m_git_ssha, true))
                   .append(", ");
                if( git_dirty() ) {
                    res.append("dirty");
                } else {
                    res.append("clean");
                }
                res.append("]");
            }
            if( hasString() ) {
                res.append(" (").append(m_version_str).append(")");
            }
            return res;
        }

    };
    
    inline std::ostream& operator<<(std::ostream& out, const VersionNumber& v) noexcept {
        return out << v.toString();
    }

}  // namespace jau::util

namespace std {
    template<> struct hash<jau::util::VersionNumber>
    {
        std::size_t operator()(const jau::util::VersionNumber& v) const noexcept {
            return v.hash();
        }
    };
}

#endif /* JAU_VERSIONNUMBER_HPP_ */
