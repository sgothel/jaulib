#include <cassert>
#include <cinttypes>
#include <cstring>

#include <jau/test/catch2_ext.hpp>

#include <jau/uuid.hpp>

using namespace jau;

TEST_CASE( "UUID Test 01", "[datatype][uuid]" ) {
    uint8_t buffer[100];
    static uint8_t uuid128_bytes[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
                                       0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };

    {
        const uuid128_t v01 = uuid128_t(uuid128_bytes, 0, true);
        REQUIRE(v01.getTypeSizeInt() == 16);
        REQUIRE(v01.getTypeSizeInt() == sizeof(v01.value));
        REQUIRE(v01.getTypeSizeInt() == sizeof(v01.value.data));
        REQUIRE( 0 == memcmp(uuid128_bytes, v01.data(), 16) );

        v01.put(buffer, 0, true /* littleEndian */);
        std::shared_ptr<const uuid_t> v02 = uuid_t::create(uuid_t::TypeSize::UUID128_SZ, buffer, 0, true);
        REQUIRE(v02->getTypeSizeInt() == 16);
        REQUIRE( 0 == memcmp(v01.data(), v02->data(), 16) );
        REQUIRE( v01.toString() == v02->toString() );
    }

    {
        const uuid32_t v01 = uuid32_t(uuid32_t(0x12345678));
        REQUIRE(v01.getTypeSizeInt() == 4);
        REQUIRE(v01.getTypeSizeInt() == sizeof(v01.value));
        REQUIRE(0x12345678 == v01.value);

        v01.put(buffer, 0, true /* littleEndian */);
        std::shared_ptr<const uuid_t> v02 = uuid_t::create(uuid_t::TypeSize::UUID32_SZ, buffer, 0, true);
        REQUIRE(v02->getTypeSizeInt() == 4);
        REQUIRE( 0 == memcmp(v01.data(), v02->data(), 4) );
        REQUIRE( v01.toString() == v02->toString() );
    }

    {
        const uuid16_t v01 = uuid16_t(uuid16_t(0x1234));
        REQUIRE(v01.getTypeSizeInt() == 2);
        REQUIRE(v01.getTypeSizeInt() == sizeof(v01.value));
        REQUIRE(0x1234 == v01.value);

        v01.put(buffer, 0, true /* littleEndian */);
        std::shared_ptr<const uuid_t> v02 = uuid_t::create(uuid_t::TypeSize::UUID16_SZ, buffer, 0, true);
        REQUIRE(v02->getTypeSizeInt() == 2);
        REQUIRE( 0 == memcmp(v01.data(), v02->data(), 2) );
        REQUIRE( v01.toString() == v02->toString() );
    }



    {
        const uuid128_t v01("00001234-5678-100A-800B-00805F9B34FB");
        REQUIRE(v01.getTypeSizeInt() == uuid_t::number( uuid_t::TypeSize::UUID128_SZ) );
        REQUIRE(v01.getTypeSizeInt() == sizeof(v01.value));
        REQUIRE("00001234-5678-100a-800b-00805f9b34fb" == v01.toString());
        REQUIRE(uuid128_t("00001234-5678-100a-800b-00805f9b34fb") == v01);
        REQUIRE(uuid128_t("00001234-5678-100a-800b-00805f9b34fc") != v01);
    }
    {
        const uuid16_t v01("1234");
        REQUIRE(v01.getTypeSizeInt() == uuid_t::number( uuid_t::TypeSize::UUID16_SZ ));
        REQUIRE(v01.getTypeSizeInt() == sizeof(v01.value));
        REQUIRE(0x1234 == v01.value);
        REQUIRE("1234" == v01.toString());

        const uuid16_t v01_copy = v01;
        REQUIRE(v01_copy == v01);
        REQUIRE(uuid16_t("1235") != v01);

        const uuid128_t v01_128 = v01.toUUID128();
        const uuid128_t v02("00001234-0000-1000-8000-00805F9B34FB");
        REQUIRE(v01_128 == v02);
        REQUIRE(v01 != v02);
        REQUIRE(v01.equivalent(v02));
    }
    {
        const uuid32_t v01("12345678");
        REQUIRE(v01.getTypeSizeInt() == uuid_t::number( uuid_t::TypeSize::UUID32_SZ ));
        REQUIRE(v01.getTypeSizeInt() == sizeof(v01.value));
        REQUIRE(0x12345678 == v01.value);
        REQUIRE("12345678" == v01.toString());

        const uuid32_t v01_copy = v01;
        REQUIRE(v01_copy == v01);
        REQUIRE(uuid32_t("12345679") != v01);

        const uuid128_t v01_128 = v01.toUUID128();
        const uuid128_t v02("12345678-0000-1000-8000-00805F9B34FB");
        REQUIRE(v01_128 == v02);

        REQUIRE(v01 != v02);
        REQUIRE(v01.equivalent(v02));
    }



    {
        std::shared_ptr<const uuid_t> v01 = uuid_t::create("1234");
        REQUIRE(v01->getTypeSizeInt() == uuid_t::number( uuid_t::TypeSize::UUID16_SZ ));
        REQUIRE("1234" == v01->toString());
    }
    {
        std::shared_ptr<const uuid_t> v01 = uuid_t::create("12345678");
        REQUIRE(v01->getTypeSizeInt() == uuid_t::number( uuid_t::TypeSize::UUID32_SZ));
        REQUIRE("12345678" == v01->toString());
    }
    {
        std::shared_ptr<const uuid_t> v01 = uuid_t::create("00001234-5678-100A-800B-00805F9B34FB");
        REQUIRE(v01->getTypeSizeInt() == uuid_t::number( uuid_t::TypeSize::UUID128_SZ ));
        REQUIRE("00001234-5678-100a-800b-00805f9b34fb" == v01->toString());
    }
}
