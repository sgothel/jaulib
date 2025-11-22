/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2024-2025 Gothel Software e.K.
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

#ifndef JAU_BASIC_COLLECTIONS_HPP_
#define JAU_BASIC_COLLECTIONS_HPP_

#include <cstdarg>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace jau {

    /** @defgroup Collections Collection Utilities
     *  Basic collection utilities supporting std::unordered_map, std::unordered_set, etc usage
     *
     *  @{
     */

    /**
     * C++20: Heterogeneous Lookup in (Un)ordered Containers
     *
     * @see https://www.cppstories.com/2021/heterogeneous-access-cpp20/
     */
    struct string_hash {
        using is_transparent = void;
        [[nodiscard]] size_t operator()(const char *txt) const {
            return std::hash<std::string_view>{}(txt);
        }
        [[nodiscard]] size_t operator()(std::string_view txt) const {
            return std::hash<std::string_view>{}(txt);
        }
        [[nodiscard]] size_t operator()(const std::string &txt) const {
            return std::hash<std::string>{}(txt);
        }
    };

    /**
     * std::unordered_map using K key and heterogenous jau::string_hash functor
     */
    template<typename K, typename V>
    using StringlikeHashMap = std::unordered_map<K, V, jau::string_hash, std::equal_to<>>;

    /**
     * std::unordered_map using std::string key and heterogenous jau::string_hash functor
     */
    template<typename T>
    using StringHashMap = std::unordered_map<std::string, T, string_hash, std::equal_to<>>;

    /**
     * std::unordered_set using std::string key and heterogenous jau::string_hash functor
     */
    using StringHashSet = std::unordered_set<std::string, string_hash, std::equal_to<>>;

    /**
     * std::unordered_map using std::string_view key and heterogenous jau::string_hash functor, use with care!
     *
     * Key values must persist through the lifecycle of the map.
     */
    template<typename T>
    using StringViewHashMap = std::unordered_map<std::string_view, T, string_hash, std::equal_to<>>;

    /**
     * std::unordered_set using std::string_view key and heterogenous jau::string_hash functor, use with care!
     *
     * Key values must persist through the lifecycle of the set.
     */
    using StringViewHashSet = std::unordered_set<std::string_view, string_hash, std::equal_to<>>;

    /**
     * HashMapWrap, generic std::unordered_set exposing a more higher level API
     */
    template<typename Key_type, typename Hash_functor, typename Value_type, typename Novalue_type, Novalue_type No_value>
    class HashMapWrap {
      public:
        typedef Key_type          key_type;
        typedef Hash_functor      hash_functor;
        typedef Value_type        value_type;
        typedef value_type*       pointer;
        // typedef const value_type* const_pointer;
        // typedef value_type&       reference;
        typedef const value_type& const_reference;
        typedef Novalue_type      novalue_type;

        typedef std::unordered_map<key_type, value_type, hash_functor, std::equal_to<>> HashMapType;

      private:
        HashMapType m_map;
        value_type no_value = No_value; /// converts novalue_type No_value to lvalue value_type to return its reference

      public:
        HashMapType& map() noexcept { return m_map; }
        const HashMapType& map() const noexcept { return m_map; }

        const_reference novalue() const { return no_value; }

        /** Returns the immutable mapped value reference for the given name or `no_value` */
        const_reference get(const key_type& key) const {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                return it->second;
            }
            return no_value;
        }
        /** Returns the mutable mapped value reference for the given name or std::nullopt */
        pointer get(const key_type& key) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                return &it->second;
            }
            return nullptr; // don't leak mutable no_value
        }

        /** Returns true if the given name maps to a value or `no_value`. */
        bool containsKey(const key_type& key) const {
            return m_map.contains(key);
        }

        /** Returns the key reference of the first value, otherwise std::nullopt. Note: O(n) operation, slow. */
        std::optional<const key_type&> containsValue(const_reference value) const {
            for (const std::pair<const key_type, value_type>& n : m_map) {
                if( n.second == value ) {
                    return std::optional<const key_type&>{n.first};
                }
            }
            return std::nullopt;
        }

        /** Clears the hash map. */
        void clear() { m_map.clear(); }

        /**
         * Maps the value for the given name, overwrites old mapping if exists.
         * @return true if value is newly mapped, otherwise false if replacing old mapping.
         */
        bool put0(const key_type& key, const_reference obj) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                value_type old = it->second;
                it->second        = obj;
                return false;
            }
            m_map.insert( {key, obj} );
            return true;
        }

        /**
         * Maps the value for the given name, overwrites old mapping if exists.
         *
         * Consider using put0 if old replaced value is not of interest.
         *
         * @return previously mapped value or `no_value`.
         */
        value_type put1(const key_type& key, const_reference obj) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                value_type old = it->second;
                it->second        = obj;
                return old;
            }
            m_map.insert( {key, obj} );
            return no_value;
        }

        /** Removes value if mapped and returns true, otherwise returns false. */
        bool remove0(const key_type& key) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                value_type old = it->second;
                m_map.erase(it);
                return true;
            }
            return false;
        }

        /**
         * Removes value if mapped and returns it, otherwise returns `no_value`.
         *
         * Consider using remove0 if removed value is not of interest.
         *
         * @return removed value or `no_value`.
         */
        value_type remove1(const key_type& key) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                value_type old = it->second;
                m_map.erase(it);
                return old;
            }
            return no_value;
        }
    };

    /**
     * StringHashMapWrap, generic std::unordered_set exposing a more higher level API
     *
     * Based on HashMapWrap, using std::string key and heterogenous jau::string_hash functor
     */
    template<typename Value_type, typename Novalue_type, Novalue_type no_value>
    using StringHashMapWrap = HashMapWrap<std::string, jau::string_hash, Value_type, Novalue_type, no_value>;

    /**
     * StringHashViewMapWrap, generic std::unordered_set exposing a more higher level API, use with care!
     *
     * Key values must persist through the lifecycle of the map.
     *
     * Based on HashMapWrap, using std::string_view key and heterogenous jau::string_hash functor
     */
    template<typename Value_type, typename Novalue_type, Novalue_type no_value>
    using StringViewHashMapWrap = HashMapWrap<std::string_view, jau::string_hash, Value_type, Novalue_type, no_value>;

    /**@}*/

}  // namespace jau

#endif /* JAU_BASIC_COLLECTIONS_HPP_ */
