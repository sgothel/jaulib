/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 1992-2022 Gothel Software e.K.
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
 *
 * Original header of salvaged code from July 1992
 *
 *   TOKEN_A.cpp ---  Einfacher TOKEN-AUTOMAT fuer STRINGS         5. Juli 1993
 *   V1.0
 *
 *   Sven Goethel - Stapenhorststr. 35a  4800 Bielefeld 1
 *
 *
 *   *************************************************************************
 *   * TOKEN AUTOMAT .... written 29.07.1992 by Sven Göthel                  *
 *   *************************************************************************
 */
#ifndef JAU_TOKEN_FSM_HPP_
#define JAU_TOKEN_FSM_HPP_

#include <string>
#include <type_traits>
#include <vector>

#include <jau/basic_algos.hpp>
#include <jau/darray.hpp>
#include <jau/int_types.hpp>
#include <jau/secmem.hpp>

// #define JAU_DO_JAU_TRACE_PRINT 1
#ifdef JAU_DO_JAU_TRACE_PRINT
    #define JAU_TRACE_PRINT(...) fprintf(stderr, __VA_ARGS__);
#else
    #define JAU_TRACE_PRINT(...)
#endif

namespace jau::lang {
    /** @defgroup Lang Languages
     *  Language functionality, programming and otherwise
     *
     *  Supported
     *  - jau::lang::token_fsm A lexical analyzer (tokenizer) using a tabular finite-state-machine (FSM), aka `endlicher automat` (EA)
     *
     *  For serious applications w/ regular expressions and more, as well as a `lex` C++ alternative to `flex`,
     *  consider using [Re-flex](https://github.com/Genivia/RE-flex).
     *
     *  @{
     */

    /**
     * Base Alphabet Specification providing the alphabet for token_fsm.
     *
     * Implementation delegates static code_point() function.
     *
     * @see token_fsm()
     */
    class alphabet {
        public:
            /**
             * Unsigned int symbol for alphabet code-point type
             */
            typedef uint16_t code_point_t;

            /**
             * token_error value, denoting an invalid alphabet code-point.
             */
            static inline constexpr const code_point_t code_error = std::numeric_limits<code_point_t>::max();

            typedef code_point_t (*code_point_func)(const char c) noexcept;

        private:
            std::string name_;
            code_point_t base_;
            code_point_func cpf;

        public:
            alphabet(std::string _name, code_point_t _base, code_point_func _cpf) noexcept
            : name_( std::move(_name) ), base_(_base), cpf(_cpf) {}

            /** Human readable name for this alphabet instance. */
            constexpr const std::string& name() const noexcept { return name_; }

            /** The fixed base used for this alphabet, i.e. number of token. */
            constexpr code_point_t base() const noexcept { return base_; }

            /** Returns the token of the given character or code_error if not element of this alphabet. */
            constexpr code_point_t code_point(const char c) const noexcept { return cpf(c); }

            std::string to_string() const noexcept {
                std::string res("alphabet[");
                res.append(name());
                res.append(", base "+std::to_string(base())+"]");
                return res;
            }
    };
    inline std::string to_string(const alphabet& v) noexcept { return v.to_string(); }

    inline bool operator!=(const alphabet& lhs, const alphabet& rhs ) noexcept {
        return lhs.base() != rhs.base() || lhs.name() != rhs.name();
    }

    inline bool operator==(const alphabet& lhs, const alphabet& rhs ) noexcept {
        return !( lhs != rhs );
    }

    /**
     * Full ASCII base 95 alphabet with ASCII code-point sorting order.
     *
     * ### Properties
     * - Base 95, i.e. full visible ASCII [32 .. 126]
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - Supporting ASCII code-point sorting.
     * - Order: ` ` < `0` < `:` < `A` < `[` < `a` < `{` < `~`
     */
    class ascii95_alphabet : public alphabet {
        private:
            static code_point_t s_code_point(const char c) noexcept {
                if( ' ' <= c && c <= '~' ) {
                    return c - ' ';
                } else {
                    return code_error;
                }
            }

