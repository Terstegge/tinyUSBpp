//    _   _             _    _  _____ ____
//   | | (_)           | |  | |/ ____|  _ \   _     _
//   | |_ _ _ __  _   _| |  | | (___ | |_) |_| |_ _| |_
//   | __| | '_ \| | | | |  | |\___ \|  _ < _   _|_   _|
//   | |_| | | | | |_| | |__| |____) | |_) | |_|   |_|
//    \__|_|_| |_|\__, |\____/|_____/|____/
//                __/ |
//               |___/
//
// This file is part of tinyUSB++, C++ based and easy to
// use library for USB host/device functionality.
// (c) 2023 A. Terstegge  (Andreas.Terstegge@gmail.com)
//
#ifndef TUPP_USB_COMMON_H
#define TUPP_USB_COMMON_H

#include "usb_cdc_defines.h"
#include "usb_msc_defines.h"

#include <cstdint>

namespace USB {

    ///////////////
    // Setup packet
    ///////////////
    enum class bRequest_t : uint8_t  {
        REQ_GET_STATUS          = 0,
        REQ_CLEAR_FEATURE       = 1,
        REQ_SET_FEATURE         = 3,
        REQ_SET_ADDRESS         = 5,
        REQ_GET_DESCRIPTOR      = 6,
        REQ_SET_DESCRIPTOR      = 7,
        REQ_GET_CONFIGURATION   = 8,
        REQ_SET_CONFIGURATION   = 9,
        REQ_GET_INTERFACE       = 10,
        REQ_SET_INTERFACE       = 11,
        REQ_SYNCH_FRAME         = 12,
        CDC_REQUESTS,
        MSC_REQUESTS
    };

    enum class direction_t : uint8_t {
        DIR_OUT                 = 0,
        DIR_IN                  = 1
    };

    enum class type_t : uint8_t {
        TYPE_STANDARD           = 0,
        TYPE_CLASS              = 1,
        TYPE_VENDOR             = 2
    };

    enum class recipient_t : uint8_t {
        REC_DEVICE              = 0,
        REC_INTERFACE           = 1,
        REC_ENDPOINT            = 2,
        REC_OTHER               = 3
    };

    struct __attribute__((__packed__)) setup_packet_t {
        recipient_t         recipient : 5;
        type_t              type      : 2;
        direction_t         direction : 1;
        bRequest_t          bRequest;
        uint16_t            wValue;
        uint16_t            wIndex;
        uint16_t            wLength;
    };
    static_assert(sizeof(setup_packet_t) == 8);

    ////////////////////////
    // USB Device Descriptor
    ////////////////////////
    enum class bDescriptorType_t : uint8_t {
        DESC_DEVICE                = 1,
        DESC_CONFIGURATION         = 2,
        DESC_STRING                = 3,
        DESC_INTERFACE             = 4,
        DESC_ENDPOINT              = 5,
        DESC_DEVICE_QUALIFIER      = 6,
        DESC_OTHER_SPEED_CONFIG    = 7,
        DESC_INTERFACE_POWER       = 8,
        DESC_OTG                   = 9,
        DESC_DEBUG                 = 10,
        DESC_INTERFACE_ASSOCIATION = 11,
        DESC_BOS                   = 15,
        DESC_DEVICE_CAPABILITY     = 16
    };

    enum class bDeviceClass_t : uint8_t {
        DEV_CLASS_APPLICATION_SPECIFIC = 0xFE,
        DEV_CLASS_VENDOR_SPECIFIC      = 0xFF,
        CDC_DEVICE_CLASSES
    };

    struct __attribute__((__packed__)) device_descriptor_t {
        uint8_t             bLength;
        bDescriptorType_t   bDescriptorType;
        uint16_t            bcdUSB;
        bDeviceClass_t      bDeviceClass;
        uint8_t             bDeviceSubClass;
        uint8_t             bDeviceProtocol;
        uint8_t             bMaxPacketSize0;
        uint16_t            idVendor;
        uint16_t            idProduct;
        uint16_t            bcdDevice;
        uint8_t             iManufacturer;
        uint8_t             iProduct;
        uint8_t             iSerialNumber;
        uint8_t             bNumConfigurations;
    };
    static_assert(sizeof(USB::device_descriptor_t) == 18);

    ///////////////////////////////
    // USB Configuration Descriptor
    ///////////////////////////////

    union __attribute__((__packed__)) conf_attr_t {
        struct __attribute__((__packed__)) {
            uint8_t                     : 5;
            uint8_t     remote_wakeup   : 1;
            uint8_t     self_powered    : 1;
            uint8_t     bus_powered     : 1;
        };
        uint8_t val;
    };

    struct __attribute__((__packed__)) configuration_descriptor_t {
        uint8_t             bLength;
        bDescriptorType_t   bDescriptorType;
        uint16_t            wTotalLength;
        uint8_t             bNumInterfaces;
        uint8_t             bConfigurationValue;
        uint8_t             iConfiguration;
        conf_attr_t         bmAttributes;
        uint8_t             bMaxPower;
    };
    static_assert(sizeof(configuration_descriptor_t) == 9);

