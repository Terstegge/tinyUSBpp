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
#ifndef TUPP_USB_MS_HEADER_H
#define TUPP_USB_MS_HEADER_H

#include "usb_ms_parent.h"
#include "usb_ms_structs.h"

class usb_ms_header : public usb_ms_parent {
public:
    usb_ms_header();

    friend class usb_ms_compat_descriptor;

    // No copy, no assignment
    usb_ms_header(const usb_ms_header &) = delete;
    usb_ms_header & operator= (const usb_ms_header &) = delete;

    inline void set_dwWindowsVersion(uint32_t ver) {
        _descriptor.dwWindowsVersion = ver;
    }

    // Read-only version of our descriptor
    const TUPP::ms_header_t & descriptor;

    inline void update() override {
        set_wTotalLength();
    }

private:
    void set_total_length() override;
    void set_wTotalLength();
    TUPP::ms_header_t _descriptor;
};

#endif  // TUPP_USB_MS_HEADER_H
