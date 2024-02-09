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
// (c) 2024 A. Terstegge  (Andreas.Terstegge@gmail.com)
//
#include "usb_fd_header.h"
#include "usb_interface.h"
using namespace USB::CDC;

usb_fd_header::usb_fd_header(usb_interface & i)
    : usb_fd_base(i, (uint8_t *)&_descriptor, sizeof(_descriptor) )
{
    TUPP_LOG(LOG_DEBUG, "usb_fd_header() @%x", this);
    _descriptor.bLength            = sizeof(_descriptor);
    _descriptor.bDescriptorType    = func_desc_type_t::CS_INTERFACE;
    _descriptor.bDescriptorSubType = func_desc_subtype_t::CDC_FUNC_DESC_HEADER;

    // Add this functional descriptor to the parent interface
    _parent.add_func_descriptor(this);
}
