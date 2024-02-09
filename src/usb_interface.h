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
#ifndef TUPP_USB_INTERFACE_H
#define TUPP_USB_INTERFACE_H

// Forward declarations (to prevent
// mutual inclusions of header files)
class usb_configuration;
class usb_interface_association;
class usb_endpoint;
class usb_fd_base;

#include "usb_common.h"
#include "usb_config.h"
#include "usb_log.h"
#include <array>
#include <functional>

class usb_interface {
public:
    explicit usb_interface(usb_configuration & p);
    explicit usb_interface(usb_interface_association & p);

    // No copy, no assignment
    usb_interface(const usb_interface &) = delete;
    usb_interface & operator= (const usb_interface &) = delete;

    // Our friends
    friend class usb_configuration;         // may change our descriptor
    friend class usb_interface_association; // may set our _assoc_ptr
    friend class usb_device_controller;     // may access _assoc_ptr, _fd_ptr, _endpoints

    // Methods to set the descriptor elements
    inline void set_bInterfaceNumber(uint8_t n) {
        TUPP_LOG(LOG_DEBUG, "set_bInterfaceNumber(%d)", n);
        _descriptor.bInterfaceNumber = n;
    }
    inline void set_bAlternateSetting(uint8_t n) {
        TUPP_LOG(LOG_DEBUG, "set_bAlternateSetting(%d)", n);
        _descriptor.bAlternateSetting = n;
    }
    inline void set_bInterfaceClass(USB::bInterfaceClass_t n) {
        TUPP_LOG(LOG_DEBUG, "set_bInterfaceClass(%d)", n);
        _descriptor.bInterfaceClass = n;
    }
    inline void set_bInterfaceSubClass(USB::bInterfaceSubClass_t n) {
        TUPP_LOG(LOG_DEBUG, "set_bInterfaceSubClass(%d)", n);
        _descriptor.bInterfaceSubClass = n;
    }
    inline void set_bInterfaceProtocol(USB::bInterfaceProtocol_t n) {
        TUPP_LOG(LOG_DEBUG, "set_bInterfaceProtocol(%d)", n);
        _descriptor.bInterfaceProtocol = n;
    }
    void set_InterfaceName(const char * s);

    // Add an endpoint to this interface
    void add_endpoint(usb_endpoint * ep);

    // Add a functional descriptor to this interface
    void add_func_descriptor(usb_fd_base * desc);

    // Calculate the total length of the interface descriptor,
    // including all functional descriptors and endpoints.
    uint16_t get_total_desc_length();

    // (De)-Activate all endpoints in this interface
    void activate_endpoints(bool b);

    // Read-only version of our descriptor
    const USB::interface_descriptor_t & descriptor;

    // The setup message handler which handles all
    // commands directed to this interface. Will be
    // called by the usb_device_controller.
    std::function<void(USB::setup_packet_t * packet)> setup_handler;

private:
    // Reference to parent configuration object
    usb_configuration & _parent;

    // The interface descriptor
    USB::interface_descriptor_t _descriptor;

    // Pointer to an interface association which
    // this interface belongs to. Only the first
    // interface in the association will have this
    // pointer set!
    usb_interface_association * _assoc_ptr;

    // Pointer to one or more functional descriptor, if existing.
    // Functional descriptors are stored as a simple single-linked
    // list, and this is the 'head'-pointer.
    usb_fd_base * _fd_ptr;

    // Array of pointers to our endpoints
    std::array<usb_endpoint *, TUPP_MAX_EP_PER_INTERFACE> _endpoints;
};

#endif  // TUPP_USB_INTERFACE_H
