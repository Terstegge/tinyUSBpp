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
#include "usb_interface.h"
#include "usb_interface_association.h"
#include "usb_configuration.h"
#include "usb_strings.h"
#include "usb_log.h"
using namespace USB;

usb_interface_association::usb_interface_association(usb_configuration & p)
    : descriptor(_descriptor), _parent(p), _descriptor{}
{
    TUPP_LOG(LOG_DEBUG, "usb_interface_association() @%x", this);
    // Set descriptor length
    _descriptor.bLength = sizeof(interface_association_descriptor_t);
    // Set descriptor type
    _descriptor.bDescriptorType = bDescriptorType_t::DESC_INTERFACE_ASSOCIATION;
    // Add this descriptor to the parent configuration
    _parent.add_interface_association(this);
}

void usb_interface_association::set_FunctionName(const char * n) {
    TUPP_LOG(LOG_DEBUG, "set_FunctionName(%s)", n);
    _descriptor.iFunction = usb_strings::inst.add_string(n);
}

void usb_interface_association::add_interface(usb_interface * interface) {
    TUPP_LOG(LOG_DEBUG, "add_interface(%d)",
             interface->descriptor.bInterfaceNumber);
    uint8_t index = _parent.add_interface(interface);
    if (!descriptor.bInterfaceCount) {
        // First interface for this association, so store
        // the index in the descriptor and a pointer to
        // this association in the interface.
        _descriptor.bFirstInterface = index;
        interface->_assoc_ptr = this;
    }
    // Increment the interface count in any case.
    _descriptor.bInterfaceCount++;
}
