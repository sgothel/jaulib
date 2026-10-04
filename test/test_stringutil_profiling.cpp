/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2024-2026 Gothel Software e.K.
 *
 * ***
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this
 * file, You can obtain one at https://opensource.org/license/mit/.
 *
 */
#include <sys/types.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string_view>
#include "jau/debug.hpp"

#include <jau/basic_types.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/cpp_pragma.hpp>
#include <jau/float_types.hpp>
#include <jau/int_types.hpp>
#include <jau/string_cfmt.hpp>
#include <jau/string_literal.hpp>
#include <jau/string_util.hpp>
#include <jau/type_concepts.hpp>

#ifdef HAS_STD_FORMAT
    #include <format>
#endif

using namespace std::literals;

using namespace jau::float_literals;

using namespace jau::int_literals;

CXX_NO_INLINE
constexpr bool my_append_string_std(std::string &s, size_t max_len, const char *vbegin, const char *vend) noexcept { // NOLINT
    try {
        const size_t osz = s.size();
        s.append(vbegin, jau::min(max_len > osz ? max_len - osz : 0, size_t(vend-vbegin)));
    } catch (...) {
        jau::fput_exception(stderr, std::current_exception(), E_FILE_LINE);
        return false;
    }
    return true;
}

#if __cplusplus > 202002L

CXX_NO_INLINE
constexpr bool my_append_string_jau(std::string &s, size_t max_len, const char *vbegin, const char *vend) noexcept { // NOLINT
    const size_t len = jau::min(max_len - 1 - s.size(), size_t(vend-vbegin));
    const size_t osz = s.size();
    return jau::string_resize_and_overwrite(s, osz + len + 1, [osz, vbegin, len](char *m, size_t msz) noexcept -> size_t {
            std::memcpy(m+osz, vbegin, len); // less EOS
            return msz-1; // less EOS
        });
}

CXX_NO_INLINE
constexpr bool my_append_string_jau(std::string &s, size_t max_len, std::string_view add) noexcept { // NOLINT
    const size_t len = jau::min(max_len - 1 - s.size(), add.size());
    const size_t osz = s.size();
    return jau::string_resize_and_overwrite(s, osz + len + 1, [osz, vbegin=add.data(), len](char *m, size_t msz) noexcept -> size_t {
            std::memcpy(m+osz, vbegin, len); // less EOS
            return msz-1; // less EOS
        });

}

#endif

/**
 * append: 42 (regular), 43 (auto-conversion)
 * - no width/prec
 *   - 3 regular (42)
 *   - 4 auto-conversion (43)
 * - with/prec
 *   - 13 regular (42)
 *   - 14 auto-conversion (43)
 */
int main(int argc, char *argv[]) {
    size_t loops = 1000000;

    for (int i = 0; i < argc; i++) {
        if (0 == strcmp("--loops", argv[i]) && i+1<argc) {
            jau::fromIntString(loops, std::string_view(argv[++i]));
        }
    }
    jau_fprintf_td(stderr, "XXX: loops %'zu\n", loops);

    const size_t bsz = jau::cfmt::default_string_capacity + 1; // including EOS
    std::string reserved;
    reserved.reserve(bsz);         // incl. EOS
    volatile size_t res = 0;
    constexpr std::string_view something = "Hello World This is Demo Data";

    {
        jau_fprintf_td(stderr, "tst1 std\n");
        reserved.clear();
        for( size_t i = 0; i < loops; ++i ) {
            size_t idx = i % (something.size() - 10);
            const char *a = something.data() + idx;
            const char *b = a + 6;
            my_append_string_std(reserved, reserved.capacity(), a, b);
            res = res + reserved.size(); // std::hash<std::string>{}(reserved);
            if (reserved.size() + 10 >= reserved.capacity()) {
                reserved.clear();
            }
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
#if __cplusplus > 202002L
    {
        jau_fprintf_td(stderr, "tst1 jau\n");
        reserved.clear();
        for( size_t i = 0; i < loops; ++i ) {
            size_t idx = i % (something.size() - 10);
            const char *a = something.data() + idx;
            const char *b = a + 6;
            my_append_string_jau(reserved, reserved.capacity(), a, b);
            res = res + reserved.size(); // std::hash<std::string>{}(reserved);
            if (reserved.size() + 10 >= reserved.capacity()) {
                reserved.clear();
            }
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
#endif
    size_t res2 = res;
    jau_fprintf_td(stderr, "Exit (res %zu)\n", res2);
    return 0;
}
