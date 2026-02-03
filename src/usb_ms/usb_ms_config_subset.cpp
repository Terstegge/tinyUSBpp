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

#include "usb_log.h"
#include "usb_ms_config_subset.h"
#include "usb_ms_header.h"

using enum usb_log::log_level;

usb_ms_config_subset::usb_ms_config_subset(usb_ms_header & header)
: descriptor{_descriptor}, _func_subsets{}, _descriptor{}, _ms_header{header}
{
    TUPP_LOG(LOG_DEBUG, "usb_ms_config_subset() @%x", this);
    // Set header values
    _descriptor.wLength             = sizeof(TUPP::ms_config_subset_header_t);
    _descriptor.wDescriptorType     = TUPP::wDescriptorType_t::DESC_CONFIG_SUBSET;
    _descriptor.bConfigurationValue = 0;
    _descriptor.bReserved           = 0;
    inc_total_length(sizeof(TUPP::ms_config_subset_header_t));
    _ms_header.add_ms_config_subset(this);
}

// Add a capability
void usb_ms_config_subset::add_ms_func_subset(usb_ms_func_subset * config_subset) {
    TUPP_LOG(LOG_DEBUG, "add_ms_func_subset()");
    int i=0;
    // Find an empty slot
    for (i=0; i < TUPP_MAX_MS_FUNC_SUBSETS; ++i) {
        if (!_func_subsets[i]) {
             _func_subsets[i] = config_subset;
            break;
        }
    }
    assert(i != TUPP_MAX_MS_FUNC_SUBSETS);
}

void usb_ms_config_subset::inc_total_length(uint16_t inc) {
    TUPP_LOG(LOG_DEBUG, "inc_total_length(%d)", inc);
    _descriptor.wTotalLength += inc;
    _ms_header.inc_total_length(inc);
}