        public:
            ascii95_alphabet() noexcept
            : alphabet("ascii95", 95, s_code_point) {}
    };

    /**
     * Case insensitive ASCII base 69 alphabet with ASCII code-point sorting order.
     *
     * ### Properties
     * - Base 69, i.e. ASCII [32 .. 96] + [123 .. 126], merging lower- and capital-letters
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - Supporting ASCII code-point sorting.
     * - Order: ` ` < `0` < `:` < `A` < `[` < `{` < `~`
     */
    class ascii69_alphabet : public alphabet {
        private:
            static code_point_t s_code_point(const char c) noexcept {
                if( ' ' <= c && c < 'a' ) { // [ 0 .. 64 ]
                    return c - ' ';
                } else if( 'a' <= c && c <= 'z' ) { // [ 33 .. 58 ]
                    return c - 'a' + 'A' - ' ';
                } else if( '{' <= c && c <= '~' ) {
                    return c - '{' + 'a' - ' '; // [ 65 .. 68 ]
                } else {
                    return code_error;
                }
            }

        public:
            ascii69_alphabet() noexcept
            : alphabet("ascii69", 69, s_code_point) {}
    };

    /**
     * Case insensitive ASCII base 26 alphabet with ASCII code-point sorting order.
     *
     * ### Properties
     * - Base 26, i.e. ASCII [65 .. 90], merging lower- and capital-letters
     * - 7-bit ASCII
     * - Code page 437 compatible
     * - Supporting ASCII code-point sorting.
     * - Order: `A` < `Z`
     */
    class ascii26_alphabet : public alphabet {
        private:
            static code_point_t s_code_point(const char c) noexcept {
                if( 'A' <= c && c < 'Z' ) { // [ 0 .. 25 ]
                    return c - 'A';
                } else if( 'a' <= c && c <= 'z' ) { // [ 0 .. 25 ]
                    return c - 'a';
                } else {
                    return code_error;
                }
            }

        public:
            ascii26_alphabet() noexcept
            : alphabet("ascii26", 26, s_code_point) {}
    };

    /**
     * A lexical analyzer (tokenizer) using a tabular finite-state-machine (FSM), aka `endlicher automat` (EA).
     *
     * Implemented initially by Sven Gothel in July 1992 using early C++ with and brought to a clean C++17 template.
     *
     * @tparam State_type used for token name and internal FSM, hence memory sensitive.
     *         Must be an unsigned integral type with minimum size of sizeof(alphabet::code_point_t), i.e. uint16_t.
     */
    template<typename State_type,
             std::enable_if_t<std::is_integral_v<State_type> &&
                              std::is_unsigned_v<State_type> &&
                              sizeof(alphabet::code_point_t) <= sizeof(State_type), bool> = true>
    class token_fsm {
        public:
            /**
             * Unsigned int symbol for token-value type
             */
            typedef State_type uint_t;

            /**
             * token_error value, denoting an invalid token or alphabet code-point.
             */
            static inline constexpr const uint_t token_error = std::numeric_limits<uint_t>::max();

            /**
             * Terminal token name and ASCII string value pair, provided by user.
             */
            struct token_value_t {
                /** Token numerical name, a terminal symbol. Value must be greater than zero and not equal to token_error. */
                uint_t name;

                /** Token ASCII string value to be tokenized. */
                std::string_view value;

                std::string to_string() const noexcept {
                    return "[ts "+std::to_string(name)+", value "+std::string(value)+"]";
                }
            };

            /**
             * Result type for token_fsm::find()
             */
            struct result_t {
                /** Token numerical name (terminal symbol) if found, otherwise token_error */
                uint_t token_name;

                /** Position of first char of token in source */
                size_t source_begin;

                /** Last position in source after token. */
                size_t source_last;

                std::string to_string() const noexcept {
                    return "[ts "+std::to_string(token_name)+", pos["+std::to_string(source_begin)+".."+std::to_string(source_last)+")]";
                }
            };

