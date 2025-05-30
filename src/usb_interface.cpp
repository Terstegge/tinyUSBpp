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
#include "usb_interface.h"
#include "usb_interface_association.h"
#include "usb_configuration.h"
#include "usb_endpoint.h"
#include "usb_strings.h"
#include "usb_fd_base.h"
#include <cassert>
#include <cstring>

using namespace TUPP;

usb_interface::usb_interface(usb_configuration & conf)
:  descriptor(_descriptor), _parent(conf), _descriptor{},
  _assoc_ptr(nullptr), _fd_ptr(nullptr), _endpoints{nullptr}
{
    TUPP_LOG(LOG_DEBUG, "usb_interface(conf) @%x", this);
    // Set descriptor length
    _descriptor.bLength = sizeof(interface_descriptor_t);
    // Set descriptor type
    _descriptor.bDescriptorType = bDescriptorType_t::DESC_INTERFACE;
    // Add this descriptor to the parent configuration
    _parent.add_interface(this);
}

usb_interface::usb_interface(usb_interface_association & assoc)
:  descriptor(_descriptor), _parent(assoc.get_parent()), _descriptor{},
  _assoc_ptr(nullptr), _fd_ptr(nullptr), _endpoints{nullptr}
{
    TUPP_LOG(LOG_DEBUG, "usb_interface(assoc) @%x", this);
    // Set descriptor length
    _descriptor.bLength = sizeof(interface_descriptor_t);
    // Set descriptor type
    _descriptor.bDescriptorType = bDescriptorType_t::DESC_INTERFACE;
    // Add this descriptor to the interface association
    assoc.add_interface(this);
}

void usb_interface::set_InterfaceName(const char * s) {
    TUPP_LOG(LOG_DEBUG, "set_InterfaceName(%s)", s);
    _descriptor.iInterface = usb_strings::inst.add_string(s);
}

void usb_interface::add_endpoint(usb_endpoint * ep) {
    TUPP_LOG(LOG_DEBUG, "add_endpoint(0x%x)",
             ep->descriptor.bEndpointAddress);
    int i=0;
    for (i=0; i < TUPP_MAX_EP_PER_INTERFACE; ++i) {
        if (!_endpoints[i]) {
             _endpoints[i] = ep;
            break;
        }
    }
    assert(i != TUPP_MAX_EP_PER_INTERFACE);
    _descriptor.bNumEndpoints = i+1;
    _parent.set_total_length();
}

void usb_interface::add_func_descriptor(usb_fd_base * desc) {
    TUPP_LOG(LOG_DEBUG, "add_func_descriptor()");
    if (!_fd_ptr) {
        // First entry
        _fd_ptr = desc;
    } else {
        // Find last functional descriptor
        usb_fd_base *ptr = _fd_ptr;
        while (ptr->next) ptr = ptr->next;
        // and link in this descriptor
        ptr->next = desc;
    }
    _parent.set_total_length();
}

uint16_t usb_interface::get_total_desc_length() {
    TUPP_LOG(LOG_DEBUG, "get_total_desc_length()");
    // Start with size of own descriptor
    uint16_t len = _descriptor.bLength;
    // Add size of all functional descriptors
    usb_fd_base * fd = _fd_ptr;
    while (fd) {
        len += fd->descriptor_length;
        fd = fd->next;
    }
    // Add size of all endpoint descriptors
    for (usb_endpoint * ep : _endpoints) {
        if (ep) {
            len += ep->descriptor.bLength;
        }
    }
    return len;
}

void usb_interface::activate_endpoints(bool b) {
    TUPP_LOG(LOG_DEBUG, "activate_endpoints(%b)", b);
    for (usb_endpoint * ep : _endpoints) {
        if (ep) ep->enable_endpoint(b);
    }
}

uint16_t usb_interface::prepare_descriptor(uint8_t * buffer, uint16_t size) {
    TUPP_LOG(LOG_DEBUG, "prepare_descriptor()");
    uint8_t * tmp_ptr = buffer;
    // Process interface association
    if (_assoc_ptr) {
        memcpy(tmp_ptr, &_assoc_ptr->descriptor, sizeof(interface_association_descriptor_t));
        tmp_ptr += sizeof(interface_association_descriptor_t);
        assert((tmp_ptr-buffer) <= size);
    }
    // Process interface descriptor
    memcpy(tmp_ptr, &_descriptor, sizeof(interface_descriptor_t));
    tmp_ptr += sizeof(interface_descriptor_t);
    assert((tmp_ptr-buffer) <= size);
    // Process functional descriptors
    if (_fd_ptr) {
        // Process Functional Descriptors
        usb_fd_base * fd_ptr = _fd_ptr;
        while(fd_ptr) {
            memcpy(tmp_ptr, fd_ptr->descriptor, fd_ptr->descriptor_length);
            tmp_ptr += fd_ptr->descriptor_length;
            assert((tmp_ptr-buffer) <= size);
            fd_ptr = fd_ptr->next;
        }
    }
    // Process all endpoint descriptors
    for (auto ep : _endpoints) {
        if (ep) {
            memcpy(tmp_ptr, &ep->descriptor, sizeof(endpoint_descriptor_t));
            tmp_ptr += sizeof(endpoint_descriptor_t);
            assert((tmp_ptr-buffer) <= size);
        }
    }
    return tmp_ptr - buffer;
}
