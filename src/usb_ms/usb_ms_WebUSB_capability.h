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
// This class represents an WebUSB capability descriptor.
//
#ifndef USB_MS_WEBUSB_CAPABILITY_H
#define USB_MS_WEBUSB_CAPABILITY_H

class usb_bos;
#include "usb_ms_parent.h"
#include "usb_ms_structs.h"

class usb_ms_WebUSB_capability : public usb_ms_parent {
public:
    usb_ms_WebUSB_capability(usb_bos & bos, TUPP::URL_FORMAT fmt, const char * url);

    // No copy, no assignment
    usb_ms_WebUSB_capability(const usb_ms_WebUSB_capability &) = delete;
    usb_ms_WebUSB_capability & operator= (const usb_ms_WebUSB_capability &) = delete;

    // Read-only version of our descriptor
    const TUPP::dev_cap_platform_ms_webusb_t & descriptor;

private:
    // Unused methods from usb_ms_parent
    inline void update() override { }
    inline void set_total_length() override { }

    TUPP::dev_cap_platform_ms_webusb_t _descriptor;
};

#endif // USB_MS_WEBUSB_CAPABILITY_H
