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
#ifndef TEST_DATATYPE02_CPP_
#define TEST_DATATYPE02_CPP_

#include <iostream>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <random>

#include <iostream>

#include <jau/cpp_lang_macros.hpp>
#include <jau/packed_attribute.hpp>
#include <jau/ordered_atomic.hpp>
#include <jau/basic_types.hpp>

#include <jau/darray.hpp>

using namespace jau;

enum GattServiceType : uint16_t {
    /** This service contains generic information about the device. This is a mandatory service. */
    GENERIC_ACCESS                              = 0x1800,
    /** The service allows receiving indications of changed services. This is a mandatory service. */
    GENERIC_ATTRIBUTE                           = 0x1801,
    /** This service exposes a control point to change the peripheral alert behavior. */
    IMMEDIATE_ALERT                             = 0x1802,
    /** The service defines behavior on the device when a link is lost between two devices. */
    LINK_LOSS                                   = 0x1803,
    /** This service exposes temperature and other data from a thermometer intended for healthcare and fitness applications. */
    HEALTH_THERMOMETER                          = 0x1809,
    /** This service exposes manufacturer and/or vendor information about a device. */
    DEVICE_INFORMATION                          = 0x180A,
    /** This service exposes the state of a battery within a device. */
    BATTERY_SERVICE                             = 0x180F,
};
JAU_TYPENAME_CUE_ALL(GattServiceType)

enum GattCharacteristicType : uint16_t {
    //
    // GENERIC_ACCESS
    //
    DEVICE_NAME                                 = 0x2A00,
    APPEARANCE                                  = 0x2A01,
    PERIPHERAL_PRIVACY_FLAG                     = 0x2A02,
    RECONNECTION_ADDRESS                        = 0x2A03,
    PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS  = 0x2A04,

    /** Mandatory: sint16 10^-2: Celsius */
    TEMPERATURE                                 = 0x2A6E,

    /** Mandatory: sint16 10^-1: Celsius */
    TEMPERATURE_CELSIUS                         = 0x2A1F,
    TEMPERATURE_FAHRENHEIT                      = 0x2A20,

    //
    // HEALTH_THERMOMETER
    //
    TEMPERATURE_MEASUREMENT                     = 0x2A1C,
    /** Mandatory: 8bit: 1 armpit, 2 body (general), 3(ear), 4 (finger), ... */
    TEMPERATURE_TYPE                            = 0x2A1D,
    INTERMEDIATE_TEMPERATURE                    = 0x2A1E,
    MEASUREMENT_INTERVAL                        = 0x2A21,

    //
    // DEVICE_INFORMATION
    //
    /** Mandatory: uint40 */
    SYSTEM_ID                                   = 0x2A23,
    MODEL_NUMBER_STRING                         = 0x2A24,
    SERIAL_NUMBER_STRING                        = 0x2A25,
    FIRMWARE_REVISION_STRING                    = 0x2A26,
    HARDWARE_REVISION_STRING                    = 0x2A27,
    SOFTWARE_REVISION_STRING                    = 0x2A28,
    MANUFACTURER_NAME_STRING                    = 0x2A29,
    REGULATORY_CERT_DATA_LIST                   = 0x2A2A,
    PNP_ID                                      = 0x2A50,
};
JAU_TYPENAME_CUE_ALL(GattCharacteristicType)

enum GattCharacteristicProperty : uint8_t {
    Broadcast = 0x01,
    Read = 0x02,
    WriteNoAck = 0x04,
    WriteWithAck = 0x08,
    Notify = 0x10,
    Indicate = 0x20,
    AuthSignedWrite = 0x40,
    ExtProps = 0x80,
    /** FIXME: extension? */
    ReliableWriteExt = 0x81,
    /** FIXME: extension? */
    AuxWriteExt = 0x82
};
JAU_TYPENAME_CUE_ALL(GattCharacteristicProperty)

enum GattRequirementSpec : uint8_t {
    Excluded    = 0x00,
    Mandatory   = 0x01,
    Optional    = 0x02,
    Conditional = 0x03,
    if_characteristic_supported = 0x11,
    if_notify_or_indicate_supported = 0x12,
    C1 = 0x21,
};
JAU_TYPENAME_CUE_ALL(GattRequirementSpec)

struct GattCharacteristicPropertySpec {
    const GattCharacteristicProperty property;
    const GattRequirementSpec requirement;

    std::string toString() const noexcept;
};
JAU_TYPENAME_CUE_ALL(GattCharacteristicPropertySpec)

