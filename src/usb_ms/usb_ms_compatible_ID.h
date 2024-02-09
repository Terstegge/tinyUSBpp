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
#ifndef TUPP_USB_MS_COMPATIBLE_ID_H
#define TUPP_USB_MS_COMPATIBLE_ID_H

// Forward declarations (to prevent
// mutual inclusions of header files)
class usb_ms_func_subset;

#include "usb_ms_common.h"
#include "usb_config.h"

class usb_ms_compatible_ID {
public:
    explicit usb_ms_compatible_ID(usb_ms_func_subset & func_subset);

    // No copy, no assignment
    usb_ms_compatible_ID(const usb_ms_compatible_ID &) = delete;
    usb_ms_compatible_ID & operator= (const usb_ms_compatible_ID &) = delete;

    void set_compatible_id(const char * str) {
        for(int i=0; i < 8; ++i) {
            _descriptor.CompatibleID[i] = *str ? *str++ : 0;
        }
    }

    void set_sub_compatible_id(const char * str) {
        for(int i=0; i < 8; ++i) {
            _descriptor.SubCompatibleID[i] = *str ? *str++ : 0;
        }
    }

    // Read-only version of our descriptor
    const USB::ms_compat_id_header_t & descriptor;

private:
    USB::ms_compat_id_header_t  _descriptor;

    // Our parent function subset
    usb_ms_func_subset & _func_subset;
};

#endif  // TUPP_USB_MS_COMPATIBLE_ID_H
