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
// (c) 2024 A. Terstegge  (Andreas.Terstegge@gmail.com)
//
#ifndef TUPP_USB_MSC_COMMON_H
#define TUPP_USB_MSC_COMMON_H

#include "usb_structs.h"

namespace USB::MSC {

    ///////////////////////////
    // Command Block Wrapper //
    ///////////////////////////
    enum class direction_t : uint8_t {
        DIR_OUT = 0,
        DIR_IN  = 1
    };

    const uint32_t cbw_signature = 0x43425355;

    struct __attribute__((__packed__)) cbw_t {
        uint32_t    dCBWSignature;
        uint32_t    dCBWTag;
        uint32_t    dCBWDataTransferLength;
        uint8_t                     : 7;
        direction_t direction       : 1;
        uint8_t     bCBWLUN         : 4;
        uint8_t                     : 4;
        uint8_t     bCBWCBLength    : 5;
        uint8_t                     : 3;
        uint8_t     CBWCB[16];
    };
    static_assert(sizeof(cbw_t) == 31);

    ////////////////////////////
    // Command Status Wrapper //
    ////////////////////////////

    const uint32_t csw_signature = 0x53425355;

    enum class csw_status_t : uint8_t {
        CMD_PASSED      = 0,
        CMD_FAILED      = 1,
        PHASE_ERROR     = 2
    };

    struct __attribute__((__packed__)) csw_t {
        uint32_t        dCSWSignature;
        uint32_t        dCSWTag;
        uint32_t        dCSWDataResidue;
        csw_status_t    bCSWStatus;
    };
    static_assert(sizeof(csw_t) == 13);

}   // namespace USB::MSC

#endif  // TUPP_USB_MSC_COMMON_H
