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
// This class represents a USB configuration
//
#ifndef TUPP_USB_CONFIGURATION_H
#define TUPP_USB_CONFIGURATION_H

// Forward declarations (to prevent
// mutual inclusions of header files)
class usb_device;
class usb_interface;
class usb_interface_association;

#include "usb_structs.h"
#include "usb_config.h"
#include <array>

class usb_configuration {
public:
    explicit usb_configuration(usb_device & device);

    // No copy, no assignment
    usb_configuration(const usb_configuration &) = delete;
    usb_configuration & operator= (const usb_configuration &) = delete;

    // Methods to modify the device descriptor
    inline void set_bConfigurationValue(uint8_t n) {
        _descriptor.bConfigurationValue = n;
    }
    inline void set_bmAttributes(TUPP::conf_attr_t n) {
        _descriptor.bmAttributes = n;
    }
    inline void set_bMaxPower_mA(uint8_t n) {
        _descriptor.bMaxPower = n/2;
    }
    void set_Description(const char *);
    void set_remote_wakeup(bool b);

    // Calculate the total length of our configuration
    // descriptor and modify our descriptor accordingly
    void set_total_length();

    // Read-only version of our descriptor
    const TUPP::configuration_descriptor_t & descriptor;

    // Read-only version of our interfaces
    const std::array<usb_interface *, TUPP_MAX_INTERF_PER_CONF> & interfaces;

    // (De-)Activate all endpoints in this configuration
    void activate_endpoints(bool b);

    // Add an interface to this configuration.
    // Return the new index of the interface
    uint8_t add_interface(usb_interface * interface);

    // Add an interface to this configuration
    void add_interface_association(usb_interface_association * function);

private:
    // The configuration descriptor
    TUPP::configuration_descriptor_t _descriptor;

    // Array of pointers to our interfaces
    std::array<usb_interface *, TUPP_MAX_INTERF_PER_CONF> _interfaces;

    // Array of pointers to our interface associations
    std::array<usb_interface_association *, TUPP_MAX_ASSOC_PER_CONF> _associations;
};

#endif  // TUPP_USB_CONFIGURATION_H

