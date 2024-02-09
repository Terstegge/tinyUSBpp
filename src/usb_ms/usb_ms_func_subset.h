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
#ifndef TUPP_USB_MS_FUNC_SUBSET_H
#define TUPP_USB_MS_FUNC_SUBSET_H

// Forward declarations (to prevent
// mutual inclusions of header files)
//class usb_ms_header;
class usb_ms_config_subset;
class usb_ms_compatible_ID;
class usb_ms_registry_property;

#include "usb_ms_func_subset.h"
#include "usb_ms_common.h"
#include "usb_config.h"
#include <array>

class usb_ms_func_subset {
public:
    explicit usb_ms_func_subset(usb_ms_config_subset & config_subset);

    // No copy, no assignment
    usb_ms_func_subset(const usb_ms_func_subset &) = delete;
    usb_ms_func_subset & operator= (const usb_ms_func_subset &) = delete;

    // Add a compatibility ID
    void add_compatible_ID(usb_ms_compatible_ID * compat_id);

    // Add a registry property
    void add_registry_property(usb_ms_registry_property * reg_prop);

    // Calculate the total length of the BOS descriptor
    void inc_subset_length(uint16_t inc);

    // Read-only version of our descriptor
    const USB::ms_func_subset_header_t & descriptor;

    // Array of pointers to our device capabilities
    usb_ms_compatible_ID *  _compat_id;
    std::array<usb_ms_registry_property *, TUPP_MAX_MS_REG_PROP> _reg_props;

private:
    // The binary object store (BOS) descriptor
    USB::ms_func_subset_header_t _descriptor;

    // Parent
    usb_ms_config_subset & _config_subset;


};

#endif  // TUPP_USB_MS_FUNC_SUBSET_H
