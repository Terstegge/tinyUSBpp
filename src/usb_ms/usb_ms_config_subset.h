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
#ifndef TUPP_USB_MS_CONFIG_SUBSET_H
#define TUPP_USB_MS_CONFIG_SUBSET_H

#include <array>
#include "usb_ms_parent.h"
#include "usb_ms_structs.h"

class usb_ms_config_subset : public usb_ms_parent {
public:
    usb_ms_config_subset();

    friend class usb_ms_compat_descriptor;

    // No copy, no assignment
    usb_ms_config_subset(const usb_ms_config_subset &) = delete;
    usb_ms_config_subset & operator= (const usb_ms_config_subset &) = delete;

    // Set configuration value
    // Note: Although the configuration numbering is starting from 1,
    // this value is starting from 0 (so one less than the configuration number!)
    inline void set_bConfigurationValue(uint8_t v) {
        _descriptor.bConfigurationValue = v - 1;
    }

    // Read-only version of our descriptor
    const TUPP::ms_config_subset_header_t & descriptor;

    // Methods from base interfaces
    inline void update() override {
        set_wTotalLength();
    }

private:
    void set_total_length() override;
    void set_wTotalLength();
    TUPP::ms_config_subset_header_t _descriptor;
};

#endif  // TUPP_USB_MS_CONFIG_SUBSET_H
