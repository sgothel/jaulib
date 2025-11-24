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
#include <jau/type_concepts.hpp>

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
     * HashMapWrap, generic std::unordered_map exposing a more higher level API
     *
     * Allows using No_value for queries to signal `no value` found in map.
     *
     * @tparam Key_type key type
     * @tparam Value_type value type
     * @tparam Novalue_type value type representing `no value`, e.g. Value_type or std::nullptr_t for pointer.
     *                      Requirement: Value_type must be constructible from Novalue_type.
     * @tparam No_value `no value` query result of Novalue_type type, e.g. a Value_type `-1`, `MAX_VALUE` or nullptr for std::nullptr_t
     * @tparam Hash_functor Hashing function object type, defaults to std::hash<Key_type>
     * @tparam Query_type key query type, defaults to Key_type. Allows to sooth query parameter, e.g. string_view instead of string.
     *                    Requirement: Key_type must be constructible from Query_type.
     * @tparam KeyEqual Comparator function object type, defaults to std::equal_to<>
     * @tparam Allocator Allocator type, defaults to std::allocator<std::pair<const Key_type, Value_type>>
     */
    template<typename Key_type, typename Value_type,
             typename Novalue_type, Novalue_type No_value,
             typename Hash_functor = std::hash<Key_type>,
             typename Query_type = Key_type,
             typename KeyEqual = std::equal_to<>,
             typename Allocator = std::allocator<std::pair<const Key_type, Value_type>> >
        requires std::constructible_from<Value_type, Novalue_type> &&
                 std::constructible_from<Key_type, Query_type>
    class HashMapWrap {
      public:
        typedef Key_type          key_type;
        typedef Query_type        query_type;
        typedef Hash_functor      hash_functor;
        typedef Value_type        value_type;
        typedef value_type*       pointer;
        // typedef const value_type* const_pointer;
        // typedef value_type&       reference;
        typedef const value_type& const_reference;
        typedef Novalue_type      novalue_type;

        typedef std::unordered_map<key_type, value_type, hash_functor, KeyEqual, Allocator> HashMapType;
        typedef typename HashMapType::size_type     size_type;
        typedef typename HashMapType::value_type    pair_type;

      private:
        HashMapType m_map;
        value_type no_value = No_value; /// converts novalue_type No_value to lvalue value_type to return its reference

      public:
        HashMapType& map() noexcept { return m_map; }
        const HashMapType& map() const noexcept { return m_map; }

        const_reference novalue() const { return no_value; }

        size_type size() const noexcept { return m_map.size(); }

        /** Clears the hash map. */
        void clear() { m_map.clear(); }

        /** Returns the immutable mapped value reference for the given key or `no_value` */
        const_reference get(const query_type& key) const {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                return it->second;
            }
            return no_value;
        }
        /** Returns the mutable mapped value pointer for the given key or nullptr. */
        pointer get2(const query_type& key) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                return &it->second;
            }
            return nullptr; // don't leak mutable no_value
        }

        /** Returns the immutable pair_type pointer for the given key or nullptr. */
        const pair_type* find(const query_type& key) const {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                return &(*it);
            }
            return nullptr;
        }

        /** Returns true if the given key maps to a value or `no_value`. */
        bool containsKey(const query_type& key) const {
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

        /**
         * Adds a new mapping of the value for the given key, does nothing if a mapping exists.
         * @return true if value is newly mapped, otherwise false doing nothing.
         */
        bool insert(const key_type& key, const_reference obj) {
            return m_map.insert( {key, obj} ).second;
        }

        /**
         * Adds a new mapping of the value for the given key, does nothing if a mapping exists.
         * @return true if value is newly mapped, otherwise false doing nothing.
         */
        template<class Q>
        requires (!std::same_as<key_type, query_type>) &&
                 std::same_as<Q, query_type>
        bool insert(const Q& key, const_reference obj) {
            return m_map.insert( {key_type(key), obj} ).second;
        }

        /**
         * Maps the value for the given key, overwrites old mapping if exists.
         * @return true if value is newly mapped, otherwise false if replacing old mapping.
         */
        bool put(const key_type& key, const_reference obj) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                it->second        = obj;
                return false;
            }
            m_map.insert( {key, obj} );
            return true;
        }

        /**
         * Maps the value for the given key, overwrites old mapping if exists.
         * @return true if value is newly mapped, otherwise false if replacing old mapping.
         */
        template<class Q>
        requires (!std::same_as<key_type, query_type>) &&
                 std::same_as<Q, query_type>
        bool put(const Q& key, const_reference obj) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                it->second        = obj;
                return false;
            }
            m_map.insert( {key_type(key), obj} );
            return true;
        }

        /**
         * Maps the value for the given key, overwrites old mapping if exists.
         *
         * Consider using put() if old replaced value is not of interest.
         *
         * @return previously mapped value or `no_value`.
         */
        value_type put2(const key_type& key, const_reference obj) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                value_type old = it->second;
                it->second        = obj;
                return old;
            }
            m_map.insert( {key, obj} );
            return no_value;
        }

        /**
         * Maps the value for the given key, overwrites old mapping if exists.
         *
         * Consider using put() if old replaced value is not of interest.
         *
         * @return previously mapped value or `no_value`.
         */
        template<class Q>
        requires (!std::same_as<key_type, query_type>) &&
                 std::same_as<Q, query_type>
        value_type put2(const Q& key, const_reference obj) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                value_type old = it->second;
                it->second        = obj;
                return old;
            }
            m_map.insert( {key_type(key), obj} );
            return no_value;
        }

        /**
         * Replaces the already mapped value for the given key, does nothing if no mapping exists.
         * @return true if mapped value is replaced, otherwise false doing nothing.
         */
        bool replace(const key_type& key, const_reference obj) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                it->second        = obj;
                return true;
            }
            return false;
        }

        /**
         * Replaces the already mapped value for the given key, does nothing if no mapping exists.
         * @return true if mapped value is replaced, otherwise false doing nothing.
         */
        template<class Q>
        requires (!std::same_as<key_type, query_type>) &&
                 std::same_as<Q, query_type>
        bool replace(const Q& key, const_reference obj) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                it->second        = obj;
                return true;
            }
            return false;
        }

        /** Removes value if mapped and returns true, otherwise returns false. */
        bool remove(const query_type& key) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                m_map.erase(it);
                return true;
            }
            return false;
        }

        /**
         * Removes value if mapped and returns it, otherwise returns `no_value`.
         *
         * Consider using remove() if removed value is not of interest.
         *
         * @return removed value or `no_value`.
         */
        value_type remove2(const query_type& key) {
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
     * StringHashMapWrap, generic std::unordered_map exposing a more higher level API
     *
     * Based on HashMapWrap, using std::string key and heterogenous jau::string_hash functor
     */
    template<typename Value_type, typename Novalue_type, Novalue_type no_value>
    using StringHashMapWrap = HashMapWrap<std::string, Value_type, Novalue_type, no_value, jau::string_hash, std::string_view>;

    /**
     * StringHashViewMapWrap, generic std::unordered_map exposing a more higher level API, use with care!
     *
     * Key values must persist through the lifecycle of the map.
     *
     * Based on HashMapWrap, using std::string_view key and heterogenous jau::string_hash functor
     */
    template<typename Value_type, typename Novalue_type, Novalue_type no_value>
    using StringViewHashMapWrap = HashMapWrap<std::string_view, Value_type, Novalue_type, no_value, jau::string_hash>;

    /**@}*/

}  // namespace jau

#endif /* JAU_BASIC_COLLECTIONS_HPP_ */
