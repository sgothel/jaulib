/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
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
#include <iostream>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <random>
#include <vector>

#define CATCH_CONFIG_RUNNER
// #define CATCH_CONFIG_MAIN
#include <catch2/catch_amalgamated.hpp>
#include <jau/test/catch2_ext.hpp>

#include "test_datatype01.hpp"

#include "test_datatype02.hpp"

#include <jau/basic_algos.hpp>
#include <jau/basic_types.hpp>
#include <jau/darray.hpp>
#include <jau/cow_darray.hpp>
#include <jau/cow_vector.hpp>
#include <jau/counting_allocator.hpp>
#include <jau/callocator.hpp>
#include <jau/counting_callocator.hpp>

/**
 * Test general use of jau::darray, jau::cow_darray and jau::cow_vector.
 */
using namespace jau;

/**********************************************************************************************************************************************/
/**********************************************************************************************************************************************/
/**********************************************************************************************************************************************/
/**********************************************************************************************************************************************/
/**********************************************************************************************************************************************/

TEST_CASE( "JAU DArray Test 01 - jau::darray initializer list", "[datatype][jau][darray]" ) {
    int i = 0;
    jau::for_each(GATT_SERVICES.begin(), GATT_SERVICES.end(), [&i](const GattServiceCharacteristic* ml){
       (void)ml;
       // printf("XX: %s\n\n", ml->toString().c_str());
        ++i;
    });
    REQUIRE(3 == i);
}

static int countGattCharacteristicSpecList01ConstRef(const jau::darray<const GattCharacteristicSpec>& clist) {
    int i = 0;
    jau::for_each(clist.begin(), clist.end(), [&i](const GattCharacteristicSpec& ml){
       (void)ml;
       // printf("XX: %s\n\n", ml->toString().c_str());
        ++i;
    });
    return i;
}
static int countGattCharacteristicSpecList02Copy(jau::darray<const GattCharacteristicSpec> clist) {
    int i = 0;
    jau::for_each(clist.begin(), clist.end(), [&i](const GattCharacteristicSpec& ml){
       (void)ml;
       // printf("XX: %s\n\n", ml->toString().c_str());
        ++i;
    });
    return i;
}

