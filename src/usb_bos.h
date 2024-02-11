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
// This class represents a Binary Object Storage (BOS), which
// can be attached to a USB device. The BOS itself can contain
// a number of device capability descriptors, which are handled
// within this class.
//
#ifndef TUPP_USB_BOS_H
#define TUPP_USB_BOS_H

// Forward declarations (to prevent
// mutual inclusions of header files)
class usb_device;
class usb_bos_dev_cap;

#include "usb_structs.h"
#include "usb_config.h"
#include <array>

class usb_bos {
public:
    explicit usb_bos(usb_device & device);

    // No copy, no assignment
    usb_bos(const usb_bos &) = delete;
    usb_bos & operator= (const usb_bos &) = delete;

    // Add a capability to this BOS
    void add_capability(usb_bos_dev_cap * cap);

    // Calculate the total length of the BOS descriptor
    void set_total_length();

    // Read-only version of our descriptor
    const USB::bos_descriptor_t & descriptor;

    friend class usb_device_controller;

private:
    // The binary object store (BOS) descriptor
    USB::bos_descriptor_t _descriptor;

    // Array of pointers to our device capabilities
    std::array<usb_bos_dev_cap *, TUPP_MAX_BOS_CAPABILITIES> _capabilities;
};

#endif  // TUPP_USB_BOS_H
