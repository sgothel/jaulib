/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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
#include <thread>
#include <cassert>
#include <cinttypes>
#include <cstring>

#include <jau/test/catch2_ext.hpp>

#include <jau/basic_types.hpp>

using namespace jau;
using namespace jau::fractions_i64_literals;
using namespace jau::int_literals;

#if 0
static fraction<int64_t> ratio_multiply(const fraction<int64_t>& r1, const fraction<int64_t>& r2) {
    const uint64_t gcd1 = gcd<uint64_t>((uint64_t)abs(r1.num), r2.denom);
    const uint64_t gcd2 = gcd<uint64_t>((uint64_t)abs(r2.num), r1.denom);

    return fraction<int64_t>( ( r1.num / gcd1 ) * ( r2.num / gcd2),
                              ( r1.denom / gcd2 ) * ( r2.denom / gcd1) );
}

static fraction<int64_t> ratio_divide(const fraction<int64_t>& r1, const fraction<int64_t>& r2) {
    return ratio_multiply(r1, fraction<int64_t>(r2.denom, r2.num));
}

static fraction<int64_t> duration_common_type(const fraction<int64_t>& p1, const fraction<int64_t>& p2) {
    const uint64_t gcd_num = gcd<uint64_t>((uint64_t)abs(p1.num), (uint64_t)abs(p2.num));
    const uint64_t gcd_den = gcd<uint64_t>(p1.denom, p2.denom);
    return fraction<int64_t>( gcd_num * jau::sign(p1.num) * jau::sign(p2.num), ( p1.denom / gcd_den ) * p2.denom );
}
#endif


template<typename int_type>
static void test_gcd_fract(const int_type n, const std::make_unsigned_t<int_type> d,
                           const int_type exp_gcd, const int_type exp_num, const std::make_unsigned_t<int_type> exp_denom) {
    {
        const int_type n1 = sign(n) * abs(n);
        const std::make_unsigned_t<int_type> d1 = sign(d) * abs(d);
        REQUIRE( n == n1 );
        REQUIRE( d == d1 );
    }

    const int_type _gcd = gcd( n, static_cast<int_type>(d) );
    REQUIRE( exp_gcd == _gcd );

    fraction<int_type> f1(n, d);
    REQUIRE( exp_num == f1.num );
    REQUIRE( exp_denom == f1.denom);
}

template<typename int_type>
static void test_gcd_fract_pm(const int_type n, const std::make_unsigned_t<int_type> d,
                              const int_type exp_gcd, const int_type exp_num, const std::make_unsigned_t<int_type> exp_denom) {
    test_gcd_fract( n,  d,  exp_gcd,  exp_num,  exp_denom);
    test_gcd_fract(-n,  d,  exp_gcd, -exp_num,  exp_denom);
}

template<typename int_type>
static void test_comp_fract(const fraction<int_type>& a, const fraction<int_type>& b,
                            const fraction<int_type>& exp_max, const fraction<int_type>& exp_min,
                            const fraction<int_type>& exp_sum, const fraction<int_type>& exp_diff,
                            const fraction<int_type>& exp_mul, const fraction<int_type>& exp_div)
{
    const bool show_double = true;
    INFO_STR( "max(a "+a.to_string(show_double)+", b "+b.to_string(show_double)+") = "+max(a,b).to_string(show_double));
    INFO_STR( "min(a "+a.to_string(show_double)+", b "+b.to_string(show_double)+") = "+min(a,b).to_string(show_double));
    INFO_STR( "a "+a.to_string(show_double)+" + b "+b.to_string(show_double)+" = "+(a+b).to_string(show_double));
    INFO_STR( "a "+a.to_string(show_double)+" - b "+b.to_string(show_double)+" = "+(a-b).to_string(show_double));
    INFO_STR( "a "+a.to_string(show_double)+" * b "+b.to_string(show_double)+" = "+(a*b).to_string(show_double));
    INFO_STR( "a "+a.to_string(show_double)+" / b "+b.to_string(show_double)+" = "+(a/b).to_string(show_double));
    {
        const double epsilon = std::numeric_limits<double>::epsilon();
        const double ad = a.to_double();
        const double bd = b.to_double();
        if( std::abs( ad - bd ) <= epsilon ) {
            REQUIRE(   a == b );
            REQUIRE( !(a != b) );
            REQUIRE(   a <= b );
            REQUIRE(   a >= b );
        } else if( std::abs( ad - bd ) > epsilon ) {
            REQUIRE(   a != b );
            REQUIRE( !(a == b) );
            if( ad - bd < -epsilon ) {
                REQUIRE( a <  b );
                REQUIRE( a <= b );
                REQUIRE( b >  a );
                REQUIRE( b >= a );
            } else {
                REQUIRE( a >  b );
                REQUIRE( a >= b );
                REQUIRE( b <  a );
                REQUIRE( b <= a );
            }
        }
    }
    {
        fraction<int_type> has_max = max(a, b);
        fraction<int_type> has_min = min(a, b);
        REQUIRE_MSG( "exp "+to_string(exp_max)+" == has "+to_string(has_max), exp_max == has_max );
        REQUIRE_MSG( "exp "+to_string(exp_min)+" == has "+to_string(has_min), exp_min == has_min );
        REQUIRE( has_max >= has_min );
        REQUIRE( has_min <= has_max );
    }
    {
        const fraction<int_type> has_sum = a + b;
        const double exp_double = a.to_double() + b.to_double();
        const double has_double = has_sum.to_double();
        REQUIRE_MSG( "exp "+std::to_string(exp_double)+" == has "+std::to_string(has_double), abs( exp_double - has_double ) <= std::numeric_limits<double>::epsilon() );
        REQUIRE_MSG( "exp "+to_string(exp_sum)+" == has "+to_string(has_sum), exp_sum == has_sum );
    }
    {
        fraction<int_type> has_diff = a - b;
        const double exp_double = a.to_double() - b.to_double();
        const double has_double = has_diff.to_double();
        REQUIRE_MSG( "exp "+std::to_string(exp_double)+" == has "+std::to_string(has_double), abs( exp_double - has_double ) <= std::numeric_limits<double>::epsilon() );
        REQUIRE_MSG( "exp "+to_string(exp_diff)+" == has "+to_string(has_diff), exp_diff == has_diff );
    }
    {
        fraction<int_type> has_mul = a * b;
        const double exp_double = a.to_double() * b.to_double();
        const double has_double = has_mul.to_double();
        REQUIRE_MSG( "exp "+std::to_string(exp_double)+" == has "+std::to_string(has_double), abs( exp_double - has_double ) <= std::numeric_limits<double>::epsilon() );
        REQUIRE_MSG( "exp "+to_string(exp_mul)+" == has "+to_string(has_mul), exp_mul == has_mul);
    }
    {
        fraction<int_type> has_div = a / b;
        const double exp_double = a.to_double() / b.to_double();
        const double has_double = has_div.to_double();
        REQUIRE_MSG( "exp "+std::to_string(exp_double)+" == has "+std::to_string(has_double), abs( exp_double - has_double ) <= std::numeric_limits<double>::epsilon() );
        REQUIRE_MSG( "exp "+to_string(exp_div)+" == has "+to_string(has_div), exp_div == has_div);
    }
    {
        const fraction<int_type> step(1, a.denom);
        const fraction<int_type> lim(a + ( step * (int_type)10 ));
        fraction<int_type> i(a);
        int count;
        for(count = 0; i < lim; i+=step, ++count) { }
        REQUIRE( i == lim );
        REQUIRE( i > a );
        REQUIRE( 10 == count );

        i+=step;
        REQUIRE( i > lim );
        REQUIRE( i == lim + step );
    }
    if( !std::is_unsigned_v<int_type> ) {
        const fraction<int_type> step(1, a.denom);
        const fraction<int_type> lim(a - ( step * (int_type)10 ));
        fraction<int_type> i(a);
        int count;
        for(count = 0; i > lim; i-=step, ++count) { }
        REQUIRE( i == lim );
        REQUIRE( i < a );
        REQUIRE( 10 == count );

        i-=step;
        REQUIRE( i < lim );
        REQUIRE( i == lim - step );
    }
}

