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
#ifndef TUPP_USB_MS_STRUCTS_H
#define TUPP_USB_MS_STRUCTS_H

#include "usb_structs.h"

namespace TUPP {

    struct __attribute__((__packed__)) dev_cap_platform_ms_webusb_t : public dev_cap_platform_t {
        uint16_t                bcdVersion {0};
        bRequest_t              bVendorCode {0};
        uint8_t                 iLandingPage {0};
    };
    static_assert(sizeof(dev_cap_platform_ms_webusb_t) == 24);


    struct __attribute__((__packed__)) dev_cap_platform_ms_os_20_t : public dev_cap_platform_t {
        uint32_t                dwWindowsVersion {0};
        uint16_t                wMSOSDescriptorSetTotalLength {0};
        bRequest_t              bMS_VendorCode {0};
        uint8_t                 bAltEnumCode {0};
    };
    static_assert(sizeof(dev_cap_platform_ms_os_20_t) == 28);

    // Base structure for all MS descriptor types
    /////////////////////////////////////////////
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

};

#endif // TUPP_USB_MS_STRUCTS_H

