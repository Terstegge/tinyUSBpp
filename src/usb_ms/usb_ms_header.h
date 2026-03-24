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

#include <array>
#include "usb_config.h"
#include "usb_ms_config_subset.h"
#include "usb_ms_feature.h"
#include "usb_ms_parent.h"
#include "usb_ms_structs.h"

class usb_ms_header : public usb_ms_parent, public usb_ms_feature {
public:
    usb_ms_header();

    friend class usb_ms_compat_descriptor;

    // No copy, no assignment
    usb_ms_header(const usb_ms_header &) = delete;
    usb_ms_header & operator= (const usb_ms_header &) = delete;

    inline void set_dwWindowsVersion(uint32_t ver) {
        _descriptor.dwWindowsVersion = ver;
    }

    // Add children
    void add_ms_feature(usb_ms_feature & feature);
    void add_ms_config_subset(usb_ms_config_subset & config_subset);

    // Read-only version of our descriptor
    const TUPP::ms_header_t & descriptor;

    inline void update() override {
        set_total_length();
    }

    inline uint8_t * get_descriptor() override {
        return (uint8_t *)&_descriptor;
    }
    inline uint16_t get_descriptor_length() override {
        return _descriptor.wLength;
    }
    inline void set_parent(usb_ms_parent * p) override {
        _parent = p;
    }

private:
    usb_ms_parent * _parent {nullptr};

    void set_total_length();

    // Child objects
    std::array<usb_ms_feature *,       TUPP_MAX_MS_FEATURES> _features {nullptr};
    std::array<usb_ms_config_subset *, TUPP_MAX_MS_CONFIG_SUBSETS> _config_subsets {nullptr};

    TUPP::ms_header_t _descriptor;
};

#endif  // TUPP_USB_MS_HEADER_H

