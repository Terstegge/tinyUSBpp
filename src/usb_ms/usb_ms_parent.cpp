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
#include "usb_ms_parent.h"

usb_ms_parent::usb_ms_parent(const uint8_t * const descriptor,
                             const size_t descriptor_size)
: _descriptor(descriptor), _descriptor_size(descriptor_size) { }

// Methods from base interface
void usb_ms_parent::desc_begin() {
    _current_index  = 0;
    _current_size   = _descriptor_size;
    _in_children    = false;
}

size_t usb_ms_parent::desc_total_size() {
    size_t res = _descriptor_size;
    for (auto child: _children) {
        if (child) res += child->desc_total_size();
    }
    return res;
}

uint8_t usb_ms_parent::desc_getNext() {
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
    return 0;
}
