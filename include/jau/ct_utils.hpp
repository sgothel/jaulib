/**
 * Functions for constant time operations on data and testing of
 * constant time annotations using valgrind.
 *
 * For more information about constant time programming see
 * Wagner, Molnar, et al "The Program Counter Security Model"
 *
 * (C) 2010 Falko Strenzke
 * (C) 2015,2016,2018 Jack Lloyd
 * (C) 2024 Sven Gothel
 *
 * jaulib including this code is released under the MIT License (see COPYING)
 * Botan itself is released under the Simplified BSD License (see COPYING)
 */

#ifndef JAU_CT_UTILS_HPP_
#define JAU_CT_UTILS_HPP_

#include <type_traits>
#include <jau/int_math.hpp>

#if defined(JAU_HAS_VALGRIND)
  #include <valgrind/memcheck.h>
#endif

namespace jau::CT {

/**
* Use valgrind to mark the contents of memory as being undefined.
* Valgrind will accept operations which manipulate undefined values,
* but will warn if an undefined value is used to decided a conditional
* jump or a load/store address. So if we poison all of our inputs we
* can confirm that the operations in question are truly const time
* when compiled by whatever compiler is in use.
*
* Even better, the VALGRIND_MAKE_MEM_* macros work even when the
* program is not run under valgrind (though with a few cycles of
* overhead, which is unfortunate in final binaries as these
* annotations tend to be used in fairly important loops).
*
* This approach was first used in ctgrind (https://github.com/agl/ctgrind)
* but calling the valgrind mecheck API directly works just as well and
* doesn't require a custom patched valgrind.
*/
template<typename T>
inline void poison([[maybe_unused]] const T* p, [[maybe_unused]] nsize_t n)
   {
#if defined(JAU_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_UNDEFINED(p, n * sizeof(T));
#endif
   }

template<typename T>
inline void unpoison([[maybe_unused]] const T* p, [[maybe_unused]] nsize_t n)
   {
#if defined(JAU_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_DEFINED(p, n * sizeof(T));
#endif
   }

template<typename T>
inline void unpoison([[maybe_unused]] T& p)
   {
#if defined(JAU_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_DEFINED(&p, sizeof(T));
#endif
   }

/**
* A Mask type used for constant-time operations. A Mask<T> always has value
* either 0 (all bits cleared) or ~0 (all bits set). All operations in a Mask<T>
* are intended to compile to code which does not contain conditional jumps.
* This must be verified with tooling (eg binary disassembly or using valgrind)
* since you never know what a compiler might do.
*
 * @tparam T unsigned integral type
 */
template <typename T,
          std::enable_if_t< std::is_integral_v<T> && std::is_unsigned_v<T>, bool> = true>
class Mask
   {
   public:
      Mask(const Mask<T>& other) noexcept
      : m_mask(other.m_mask) { }

      Mask<T>& operator=(const Mask<T>& other) noexcept {
          m_mask = other.m_mask;
          return *this;
      }

      /**
      * Derive a Mask from a Mask of a larger type
      */
      template<typename U>
      Mask(Mask<U> o) noexcept : m_mask(static_cast<T>(o.value()))
         {
         static_assert(sizeof(U) > sizeof(T), "sizes ok");
         }

      /**
      * Return a Mask<T> with all bits set
      */
      static Mask<T> set() noexcept
         {
         return Mask<T>(static_cast<T>(~0));
         }

      /**
      * Return a Mask<T> with all bits cleared
      */
      static Mask<T> cleared() noexcept
         {
         return Mask<T>(0);
         }

      /**
      * Return a Mask<T> which is set if v is != 0
      */
      static Mask<T> expand(T v) noexcept
         {
         return ~Mask<T>::is_zero(v);
         }

      /**
      * Return a Mask<T> which is set if m is set
      */
      template<typename U>
      static Mask<T> expand(Mask<U> m) noexcept
         {
         static_assert(sizeof(U) < sizeof(T), "sizes ok");
         return ~Mask<T>::is_zero(m.value());
         }

      /**
      * Return a Mask<T> which is set if v is == 0 or cleared otherwise
      */
      static Mask<T> is_zero(T x) noexcept
         {
         return Mask<T>(ct_is_zero<T>(x));
         }

      /**
      * Return a Mask<T> which is set if x == y
      */
      static Mask<T> is_equal(T x, T y) noexcept
         {
         return Mask<T>::is_zero(static_cast<T>(x ^ y));
         }

      /**
      * Return a Mask<T> which is set if x < y
      */
      static Mask<T> is_lt(T x, T y) noexcept
         {
         return Mask<T>(ct_expand_top_bit<T>(x^((x^y) | ((x-y)^x))));
         }

      /**
      * Return a Mask<T> which is set if x > y
      */
      static Mask<T> is_gt(T x, T y) noexcept
         {
         return Mask<T>::is_lt(y, x);
         }

      /**
      * Return a Mask<T> which is set if x <= y
      */
      static Mask<T> is_lte(T x, T y) noexcept
         {
         return ~Mask<T>::is_gt(x, y);
         }

      /**
      * Return a Mask<T> which is set if x >= y
      */
      static Mask<T> is_gte(T x, T y) noexcept
         {
         return ~Mask<T>::is_lt(x, y);
         }

