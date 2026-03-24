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

#include "usb_ms_header.h"
#include "usb_log.h"
using enum usb_log::log_level;

usb_ms_header::usb_ms_header()
: descriptor{_descriptor}, _descriptor{}
{
    TUPP_LOG(LOG_DEBUG, "usb_ms_header() @%x", this);
    // Set header values
    _descriptor.wLength         = sizeof(TUPP::ms_header_t);
    _descriptor.wDescriptorType = TUPP::wDescriptorType_t::DESC_SET_HEADER;
    set_total_length();
}

void usb_ms_header::add_ms_feature(usb_ms_feature & feature) {
    TUPP_LOG(LOG_DEBUG, "add_feature()");
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

// Add a configuration subset
void usb_ms_header::add_ms_config_subset(usb_ms_config_subset & config_subset) {
    TUPP_LOG(LOG_DEBUG, "add_config_subset()");
    size_t i=0;
    // Find an empty slot
    for (i=0; i < _config_subsets.size(); ++i) {
        if (!_config_subsets[i]) {
             _config_subsets[i] = &config_subset;
            break;
        }
    }
    assert(i != TUPP_MAX_MS_CONFIG_SUBSETS);
    config_subset.set_parent(this);
    set_total_length();
}

void usb_ms_header::set_total_length() {
    uint16_t res = _descriptor.wLength;
    for (auto feature : _features) {
        if (feature) {
            res += feature->get_descriptor_length();
        }
    }
    for (auto config_subset : _config_subsets) {
        if (config_subset) {
            res += config_subset->descriptor.wTotalLength;
        }
    }
    _descriptor.wTotalLength = res;
    if (_parent) _parent->update();
}
