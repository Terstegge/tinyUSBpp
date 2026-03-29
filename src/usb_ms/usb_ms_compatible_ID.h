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

#include "usb_ms_descriptor_base.h"
#include "usb_ms_structs.h"

class usb_ms_compatible_ID : public usb_ms_descriptor_base {
public:
    usb_ms_compatible_ID();

    // No copy, no assignment
    usb_ms_compatible_ID(const usb_ms_compatible_ID &) = delete;
    usb_ms_compatible_ID & operator= (const usb_ms_compatible_ID &) = delete;

    void set_compatible_id(const char * str);
    void set_sub_compatible_id(const char * str);

    // Read-only version of our descriptor
    const TUPP::ms_compat_id_header_t & descriptor;

    inline void desc_begin() override {
        _descriptor_ptr = (uint8_t *)&_descriptor;
    }
    inline size_t desc_total_size() override {
        return sizeof(TUPP::ms_compat_id_header_t );
    }
    inline uint8_t desc_getNext() override {
        return *_descriptor_ptr++;
    }

private:
    TUPP::ms_compat_id_header_t _descriptor;
    uint8_t * _descriptor_ptr {nullptr};
};

#endif  // TUPP_USB_MS_COMPATIBLE_ID_H
