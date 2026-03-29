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
// Interface for all parent objects in the descriptor tree.
// Contains an array of child descriptors and implements the
// usb_ms_descriptor_base interface.
//
#ifndef TUPP_USB_PARENT_H
#define TUPP_USB_PARENT_H

#include <array>
#include "usb_config.h"
#include "usb_ms_descriptor_base.h"

class usb_ms_parent : public usb_ms_descriptor_base {
public:

    usb_ms_parent(const uint8_t * descriptor,
                  size_t descriptor_size);

    // No copy, no assignment
    usb_ms_parent(const usb_ms_parent &) = delete;
    usb_ms_parent & operator= (const usb_ms_parent &) = delete;

    void add(usb_ms_descriptor_base & child);

    virtual void update() = 0;

    // Implemented methods from base interface
    void desc_begin() override;
    size_t desc_total_size() override;
    uint8_t desc_getNext() override;

    virtual void set_total_length() = 0;

protected:
    const uint8_t * const _descriptor;
    const size_t          _descriptor_size;

    size_t    _current_index{0};
    size_t    _current_size {0};

    bool      _in_children  {false};
    size_t    _child_index  {0};

    // Child objects
    std::array<usb_ms_descriptor_base *, TUPP_MAX_MS_CHILDREN> _children{};
};

#endif // TUPP_USB_PARENT_H