            token_fsm ( const token_fsm& src ) noexcept = default;
            token_fsm ( token_fsm&& src ) noexcept = default;
            token_fsm& operator=(const token_fsm& x) noexcept = default;
            token_fsm& operator=(token_fsm&& x) noexcept = default;

            uint_t state_count() const noexcept { return m_next_state-1; }
            uint_t next_state() const noexcept { return m_next_state; }

            bool empty() const noexcept { return 0 == state_count(); }

            /** Returns true if this FSM containes the given token name */
            bool contains(uint_t token_name) const noexcept {
                return m_token_names.cend() != std::find(m_token_names.cbegin(), m_token_names.cend(), token_name);
            }

            /** Returns the number of contained token. */
            size_t count() const noexcept { return m_token_names.size(); }

            /** Returns true if the given char is listed as a separator. */
            bool is_separator(const char c) const noexcept {
                return m_separators.cend() != std::find(m_separators.cbegin(), m_separators.cend(), c);
            }

        private:
            typedef jau::darray<uint_t, jau::nsize_t> darray_t;

            void grow(const uint_t required_sz) {
                m_matrix.reserve( required_sz + 100 );
                while( m_matrix.size() < required_sz ) {
                    m_matrix.resize(m_matrix.size() + m_row_len, 0);
                }
            }

            alphabet m_alphabet;
            uint_t  m_row_len;
            uint_t  m_end;
            std::string m_separators;

            darray_t m_matrix;
            uint_t  m_next_state;
            darray_t m_token_names;

        public:

            /**
             * Clears the FSM. Afterwards, the FSM can be filled over again from scratch.
             */
            void clear() noexcept {
                m_matrix.clear();
                m_next_state = 1;
                m_token_names.clear();
            }

            /**
             * Constructs an empty instance.
             * @param alphabet the used alphabet
             * @param separators separator, defaults to SPACE, TAB, LF, CR
             * @see add()
             */
            token_fsm (alphabet alphabet, const std::string_view separators = "\040\011\012\015")
            : m_alphabet( std::move(alphabet) ),
              m_row_len(m_alphabet.base()), m_end(m_row_len-1),
              m_separators(separators),
              m_matrix(), m_next_state(1), m_token_names()
            { }

            /**
             * Constructs a new instance w/ given token_value_t name and value pairs.
             *
             * In case of an error, method will clear() and abort, user might validated via empty().
             *
             * Reasons for failures could be
             * - invalid token name, e.g. 0
             * - duplicate token name in input key_words
             * - invalid token value
             *   - empty string
             *   - invalid character according to given alphabet or a separator
             *
             * @param alphabet the used alphabet
             * @param key_words vector of to be added token_value_t name and values
             * @param separators separator, defaults to SPACE, TAB, LF, CR
             * @see add()
             */
            token_fsm ( const alphabet& alphabet, const std::vector<token_value_t>& key_words, const std::string_view separators = "\040\011\012\015")
            : token_fsm(alphabet, separators)
            {
                const uint_t  max_state = (uint_t) std::numeric_limits<uint_t>::max();

                for( size_t word_num=0;
                     word_num < key_words.size() && m_next_state < max_state;
                     word_num++
                   )
                {
                    if( !add( key_words[word_num] ) ) {
                        return;
                    }
                }
            }

