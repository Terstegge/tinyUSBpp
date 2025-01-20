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

    enum class wDescriptorType_t : uint16_t {
        DESC_HEADER         = 0,
        DESC_CONFIG_SUBSET  = 1,
        DESC_FUNC_SUBSET    = 2,
        DESC_COMPAT_ID      = 3,
        DESC_REG_PROP       = 4
    };

    struct __attribute__((__packed__)) ms_descriptor_t {
        uint16_t            wLength {0};
        wDescriptorType_t   wDescriptorType {0};
    };

    struct __attribute__((__packed__)) ms_header_t : public ms_descriptor_t {
        uint32_t            dwWindowsVersion {0};
        uint16_t            wTotalLength {0};
    };

    struct __attribute__((__packed__)) ms_config_subset_header_t : public ms_descriptor_t {
        uint8_t             bConfigurationValue {0};
        uint8_t             bReserved {0};
        uint16_t            wTotalLength {0};
    };

    struct __attribute__((__packed__)) ms_func_subset_header_t : public ms_descriptor_t {
        uint8_t             bFirstInterface {0};
        uint8_t             bReserved {0};
        uint16_t            wSubsetLength {0};
    };

    struct __attribute__((__packed__)) ms_compat_id_header_t : public ms_descriptor_t {
        uint8_t             CompatibleID[8] {0};
        uint8_t             SubCompatibleID[8] {0};
    };

    struct __attribute__((__packed__)) ms_reg_prop_header_t : public ms_descriptor_t {
        uint16_t            wPropertyDataType {0};
    };

};

#endif // TUPP_USB_MS_STRUCTS_H

