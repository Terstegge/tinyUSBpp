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
// Interface for all features in a descriptor tree
//
#ifndef TUPP_USB_MS_DESCRIPTOR_BASE_H
#define TUPP_USB_MS_DESCRIPTOR_BASE_H

#include <cstdint>
#include <cstddef>
//#include "usb_ms_parent.h"
class usb_ms_parent;

class usb_ms_descriptor_base {
public:
    // Set the parent of this descriptor
    inline void set_parent(usb_ms_parent * p) {
        _parent = p;
    }

    // Iterator interface to access the descriptor
    // (including all sub-descriptors)
    virtual void    desc_begin() = 0;
    virtual size_t  desc_total_size() = 0;
    virtual uint8_t desc_getNext() = 0;

protected:
    usb_ms_parent *_parent {nullptr};
};

#endif // TUPP_USB_MS_DESCRIPTOR_BASE_H