template<typename Rep, typename Period, typename int_type>
static void test_duration(const fraction<int_type>& a, const std::chrono::duration<Rep, Period>& dur_ref,
                          const Rep exp_count)
{
    INFO_STR( " given duration: ( " + std::to_string(dur_ref.count()) + " * " + std::to_string(Period::num) + " = " + std::to_string(dur_ref.count() * Period::num) + " ) / " + std::to_string(Period::den) );
    {
        const int64_t d_num = a.to_num_of( fraction_i64(Period::num, Period::den) );
        std::chrono::duration<Rep, Period> d = a.to_duration( dur_ref );
        INFO_STR( " fraction-1 " + a.to_string(true) + " -> duration_count " + std::to_string( d_num ) + ", duration " + std::to_string( d.count() ) );
        INFO_STR( " resulting duration-1: ( " + std::to_string(d.count()) + " * " + std::to_string(Period::num) + " = " + std::to_string(d.count() * Period::num) + " ) / " + std::to_string(Period::den) );

        // fully functional conversion check
        fraction<int_type> b(d);
        INFO_STR( " reconverted fraction-2 " + b.to_string(true));
        REQUIRE( exp_count == d_num );
        REQUIRE( exp_count == d.count() );
        REQUIRE( a == b );
    }
#if 0
    {
        typename std::chrono::duration<Rep, Period>::rep d_count = a.to_duration_count( dur_ref );
        auto d = a.to_auto_duration();
        INFO_STR( " fraction-2 " + a.to_string(true) + " -> duration_count " + std::to_string( d_count ) + ", duration " + std::to_string( d.count() ) );
        INFO_STR( " resulting duration-2: ( " + std::to_string(d.count()) + " * " + std::to_string(Period::num) + " = " + std::to_string(d.count() * Period::num) + " ) / " + std::to_string(Period::den) );
        INFO_STR( " resulting duration-2: ( " + std::to_string(d.count()) + " * " + std::to_string(decltype(d)::num) + " = " + std::to_string(d.count() * decltype(d)::num) + " ) / " + std::to_string(decltype(d)::den) );

        // fully functional conversion check
        fraction<int_type> b(d);
        INFO_STR( " reconverted fraction-2 " + b.to_string(true));
        REQUIRE( exp_count == d_count );
        REQUIRE( exp_count == d.count() );
        REQUIRE( a == b );
    }
#endif
}


