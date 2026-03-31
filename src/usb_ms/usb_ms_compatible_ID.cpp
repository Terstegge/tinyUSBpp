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
#include "usb_ms_compatible_ID.h"
#include "usb_log.h"
using enum usb_log::log_level;

usb_ms_compatible_ID::usb_ms_compatible_ID(usb_ms_parent & parent)
: descriptor(_descriptor)
{
    TUPP_LOG(LOG_DEBUG, "usb_ms_compatible_ID() @%x", this);
    // Set header values
    _descriptor.wLength         = sizeof(TUPP::ms_compat_id_header_t);
    _descriptor.wDescriptorType = TUPP::wDescriptorType_t::DESC_FEATURE_COMPAT_ID;
    // Update parent
    parent.add(this);
}

void usb_ms_compatible_ID::set_compatible_id(const char * str) {
    TUPP_LOG(LOG_DEBUG, "set_compatible_id(%s)", str);
    for(unsigned char & i : _descriptor.CompatibleID) {
        i = *str ? *str++ : 0;
    }
}

void usb_ms_compatible_ID::set_sub_compatible_id(const char * str) {
    TUPP_LOG(LOG_DEBUG, "set_sub_compatible_id(%s)", str);
    for(unsigned char & i : _descriptor.SubCompatibleID) {
        i = *str ? *str++ : 0;
    }
}