      static Mask<T> is_within_range(T v, T l, T u) noexcept
         {
         //return Mask<T>::is_gte(v, l) & Mask<T>::is_lte(v, u);

         const T v_lt_l = v^((v^l) | ((v-l)^v));
         const T v_gt_u = u^((u^v) | ((u-v)^u));
         const T either = v_lt_l | v_gt_u;
         return ~Mask<T>(ct_expand_top_bit(either));
         }

      static Mask<T> is_any_of(T v, std::initializer_list<T> accepted) noexcept
         {
         T accept = 0;

         for(auto a: accepted)
            {
            const T diff = a ^ v;
            const T eq_zero = ~diff & (diff - 1);
            accept |= eq_zero;
            }

         return Mask<T>(ct_expand_top_bit(accept));
         }

      /**
      * AND-combine two masks
      */
      Mask<T>& operator&=(Mask<T> o) noexcept
         {
         m_mask &= o.value();
         return (*this);
         }

      /**
      * XOR-combine two masks
      */
      Mask<T>& operator^=(Mask<T> o) noexcept
         {
         m_mask ^= o.value();
         return (*this);
         }

      /**
      * OR-combine two masks
      */
      Mask<T>& operator|=(Mask<T> o) noexcept
         {
         m_mask |= o.value();
         return (*this);
         }

      /**
      * AND-combine two masks
      */
      friend Mask<T> operator&(Mask<T> x, Mask<T> y) noexcept
         {
         return Mask<T>(x.value() & y.value());
         }

      /**
      * XOR-combine two masks
      */
      friend Mask<T> operator^(Mask<T> x, Mask<T> y) noexcept
         {
         return Mask<T>(x.value() ^ y.value());
         }

      /**
      * OR-combine two masks
      */
      friend Mask<T> operator|(Mask<T> x, Mask<T> y) noexcept
         {
         return Mask<T>(x.value() | y.value());
         }

      /**
      * Negate this mask
      */
      Mask<T> operator~() const noexcept
         {
         return Mask<T>(~value());
         }

      /**
      * Return x if the mask is set, or otherwise zero
      */
      T if_set_return(T x) const noexcept
         {
         return m_mask & x;
         }

      /**
      * Return x if the mask is cleared, or otherwise zero
      */
      T if_not_set_return(T x) const noexcept
         {
         return ~m_mask & x;
         }

      /**
      * If this mask is set, return x, otherwise return y
      */
      T select(T x, T y) const noexcept
         {
         return ct_masked_merge(value(), x, y);
         }

      T select_and_unpoison(T x, T y) const noexcept
         {
         T r = this->select(x, y);
         CT::unpoison(r);
         return r;
         }

      /**
      * If this mask is set, return x, otherwise return y
      */
      Mask<T> select_mask(Mask<T> x, Mask<T> y) const noexcept
         {
         return Mask<T>(select(x.value(), y.value()));
         }

      /**
      * Conditionally set output to x or y, depending on if mask is set or
      * cleared (resp)
      */
      void select_n(T output[], const T x[], const T y[], nsize_t len) const noexcept
         {
         for(nsize_t i = 0; i != len; ++i)
            output[i] = this->select(x[i], y[i]);
         }

      /**
      * If this mask is set, zero out buf, otherwise do nothing
      */
      void if_set_zero_out(T buf[], nsize_t elems) noexcept
         {
         for(nsize_t i = 0; i != elems; ++i)
            {
            buf[i] = this->if_not_set_return(buf[i]);
            }
         }

      /**
      * Return the value of the mask, unpoisoned
      */
      T unpoisoned_value() const noexcept
         {
         T r = value();
         CT::unpoison(r);
         return r;
         }

      /**
      * Return true iff this mask is set
      */
      bool is_set() const noexcept
         {
         return unpoisoned_value() != 0;
         }

      /**
      * Return the underlying value of the mask
      */
      T value() const noexcept
         {
         return m_mask;
         }

   private:
      Mask(T m) noexcept : m_mask(m) {}

      T m_mask;
   };

template<typename T>
inline Mask<T> conditional_copy_mem(T cnd,
                                    T* to,
                                    const T* from0,
                                    const T* from1,
                                    nsize_t elems) noexcept
   {
   const auto mask = CT::Mask<T>::expand(cnd);
   mask.select_n(to, from0, from1, elems);
   return mask;
   }

template<typename T>
inline void conditional_swap(bool cnd, T& x, T& y) noexcept
   {
   const auto swap = CT::Mask<T>::expand(cnd);

   T t0 = swap.select(y, x);
   T t1 = swap.select(x, y);
   x = t0;
   y = t1;
   }

template<typename T>
inline void conditional_swap_ptr(bool cnd, T& x, T& y) noexcept
   {
   uintptr_t xp = reinterpret_cast<uintptr_t>(x);
   uintptr_t yp = reinterpret_cast<uintptr_t>(y);

   conditional_swap<uintptr_t>(cnd, xp, yp);

   x = reinterpret_cast<T>(xp);
   y = reinterpret_cast<T>(yp);
   }

} // jau::ct

#endif
