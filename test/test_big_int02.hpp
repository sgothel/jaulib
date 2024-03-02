/*
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
#include <thread>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <iostream>

#include <jau/test/catch2_ext.hpp>

#include <jau/int_types.hpp>
#include <jau/mp/big_int_t.hpp>

using namespace jau;
using namespace jau::mp;
using namespace jau::int_literals;

static big_int_t phi(const big_int_t& P, const big_int_t& Q) {
    const big_int_t one(big_int_t::one());
    return (P-one)*(Q-one);
}
/**
 * Returns e with `1 < e < Φ(n)`
 *
 * e must be co-prime to phi and smaller than phi
 */
static big_int_t eval_e(big_int_t e, const big_int_t& phi) {
    const big_int_t one(big_int_t::one());
    while (e < phi && gcd(e, phi) != one ) {
        ++e;
    }
    return e;
}

TEST_CASE( "MP Big Encryption Test 00", "[big_int_t][arithmetic][math]" ) {
    std::cout << "big_int mp_word_bits " << std::to_string( mp_word_bits ) << std::endl;
    // textbook RSA (insecure)
    {
        big_int_t pub_P(53), pub_Q(59), pub_n(pub_P*pub_Q);
        big_int_t sec_phi = phi(pub_P, pub_Q);
        big_int_t pub_e = eval_e(big_int_t(2), sec_phi);
        std::cout << "Public Key:: P " << pub_P << ", Q " << pub_Q << ", n " << pub_n << ", e " << pub_e << std::endl;

        // Private key (d stands for decrypt)
        // choosing d such that it satisfies
        // d*e = 1 + k * totient
        big_int_t sec_k = 2; // an arbitrary constant
        big_int_t sec_d = ( sec_k * sec_phi + 1 ) / pub_e;
        std::cout << "Private Key:: phi " << sec_phi << ", k " << sec_k << ", d " << sec_d << std::endl;

        // big_int_t clear("0x112233445566778899aabbccddeeff0102030405060708090a0b0c0d0e0f");
        big_int_t clear(1122);
        std::cout << "clear:: " << clear.to_hex_string(true) << std::endl;

        big_int_t cipher = clear.mod_pow(pub_e, pub_n);
        std::cout << "encrypted:: " << cipher.to_hex_string(true) << std::endl;

        big_int_t decrypted = cipher.mod_pow(sec_d, pub_n);
        std::cout << "decrypted:: " << decrypted.to_hex_string(true) << std::endl;

        REQUIRE( clear == decrypted );
    }
}
