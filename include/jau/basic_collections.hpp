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
    template<typename Key_type, typename Hash_functor, typename Value_type, typename Novalue_type, Novalue_type no_value>
    class HashMapWrap {
      public:
        typedef std::unordered_map<Key_type, Value_type, Hash_functor, std::equal_to<>> HashMapType;

      private:
        HashMapType m_map;

      public:
        HashMapType& map() noexcept { return m_map; }
        const HashMapType& map() const noexcept { return m_map; }

        /** Returns the mapped value for the given name or `no_value` */
        Value_type get(std::string_view key) const {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                return it->second;
            }
            return no_value;
        }

        /** Returns true if the given name maps to a value or `no_value`. */
        bool containsKey(std::string_view key) const {
            return m_map.contains(key);
        }

        /** Returns the string_view key of the first value, otherwise std::nullopt. Note: O(n) operation, slow. */
        std::optional<std::string_view> containsValue(const Value_type& value) const {
            for (const std::pair<const std::string, Value_type>& n : m_map) {
                if( n.second == value ) {
                    return std::optional<std::string_view>{n.first};
                }
            }
            return std::nullopt;
        }

        /** Clears the hash map. */
        void clear() { m_map.clear(); }

        /**
         * Maps the value for the given name, overwrites old mapping if exists.
         * @return previously mapped value or `no_value`.
         */
        Value_type put(std::string_view key, const Value_type& obj) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                Value_type old = it->second;
                it->second        = obj;
                return old;
            }
            m_map.insert({std::string(key), obj });
            // m_attachedMap[key] = obj;
            return no_value;
        }

        /** Removes value if mapped and returns it, otherwise returns `no_value`. */
        Value_type remove(std::string_view key) {
            auto it = m_map.find(key);
            if( it != m_map.end() ) {
                Value_type old = it->second;
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
