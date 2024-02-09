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
#ifndef TUPP_USB_MS_REGISTRY_PROPERTY_H
#define TUPP_USB_MS_REGISTRY_PROPERTY_H

// Forward declarations (to prevent
// mutual inclusions of header files)
class usb_ms_func_subset;

#include "usb_ms_common.h"
#include "usb_config.h"

class usb_ms_registry_property {
public:
    explicit usb_ms_registry_property(usb_ms_func_subset & func_subset);

    // No copy, no assignment
    usb_ms_registry_property(const usb_ms_registry_property &) = delete;
    usb_ms_registry_property & operator= (const usb_ms_registry_property &) = delete;

    inline USB::ms_reg_prop_header_t * descriptor() {
        return (USB::ms_reg_prop_header_t *)_desc_buffer;
    }

    void add_property_name (const char * name);
    void add_property_value(const char * value);

    void inc_length(uint16_t inc);

    inline uint16_t get_length() {
        return _next_free_byte - _desc_buffer;
    }

private:
    // Our parent function subset
    usb_ms_func_subset & _func_subset;

    // The buffer to store this descriptor
    uint8_t     _desc_buffer[TUPP_MS_REG_PROP_SIZE];
    uint8_t *   _next_free_byte;
};

#endif  // TUPP_USB_MS_REGISTRY_PROPERTY_H
