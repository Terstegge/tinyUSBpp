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
#include "usb_ms_parent.h"
#include "usb_log.h"
using enum usb_log::log_level;

usb_ms_parent::usb_ms_parent(const uint8_t * const descriptor,
                             const size_t descriptor_size)
: _descriptor(descriptor), _descriptor_size(descriptor_size) { }

void usb_ms_parent::add(usb_ms_descriptor_base * child) {
    TUPP_LOG(LOG_DEBUG, "add_feature()");
    size_t i=0;
    // Find an empty slot
    for (i=0; i < _children.size(); ++i) {
        if (!_children[i]) {
            _children[i] = child;
            break;
        }
    }
    assert(i != TUPP_MAX_MS_CHILDREN);
    child->set_parent(this);
    set_total_length();
}

// Methods from base interface
void usb_ms_parent::desc_begin() {
    TUPP_LOG(LOG_DEBUG, "desc_begin()");
    _current_index = 0;
    _current_size = _descriptor_size;
    _in_children = false;
}

size_t usb_ms_parent::desc_total_size() {
    TUPP_LOG(LOG_DEBUG, "desc_total_size()");
    size_t res = _descriptor_size;
    for (auto child: _children) {
        if (child) res += child->desc_total_size();
    }
    return res;
}

uint8_t usb_ms_parent::desc_getNext() {
    TUPP_LOG(LOG_DEBUG, "desc_getNext()");
    if (!_in_children) {
        if (_current_index < _current_size) {
            return _descriptor[_current_index++];
        } else {
            _in_children = true;
            _child_index = 0;
            _current_index = 0;
            _current_size = _children[_child_index]->desc_total_size();
            _children[_child_index]->desc_begin();
            return desc_getNext();
        }
    } else {
        if (_current_index < _current_size) {
            _current_index++;
            return _children[_child_index]->desc_getNext();
        } else {
            _child_index++;
            _current_index = 0;
            _current_size = _children[_child_index]->desc_total_size();
            _children[_child_index]->desc_begin();
            return desc_getNext();
        }
    }
}
