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
#include <cassert>
#include "usb_ms_registry_property.h"
#include "usb_ms_parent.h"
#include "usb_strings.h"
#include "usb_log.h"
using enum usb_log::log_level;

usb_ms_registry_property::usb_ms_registry_property(usb_ms_parent & parent) {
    TUPP_LOG(LOG_DEBUG, "usb_ms_registry_property() @%x", this);
    // Set header values
    descriptor()->wDescriptorType = TUPP::wDescriptorType_t::DESC_FEATURE_REG_PROPERTY;
    // Update the descriptor size
    _next_free_byte += sizeof(TUPP::ms_reg_prop_header_t);
    set_wLength();
    // Update parent
    parent.add(this);
}

void usb_ms_registry_property::add_string(const char * s) {
    TUPP_LOG(LOG_DEBUG, "add_string(%s)", s);
    // Prepare the string as UTF16 (PropertyName)
    uint16_t len = usb_strings::inst.convert_to_utf16(s, _next_free_byte + 2);
    // Check buffer
    assert((_next_free_byte + len + 2) < (_desc_buffer + TUPP_MS_REG_PROP_SIZE));
    // Store wPropertyNameLength
    _next_free_byte[0] = len & 0xff;
    _next_free_byte[1] = len >> 8;
    // Update the pointer to next free byte
    _next_free_byte += (len+2);
    // Update descriptor length
    set_wLength();
}

void usb_ms_registry_property::add_end_marker() {
    TUPP_LOG(LOG_DEBUG, "add_end_marker()");
    // Check buffer
    assert((_next_free_byte + 2) < (_desc_buffer + TUPP_MS_REG_PROP_SIZE));
    // Add a terminating 0 at the end of the buffer
    _next_free_byte[0] = 0;
    _next_free_byte[1] = 0;
    // Update the pointer to next free byte
    _next_free_byte += 2;
    // Update descriptor length
    set_wLength();
}

void usb_ms_registry_property::set_wLength() {
    TUPP_LOG(LOG_DEBUG, "set_length(%d)");
    descriptor()->wLength = desc_total_size();
    if (_parent) _parent->update();
}
