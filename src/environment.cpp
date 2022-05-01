/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2020 ZAFENA AB
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

#include <cstring>
#include <string>
#include <algorithm>
#include <memory>
#include <cstdint>
#include <vector>
#include <cstdio>

#include <jau/environment.hpp>
#include <jau/debug.hpp>

using namespace jau;

const uint64_t environment::startupTimeMilliseconds = jau::getCurrentMilliseconds();
const fraction_timespec environment::startupTimeMonotonic = jau::getMonotonicTime();

bool environment::local_debug = false;

static const std::string s_true("true");
static const std::string s_false("false");

std::string environment::getProperty(const std::string & name) noexcept {
    const char * value = getenv(name.c_str());
    if( nullptr != value ) {
        COND_PRINT(local_debug, "env::getProperty0 '%s': '%s'", name.c_str(), value);
        return std::string( value );
    }
    if( std::string::npos != name.find('.', 0) ) {
        // Retry with '.' -> '_' to please unix shell
        std::string alt_name(name);
        std::replace( alt_name.begin(), alt_name.end(), '.', '_');
        value = getenv(alt_name.c_str());
        if( nullptr != value ) {
            COND_PRINT(local_debug, "env::getProperty0 '%s' -> '%s': '%s'", name.c_str(), alt_name.c_str(), value);
            return std::string( value );
        }
        COND_PRINT(local_debug, "env::getProperty0 '%s' -> '%s': NOT FOUND", name.c_str(), alt_name.c_str());
    } else {
        COND_PRINT(local_debug, "env::getProperty0 '%s': NOT FOUND", name.c_str());
    }
    // not found: empty string
    return std::string();
}

std::string environment::getProperty(const std::string & name, const std::string & default_value) noexcept {
    const std::string value = getProperty(name);
    if( 0 == value.length() ) {
        COND_PRINT(local_debug, "env::getProperty1 %s: null -> %s (default)", name.c_str(), default_value.c_str());
        return default_value;
    } else {
        COND_PRINT(local_debug, "env::getProperty1 %s (default %s): %s", name.c_str(), default_value.c_str(), value.c_str());
        return value;
    }
}

bool environment::getBooleanProperty(const std::string & name, const bool default_value) noexcept {
    const std::string value = getProperty(name);
    if( 0 == value.length() ) {
        COND_PRINT(local_debug, "env::getBooleanProperty %s: null -> %d (default)", name.c_str(), default_value);
        return default_value;
    } else {
        const bool res = "true" == value;
        COND_PRINT(local_debug, "env::getBooleanProperty %s (default %d): %d/%s", name.c_str(), default_value, res, value.c_str());
        return res;
    }
}

#include <limits.h>

int32_t environment::getInt32Property(const std::string & name, const int32_t default_value,
                                 const int32_t min_allowed, const int32_t max_allowed) noexcept
{
    const std::string value = getProperty(name);
    if( 0 == value.length() ) {
        COND_PRINT(local_debug, "env::getInt32Property %s: null -> %" PRId32 " (default)", name.c_str(), default_value);
        return default_value;
    } else {
        int32_t res = default_value;
        char *endptr = NULL;
        const long int res0 = strtol(value.c_str(), &endptr, 10);
        if( *endptr == '\0' ) {
            // string value completely valid
            if( INT32_MIN <= res0 && res0 <= INT32_MAX ) {
                // matching int32_t value range
                const int32_t res1 = (int32_t)res0;
                if( min_allowed <= res1 && res1 <= max_allowed ) {
                    // matching user value range
                    res = res1;
                    COND_PRINT(local_debug, "env::getInt32Property %s (default %" PRId32 "): %" PRId32 "/%s",
                            name.c_str(), default_value, res, value.c_str());
                } else {
                    // invalid user value range
                    ERR_PRINT("env::getInt32Property %s: %" PRId32 "/%s (invalid user range [% " PRId32 "..%" PRId32 "]) -> %" PRId32 " (default)",
                            name.c_str(), res1, value.c_str(), min_allowed, max_allowed, res);
                }
            } else {
                // invalid int32_t range
                ERR_PRINT("env::getInt32Property %s: %" PRIu64 "/%s (invalid int32_t range) -> %" PRId32 " (default)",
                        name.c_str(), (uint64_t)res0, value.c_str(), res);
            }
        } else {
            // string value not fully valid
            ERR_PRINT("env::getInt32Property %s: %s (invalid string) -> %" PRId32 " (default)",
                    name.c_str(), value.c_str(), res);
        }
        return res;
    }
}