struct GattClientCharacteristicConfigSpec {
    GattRequirementSpec requirement;
    GattCharacteristicPropertySpec read;
    GattCharacteristicPropertySpec writeWithAck;

    std::string toString() const noexcept;
};
JAU_TYPENAME_CUE_ALL(GattClientCharacteristicConfigSpec)

struct GattCharacteristicSpec {
    GattCharacteristicType characteristic;
    GattRequirementSpec requirement;

    enum PropertySpecIdx : int {
        ReadIdx = 0,
        WriteNoAckIdx,
        WriteWithAckIdx,
        AuthSignedWriteIdx,
        ReliableWriteExtIdx,
        NotifyIdx,
        IndicateIdx,
        AuxWriteExtIdx,
        BroadcastIdx
    };
    /** Aggregated in PropertySpecIdx order */
    jau::darray<GattCharacteristicPropertySpec> propertySpec;

    GattClientCharacteristicConfigSpec clientConfig;

    std::string toString() const noexcept;
};
JAU_TYPENAME_CUE_ALL(GattCharacteristicSpec)

struct GattServiceCharacteristic {
    GattServiceType service;
    jau::darray<GattCharacteristicSpec> characteristics;

    std::string toString() const noexcept;
};
JAU_TYPENAME_CUE_ALL(GattServiceCharacteristic)

const GattServiceCharacteristic GATT_GENERIC_ACCESS_SRVC = { GENERIC_ACCESS,
        { { DEVICE_NAME, Mandatory,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Optional }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Excluded }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          },
          { APPEARANCE, Mandatory,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Excluded }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Excluded }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          },
          { PERIPHERAL_PRIVACY_FLAG, Optional,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Excluded }, { WriteNoAck, C1 }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Excluded }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          },
          { RECONNECTION_ADDRESS, Conditional,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Excluded },
              { WriteWithAck, Mandatory }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Excluded }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          },
          { PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS, Optional,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Excluded }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Excluded }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          },
        } };

/** https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Services/org.bluetooth.service.health_thermometer.xml */
const GattServiceCharacteristic GATT_HEALTH_THERMOMETER_SRVC = { HEALTH_THERMOMETER,
        { { TEMPERATURE_MEASUREMENT, Mandatory,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Excluded },
              { WriteWithAck, Excluded }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Mandatory }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Mandatory, { Read, Mandatory}, { WriteWithAck, Mandatory } }
          },
          { TEMPERATURE_TYPE, Optional,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Excluded }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Excluded }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          },
          { INTERMEDIATE_TEMPERATURE, Optional,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Excluded },
              { WriteWithAck, Excluded }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Mandatory }, { Indicate, Excluded }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { if_characteristic_supported, { Read, Mandatory}, { WriteWithAck, Mandatory } }
          },
          { MEASUREMENT_INTERVAL, Optional,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Optional }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Optional }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { if_notify_or_indicate_supported, { Read, Mandatory}, { WriteWithAck, Mandatory } }
          },
        } };

const GattServiceCharacteristic GATT_DEVICE_INFORMATION_SRVC = { DEVICE_INFORMATION,
        { { MANUFACTURER_NAME_STRING, Optional,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Excluded }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Mandatory }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          },
          { MODEL_NUMBER_STRING, Optional,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Excluded }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Mandatory }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          },
          { SERIAL_NUMBER_STRING, Optional,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Excluded }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Mandatory }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          },
          { HARDWARE_REVISION_STRING, Optional,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Excluded }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Mandatory }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          },
          { FIRMWARE_REVISION_STRING, Optional,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Excluded }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Mandatory }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          },
          { SOFTWARE_REVISION_STRING, Optional,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Excluded }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Mandatory }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          },
          { SYSTEM_ID, Optional,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Excluded }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Mandatory }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          },
          { REGULATORY_CERT_DATA_LIST, Optional,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Excluded }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Mandatory }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          },
          { PNP_ID, Optional,
            // GattCharacteristicPropertySpec[9]:
            { { Read, Mandatory },
              { WriteWithAck, Excluded }, { WriteNoAck, Excluded }, { AuthSignedWrite, Excluded }, { ReliableWriteExt, Excluded },
              { Notify, Excluded }, { Indicate, Mandatory }, { AuxWriteExt, Excluded }, { Broadcast, Excluded } },
            // GattClientCharacteristicConfigSpec:
            { Excluded, { Read, Excluded}, { WriteWithAck, Excluded } }
          }
        } };

