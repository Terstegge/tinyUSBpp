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
#ifndef TUPP_USB_MS_FUNC_SUBSET_H
#define TUPP_USB_MS_FUNC_SUBSET_H

#include "usb_ms_parent.h"
#include "usb_ms_structs.h"

class usb_ms_func_subset : public usb_ms_parent {
public:
    usb_ms_func_subset();

    friend class usb_ms_compat_descriptor;

    // No copy, no assignment
    usb_ms_func_subset(const usb_ms_func_subset &) = delete;
    usb_ms_func_subset & operator= (const usb_ms_func_subset &) = delete;

    // Set first interface number
    inline void set_bFirstInterface(uint8_t i) {
        _descriptor.bFirstInterface = i;
    }

    // Read-only version of our descriptor
    const TUPP::ms_func_subset_header_t & descriptor;

    inline void update() override {
        set_wSubsetLength();
    }

private:
    void set_total_length() override;
    void set_wSubsetLength();
    TUPP::ms_func_subset_header_t _descriptor;
 };

#endif  // TUPP_USB_MS_FUNC_SUBSET_H