TEST_CASE( "Fraction Types Test 00", "[fraction][type]" ) {
    {
        INFO_STR(" is_trivially_copyable_v<fraction_i64>:    " + std::to_string(std::is_trivially_copyable_v<fraction_i64>));
        INFO_STR(" is_trivially_copyable<fraction_timespec>: " + std::to_string(std::is_trivially_copyable_v<fraction_timespec>));
        REQUIRE( true == std::is_trivially_copyable_v<fraction_i64> );
        REQUIRE( true == std::is_trivially_copyable_v<fraction_timespec> );
        sc_atomic_fraction_i64 check_type_01 = fractions_i64::seconds;
        (void)check_type_01;
    }
    {
        INFO_STR(" is_trivially_copyable_v<fraction_i64>:    " + std::to_string(std::is_trivially_copyable_v<fraction_i64>));
        REQUIRE( true == std::is_trivially_copyable_v<fraction_i64> );
        sc_atomic_fraction_i64 check_type_01 = fractions_i64::seconds;
        (void)check_type_01;
    }
    {
        REQUIRE( INT64_MAX == std::numeric_limits<int64_t>::max() );
        REQUIRE( INT64_MIN == std::numeric_limits<int64_t>::min() );

        REQUIRE( UINT64_MAX == std::numeric_limits<uint64_t>::max() );
    }
    {
        // copy-ctor
        fraction<int> a(1, 6);
        fraction<int> b(a);
        REQUIRE( a == b );
    }
    {
        // move-ctor
        fraction<int> a(1, 6);
        fraction<int> b( std::move(a) );
        REQUIRE( a == b );
    }
    {
        // assignment
        fraction<int> a(1, 6);
        fraction<int> b(6, 1);
        b = a;
        REQUIRE( a == b );
    }
    {
        // move-assignment
        fraction<int> a(1, 6);
        fraction<int> a2(a);
        fraction<int> b(6, 1);
        b = std::move(a2);
        REQUIRE( a == b );
    }
    {
        REQUIRE( fractions_i64::zero == 0_s );
        REQUIRE( fractions_i64::zero == 0_one );

        REQUIRE( fractions_i64::one == 1_one );
        REQUIRE( fractions_i64::one == 1_s );

        REQUIRE( 3_i64*fractions_i64::tera == 3_T );
        REQUIRE( 3_i64*fractions_i64::giga == 3_G );
        REQUIRE( 3_i64*fractions_i64::mega == 3_M );
        REQUIRE( 3_i64*fractions_i64::kilo == 3_k );
        REQUIRE( 3_i64*fractions_i64::one == 3_one );
        REQUIRE( 3_i64*fractions_i64::milli == 3_m );
        REQUIRE( 3_i64*fractions_i64::micro == 3_u );
        REQUIRE( 3_i64*fractions_i64::nano == 3_n );
        REQUIRE( 3_i64*fractions_i64::pico == 3_p );

        REQUIRE( 3_i64*fractions_i64::days == 3_d );
        REQUIRE( 3_i64*fractions_i64::hours == 3_h );
        REQUIRE( 3_i64*fractions_i64::minutes == 3_min );
        REQUIRE( 3_i64*fractions_i64::seconds == 3_s );
        REQUIRE( 3_i64*fractions_i64::milli == 3_ms );
        REQUIRE( 3_i64*fractions_i64::micro == 3_us );
        REQUIRE( 3_i64*fractions_i64::nano == 3_ns );
    }
}

TEST_CASE( "Fraction GCD and Modulo Test 00", "[integer][fraction][gcd]" ) {
    test_gcd_fract<int>(0, 0, 0, 0, 1);
    test_gcd_fract<uint32_t>(0, 0, 0, 0, 1);

    test_gcd_fract_pm<int>(15, 5, 5,  3, 1);
    test_gcd_fract_pm<int>(17, 5, 1, 17, 5);

    test_gcd_fract<uint32_t>(15, 5, 5,  3, 1);
    test_gcd_fract<uint32_t>(17, 5, 1, 17, 5);
}

static void test_to_num_of(const int64_t exp, const fraction<int64_t>& v, const fraction<int64_t>& new_base, bool exp_overflow=false) noexcept {
    bool overflow = false;
    int64_t rr = v.to_num_of(new_base, &overflow);
    INFO_STR(" value " + v.to_string() );
    INFO_STR(" new_base " + new_base.to_string() );
    std::string rr_s = exp == rr ? " - OK " : " - ********* ERROR ";
    INFO_STR(" rr " + std::to_string(rr) + " =?= " + std::to_string(exp) + rr_s + ", overflow[exp " + std::to_string(exp_overflow) + ", has " + std::to_string(overflow) + "]");
    if constexpr ( false ) {
        // leaving elaboration code in .. for future testing overflows
        const uint64_t _gcd = gcd<uint64_t>( v.denom, new_base.denom );
        const uint64_t _lcm = ( v.denom * new_base.denom ) / _gcd;
        int64_t r0 = ( v.num * (int64_t)( _lcm / v.denom ) ) / new_base.num;
        int64_t r2 = ( v.num * (int64_t)new_base.denom ) / ( (int64_t) v.denom ) / new_base.num;
        INFO_STR(" gcd " + std::to_string(_gcd) );
        INFO_STR(" lcm " + std::to_string(_lcm) );
        INFO_STR(" lcm / v_denom " + std::to_string( _lcm / v.denom) );
        INFO_STR(" v_num * nb_denum " + std::to_string( v.num * (int64_t)new_base.denom ) );
        std::string r0_s = exp == r0 ? " - OK " : " - ********* ERROR ";
        INFO_STR(" r0 " + std::to_string(r0) + " =?= " + std::to_string(exp) + r0_s);
        std::string r2_s = exp == r2 ? " - OK " : " - ********* ERROR ";
        INFO_STR(" r2 " + std::to_string(r2) + " =?= " + std::to_string(exp) + r2_s);
    }
    REQUIRE( exp_overflow == overflow );
    if( !exp_overflow ) {
        REQUIRE( exp == rr );
    }
}

