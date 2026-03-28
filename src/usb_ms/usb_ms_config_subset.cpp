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

#include "usb_ms_config_subset.h"
#include "usb_log.h"
using enum usb_log::log_level;

usb_ms_config_subset::usb_ms_config_subset()
: usb_ms_parent((uint8_t *)&_descriptor, sizeof(_descriptor)),
  descriptor{_descriptor}
{
    TUPP_LOG(LOG_DEBUG, "usb_ms_config_subset() @%x", this);
    // Set header values
    _descriptor.wLength         = sizeof(TUPP::ms_config_subset_header_t);
    _descriptor.wDescriptorType = TUPP::wDescriptorType_t::DESC_SUBSET_CONFIG;
    set_total_length();
}

void usb_ms_config_subset::add(usb_ms_descriptor_base & child) {
    TUPP_LOG(LOG_DEBUG, "add()");
    size_t i=0;
    // Find an empty slot
    for (i=0; i < _children.size(); ++i) {
        if (!_children[i]) {
            _children[i] = &child;
            break;
        }
    }
    assert(i != TUPP_MAX_MS_CHILDREN);
    child.set_parent(this);
    set_total_length();
}

void usb_ms_config_subset::set_total_length() {
    uint16_t res = _descriptor.wLength;
    for (auto child : _children) {
        if (child) {
            res += child->desc_total_size();
        }
    }
    _descriptor.wTotalLength = res;
    if (_parent) _parent->update();
}
