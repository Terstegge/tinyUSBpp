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
// (c) 2023 A. Terstegge  (Andreas.Terstegge@gmail.com)
//
#include "usb_device.h"
#include "usb_configuration.h"
#include "usb_interface.h"
#include "usb_interface_association.h"
#include "usb_strings.h"
#include <cassert>

using namespace USB;

usb_configuration::usb_configuration(usb_device & device)
: descriptor(_descriptor), interfaces(_interfaces),
 _descriptor{},           _interfaces{nullptr}, _associations{nullptr}
{
    // Set descriptor length
    _descriptor.bLength = sizeof(configuration_descriptor_t);
    // Set descriptor type
    _descriptor.bDescriptorType = bDescriptorType_t::DESC_CONFIGURATION;
    // Set initial total length
    set_total_length();
    // Add this configuration to our parent device
    device.add_configuration(this);
}

void usb_configuration::set_Description(const char * n) {
    _descriptor.iConfiguration = usb_strings::inst.add_string(n);
}

void usb_configuration::set_total_length() {
    // Start with our own descriptor
    uint16_t len = sizeof(configuration_descriptor_t);
    // Add all interface associations
    for (usb_interface_association *assoc : _associations) {
        // Add all interface associations which are in use
        if (assoc && assoc->descriptor.bInterfaceCount) {
            len += sizeof(interface_association_descriptor_t);
        }
    }
    // Add all interfaces and their endpoints
    for (usb_interface *interface : _interfaces) {
        if (interface) {
            len += interface->get_total_desc_length();
        }
    }
    _descriptor.wTotalLength = len;
}

uint8_t usb_configuration::add_interface(usb_interface * interface) {
    int i = 0;
    for (i = 0; i < TUPP_MAX_INTERF_PER_CONF; ++i) {
        if (!_interfaces[i]) {
             _interfaces[i] = interface;
            break;
        }
    }
    assert(i != 5);
    // Set the interface number in the interface
    interface->_descriptor.bInterfaceNumber = i;
    // Set our own descriptor accordingly
    _descriptor.bNumInterfaces = i + 1;
    set_total_length();
    return i;
}

void usb_configuration::add_interface_association(usb_interface_association * assoc) {
    int i = 0;
    for (i = 0; i < 5; ++i) {
        if (!_associations[i]) {
             _associations[i] = assoc;
            break;
        }
    }
    assert(i != 5);
    set_total_length();
}

void usb_configuration::activate_endpoints(bool b) {
    for (usb_interface *uif: _interfaces) {
        if (uif) uif->activate_endpoints(b);
    }
}