            /**
             * Adds given token_value_t name and value pair.
             *
             * In case of an error, method will clear() and abort, user might validated via empty().
             *
             * Reasons for failures could be
             * - invalid token name, e.g. 0 or token_error
             * - duplicate token name in input key_words
             * - invalid token value
             *   - empty string
             *   - invalid character according to given alphabet or a separator
             *
             * @param tkey_word the given token name and value pair
             * @return true if successful, otherwise false
             */
            bool add(const token_value_t& tkey_word) noexcept {
                if( 0 == tkey_word.name || token_error == tkey_word.name ) {
                    // invalid token name
                    return false;
                }
                if( contains( tkey_word.name ) ) {
                    // already contained -> ERROR
                    return false;
                }
                const std::string_view& key_word = tkey_word.value;
                uint_t current_state = 0;
                size_t char_num = 0;
                uint_t c = token_error;
                JAU_TRACE_PRINT("token_fsm::add: %s:\n", tkey_word.to_string().c_str());

                const uint_t  max_state = (uint_t) std::numeric_limits<uint_t>::max();
                uint_t  next_state = m_next_state;

                for( ;
                     char_num < key_word.size() &&
                     next_state < max_state;
                     ++char_num
                   )
                {
                    c = key_word[char_num];
                    JAU_TRACE_PRINT("   [%c, ", (char)c);

                    if( is_separator( c ) ) {
                        c = token_error;
                        break; // invalid character
                    }

                    const alphabet::code_point_t cp = m_alphabet.code_point(c);
                    if( alphabet::code_error == cp ) {
                        c = token_error;
                        break; // invalid character
                    } else {
                        c = cp;
                    }
                    const uint_t current_idx = m_row_len*current_state+c;
                    grow(current_idx+1);
                    JAU_TRACE_PRINT("c-off %zu, state %zu, idx %zu] ", (size_t)c, (size_t)current_state, (size_t)current_idx);

                    const uint_t current_token = m_matrix[current_idx];
                    if( !current_token ) {
                        m_matrix[current_idx] = next_state;
                        current_state = next_state++;
                        JAU_TRACE_PRINT("-> state %zu (new),\n", (size_t)current_state);
                    } else {
                        current_state = current_token;
                        JAU_TRACE_PRINT("-> state %zu (jmp),\n", (size_t)current_state);
                    }
                }

                if( char_num > 0 && c != token_error ) {
                    // token value exists (char_num) and is valid (c)
                    const uint_t current_idx = m_row_len*current_state+m_end;
                    grow(current_idx+1);

                    m_matrix[current_idx] = tkey_word.name;
                    m_token_names.push_back( tkey_word.name );
                    JAU_TRACE_PRINT("   -> terminal [c-off %zu, state %zu, idx %zu] = %zu\n", (size_t)m_end, (size_t)current_state, (size_t)current_idx, (size_t)tkey_word.name);
                } else {
                    // abort on invalid char (c) or non-existing word.(char_nu,)
                    JAU_TRACE_PRINT("   -> error\n");
                    clear();
                    return false;
                }

                if( next_state >= max_state ) {
                    // FSM exceeded, abort
                    clear();
                    return false;
                } else {
                    m_next_state = next_state;
                    return true;
                }
            }

            /**
             * Find a token within the given haystack, starting from given start position.
             *
             * This method reads over all characters until a token has been found or end-of-view.
             *
             * This method considers given separators.
             *
             * @param haystack string view to search for tokens
             * @param start start position, allowing to reuse the view
             * @return result_t denoting the found token, where result_t::token_name == token_error denotes not found.
             * @see get()
             */
            result_t find(const std::string_view& haystack, int start=0) noexcept {
                if( 0 == m_matrix.size() ) {
                    return token_fsm::result_t { .token_name = token_error, .source_begin = 0, .source_last = 0 };
                }

                /* Bis Zeilenende oder Gefundener Token durchsuchen */
                uint_t c = 0;
                jau::nsize_t i = start;
                uint_t current_state = 0;
                jau::nsize_t i2 = 0;
                while( i < haystack.size() && !current_state ) {
                    i2=i++;
                    if( is_separator(haystack[i2-1]) || i2==0 ) {
                        do {
                            if( i2 == haystack.size() ) {
                                // position after token end
                                c = m_end;
                            } else if( is_separator( c = haystack[i2++] ) ) {
                                i2--; // position after token end
                                c = m_end;
                            } else {
                                const alphabet::code_point_t cp = m_alphabet.code_point(c);
                                if( alphabet::code_error == cp ) {
                                    c = token_error;
                                    current_state=0;
                                    break; // invalid character
                                } else {
                                    c = cp;
                                }
                            }
                            const uint_t current_idx = m_row_len*current_state+c;
                            if( current_idx >= m_matrix.size() ) {
                                /** end-of-matrix **/
                                break;
                            }
                            current_state = m_matrix[current_idx];
                        } while( current_state && c != m_end );
                    }
                }

                if( c == m_end && current_state ) {
                    return token_fsm::result_t { .token_name = current_state, .source_begin = i - 1, .source_last = i2 };
                } else {
                    return token_fsm::result_t { .token_name = token_error, .source_begin = 0, .source_last = 0 };
                }
            }