    ///////////////////////////
    // USB Interface Descriptor
    ///////////////////////////

    enum class bInterfaceClass_t : uint8_t {
        IF_CLASS_APPLICATION_SPECIFIC = 0xFE,
        IF_CLASS_VENDOR_SPECIFIC      = 0xFF,
        CDC_INTERFACE_CLASSES,
        MSC_INTERFACE_CLASSES
    };

    enum class bInterfaceSubClass_t : uint8_t {
        CDC_INTERFACE_SUBCLASSES,
        MSC_INTERFACE_SUBCLASSES
    };

    enum class bInterfaceProtocol_t : uint8_t {
        IF_PROTOCOL_NONE               = 0x00,
        IF_PROTOCOL_FROM_DESCRIPTOR    = 0xfe,
        IF_PROTOCOL_VENDOR_SPECIFIC    = 0xff,
        CDC_INTERFACE_PROTOCOLS,
        MSC_INTERFACE_PROTOCOLS
    };

    struct __attribute__((__packed__)) interface_descriptor_t {
        uint8_t                 bLength;
        bDescriptorType_t       bDescriptorType;
        uint8_t                 bInterfaceNumber;
        uint8_t                 bAlternateSetting;
        uint8_t                 bNumEndpoints;
        bInterfaceClass_t       bInterfaceClass;
        bInterfaceSubClass_t    bInterfaceSubClass;
        bInterfaceProtocol_t    bInterfaceProtocol;
        uint8_t                 iInterface;
    };
    static_assert(sizeof(interface_descriptor_t) == 9);

    ///////////////////////////////////////
    // USB Interface Association Descriptor
    ///////////////////////////////////////

    struct __attribute__((__packed__)) interface_association_descriptor_t {
        uint8_t                 bLength;
        bDescriptorType_t       bDescriptorType;
        uint8_t                 bFirstInterface;
        uint8_t                 bInterfaceCount;
        bInterfaceClass_t       bFunctionClass;
        bInterfaceSubClass_t    bFunctionSubClass;
        bInterfaceProtocol_t    bFunctionProtocol;
        uint8_t                 iFunction;
    };
    static_assert(sizeof(interface_association_descriptor_t) == 8);

    //////////////////////////
    // USB Endpoint Descriptor
    //////////////////////////

    enum class ep_attributes_t : uint8_t {
        TRANS_CONTROL       = 0,
        TRANS_ISOCHRONOUS   = 1,
        TRANS_BULK          = 2,
        TRANS_INTERRUPT     = 3
    };

    struct __attribute__((__packed__)) endpoint_descriptor_t {
        uint8_t             bLength;
        bDescriptorType_t   bDescriptorType;
        uint8_t             bEndpointAddress;
        ep_attributes_t     bmAttributes;
        uint16_t            wMaxPacketSize;
        uint8_t             bInterval;
    };
    static_assert(sizeof(endpoint_descriptor_t) == 7);

    /////////////////////
    // BOS descriptors //
    /////////////////////

    struct __attribute__((__packed__)) bos_descriptor_t {
        uint8_t             bLength;
        bDescriptorType_t   bDescriptorType;
        uint16_t            wTotalLength;
        uint8_t             bNumDeviceCaps;
    };
    static_assert(sizeof(bos_descriptor_t) == 5);

    enum class bDevCapabilityType_t : uint8_t {
        CAP_WIRELESS_USB                = 0x01,
        CAP_USB20_EXTENSION             = 0x02,
        CAP_SUPERSPEED_USB              = 0x03,
        CAP_CONTAINER_ID                = 0x04,
        CAP_PLATFORM                    = 0x05,
        CAP_POWER_DELIVERY              = 0x06,
        CAP_BATTERY_INFO                = 0x07,
        CAP_PD_CONSUMER_PORT            = 0x08,
        CAP_PD_PROVIDER_PORT            = 0x09,
        CAP_SUPERSPEED_PLUS             = 0x0A,
        CAP_PRECESION_TIME_MEASUREMENT  = 0x0B,
        CAP_WIRELESS_USB_EXT            = 0x0C,
        CAP_BILLBOARD                   = 0x0D,
        CAP_AUTHENTICATION              = 0x0E,
        CAP_BILLBOARD_EX                = 0x0F,
        CAP_CONFIGURATION_SUMMARY       = 0x10,
        CAP_FW_STATUS                   = 0x11
    };

    struct __attribute__((__packed__)) dev_capability_t {
        uint8_t                 bLength;
        bDescriptorType_t       bDescriptorType;
        bDevCapabilityType_t    bDevCapabilityType;
    };
    static_assert(sizeof(dev_capability_t) == 3);

    struct __attribute__((__packed__)) dev_cap_platform_t : public dev_capability_t {
        uint8_t                 bReserved;
        uint8_t                 PlatformCapabilityUUID[16];
    };
    static_assert(sizeof(dev_cap_platform_t) == 20);
};

#endif // TUPP_USB_COMMON_H
