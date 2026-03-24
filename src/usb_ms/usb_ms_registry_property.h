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
#ifndef TUPP_USB_MS_REGISTRY_PROPERTY_H
#define TUPP_USB_MS_REGISTRY_PROPERTY_H

#include "usb_config.h"
#include "usb_ms_feature.h"
#include "usb_ms_func_subset.h"
#include "usb_ms_parent.h"
#include "usb_ms_structs.h"

class usb_ms_registry_property : public usb_ms_feature {
public:
    usb_ms_registry_property();

    // No copy, no assignment
    usb_ms_registry_property(const usb_ms_registry_property &) = delete;
    usb_ms_registry_property & operator= (const usb_ms_registry_property &) = delete;

    void add_string(const char * name);
    void add_end_marker();

    inline uint8_t * get_descriptor() override {
        return _desc_buffer;
    }
    inline uint16_t get_descriptor_length() override {
        return _next_free_byte - _desc_buffer;
    }
    inline void set_parent(usb_ms_parent * p) override {
        _parent = p;
    }

private:
    usb_ms_parent * _parent {nullptr};

    inline TUPP::ms_reg_prop_header_t * descriptor() {
        return (TUPP::ms_reg_prop_header_t *)_desc_buffer;
    }

    void set_length();

    // The buffer to store this descriptor
    uint8_t     _desc_buffer[TUPP_MS_REG_PROP_SIZE] {0};
    uint8_t *   _next_free_byte {_desc_buffer};
};

#endif  // TUPP_USB_MS_REGISTRY_PROPERTY_H