            /**
             * Returns the token numerical name (terminal symbol) if found, otherwise token_error.
             *
             * This method does not consider given separators and expects given word to match a token 1:1.
             *
             * @param word the key word to lookup
             * @see find()
             */
            uint_t get(const std::string_view& word) noexcept {
                if( 0 == m_matrix.size() ) {
                    return 0;
                }
                JAU_TRACE_PRINT("token_fsm::get: %s:\n", std::string(word).c_str());

                uint_t c = 0;
                uint_t current_state = 0;
                jau::nsize_t i2 = 0;
                do {
                    if( i2 == word.size() ) {
                        c = m_end;
                    } else {
                        c = word[i2++];
                        const alphabet::code_point_t cp = m_alphabet.code_point(c);
                        if( alphabet::code_error == cp ) {
                            c = token_error;
                            current_state=0;
                            break; // invalid character
                        } else {
                            c = cp;
                        }
                    }
                    const uint_t current_idx = m_row_len*current_state+c;
                    JAU_TRACE_PRINT("   [c-off %zu, state %zu, idx %zu] ", (size_t)c, (size_t)current_state, (size_t)current_idx);
                    if( current_idx >= m_matrix.size() ) {
                        /** end-of-matrix **/
                        JAU_TRACE_PRINT("-> state %zu (eom),\n", (size_t)current_state);
                        break;
                    }
                    current_state = m_matrix[current_idx];
                    JAU_TRACE_PRINT("-> state %zu (ok),\n", (size_t)current_state);
                } while( current_state && c != m_end );

                if( c == m_end && current_state ) {
                    JAU_TRACE_PRINT("   -> final token %zu\n", (size_t)current_state);
                    return current_state;
                } else {
                    JAU_TRACE_PRINT("   -> not found\n");
                    return token_error;
                }
            }

            std::string fsm_to_string(const int token_per_row) const noexcept {
                const uint_t sz = m_matrix.size();
                const uint_t rows = sz / m_row_len;

                std::string s = "token_fsm["+m_alphabet.to_string()+", "+std::to_string(count())+" token, sz "+
                                std::to_string(sz)+" cells / "+std::to_string(sz*sizeof(uint_t))+
                                " bytes, "+std::to_string(m_row_len)+"x"+std::to_string(rows)+
                                ", next_state "+std::to_string(m_next_state)+":";
                char buf[80];
                uint_t idx=0;
                for(uint_t y=0; y<rows && idx<sz; ++y) {
                    snprintf(buf, sizeof(buf), "\n%3zu: ", (size_t)y);
                    s.append(buf);
                    for(uint_t x=0; x<m_row_len && idx<sz; ++x, ++idx) {
                        const uint_t t = m_matrix[m_row_len*y+x];
                        snprintf(buf, sizeof(buf), "%3zu, ", (size_t)t);
                        s.append(buf);
                        if( x < m_row_len-1 && ( x + 1 ) % token_per_row == 0 ) {
                            s.append("\n     ");
                        }
                    }
                }
                s.append("]\n");
                return s;
            }

            std::string to_string() const noexcept {
                const uint_t sz = m_matrix.size();
                const uint_t rows = sz / m_row_len;
                return "token_fsm["+m_alphabet.to_string()+", "+std::to_string(count())+" token, sz "+
                        std::to_string(sz)+" cells / "+std::to_string(sz*sizeof(uint_t))
                        +" bytes, "+std::to_string(m_row_len)+"x"+std::to_string(rows)+
                        ", next_state "+std::to_string(m_next_state)+"]";
            }
    };

    /**@}*/

} // namespace jau::lexer



#endif /* JAU_TOKEN_FSM_HPP_ */
