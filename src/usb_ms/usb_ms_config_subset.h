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
#ifndef TUPP_USB_MS_CONFIG_SUBSET_H
#define TUPP_USB_MS_CONFIG_SUBSET_H

// Forward declarations (to prevent
// mutual inclusions of header files)
class usb_ms_header;
class usb_ms_func_subset;

#include "usb_ms_common.h"
#include "usb_config.h"
#include <array>

class usb_ms_config_subset {
public:
    explicit usb_ms_config_subset(usb_ms_header & header);

    // No copy, no assignment
    usb_ms_config_subset(const usb_ms_config_subset &) = delete;
    usb_ms_config_subset & operator= (const usb_ms_config_subset &) = delete;

    // Add a capability
    void add_ms_func_subset(usb_ms_func_subset * func_subset);

    // Calculate the total length of the BOS descriptor
    void inc_total_length(uint16_t inc);

    // Read-only version of our descriptor
    const USB::ms_config_subset_header_t & descriptor;

    // Array of pointers to our device capabilities
    std::array<usb_ms_func_subset *, TUPP_MAX_MS_FUNC_SUBSETS> _func_subsets;

private:
    // The binary object store (BOS) descriptor
    USB::ms_config_subset_header_t _descriptor;

    // Parent
    usb_ms_header & _ms_header;
};

#endif  // TUPP_USB_MS_CONFIG_SUBSET_H
