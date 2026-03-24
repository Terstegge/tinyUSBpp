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
#ifndef TUPP_USB_FEATURE_H
#define TUPP_USB_FEATURE_H

#include <cstdint>
#include "usb_ms_parent.h"

class usb_ms_feature {
public:
    virtual uint8_t * get_descriptor() = 0;
    virtual uint16_t  get_descriptor_length() = 0;
    virtual void      set_parent(usb_ms_parent *) = 0;
};

#endif // TUPP_USB_FEATURE_H