TEST_CASE( "Fraction Cast Test 01.1", "[integer][fraction][type][to_num_of]" ) {
    {
        test_to_num_of(    2_i64, fractions_i64::one, fraction_i64(1_i64, 2_u64) ); // one -> halves
        test_to_num_of( 1000_i64, fractions_i64::milli, fractions_i64::micro );
        test_to_num_of(   60_i64, fractions_i64::minutes, fractions_i64::seconds );
        test_to_num_of(  120_i64, fractions_i64::hours * 2_i64, fractions_i64::minutes );
        test_to_num_of(   48_i64, 2_i64 * fractions_i64::days, fractions_i64::hours );
    }
    {
        fraction_i64 a ( 10_s + 400_ms );
        fraction_i64 b (  0_s + 400_ms );
        fraction_i64 exp_sum (  10_s + 800_ms );

        test_to_num_of(  10_i64, a, fractions_i64::seconds );
        test_to_num_of( 10'400'000'000_i64, a, fractions_i64::nano );
        test_to_num_of(   0_i64, b, fractions_i64::seconds );
        test_to_num_of(    400'000'000_i64, b, fractions_i64::nano );
        test_to_num_of(             10_i64, exp_sum, fractions_i64::seconds );
        test_to_num_of( 10'800'000'000_i64, exp_sum, fractions_i64::nano );
    }
    {
        fraction_i64 n1 ( 999'999'999_ns );
        fraction_i64 n2 ( 999'999'999_ns );
        fraction_i64 exp_nsum ( 1'999'999'998_ns );
        REQUIRE( exp_nsum == n1 + n2 );
        test_to_num_of(   999'999'999_i64, n1, fractions_i64::nano );
        test_to_num_of( 1'999'999'998_i64, exp_nsum, fractions_i64::nano );
        test_to_num_of(   999'999_i64, n1, fractions_i64::micro );
        test_to_num_of( 1'999'999_i64, exp_nsum, fractions_i64::micro );
        // OVERFLOW
        test_to_num_of(   999'999'999'000_i64, n1, fractions_i64::pico, true /* overflow */ );
        test_to_num_of( 1'999'999'998'000_i64, exp_nsum, fractions_i64::pico, true /* overflow */ );
    }
    { // OVERFLOW
        // 1'000'000'000'000
        //   999'999'999'999
        // 1'999'999'999'998
        fraction_i64 p1 ( 999'999'999'999_p );
        // fraction_i64 p2 ( 999'999'999'999_p );
        fraction_i64 exp_sum ( 1'999'999'999'998_p );
        test_to_num_of(   999'999'999_i64, p1, fractions_i64::pico, true /* overflow */ );
        test_to_num_of( 1'999'999'999'998_i64, exp_sum, fractions_i64::pico, true /* overflow */);
        // REQUIRE( exp_sum == p1 + p2 );
    }
}

TEST_CASE( "Fraction String Test 01.2", "[integer][fraction][type][string]" ) {
    {
        fraction_i64 a;
        fraction_i64 exp = 10_s;

        REQUIRE( true  == to_fraction_i64(a, "10/1", 0_s, 365_d) );
        REQUIRE( exp == a );
        {
            fraction_i64 b;
            REQUIRE( true  == to_fraction_i64(b, a.to_string(), a, a) );
            REQUIRE( exp == b );
        }

        REQUIRE( true  == to_fraction_i64(a, "10/1", 10_s, 10_s) );
        REQUIRE( exp == a );
        {
            fraction_i64 b;
            REQUIRE( true  == to_fraction_i64(b, a.to_string(), a, a) );
            REQUIRE( exp == b );
        }

        REQUIRE( false == to_fraction_i64(a, "10/1",  100_ns, 9_s) );
        REQUIRE( false == to_fraction_i64(a, "10/1",  11_s, 365_d) );
    }
    {
        fraction_i64 a;
        REQUIRE( true  == to_fraction_i64(a, " 10 / 1000000 ", 0_s, 365_d) );
        REQUIRE( 10_us == a );
        {
            fraction_i64 b;
            REQUIRE( true  == to_fraction_i64(b, a.to_string(), a, a) );
            REQUIRE( 10_us == b );
        }

        REQUIRE( false == to_fraction_i64(a, " 10x / 1000000 ", 0_s, 365_d) );
        REQUIRE( false == to_fraction_i64(a, " 10 / 1000000x ", 0_s, 365_d) );
        REQUIRE( false == to_fraction_i64(a, " 10 % 1000000x ", 0_s, 365_d) );
        REQUIRE( false == to_fraction_i64(a, " 10 ", 0_s, 365_d) );
    }
}

TEST_CASE( "Fraction Arithmetic Test 02", "[integer][fraction]" ) {
    {
        fraction<int> b(1, 6);
        REQUIRE( b == fraction<int>(2, 12U));
    }
    {
        fraction<int> b(6, 1);
        REQUIRE( b == fraction<int>(12, 2U));
    }
    {
        fraction<int> a(1, 4), b(1, 6);
        fraction<int> exp_sum(5, 12);
        fraction<int> exp_diff(1, 12);
        fraction<int> exp_mul(1, 24);
        fraction<int> exp_div(3, 2);
        test_comp_fract(a, b, a, b, exp_sum, exp_diff, exp_mul, exp_div);
    }
    {
        fraction<int> a(1, 4), b(6, 1);
        fraction<int> exp_sum(25, 4);
        fraction<int> exp_diff(-23, 4);
        fraction<int> exp_mul(3, 2);
        fraction<int> exp_div(1, 24);
        test_comp_fract(a, b, b, a, exp_sum, exp_diff, exp_mul, exp_div);
    }
    {
        fraction<int64_t> a(-1, 4), b(-1, 6);
        fraction<int64_t> exp_sum(-5, 12);
        fraction<int64_t> exp_diff(-1, 12);
        fraction<int64_t> exp_mul(1, 24);
        fraction<int64_t> exp_div(3, 2);
        test_comp_fract(a, b, b, a, exp_sum, exp_diff, exp_mul, exp_div);
    }
    {
        fraction<int> a(-1, 4), b(-1, 6);
        fraction<int> exp_sum(-5, 12);
        fraction<int> exp_diff(-1, 12);
        fraction<int> exp_mul(1, 24);
        fraction<int> exp_div(3, 2);
        test_comp_fract(a, b, b, a, exp_sum, exp_diff, exp_mul, exp_div);
    }
    {
        fraction<int> a(-1, 4), b( 1, 6);
        fraction<int> exp_sum(-1, 12);
        fraction<int> exp_diff(-5, 12);
        fraction<int> exp_mul(-1, 24);
        fraction<int> exp_div(-3, 2);
        test_comp_fract(a, b, b, a, exp_sum, exp_diff, exp_mul, exp_div);
    }
    {
        fraction<int> a(1, 4), b(-1, 6);
        fraction<int> exp_sum(1, 12);
        fraction<int> exp_diff(5, 12);
        fraction<int> exp_mul(-1, 24);
        fraction<int> exp_div(-3, 2);
        test_comp_fract(a, b, a, b, exp_sum, exp_diff, exp_mul, exp_div);
    }
    {
        // unsigned: micro + nano
        fraction_u64 a(1, 1'000_u64), b(1, 1'000'000'000_u64);
        fraction_u64 exp_sum(  1000001_u64,      100'0000'000_u64);
        fraction_u64 exp_diff(  999999_u64,     1'000'000'000_u64);
        fraction_u64 exp_mul(        1_u64, 1'000'000'000'000_u64);
        fraction_u64 exp_div(  1000000_u64,                 1_u64);
        test_comp_fract(a, b, a, b, exp_sum, exp_diff, exp_mul, exp_div);
    }
    {
        // signed: micro + nano
        fraction_i64 a(1_m), b(1_n);
        fraction_i64 exp_sum(  1000001_n);
        fraction_i64 exp_diff(  999999_n);
        fraction_i64 exp_mul(        1_p);
        fraction_i64 exp_div(  1000000_one);
        test_comp_fract(a, b, a, b, exp_sum, exp_diff, exp_mul, exp_div);
    }
    {
        // signed: 100 micro + 1'000'000 nano
        fraction_i64 a(100_i64*fractions_i64::milli), b(1'000'000_i64*fractions_i64::nano);
        fraction_i64 exp_sum(      101_i64,   1'000_u64);
        fraction_i64 exp_diff(      99_i64,   1'000_u64);
        fraction_i64 exp_mul(        1_i64,  10'000_u64);
        fraction_i64 exp_div(      100_i64,       1_u64);
        test_comp_fract(a, b, a, b, exp_sum, exp_diff, exp_mul, exp_div);
    }
    {
        const std::chrono::milliseconds::rep exp_count = 100;
        const fraction<int64_t> a(static_cast<int64_t>(exp_count) * fractions_i64::milli);
        test_duration(a, std::chrono::milliseconds::zero(), exp_count);
    }
    {
        const std::chrono::nanoseconds::rep exp_count = -50;
        const fraction_i64 a(static_cast<int64_t>(exp_count) * fractions_i64::nano);
        test_duration(a, std::chrono::nanoseconds::zero(), exp_count);
    }
    {
        const std::chrono::hours::rep exp_count = 24;
        const fraction_i64 a(static_cast<int64_t>(exp_count) * fractions_i64::hours);
        test_duration(a, std::chrono::hours::zero(), exp_count);
    }
    {
        const fraction_i64 refresh_rate = 60_i64/1_s;
        const fraction_i64 fps = 1_i64 / refresh_rate;
        REQUIRE( 1_i64 / fps == refresh_rate );
        REQUIRE( fps == 1_i64 / refresh_rate );
        REQUIRE( 2_i64 * fps == 1_i64 / ( refresh_rate / 2_i64 ) );

        REQUIRE( fractions_i64::milli / 2_i64 ==   500_i64 * fractions_i64::micro );
        REQUIRE( 1_m / 2_one                  ==   500_one * 1_u );
        REQUIRE(    1_i64 / fractions_i64::milli == fractions_i64::kilo );
        REQUIRE( fractions_i64::milli/-1000_i64 == -fractions_i64::micro );
        REQUIRE(  500_i64 * fractions_i64::milli == fractions_i64::seconds/2_i64 );
        REQUIRE( 1000_ms == fractions_i64::seconds );
        REQUIRE(  1_i64 * fractions_i64::seconds == 60_i64/fractions_i64::minutes );
        REQUIRE( 60_s == fractions_i64::minutes );
        REQUIRE( 60_s                         == 1_min );
        REQUIRE( 60_i64 * fractions_i64::minutes == fractions_i64::hours );
        REQUIRE( 24_i64 * fractions_i64::hours   == fractions_i64::days );
        REQUIRE( 24_h                         == 1_d );

        const fraction_i64 m(720_i64 * fractions_i64::minutes); // 12 hours
        const fraction_i64 h( 48_i64 * fractions_i64::hours);
        const fraction_i64 d(  2_i64 * fractions_i64::days);
        const fraction_i64 t = m + h + d;
        REQUIRE( m == h/4_i64 );
        REQUIRE( h == d );
        REQUIRE( t > 4_i64 * fractions_i64::days );
    }
    {
        fraction_i64 a ( 1'000l,             1lu ); // 1'000s
        fraction_i64 b ( 1'000l, 1'000'000'000lu ); // 1'000ns
        REQUIRE( 1000_s  == a );
        REQUIRE( 1000_ns == b );
        fraction_i64 c = a + b;
        fraction_i64 exp_c ( 1'000'000'000lu + 1lu, 1'000'000lu ); // 1'000ns
        REQUIRE( exp_c == c );
    }
}

/**
 * Resembling the GNU/Linux bits/types.h,
 * documenting whether time_t is 32-bit (arm-32) or 64-bit (arm-64, x86_64, ..).
 */
static int sizeof_time_t() {
/* X32 kernel interface is 64-bit.  */
#if defined __x86_64__ && defined __ILP32__
    // 64 bit size
    #if __WORDSIZE == 32
        return sizeof( __int64_t );
    #else
        return sizeof( long int );
    #endif
#else
    // 32 bit or 64 bit
    return sizeof( long int );
#endif
}

/**
 * Resembling the GNU/Linux bits/types.h,
 * documenting whether tv_nsec of struct timespec is 32-bit (arm-32) or 64-bit (arm-64, x86_64, ..).
 */
static int sizeof_tv_nsec() {
#if __WORDSIZE == 64 \
  || (defined __SYSCALL_WORDSIZE && __SYSCALL_WORDSIZE == 64) \
  || __TIMESIZE == 32
    // 32 bit or 64 bit: __syscall_slong_t
    return sizeof( int64_t );
#else
    // 32 bit or 64 bit
    return sizeof( long int );
#endif
}

TEST_CASE( "struct timespec type validation Test 03.00", "[fraction][struct_timespec][time]" ) {
    // testing fraction_timespec::to_timespec()
    {
        using time_t_type = decltype(timespec::tv_sec);
        INFO_STR(" tv_sec: sizeof=" + std::to_string( sizeof( time_t_type ) ) + ", signed " + std::to_string( std::is_signed_v<time_t_type>) );
        CHECK( sizeof_time_t() == sizeof( time_t_type ) );
        CHECK( true == std::is_signed_v<time_t_type> );

        using ns_type = decltype(timespec::tv_nsec);
        INFO_STR(" tv_nsec: sizeof=" + std::to_string( sizeof( ns_type ) ) + ", signed " + std::to_string( std::is_signed_v<ns_type>) );
        CHECK( sizeof_tv_nsec() == sizeof( ns_type ) );
        CHECK( true == std::is_signed_v<ns_type> );
    }
}


TEST_CASE( "Fraction Time Arithmetic Add Test 03.1", "[fraction][fraction_timespec][add]" ) {
    const int64_t ns_per_sec = 1'000'000'000_i64;

    // 12.4 + 12.4 = 24.8 w/ double overflow in tv_nsec
    {
        fraction_timespec a ( 10_i64, 2 * ns_per_sec + 400000000_i64 );
        fraction_timespec b ( 10_i64, 2 * ns_per_sec + 400000000_i64 );
        fraction_timespec exp_sum (  24_i64, 800000000_i64 );
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" a+b " + (a+b).to_string() );
        REQUIRE( ( a + b ) == exp_sum );
    }
    // 13.4 - 3.4 = 10.0 w/ double overflow in tv_nsec
    {
        fraction_timespec a ( 13_i64,                  400000000_i64 );
        fraction_timespec b (  1_i64, 2 * ns_per_sec + 400000000_i64 );
        fraction_timespec exp_sum (  10_i64, 0_i64 );
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" a-b " + (a-b).to_string() );
        REQUIRE( ( a - b ) == exp_sum );
    }
    // 12.0 - 1.9 = 10.1 w/ double overflow in tv_nsec
    {
        fraction_timespec a ( 12_i64,                           0_i64 );
        fraction_timespec b (  3_i64, -2 * ns_per_sec + 900000000_i64 );
        fraction_timespec exp_sum (  10_i64, 100000000_i64 );
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" a-b " + (a-b).to_string() );
        REQUIRE( ( a - b ) == exp_sum );
    }
    // 10.4 + 0.4 = 10.8
    {
        fraction_timespec a ( 10_i64, 400000000_i64 );
        fraction_timespec b (  0_i64, 400000000_i64 );
        fraction_timespec exp_sum (  10_i64, 800000000_i64 );
        REQUIRE( ( a + b ) == exp_sum );
    }
    // 10.4 + 0.4 = 10.8
    {
        fraction_i64 a ( 10_s + 400_ms );
        fraction_i64 b (  0_s + 400_ms );
        fraction_i64 exp_sum (  10_s + 800_ms );
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" a+b " + (a+b).to_string() );
        REQUIRE( ( a + b ) == exp_sum );
    }
    // 10.4 + 0.4 = 10.8
    {
        fraction_timespec a ( 10_s + 400_ms );
        fraction_timespec b (  0_s + 400_ms );
        fraction_timespec exp_sum (  10_s + 800_ms );
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" a+b " + (a+b).to_string() );
        REQUIRE( ( a + b ) == exp_sum );
    }
    // 10.4 + 0.7 = 11.1
    {
        fraction_timespec a { 10_s + 400_ms };
        fraction_timespec b {  0_s + 700_ms };
        fraction_timespec exp_sum {  11_s + 100_ms };
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" a+b " + (a+b).to_string() );
        REQUIRE( ( a + b ) == exp_sum );
    }
    // 10.4 + 2.7 (in denominator) = 13.1
    {
        fraction_timespec a { 10_s +  400_ms };
        fraction_timespec b {  0_s + 2700_ms };
        fraction_timespec exp_sum {  13_s + 100_ms };
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" a+b " + (a+b).to_string() );
        REQUIRE( ( a + b ) == exp_sum );
    }
    // 10.4 + -0.3 = 10.1
    {
        fraction_timespec a { 10_s + 400_ms };
        fraction_timespec b {  0_s + -300_ms };
        fraction_timespec exp_sum {  10_s + 100_ms };
        REQUIRE( ( a + b ) == exp_sum );
    }
    // 10.-3 + 0.4 = 10.1
    {
        fraction_timespec a { 10_s + -300_ms };
        fraction_timespec b {  0_s + 400_ms };
        fraction_timespec exp_sum {  10_s + 100_ms };
        REQUIRE( ( a + b ) == exp_sum );
    }
    // 10.4 + -0.9 = 9.5
    {
        fraction_timespec a { 10_s + 400_ms };
        fraction_timespec b {  0_s + -900_ms };
        fraction_timespec exp_sum {  9_s + 500_ms };
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" a+b " + (a+b).to_string() );
        REQUIRE( ( a + b ) == exp_sum );
    }
    // 10.4 + -2.7 = 7.7
    {
        fraction_timespec a { 10_s +   400_ms };
        fraction_timespec b {  0_s + -2700_ms };
        fraction_timespec exp_sum {  7_s + 700_ms };
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" a+b " + (a+b).to_string() );
        REQUIRE( ( a + b ) == exp_sum );
    }
    // 10.-9 + 0.4 = 9.5
    {
        fraction_timespec a { 10_s + -900_ms };
        fraction_timespec b {  0_s +  400_ms };
        fraction_timespec exp_sum {  9_s + 500_ms };
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" a+b " + (a+b).to_string() );
        REQUIRE( ( a + b ) == exp_sum );
    }
}

TEST_CASE( "Fraction Time Arithmetic Sub Test 03.2", "[fraction][fraction_timespec][sub]" ) {
    // normalize tests
    // normalize: 1 s + 4*1000000000 ns = 5s
    {
        fraction_timespec a( 1, 4000000000_i64 );
        INFO_STR(" a " + a.to_string() );
        REQUIRE( a.tv_sec == 5 );
        REQUIRE( a.tv_nsec == 0_i64 );
    }
    // normalize: -1 s - 4*1000000000 ns = -5s
    {
        fraction_timespec a( -1, -4000000000_i64 );
        INFO_STR(" a " + a.to_string() );
        REQUIRE( a.tv_sec == -5 );
        REQUIRE( a.tv_nsec == 0_i64 );
    }
    // normalize: -1 s + 4*1000000000 ns = 3s
    {
        fraction_timespec a( -1, 4000000000_i64 );
        INFO_STR(" a " + a.to_string() );
        REQUIRE( a.tv_sec == 3 );
        REQUIRE( a.tv_nsec == 0_i64 );
    }
    // normalize: 1 - 0.4 = 0.6
    {
        fraction_timespec a( 1, -400000000_i64 );
        INFO_STR(" a " + a.to_string() );
        REQUIRE( a.tv_sec == 0 );
        REQUIRE( a.tv_nsec == 600000000_i64 );
    }
    // normalize: -1 + 0.4 = -0.6
    {
        fraction_timespec a( -1, +400000000_i64 );
        INFO_STR(" a " + a.to_string() );
        REQUIRE( a.tv_sec == 0 );
        REQUIRE( a.tv_nsec == -600000000_i64 );
    }
    // 674.0 - 675.547 = -1.547
    {
        fraction_timespec a( 674, 0 );
        fraction_timespec b( 675, 547000000_i64 );
        fraction_timespec exp_sum( -1, -547000000_i64 );
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" exp " + exp_sum.to_string() );
        INFO_STR(" a-b " + (a-b).to_string() );
        REQUIRE( ( a - b ) == exp_sum );
    }
    // 674.0 - 675.547 = -1.547
    {
        fraction_timespec a { 674_s + 0_ms };
        fraction_timespec b { 675_s + 547_ms };
        fraction_timespec exp_sum {  -1_s - 547_ms };
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" exp " + exp_sum.to_string() );
        INFO_STR(" a-b " + (a-b).to_string() );
        REQUIRE( ( a - b ) == exp_sum );
    }
    // 10.4 - 0.3 = 10.1
    {
        fraction_timespec a ( 10_i64, 400000000_i64 );
        fraction_timespec b (  0_i64, 300000000_i64 );
        fraction_timespec exp_sum (  10_i64, 100000000_i64 );
        REQUIRE( ( a - b ) == exp_sum );
    }
    // 10.4 - 0.3 = 10.1
    {
        fraction_timespec a ( 10_s + 400_ms );
        fraction_timespec b (  0_s + 300_ms );
        fraction_timespec exp_sum (  10_s + 100_ms );
        REQUIRE( ( a - b ) == exp_sum );
    }
    // 10.4 - 0.7 = 9.7
    {
        fraction_timespec a { 10_s + 400_ms };
        fraction_timespec b {  0_s + 700_ms };
        fraction_timespec exp_sum {  9_s + 700_ms };
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" a-b " + (a-b).to_string() );
        REQUIRE( ( a - b ) == exp_sum );
    }
    // 10.4 - 2.7 (in denominator) = 7.7
    {
        fraction_timespec a { 10_s +  400_ms };
        fraction_timespec b {  0_s + 2700_ms };
        fraction_timespec exp_sum {  7_s + 700_ms };
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" a-b " + (a-b).to_string() );
        REQUIRE( ( a - b ) == exp_sum );
    }

    // 10.4 - -0.3 = 10.7
    {
        fraction_timespec a { 10_s + 400_ms };
        fraction_timespec b {  0_s + -300_ms };
        fraction_timespec exp_sum {  10_s + 700_ms };
        REQUIRE( ( a - b ) == exp_sum );
    }
    // 10.-2 - 0.4 = 9.4
    {
        fraction_timespec a { 10_s + -200_ms };
        fraction_timespec b {  0_s + 400_ms };
        fraction_timespec exp_sum {  9_s + 400_ms };
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" a-b " + (a-b).to_string() );
        REQUIRE( ( a - b ) == exp_sum );
    }
    // 10.4 - -0.9 = 11.3
    {
        fraction_timespec a { 10_s + 400_ms };
        fraction_timespec b {  0_s + -900_ms };
        fraction_timespec exp_sum { 11_s + 300_ms };
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" a-b " + (a-b).to_string() );
        REQUIRE( ( a - b ) == exp_sum );
    }
    // 10.-9 - 0.4 = 8.7
    {
        fraction_timespec a { 10_s + -900_ms };
        fraction_timespec b {  0_s +  400_ms };
        fraction_timespec exp_sum {  8_s + 700_ms };
        INFO_STR(" a " + a.to_string() );
        INFO_STR(" b " + b.to_string() );
        INFO_STR(" a-b " + (a-b).to_string() );
        REQUIRE( ( a - b ) == exp_sum );
    }
}

