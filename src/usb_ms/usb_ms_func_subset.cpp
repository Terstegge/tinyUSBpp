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
#include "usb_ms_func_subset.h"
#include "usb_log.h"
using enum usb_log::log_level;

usb_ms_func_subset::usb_ms_func_subset(usb_ms_parent & parent)
: usb_ms_parent((uint8_t *)&_descriptor, sizeof (_descriptor)),
  descriptor{_descriptor}
{
    TUPP_LOG(LOG_DEBUG, "usb_ms_func_subset() @%x", this);
    // Set header values
    _descriptor.wLength = sizeof(TUPP::ms_func_subset_header_t);
    _descriptor.wDescriptorType = TUPP::wDescriptorType_t::DESC_SUBSET_FUNCTION;
    set_wSubsetLength();
    // Update parent
    parent.add(this);
}

void usb_ms_func_subset::set_total_length() {
    set_wSubsetLength();
}

void usb_ms_func_subset::set_wSubsetLength() {
    _descriptor.wSubsetLength = desc_total_size();
    if (_parent) _parent->update();
}
