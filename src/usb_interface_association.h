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
// This class represents a USB interface association descriptor,
// which can be added to a USB configuration.
//
#ifndef TUPP_USB_INTERFACE_ASSOCIATION_H
#define TUPP_USB_INTERFACE_ASSOCIATION_H

// Forward declarations
// (to prevent mutual inclusions of header files)
class usb_configuration;
class usb_interface;

#include "usb_structs.h"

class usb_interface_association {
public:
    explicit usb_interface_association(usb_configuration & p);

    // No copy, no assignment
    usb_interface_association(const usb_interface_association &) = delete;
    usb_interface_association & operator= (const usb_interface_association &) = delete;

    // Methods to set the descriptor elements
    inline void set_bFunctionClass(USB::bInterfaceClass_t n) {
        _descriptor.bFunctionClass = n;
    }
    inline void set_bFunctionSubClass(USB::bInterfaceSubClass_t  n) {
        _descriptor.bFunctionSubClass = n;
    }
    inline void set_bFunctionProtocol(USB::bInterfaceProtocol_t n) {
        _descriptor.bFunctionProtocol = n;
    }
    void set_FunctionName(const char *);

    // Add an interface to this interface association.
    // The start index and number of interfaces will be
    // automatically set in the descriptor.
    void add_interface(usb_interface * interface);

    // Getter for the parent. Needed in usb_interface.
    inline usb_configuration & get_parent() {
        return _parent;
    }

    // Read-only version of our descriptor
    const USB::interface_association_descriptor_t & descriptor;

private:
    usb_configuration & _parent;

    // The interface association descriptor
    USB::interface_association_descriptor_t _descriptor;
};

#endif  // TUPP_USB_INTERFACE_ASSOCIATION_H