TEST_CASE( "Fraction Time Measurement Test 04.01", "[fraction][fraction_timespec][time]" ) {
    const int64_t sleep_ms = 50;
    //
    // Ideally we assume accuracy of at least 1/2 millisecond, hence the difference shall not be greater
    //    const fraction_i64 accuracy = fractions_i64::milli*2_i64/3_i64;
    // However, running within virtual machines etc, we have to be more generous here: 60_ms
    // Detected using KVM on GNU/Linux host for FreeBSD 13.1 target
    const fraction_i64 accuracy = fractions_i64::milli*60_i64;
    //
    {
        const fraction_timespec t0 = getMonotonicTime();
        sleep_for( sleep_ms * 1_ms );
        const fraction_timespec t1 = getMonotonicTime();
        const fraction_timespec td_1 = t1 - t0;
        const fraction_i64 td_2 = td_1.to_fraction_i64();
        const fraction_i64 terr = abs( td_2 - fractions_i64::milli * sleep_ms );
        INFO_STR( " Test-1: sleep_for() getMonotonicTime:");
        INFO_STR( " - t0 " + t0.to_string() );
        INFO_STR( " - t1 " + t1.to_string() );
        INFO_STR( " - td_1 " + td_1.to_string() );
        INFO_STR( " - td_2 " + td_2.to_string(true) + ", " + std::to_string( td_2.to_num_of(1_ms) ) + "ms, err " + terr.to_string(true) + " <?= " + accuracy.to_string(true) );
        REQUIRE( t0.tv_sec >= 0 );
        REQUIRE( t0.tv_nsec >= 0 );
        REQUIRE( t1.tv_sec >= 0 );
        REQUIRE( t1.tv_nsec >= 0 );
        REQUIRE( td_1.tv_sec >= 0 );
        REQUIRE( td_1.tv_nsec >= 0 );
        REQUIRE( td_2 >= fractions_i64::zero );
        // Check accuracy
        REQUIRE( terr <= accuracy );
    }
    {
        const fraction_timespec t0 = getMonotonicTime();
        sleep_until( t0 + fraction_timespec( sleep_ms * 1_ms ) );
        const fraction_timespec t1 = getMonotonicTime();
        const fraction_timespec td_1 = t1 - t0;
        const fraction_i64 td_2 = td_1.to_fraction_i64();
        const fraction_i64 terr = abs( td_2 - fractions_i64::milli * sleep_ms );
        INFO_STR( " Test-2: sleep_until() getMonotonicTime:");
        INFO_STR( " - t0 " + t0.to_string() );
        INFO_STR( " - t1 " + t1.to_string() );
        INFO_STR( " - td_1 " + td_1.to_string() );
        INFO_STR( " - td_2 " + td_2.to_string(true) + ", " + std::to_string( td_2.to_num_of(1_ms) ) + "ms, err " + terr.to_string(true) + " <?= " + accuracy.to_string(true) );
        REQUIRE( t0.tv_sec >= 0 );
        REQUIRE( t0.tv_nsec >= 0 );
        REQUIRE( t1.tv_sec >= 0 );
        REQUIRE( t1.tv_nsec >= 0 );
        REQUIRE( td_1.tv_sec >= 0 );
        REQUIRE( td_1.tv_nsec >= 0 );
        REQUIRE( td_2 >= fractions_i64::zero );
        // Check accuracy
        REQUIRE( terr <= accuracy );
    }
}
