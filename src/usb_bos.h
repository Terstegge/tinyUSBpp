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
class usb_device_controller;
//class usb_bos_dev_cap;
class usb_ms_descriptor_base;
class usb_ms_header;

#include <array>
#include "usb_structs.h"
#include "usb_ms_structs.h"
#include "usb_config.h"

class usb_bos {
public:
    usb_bos(usb_device_controller & controller, usb_device & device);

    // No copy, no assignment
    usb_bos(const usb_bos &) = delete;
    usb_bos & operator= (const usb_bos &) = delete;

    // Add a capability to this BOS
    void add_capability(usb_ms_descriptor_base & cap);

    // Calculate the total length of the BOS descriptor
    void set_total_length();

    // Read-only version of our descriptor
    const TUPP::bos_descriptor_t & descriptor;

    // Prepare the complete BOS descriptor with all device capabilities.
    uint16_t prepare_descriptor(uint8_t * buffer, uint16_t size) const;

    // Set the MS OS 2.0 header
    inline void set_ms_header(usb_ms_header * h) {
        _ms_header = h;
    }

    // Set the URL format
    inline void set_url_format(TUPP::URL_FORMAT f) {
        _url_format = f;
    }

private:
    // The binary object store (BOS) descriptor
    TUPP::bos_descriptor_t _descriptor;

    // Array of pointers to our device capabilities
    std::array<usb_ms_descriptor_base *, TUPP_MAX_BOS_CAPABILITIES> _capabilities;

    // Buffer for preparation of descriptors
    uint8_t _buffer[256] {0};

    // Pointer to MS OS 2.0 header
    usb_ms_header * _ms_header {nullptr};

    // The WebUSB URL format
    TUPP::URL_FORMAT _url_format {TUPP::URL_FORMAT::URL_FULL};
};

#endif  // TUPP_USB_BOS_H

