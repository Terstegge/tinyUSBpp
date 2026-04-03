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
// (c) A. Terstegge  (Andreas.Terstegge@gmail.com)
//
// Descriptor structures and enums for the
// MS OS 20.0 descriptor tree.
//
#ifndef TUPP_USB_MS_STRUCTS_H
#define TUPP_USB_MS_STRUCTS_H

#include "usb_structs.h"

namespace TUPP {

    // Constants
    ////////////

    // Code for Windows version 8.1 and newer
    constexpr uint32_t WIN_VER_81 = 0x06030000;

    constexpr uint8_t UUID_ms_os_20[16] =
    // Old MS OS 1.0 UUID {d8dd60df-4589-4cc7-9cd2-659d9e648a9f}
            { 0xDF, 0x60, 0xDD, 0xD8, 0x89, 0x45, 0xC7, 0x4C,
              0x9C, 0xD2, 0x65, 0x9D, 0x9E, 0x64, 0x8A, 0x9F };

    // Platform capability UUID {3408B638-09A9-47A0-8BFD-A0768815B665}
    // Indicates that this device supports the WebUSB descriptor
    constexpr uint8_t UUID_webusb[16] =
            { 0x38, 0xB6, 0x08, 0x34, 0xA9, 0x09, 0xA0, 0x47,
              0x8B, 0xFD, 0xA0, 0x76, 0x88, 0x15, 0xB6, 0x65 };

    // Vendor codes
    constexpr bRequest_t GET_MS_OS20_DESC = ((TUPP::bRequest_t)0x1);
    constexpr bRequest_t GET_MS_WEBUSB    = ((TUPP::bRequest_t)0x2);

    // wIndex values
    constexpr uint8_t MS_OS20_INDEX = 7;
    constexpr uint8_t WEBUSB_INDEX  = 2;

    // Device capability MS OS 2.0 descriptor
    /////////////////////////////////////////

    struct __attribute__((__packed__)) dev_cap_platform_ms_os_20_t : public dev_cap_platform_t {
        uint32_t    dwWindowsVersion {0};
        uint16_t    wMSOSDescriptorSetTotalLength {0};
        bRequest_t  bMS_VendorCode {0};
        uint8_t     bAltEnumCode {0};
    };
    static_assert(sizeof(dev_cap_platform_ms_os_20_t) == 28);

    // Device capability WebUSB descriptor
    //////////////////////////////////////

    struct __attribute__((__packed__)) dev_cap_platform_ms_webusb_t : public dev_cap_platform_t {
        uint16_t    bcdVersion {0};
        bRequest_t  bVendorCode {0};
        uint8_t     iLandingPage {0};
    };
    static_assert(sizeof(dev_cap_platform_ms_webusb_t) == 24);


    // Common header for all MS OS 2.0 descriptors
    //////////////////////////////////////////////
    enum class wDescriptorType_t : uint16_t {
        DESC_SET_HEADER             = 0,
        DESC_SUBSET_CONFIG          = 1,
        DESC_SUBSET_FUNCTION        = 2,
        DESC_FEATURE_COMPAT_ID      = 3,
        DESC_FEATURE_REG_PROPERTY   = 4,
        DESC_FEATURE_MIN_RESUME_TIME= 5,
        DESC_FEATURE_MODEL_ID       = 6,
        DESC_FEATURE_CCGP_DEVICE    = 7,
        DESC_FEATURE_VENDOR_REVISION= 8
    };

    // Common header for all MS OS 2.0 descriptors
    //////////////////////////////////////////////
    struct __attribute__((__packed__)) ms_descriptor_t {
        uint16_t            wLength {0};
        wDescriptorType_t   wDescriptorType {0};
    };

    // Microsoft OS 2.0 Descriptor Set Header
    /////////////////////////////////////////
    struct __attribute__((__packed__)) ms_header_t : public ms_descriptor_t {
        uint32_t            dwWindowsVersion {0};
        uint16_t            wTotalLength {0};
    };

    // Microsoft OS 2.0 Configuration Subset Header
    ///////////////////////////////////////////////
    struct __attribute__((__packed__)) ms_config_subset_header_t : public ms_descriptor_t {
        uint8_t             bConfigurationValue {0};
        uint8_t             bReserved {0};
        uint16_t            wTotalLength {0};
    };

    // Microsoft OS 2.0 Functional Subset Header
    ////////////////////////////////////////////
    struct __attribute__((__packed__)) ms_func_subset_header_t : public ms_descriptor_t {
        uint8_t             bFirstInterface {0};
        uint8_t             bReserved {0};
        uint16_t            wSubsetLength {0};
    };

    // Microsoft OS 2.0 Compatible ID Descriptor
    ////////////////////////////////////////////
    struct __attribute__((__packed__)) ms_compat_id_header_t : public ms_descriptor_t {
        uint8_t             CompatibleID[8] {0};
        uint8_t             SubCompatibleID[8] {0};
    };

    // Microsoft OS 2.0 Registry Property Descriptor
    ////////////////////////////////////////////////
    enum class wPropertyDataType_t : uint16_t {
        REG_SZ                  = 1, // NULL-terminated Unicode String
        REG_EXPAND_SZ           = 2, // REG_SZ including env variables
        REG_BINARY              = 3, // Free-form binary
        REG_DWORD_LITTLE_ENDIAN = 4, // Little-endian 32-bit integer
        REG_DWORD_BIG_ENDIAN    = 5, // Big-endian 32-bit integer
        REG_LINK                = 6, // REG_SZ containing a symbolic link
        REG_MULTI_SZ            = 7  // Multiple REG_SZ entries
    };

    struct __attribute__((__packed__)) ms_reg_prop_header_t : public ms_descriptor_t {
        wPropertyDataType_t wPropertyDataType {0};
    };
    static_assert( sizeof(ms_reg_prop_header_t) ==  6);

    // URL format for WebUSB
    enum class URL_FORMAT : uint8_t {
        URL_HTTP  = 0x00,
        URL_HTTPS = 0x01,
        URL_FULL  = 0xff
    };
};

#endif // TUPP_USB_MS_STRUCTS_H

