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
#include "usb_ms_parent.h"
#include "usb_ms_structs.h"

class usb_ms_registry_property : public usb_ms_descriptor_base {
public:
    explicit usb_ms_registry_property(usb_ms_parent & parent);

    // No copy, no assignment
    usb_ms_registry_property(const usb_ms_registry_property &) = delete;
    usb_ms_registry_property & operator= (const usb_ms_registry_property &) = delete;

    inline void set_wPropertyDataType(TUPP::wPropertyDataType_t t) {
        descriptor()->wPropertyDataType = t;
    }

    void add_string(const char * name);
    void add_end_marker();

    // Methods from base interface
    inline void desc_begin() override {
        _descriptor_ptr = _desc_buffer;
    }
    inline size_t desc_total_size() override {
        return _next_free_byte - _desc_buffer;
    }
    inline uint8_t desc_getNext() override {
        return *_descriptor_ptr++;
    }

private:
    inline TUPP::ms_reg_prop_header_t * descriptor() {
        return (TUPP::ms_reg_prop_header_t *)_desc_buffer;
    }

    void set_wLength();

    // The buffer to store this descriptor
    uint8_t     _desc_buffer[TUPP_MS_REG_PROP_SIZE] {0};
    uint8_t *   _next_free_byte {_desc_buffer};
    uint8_t *   _descriptor_ptr {nullptr};
};

#endif  // TUPP_USB_MS_REGISTRY_PROPERTY_H
