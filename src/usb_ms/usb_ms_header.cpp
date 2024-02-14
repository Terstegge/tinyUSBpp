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
#include "usb_ms_dev_cap_platform.h"
#include "usb_ms_header.h"
#include <cassert>

usb_ms_header::usb_ms_header(usb_ms_dev_cap_platform & cap)
: descriptor{_descriptor}, _config_subsets{}, _descriptor{}, _dev_cap_platform_ms{cap}
{
    // Set header values
    _descriptor.wLength         = sizeof(USB::ms_header_t);
    _descriptor.wDescriptorType = USB::wDescriptorType_t::DESC_HEADER;
    inc_total_length(sizeof(USB::ms_header_t));
}

// Add a capability
void usb_ms_header::add_ms_config_subset(usb_ms_config_subset * config_subset) {
    int i=0;
    // Find an empty slot
    for (i=0; i < TUPP_MAX_MS_CONFIG_SUBSETS; ++i) {
        if (!_config_subsets[i]) {
             _config_subsets[i] = config_subset;
            break;
        }
    }
    assert(i != TUPP_MAX_MS_CONFIG_SUBSETS);
}

void usb_ms_header::inc_total_length(uint16_t inc) {
    _descriptor.wTotalLength += inc;
    _dev_cap_platform_ms._descriptor.wMSOSDescriptorSetTotalLength += inc;
}
