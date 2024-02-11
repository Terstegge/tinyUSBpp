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
#ifndef TUPP_USB_MS_HEADER_H
#define TUPP_USB_MS_HEADER_H

// Forward declarations (to prevent
// mutual inclusions of header files)
class usb_bos_dev_cap_platform_ms;
class usb_ms_config_subset;

#include "usb_ms_structs.h"
#include "usb_config.h"
#include <array>

class usb_ms_header {
public:
    explicit usb_ms_header(usb_bos_dev_cap_platform_ms & cap);

    // No copy, no assignment
    usb_ms_header(const usb_ms_header &) = delete;
    usb_ms_header & operator= (const usb_ms_header &) = delete;

    void set_dwWindowsVersion(uint32_t ver) {
        _descriptor.dwWindowsVersion = ver;
    }

    // Add a capability
    void add_ms_config_subset(usb_ms_config_subset * config_subset);

    // Calculate the total length of the BOS descriptor
    void inc_total_length(uint16_t inc);

    // Read-only version of our descriptor
    const USB::ms_header_t & descriptor;

    // Array of pointers to our device capabilities
    std::array<usb_ms_config_subset *, TUPP_MAX_MS_CONFIG_SUBSETS> _config_subsets;

private:
    // The binary object store (BOS) descriptor
    USB::ms_header_t _descriptor;

    // Parent
    usb_bos_dev_cap_platform_ms & _dev_cap_platform_ms;
};

#endif  // TUPP_USB_MS_HEADER_H
