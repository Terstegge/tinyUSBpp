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
#include "usb_ms_registry_property.h"
#include "usb_ms_func_subset.h"
#include "usb_strings.h"

using namespace USB;

#define REG_MULTI_SZ 7

usb_ms_registry_property::usb_ms_registry_property(usb_ms_func_subset & func_subset)
: _func_subset(func_subset), _desc_buffer{}, _next_free_byte{_desc_buffer}
{
    // Set header values
    descriptor()->wLength           = sizeof(USB::ms_reg_prop_header_t);
    descriptor()->wDescriptorType   = USB::wDescriptorType_t::DESC_REG_PROP;
    descriptor()->wPropertyDataType = REG_MULTI_SZ;
    _next_free_byte += sizeof(USB::ms_reg_prop_header_t);
    _func_subset.add_registry_property(this);
}

void usb_ms_registry_property::add_property_name(const char * name) {
    // Prepare the string as UTF16 (PropertyName)
    uint16_t len = usb_strings::inst.convert_to_utf16(name, _next_free_byte + 2);
    // Store wPropertyNameLength
    _next_free_byte[0] = len & 0xff;
    _next_free_byte[1] = len >> 8;
    // Update the pointer to next free byte
    _next_free_byte += (len+2);
    // Update descriptor length
    inc_length(len+2);
}

void usb_ms_registry_property::add_property_value(const char * value) {
    // Prepare the string as UTF16
    usb_strings::inst.convert_to_utf16(value, _next_free_byte + 2);
    // Store the length
    _next_free_byte[0] = 0x50;
    _next_free_byte[1] = 0x00;
    // Update the pointer to next free byte
    _next_free_byte += (0x50 + 2);
    // Update descriptor length
    inc_length(0x50 + 2);
}

void usb_ms_registry_property::inc_length(uint16_t inc) {
    descriptor()->wLength += inc;
    _func_subset.inc_subset_length(inc);
}
