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
#include "usb_ms_func_subset.h"
#include "usb_ms_config_subset.h"

using enum usb_log::log_level;

usb_ms_func_subset::usb_ms_func_subset(usb_ms_config_subset & config_subset)
: descriptor{_descriptor}, _compat_id{nullptr}, _reg_props{}, _descriptor{}, _config_subset{config_subset}
{
    TUPP_LOG(LOG_DEBUG, "usb_ms_func_subset() @%x", this);
    // Set header values
    _descriptor.wLength             = sizeof(TUPP::ms_func_subset_header_t);
    _descriptor.wDescriptorType     = TUPP::wDescriptorType_t::DESC_FUNC_SUBSET;
    _descriptor.bFirstInterface     = 1;
    _descriptor.bReserved           = 0;
    inc_subset_length(sizeof(TUPP::ms_func_subset_header_t));
    _config_subset.add_ms_func_subset(this);
}

// Add a compatibility ID
void usb_ms_func_subset::add_compatible_ID(usb_ms_compatible_ID * compat_id) {
    TUPP_LOG(LOG_DEBUG, "add_compatible_ID()");
    assert(!_compat_id);
    _compat_id = compat_id;
    inc_subset_length(sizeof(TUPP::ms_compat_id_header_t));
}

// Add a registry property
void usb_ms_func_subset::add_registry_property(usb_ms_registry_property * reg_prop) {
    TUPP_LOG(LOG_DEBUG, "add_registry_property()");
    int i=0;
    // Find an empty slot
    for (i=0; i < TUPP_MAX_MS_REG_PROP; ++i) {
        if (!_reg_props[i]) {
             _reg_props[i] = reg_prop;
            inc_subset_length(sizeof(TUPP::ms_reg_prop_header_t));
            break;
        }
    }
    assert(i != TUPP_MAX_MS_REG_PROP);
}

void usb_ms_func_subset::inc_subset_length(uint16_t inc) {
    TUPP_LOG(LOG_DEBUG, "inc_subset_length(%d)", inc);
    _descriptor.wSubsetLength += inc;
    _config_subset.inc_total_length(inc);
}
