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
#ifndef TUPP_USB_MS_COMPATIBLE_ID_H
#define TUPP_USB_MS_COMPATIBLE_ID_H

#include "usb_config.h"
#include "usb_ms_feature.h"
#include "usb_ms_parent.h"
#include "usb_ms_structs.h"

class usb_ms_compatible_ID : public usb_ms_feature {
public:
    usb_ms_compatible_ID();

    // No copy, no assignment
    usb_ms_compatible_ID(const usb_ms_compatible_ID &) = delete;
    usb_ms_compatible_ID & operator= (const usb_ms_compatible_ID &) = delete;

    void set_compatible_id(const char * str);
    void set_sub_compatible_id(const char * str);

    // Read-only version of our descriptor
    const TUPP::ms_compat_id_header_t & descriptor;

    inline uint8_t * get_descriptor() override {
        return (uint8_t *)&_descriptor;
    }

    inline uint16_t get_descriptor_length() override {
        return _descriptor.wLength;
    }

    inline void set_parent(usb_ms_parent *) override {
    }

private:
    TUPP::ms_compat_id_header_t  _descriptor;
};

#endif  // TUPP_USB_MS_COMPATIBLE_ID_H
