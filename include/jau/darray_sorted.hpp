/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 1994-2022 Gothel Software e.K.
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

#ifndef JAU_DYN_ARRAY_SORTED_HPP_
#define JAU_DYN_ARRAY_SORTED_HPP_

#include <jau/darray.hpp>

namespace jau {

    /** \addtogroup DataStructs
     *
     *  @{
     */

    /**
     * Extension to darray resulting in a sorted darray via insert().
     *
     * In der hier deklarierten Template-Klasse 'darray_sorted' werden die
     * einzelnen Listen Elemente sofort beim Einfuegen der Groesse nach
     * sortiert ( In aufsteigender(UP)/absteigender(DOWN) Reihenfolge ).
     * D.h. die Methode 'fuegeEin' fuegt die Listenelemente mittels
     * Bisektionierung gleich sortiert ein.
     *
     * fuegeEin liefert nun den Index zurueck, den das eingefuegte Element
     * besitzt, ansonsten npos.
     * die Listenelemente gleich sortiert eingefuegt werden, mittel Bisektionierung.
     *
     * Neu ist die Methode findeElement, welche ebenfalls nach der MEthode der
     * Bisektionierung arbeitet.
     *
     * @tparam Value_type
     * @tparam Alloc_type
     * @tparam Size_type
     * @tparam use_memmove
     * @tparam use_secmem
     */
    template <typename Value_type, typename Alloc_type = jau::callocator<Value_type>, typename Size_type = jau::nsize_t,
              bool use_memmove = std::is_trivially_copyable_v<Value_type> || is_container_memmove_compliant_v<Value_type>,
              bool use_secmem  = is_enforcing_secmem_v<Value_type>
             >
    class darray_sorted : protected darray<Value_type, Alloc_type, Size_type, use_memmove, use_secmem> {
        public:
            typedef darray<Value_type, Alloc_type, Size_type, use_memmove, use_secmem> darray_type;
            using typename darray_type::value_type;
            using typename darray_type::size_type;

            using darray_type::erase;
            using darray_type::operator[];
            using darray_type::size;

            /**
             * Special value representing maximal value of size_type, used to denote an invalid index position return value, i.e. `no position`.
             */
            static constexpr const size_type npos = std::numeric_limits<size_type>::max();

            enum order_t { UP, DOWN };

            darray_sorted(order_t order=UP) : darray_type(), m_order(order) { }
            darray_sorted(const darray_sorted& m) : darray_type(m), m_order(m.m_order) {}

            darray_sorted& operator=(const darray_sorted &m)
            {
                m_order=m.m_order;
                darray_type::operator=(m);
                return *this;
            }

            size_type insert(const value_type& a) {
                size_type u=0, o=size()>0 ? size()-1 : 0;
                size_type i=index_of(a, u, o);
                if ( npos == i ) {
                    darray_type::insert(o, a);
                    return o;
                } else {
                    darray_type::insert(i, a);
                    return i;
                }
            }

            bool contains(const value_type& x) const noexcept {
                return npos != index_of(x);
            }

            /**
             * Returns index of given element if found or npos
             * @param x
             * @return
             */
            size_type index_of(const value_type& x) const {
                size_type u=0, o=size()>0 ? size()-1 : 0;
                return index_of(x, u, o);
            }

        private:
            /**
             * @param x the desired value_type
             * @param l lower border (bottom), inclusive
             * @param h higher border (top), inclusive
             * @return >= 0 gefunden index,  npos nicht gefunden
             */
            size_type index_of(const value_type &x, size_type &l, size_type &h) const {
                size_type i=0;

                if ( size() == 0 ) { return npos; }

                if(m_order==UP) {
                    if( l == h && (*this)[h] < x ) { h++; }
                    else if( (*this)[l] > x   )  { h=l;           }
                    else if( (*this)[h] < x   )  { l=h; h++;      }
                    else if( (*this)[l] == x  )  { return l;   }
                    else if( (*this)[h] == x  )  { return h;   }
                } else {
                    if( l == h && (*this)[h] > x ) { h++; }
                    else if( (*this)[l] < x   )  { h=l;           }
                    else if( (*this)[h] > x   )  { l=h; h++;      }
                    else if( (*this)[l] == x  )  { return l; }
                    else if( (*this)[h] == x  )  { return h; }
                }

                while( h-l > 1 ) {
                    i=(l+h)/2;
                    if ( (*this)[i] < x ) {
                        if ( m_order==UP ) l=i;
                        else h=i;
                    } else if ( (*this)[i] > x ) {
                        if ( m_order==UP ) h=i;
                        else l=i;
                    } else {
                        return i;
                    }
                }
                return npos;
            }
            order_t m_order;
    };

    /**@}*/

} /* namespace jau */

#endif /* JAU_DYN_ARRAY_SORTED_HPP_ */