TEST_CASE( "JAU DArray Test 02 - jau::darray immutable type (const)", "[const][jau][darray]" ) {
    const GattCharacteristicSpec cs1 = { DEVICE_NAME, Mandatory,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Optional }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Excluded }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          };
    jau::darray<const GattCharacteristicSpec> clist = {
            cs1,
            { APPEARANCE, Mandatory,
              // GattCharacteristicPropertySpec[9]:
              { { Read, Mandatory },
                { WriteWithAck, Excluded }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
                { Notify, Excluded }, { Indicate, Excluded }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
              // GattClientCharacteristicConfigSpec:
              { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
            } };
    clist.push_back( cs1 );
    int i = 0;
    jau::for_each(clist.begin(), clist.end(), [&i](const GattCharacteristicSpec& ml){
       (void)ml;
       // printf("XX: %s\n\n", ml->toString().c_str());
        ++i;
    });
    REQUIRE(3 == i);
    REQUIRE(3 == countGattCharacteristicSpecList01ConstRef(clist));
    REQUIRE(3 == countGattCharacteristicSpecList02Copy(clist));
}

/**********************************************************************************************************************************************/
/**********************************************************************************************************************************************/
/**********************************************************************************************************************************************/
/**********************************************************************************************************************************************/
/**********************************************************************************************************************************************/

template<class Payload>
using SharedPayloadListMemMove = jau::darray<std::shared_ptr<Payload>, jau::callocator<std::shared_ptr<Payload>>, jau::nsize_t, true /* use_memmove */, true /* use_realloc */>;
// JAU_TYPENAME_CUE_ALL(SharedPayloadListMemMove)

template<class Payload>
using SharedPayloadListDefault = jau::darray<std::shared_ptr<Payload>>;
// JAU_TYPENAME_CUE_ALL(SharedPayloadListDefault)

template<class Payload>
struct NamedSharedPayloadListDefault {
    int name;
    SharedPayloadListDefault<Payload> payload;

    std::string toString() const noexcept {
        std::string res = "NSPL-Default-"+std::to_string(name)+"[sz"+std::to_string(payload.size())+": ";
        int i=0;
        jau::for_each(payload.cbegin(), payload.cend(), [&](const std::shared_ptr<Payload>& e) {
            if(0<i) {
                res += ", ";
            }
            res += "["+jau::to_string(e)+"]";
            ++i;
        } );
        res += "]";
        return res;
    }
};
// JAU_TYPENAME_CUE_ALL(NamedSharedPayloadListDefault)

template<class Payload>
struct NamedSharedPayloadListMemMove {
    int name;
    SharedPayloadListMemMove<Payload> payload;

    std::string toString() const noexcept {
        std::string res = "NSPL-MemMove-"+std::to_string(name)+"[sz"+std::to_string(payload.size())+": ";
        int i=0;
        jau::for_each(payload.cbegin(), payload.cend(), [&](const std::shared_ptr<Payload>& e) {
            if(0<i) {
                res += ", ";
            }
            res += "["+jau::to_string(e)+"]";
            ++i;
        } );
        res += "]";
        return res;
    }
};
// JAU_TYPENAME_CUE_ALL(NamedSharedPayloadListMemMove)

template<class Payload>
using PayloadListMemMove = jau::darray<Payload, jau::callocator<Payload>, jau::nsize_t, true /* use_memmove */, true /* use_realloc */>;
// JAU_TYPENAME_CUE_ALL(PayloadListMemMove)

template<class Payload>
using PayloadListDefault = jau::darray<Payload>;
// JAU_TYPENAME_CUE_ALL(PayloadListDefault)

template<class Payload>
struct NamedPayloadListDefault {
    int name;
    PayloadListDefault<Payload> payload;

    std::string toString() const noexcept {
        std::string res = "NPL-Default-"+std::to_string(name)+"[sz"+std::to_string(payload.size())+": ";
        int i=0;
        jau::for_each(payload.cbegin(), payload.cend(), [&](const typename PayloadListDefault<Payload>::value_type & e) {
            if(0<i) {
                res += ", ";
            }
            res += "["+jau::to_string(e)+"]";
            ++i;
        } );
        res += "]";
        return res;
    }
};
// JAU_TYPENAME_CUE_ALL(NamedPayloadListDefault)

template<class Payload>
struct NamedPayloadListMemMove {
    int name;
    PayloadListMemMove<Payload> payload;

    std::string toString() const noexcept {
        std::string res = "NPL-MemMove-"+std::to_string(name)+"[sz"+std::to_string(payload.size())+": ";
        int i=0;
        jau::for_each(payload.cbegin(), payload.cend(), [&](const Payload& e) {
            if(0<i) {
                res += ", ";
            }
            res += "["+jau::to_string(e)+"]";
            ++i;
        } );
        res += "]";
        return res;
    }
};
// JAU_TYPENAME_CUE_ALL(NamedPayloadListMemMove)

template<class Payload>
static NamedSharedPayloadListDefault<Payload> makeNamedSharedPayloadListDefault(int name) {
    SharedPayloadListDefault<Payload> data;
    int i=0;
    for(i=0; i<2; i++) {
        std::shared_ptr<Payload> sp(std::make_shared<Payload>( name+i )); // copy-elision + make_shared in-place
        data.push_back( sp );
    }
    for(i=2; i<4; i++) {
        std::shared_ptr<Payload> sp(new Payload( name+i )); // double malloc: 1 Payload, 2 shared_ptr
        data.push_back( std::move( sp ) ); // move the less efficient into
    }
    return NamedSharedPayloadListDefault<Payload>{name, data};
}
template<class Payload>
static NamedSharedPayloadListDefault<Payload> modifyCopyOfNamedSharedPayloadListDefault(NamedSharedPayloadListDefault<Payload> src) {
    printf("XXX1: %s\n", src.toString().c_str());
    src.payload.pop_back();
    src.payload.erase(src.payload.cbegin());
    printf("XXX2: %s\n", src.toString().c_str());
    return src;
}

template<class Payload>
static NamedSharedPayloadListMemMove<Payload> makeNamedSharedPayloadListMemMove(int name) {
    SharedPayloadListMemMove<Payload> data;
    int i=0;
    for(i=0; i<2; i++) {
        std::shared_ptr<Payload> sp(std::make_shared<Payload>( name+i )); // copy-elision + make_shared in-place
        data.push_back( sp );
    }
    for(i=2; i<4; i++) {
        std::shared_ptr<Payload> sp(new Payload( name+i )); // double malloc: 1 Payload, 2 shared_ptr
        data.push_back( std::move( sp ) ); // move the less efficient into
    }
    return NamedSharedPayloadListMemMove<Payload>{name, data};
}
template<class Payload>
static NamedPayloadListDefault<Payload> makeNamedPayloadListDefault(int name) {
    PayloadListDefault<Payload> data;
    int i=0;
    for(i=0; i<2; i++) {
        Payload sp( name+i ); // copy-elision
        data.push_back( sp );
    }
    for(i=2; i<4; i++) {
        Payload sp( name+i );
        data.push_back( std::move( sp ) ); // move the less efficient into
    }
    return NamedPayloadListDefault<Payload>{name, data};
}
template<class Payload>
static NamedPayloadListMemMove<Payload> makeNamedPayloadListMemMove(int name) {
    PayloadListMemMove<Payload> data;
    int i=0;
    for(i=0; i<2; i++) {
        Payload sp( name+i ); // copy-elision
        data.push_back( sp );
    }
    for(i=2; i<4; i++) {
        Payload sp( name+i );
        data.push_back( std::move( sp ) ); // move the less efficient into
    }
    return NamedPayloadListMemMove<Payload>{name, data};
}

JAU_TYPENAME_CUE_ALL(std::shared_ptr<Addr48Bit>)
JAU_TYPENAME_CUE_ALL(jau::darray<Addr48Bit>)
JAU_TYPENAME_CUE_ALL(jau::darray<std::shared_ptr<Addr48Bit>>)

JAU_TYPENAME_CUE_ALL(std::shared_ptr<DataType01>)
JAU_TYPENAME_CUE_ALL(jau::darray<DataType01>)
JAU_TYPENAME_CUE_ALL(jau::darray<std::shared_ptr<DataType01>>)

#define CHECK_TRAITS 0

template< class Cont >
static void print_container_info(const std::string& type_id, const Cont &c,
        std::enable_if_t< jau::is_darray_type<Cont>::value, bool> = true )
{
    printf("\nContainer Type %s (a darray, a cow %d):\n  - Uses memcpy %d (trivially_copyable %d); realloc %d; base_of jau::callocator %d; size %d bytes\n",
                type_id.c_str(), jau::is_cow_type<Cont>::value,
                Cont::uses_memmove,
                std::is_trivially_copyable<typename Cont::value_type>::value,
                Cont::uses_realloc,
                std::is_base_of<jau::callocator<typename Cont::value_type>, typename Cont::allocator_type>::value,
                (int)sizeof(c));
}

template<class Cont>
static void print_container_info(const std::string& type_id, const Cont &c,
        std::enable_if_t< !jau::is_darray_type<Cont>::value, bool> = true )
{
    printf("\nContainer Type %s (!darray, a cow %d); size %d bytes\n",
                type_id.c_str(), jau::is_cow_type<Cont>::value, (int)sizeof(c));
}


template<class Payload>
static void testDArrayValueType(const std::string& type_id) {
    {
        // jau::type_cue<Payload>::print(type_id, jau::TypeTraitGroup::ALL);
        // jau::type_cue<std::shared_ptr<Payload>>::print("std::shared_ptr<"+type_id+">", jau::TypeTraitGroup::ALL);
    }
    {
#if CHECK_TRAITS
        CHECK( true == std::is_base_of<jau::callocator<Payload>, jau::callocator<Payload>>::value);
        CHECK( true == PayloadListDefault<Payload>::uses_realloc);
        CHECK( true == std::is_trivially_copyable<Payload>::value);
        CHECK( true == PayloadListDefault<Payload>::uses_memmove);
#endif

        NamedPayloadListDefault<Payload> data = makeNamedPayloadListDefault<Payload>(1);
        print_container_info("NamedPayloadListDefault<"+type_id+">", data.payload);

        NamedPayloadListDefault<Payload> data2 = data;
        data2.payload.erase(data2.payload.cbegin());

        NamedPayloadListDefault<Payload> data3(data);
        data3.payload.erase(data3.payload.begin(), data3.payload.cbegin()+data3.payload.size()/2);

        NamedPayloadListDefault<Payload> data8 = makeNamedPayloadListDefault<Payload>(8);
        data8.payload.insert(data8.payload.begin(), data.payload.cbegin(), data.payload.cend());

        printf("COPY-0: %s\n\n", data.toString().c_str());
        printf("COPY-1: %s\n\n", data2.toString().c_str());
        printf("COPY-2: %s\n\n", data3.toString().c_str());
        printf("COPY+2: %s\n\n", data8.toString().c_str());
    }
    {
#if CHECK_TRAITS
        CHECK( true == std::is_base_of<jau::callocator<Payload>, jau::callocator<Payload>>::value);
        CHECK( true == PayloadListMemMove<Payload>::uses_realloc);
        CHECK( true == std::is_trivially_copyable<Payload>::value);
        CHECK( true == PayloadListMemMove<Payload>::uses_memmove);
#endif

        NamedPayloadListMemMove<Payload> data = makeNamedPayloadListMemMove<Payload>(1);
        print_container_info("NamedPayloadListMemMove<"+type_id+">", data.payload);

        NamedPayloadListMemMove<Payload> data2 = data;
        data2.payload.erase(data2.payload.cbegin());

        NamedPayloadListMemMove<Payload> data3(data);
        data3.payload.erase(data3.payload.begin(), data3.payload.cbegin()+data3.payload.size()/2);

        NamedPayloadListMemMove<Payload> data8 = makeNamedPayloadListMemMove<Payload>(8);
        data8.payload.insert(data8.payload.begin(), data.payload.cbegin(), data.payload.cend());

        printf("COPY-0: %s\n\n", data.toString().c_str());
        printf("COPY-1: %s\n\n", data2.toString().c_str());
        printf("COPY-2: %s\n\n", data3.toString().c_str());
        printf("COPY+2: %s\n\n", data8.toString().c_str());
    }
    {
#if CHECK_TRAITS
        CHECK( true == std::is_base_of<jau::callocator<std::shared_ptr<Payload>>, jau::callocator<std::shared_ptr<Payload>>>::value);
        CHECK( true == SharedPayloadListDefault<Payload>::uses_realloc);
        CHECK( true == std::is_trivially_copyable<std::shared_ptr<Payload>>::value);
        CHECK( true == SharedPayloadListDefault<Payload>::uses_memmove);
#endif

        NamedSharedPayloadListDefault<Payload> data = makeNamedSharedPayloadListDefault<Payload>(1);
        print_container_info("NamedSharedPayloadListDefault<"+type_id+">", data.payload);

        NamedSharedPayloadListDefault<Payload> data2 = data;
        data2.payload.erase(data2.payload.cbegin());

        NamedSharedPayloadListDefault<Payload> data3(data);
        data3.payload.erase(data3.payload.begin(), data3.payload.cbegin()+data3.payload.size()/2);

        NamedSharedPayloadListDefault<Payload> data8 = makeNamedSharedPayloadListDefault<Payload>(8);
        data8.payload.insert(data8.payload.begin(), data.payload.cbegin(), data.payload.cend());

        printf("COPY-0: %s\n\n", data.toString().c_str());
        printf("COPY-1: %s\n\n", data2.toString().c_str());
        printf("COPY-2: %s\n\n", data3.toString().c_str());
        printf("COPY+2: %s\n\n", data8.toString().c_str());

        NamedSharedPayloadListDefault<Payload> data8_mod(modifyCopyOfNamedSharedPayloadListDefault(data8));
        printf("MODI+2-2: %s\n\n", data8_mod.toString().c_str());

        struct Holder {
            NamedSharedPayloadListDefault<Payload> lala;
            NamedSharedPayloadListDefault<Payload> & get_ref() { return lala; }
            NamedSharedPayloadListDefault<Payload> & get_ref2() { lala.payload.pop_back(); return lala; }
            NamedSharedPayloadListDefault<Payload> get_copy() { return lala; }
        };
        Holder holder{ data };
        NamedSharedPayloadListDefault<Payload> & r1r1 = holder.get_ref();
        printf("R1R1: %s\n\n", r1r1.toString().c_str());

        NamedSharedPayloadListDefault<Payload> r2c1 = holder.get_ref();
        printf("R1C1: %s\n\n", r2c1.toString().c_str());

        NamedSharedPayloadListDefault<Payload> c1c2 = holder.get_copy();
        printf("C1C2: %s\n\n", c1c2.toString().c_str());

        r1r1 = holder.get_ref2();
        printf("R2R2: %s\n\n", r1r1.toString().c_str());
    }
    {
#if CHECK_TRAITS
        CHECK( true == std::is_base_of<jau::callocator<std::shared_ptr<Payload>>, jau::callocator<std::shared_ptr<Payload>>>::value);
        CHECK( true == SharedPayloadListMemMove<Payload>::uses_realloc);
        CHECK( true == std::is_trivially_copyable<std::shared_ptr<Payload>>::value);
        CHECK( true == SharedPayloadListMemMove<Payload>::uses_memmove);
#endif

        NamedSharedPayloadListMemMove<Payload> data = makeNamedSharedPayloadListMemMove<Payload>(1);
        print_container_info("NamedSharedPayloadListMemMove<"+type_id+">", data.payload);

        NamedSharedPayloadListMemMove<Payload> data2 = data;
        data2.payload.erase(data2.payload.cbegin());

        NamedSharedPayloadListMemMove<Payload> data3(data);
        data3.payload.erase(data3.payload.begin(), data3.payload.cbegin()+data3.payload.size()/2);

        NamedSharedPayloadListMemMove<Payload> data8 = makeNamedSharedPayloadListMemMove<Payload>(8);
        data8.payload.insert(data8.payload.begin(), data.payload.cbegin(), data.payload.cend());

        printf("COPY-0: %s\n\n", data.toString().c_str());
        printf("COPY-1: %s\n\n", data2.toString().c_str());
        printf("COPY-2: %s\n\n", data3.toString().c_str());
        printf("COPY+2: %s\n\n", data8.toString().c_str());
    }
}

static GattServiceCharacteristic returnGattSrvcChar(int i) {
    return *GATT_SERVICES[i];
}

static void testDArrayGattServiceCharacteristic() {
#if CHECK_TRAITS
    CHECK( true == std::is_base_of<jau::callocator<GattCharacteristicSpec>, jau::callocator<GattCharacteristicSpec>>::value);
    CHECK( true == GattCharacteristicSpecList::uses_realloc);

    CHECK( true == GattCharacteristicSpecList::uses_memmove);
    CHECK( true == std::is_trivially_copyable<GattCharacteristicSpec>::value);
#endif

    GattServiceCharacteristic gatt2 = returnGattSrvcChar(1);
    print_container_info("darray<GattCharacteristicSpec>", gatt2.characteristics);

    gatt2.characteristics.erase(gatt2.characteristics.cbegin());

    GattServiceCharacteristic gatt2b = gatt2;
    gatt2b.characteristics.erase(gatt2b.characteristics.cbegin());

    GattServiceCharacteristic gatt2c(gatt2);
    gatt2c.characteristics.erase(gatt2c.characteristics.cbegin());

    printf("COPY0-1: %s\n\n", gatt2.toString().c_str());
    printf("COPY1-2: %s\n\n", gatt2b.toString().c_str());
    printf("COPY2-3: %s\n\n", gatt2c.toString().c_str());
}

TEST_CASE( "JAU DArray Test 10 - jau::darray value_type behavior (type traits)", "[datatype][jau][darray]" ) {
    testDArrayValueType<uint64_t>("uint64_t");
    testDArrayValueType<Addr48Bit>("Addr48Bit");
    testDArrayValueType<DataType01>("DataType01");
    testDArrayGattServiceCharacteristic();
}

/**********************************************************************************************************************************************/
/**********************************************************************************************************************************************/
/**********************************************************************************************************************************************/
/**********************************************************************************************************************************************/
/**********************************************************************************************************************************************/