uint32_t environment::getUint32Property(const std::string & name, const uint32_t default_value,
                                   const uint32_t min_allowed, const uint32_t max_allowed) noexcept
{
    const std::string value = getProperty(name);
    if( 0 == value.length() ) {
        COND_PRINT(local_debug, "env::getUint32Property %s: null -> %" PRIu32 " (default)", name.c_str(), default_value);
        return default_value;
    } else {
        uint32_t res = default_value;
        char *endptr = NULL;
        unsigned long int res0 = strtoul(value.c_str(), &endptr, 10);
        if( *endptr == '\0' ) {
            // string value completely valid
            if( res0 <= UINT32_MAX ) {
                // matching uint32_t value range
                const uint32_t res1 = (uint32_t)res0;
                if( min_allowed <= res1 && res1 <= max_allowed ) {
                    // matching user value range
                    res = res1;
                    COND_PRINT(local_debug, "env::getUint32Property %s (default %" PRIu32 "): %" PRIu32 "/%s",
                            name.c_str(), default_value, res, value.c_str());
                } else {
                    // invalid user value range
                    ERR_PRINT("env::getUint32Property %s: %" PRIu32 "/%s (invalid user range [% " PRIu32 "..%" PRIu32 "]) -> %" PRIu32 " (default)",
                            name.c_str(), res1, value.c_str(), min_allowed, max_allowed, res);
                }
            } else {
                // invalid uint32_t range
                ERR_PRINT("env::getUint32Property %s: %" PRIu64 "/%s (invalid uint32_t range) -> %" PRIu32 " (default)",
                        name.c_str(), (uint64_t)res0, value.c_str(), res);
            }
        } else {
            // string value not fully valid
            ERR_PRINT("env::getUint32Property %s: %s (invalid string) -> %" PRIu32 " (default)",
                    name.c_str(), value.c_str(), res);
        }
        return res;
    }
}

fraction_i64 environment::getFractionProperty(const std::string & name, const fraction_i64& default_value,
                                              const fraction_i64& min_allowed, const fraction_i64& max_allowed) noexcept {
    const std::string value = getProperty(name);
    if( 0 == value.length() ) {
        COND_PRINT(local_debug, "env::getFractionProperty %s: null -> %s (default)", name.c_str(), default_value.to_string().c_str());
        return default_value;
    } else {
        fraction_i64 result = default_value;
        if( !to_fraction_i64(result, value, min_allowed, max_allowed) ) {
            ERR_PRINT("env::getFractionProperty %s: value %s not valid or in range[%s .. %s] -> %s (default)",
                    name.c_str(), value.c_str(), min_allowed.to_string().c_str(), max_allowed.to_string().c_str(), default_value.to_string().c_str());
        }
        return result;
    }
}

void environment::envSet(std::string prefix_domain, std::string basepair) noexcept {
    trimInPlace(basepair);
    if( basepair.length() > 0 ) {
        size_t pos = 0, start = 0;
        if( (pos = basepair.find('=', start)) != std::string::npos ) {
            const size_t elem_len = pos-start; // excluding '='
            std::string name = prefix_domain+"."+basepair.substr(start, elem_len);
            std::string value = basepair.substr(pos+1, std::string::npos);
            trimInPlace(name);
            trimInPlace(value);
            if( name.length() > 0 ) {
                if( value.length() > 0 ) {
                    COND_PRINT(local_debug, "env::setProperty %s -> %s (explode)", name.c_str(), value.c_str());
                    setenv(name.c_str(), value.c_str(), 1 /* overwrite */);
                } else {
                    COND_PRINT(local_debug, "env::setProperty %s -> true (explode default-1)", name.c_str());
                    setenv(name.c_str(), "true", 1 /* overwrite */);
                }
            }
        } else {
            const std::string name = prefix_domain+"."+basepair;
            COND_PRINT(local_debug, "env::setProperty %s -> true (explode default-0)", name.c_str());
            setenv(name.c_str(), "true", 1 /* overwrite */);
        }
    }
}

void environment::envExplodeProperties(std::string prefix_domain, std::string list) noexcept {
    size_t pos = 0, start = 0;
    while( (pos = list.find(',', start)) != std::string::npos ) {
        const size_t elem_len = pos-start; // excluding ','
        envSet(prefix_domain, list.substr(start, elem_len));
        start = pos+1; // skip ','
    }
    const size_t elem_len = list.length()-start; // last one
    if( elem_len > 0 ) {
        envSet(prefix_domain, list.substr(start, elem_len));
    }
    COND_PRINT(local_debug, "env::setProperty %s -> true (explode default)", prefix_domain.c_str());
    setenv(prefix_domain.c_str(), "true", 1 /* overwrite */);
}

bool environment::getExplodingPropertiesImpl(const std::string & root_prefix_domain, const std::string & prefix_domain) noexcept {
    std::string value = environment::getProperty(prefix_domain, s_false);
    if( s_false == value ) {
        return false;
    }
    if( s_true == value ) {
        return true;
    }
    if( root_prefix_domain.length() > 0 && root_prefix_domain+".debug" == prefix_domain ) {
        local_debug = true;
    }
    envExplodeProperties(prefix_domain, value);
    return true;
}

environment::environment(const std::string & root_prefix_domain_) noexcept
: root_prefix_domain(root_prefix_domain_),
  debug( getExplodingPropertiesImpl(root_prefix_domain_, root_prefix_domain_+".debug") ),
  debug_jni( getBooleanProperty(root_prefix_domain_+".debug.jni", false) ),
  verbose( getExplodingPropertiesImpl(root_prefix_domain_, root_prefix_domain_+".verbose") || environment::debug )
{
}