const jau::darray<const GattServiceCharacteristic*> GATT_SERVICES = {
        &GATT_GENERIC_ACCESS_SRVC, &GATT_HEALTH_THERMOMETER_SRVC, &GATT_DEVICE_INFORMATION_SRVC };

#define CASE_TO_STRING(V) case V: return #V;

#define SERVICE_TYPE_ENUM(X) \
    X(GENERIC_ACCESS) \
    X(HEALTH_THERMOMETER) \
    X(DEVICE_INFORMATION) \
    X(BATTERY_SERVICE)

std::string GattServiceTypeToString(const GattServiceType v) noexcept {
    switch(v) {
        SERVICE_TYPE_ENUM(CASE_TO_STRING)
        default: ; // fall through intended
    }
    return "Unknown";
}

#define CHARACTERISTIC_TYPE_ENUM(X) \
    X(DEVICE_NAME) \
    X(APPEARANCE) \
    X(PERIPHERAL_PRIVACY_FLAG) \
    X(RECONNECTION_ADDRESS) \
    X(PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS) \
    X(TEMPERATURE) \
    X(TEMPERATURE_CELSIUS) \
    X(TEMPERATURE_FAHRENHEIT) \
    X(TEMPERATURE_MEASUREMENT) \
    X(TEMPERATURE_TYPE) \
    X(INTERMEDIATE_TEMPERATURE) \
    X(MEASUREMENT_INTERVAL) \
    X(SYSTEM_ID) \
    X(MODEL_NUMBER_STRING) \
    X(SERIAL_NUMBER_STRING) \
    X(FIRMWARE_REVISION_STRING) \
    X(HARDWARE_REVISION_STRING) \
    X(SOFTWARE_REVISION_STRING) \
    X(MANUFACTURER_NAME_STRING) \
    X(REGULATORY_CERT_DATA_LIST) \
    X(PNP_ID)


std::string GattCharacteristicTypeToString(const GattCharacteristicType v) noexcept {
    switch(v) {
        CHARACTERISTIC_TYPE_ENUM(CASE_TO_STRING)
        default: ; // fall through intended
    }
    return "Unknown";
}

#define CHARACTERISTIC_PROP_ENUM(X) \
        X(Broadcast) \
        X(Read) \
        X(WriteNoAck) \
        X(WriteWithAck) \
        X(Notify) \
        X(Indicate) \
        X(AuthSignedWrite) \
        X(ExtProps) \
        X(ReliableWriteExt) \
        X(AuxWriteExt)

std::string GattCharacteristicPropertyToString(const GattCharacteristicProperty v) noexcept {
    switch(v) {
        CHARACTERISTIC_PROP_ENUM(CASE_TO_STRING)
        default: ; // fall through intended
    }
    return "Unknown";
}

#define REQUIREMENT_SPEC_ENUM(X) \
    X(Excluded) \
    X(Mandatory) \
    X(Optional) \
    X(Conditional) \
    X(if_characteristic_supported) \
    X(if_notify_or_indicate_supported) \
    X(C1)

std::string GattRequirementSpecToString(const GattRequirementSpec v) noexcept {
    switch(v) {
        REQUIREMENT_SPEC_ENUM(CASE_TO_STRING)
        default: ; // fall through intended
    }
    return "Unknown";
}

std::string GattCharacteristicPropertySpec::toString() const noexcept {
    return GattCharacteristicPropertyToString(property)+": "+GattRequirementSpecToString(requirement);
}

std::string GattClientCharacteristicConfigSpec::toString() const noexcept {
    return "ClientCharCfg["+GattRequirementSpecToString(requirement)+"["+read.toString()+", "+writeWithAck.toString()+"]]";
}

std::string GattCharacteristicSpec::toString() const noexcept {
    std::string res = GattCharacteristicTypeToString(characteristic)+": "+GattRequirementSpecToString(requirement)+", Properties[";
    for(size_t i=0; i<propertySpec.size(); i++) {
        if(0<i) {
            res += ", ";
        }
        res += propertySpec.at(i).toString();
    }
    res += "], "+clientConfig.toString();
    return res;
}

std::string GattServiceCharacteristic::toString() const noexcept {
    std::string res = GattServiceTypeToString(service)+": [";
    for(size_t i=0; i<characteristics.size(); i++) {
        if(0<i) {
            res += ", ";
        }
        res += "["+characteristics.at(i).toString()+"]";
    }
    res += "]";
    return res;
}


#endif /* TEST_DATATYPE01_CPP_ */
