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
#include "usb_fd_call_mgmt.h"
#include "usb_interface.h"

usb_fd_call_mgmt::usb_fd_call_mgmt(usb_interface & i)
    : usb_fd_base(i, (uint8_t *)&_descriptor, sizeof(_descriptor) )
{
    TUPP_LOG(LOG_DEBUG, "usb_fd_call_mgmt() @%x", this);
    _descriptor.bLength            = sizeof(_descriptor);
    _descriptor.bDescriptorType    = TUPP::CDC::func_desc_type_t::CS_INTERFACE;
    _descriptor.bDescriptorSubType = TUPP::CDC::func_desc_subtype_t::CDC_FUNC_DESC_CALL_MANAGEMENT;

    // Add this functional descriptor to the parent interface
    _parent.add_func_descriptor(this);
}
