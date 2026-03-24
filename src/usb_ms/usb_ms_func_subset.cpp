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
#include <cstdlib>

#include "usb_ms_func_subset.h"
#include "usb_log.h"
using enum usb_log::log_level;

usb_ms_func_subset::usb_ms_func_subset()
: descriptor{_descriptor}, _descriptor{}
{
    TUPP_LOG(LOG_DEBUG, "usb_ms_func_subset() @%x", this);
    // Set header values
    _descriptor.wLength  = sizeof(TUPP::ms_func_subset_header_t);
    _descriptor.wDescriptorType = TUPP::wDescriptorType_t::DESC_SUBSET_FUNCTION;
    set_total_length();
}

// Add a registry property
void usb_ms_func_subset::add_ms_feature(usb_ms_feature & feature) {
    TUPP_LOG(LOG_DEBUG, "add_registry_property()");
    size_t i=0;
    // Find an empty slot
    for (i=0; i < _features.size(); ++i) {
        if (!_features[i]) {
             _features[i] = &feature;
            break;
        }
    }
    assert(i != TUPP_MAX_MS_FEATURES);
    feature.set_parent(this);
    set_total_length();
}

void usb_ms_func_subset::set_total_length() {
    uint16_t res = _descriptor.wLength;
    for (auto feature : _features) {
        if (feature) {
            res += feature->get_descriptor_length();
        }
    }
    _descriptor.wSubsetLength = res;
    if (_parent) _parent->update();
}
