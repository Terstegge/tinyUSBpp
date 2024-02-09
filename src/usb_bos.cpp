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
#include "usb_bos.h"
#include "usb_bos_dev_cap.h"
#include "usb_device.h"
#include <cassert>

using namespace USB;

usb_bos::usb_bos(usb_device & device)
: descriptor(_descriptor), _descriptor{}, _capabilities{nullptr}
{
    // Set descriptor length
    _descriptor.bLength = sizeof(bos_descriptor_t);
    // Set descriptor type
    _descriptor.bDescriptorType = bDescriptorType_t::DESC_BOS;
    // No device capabilities so far...
    _descriptor.bNumDeviceCaps = 0;
    // Update total length
    set_total_length();
    // Add this configuration to our parent device
    device.add_bos(this);
}

void usb_bos::add_capability(usb_bos_dev_cap * cap) {
    int i=0;
    // Find an empty slot
    for (i=0; i < TUPP_MAX_BOS_CAPABILITIES; ++i) {
        if (!_capabilities[i]) {
            _capabilities[i] = cap;
            break;
        }
    }
    assert(i != TUPP_MAX_BOS_CAPABILITIES);
    // Update number of device capabilities
    _descriptor.bNumDeviceCaps = i+1;
    // Update total length
    set_total_length();
}

void usb_bos::set_total_length() {
    // Start with our own descriptor
    uint16_t len = sizeof(bos_descriptor_t);
    for(int i=0; i < _descriptor.bNumDeviceCaps; ++i) {
        len += _capabilities[i]->get_bLength();
    }
    _descriptor.wTotalLength = len;
}
