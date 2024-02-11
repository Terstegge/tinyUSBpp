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
#ifndef TUPP_USB_DEVICE_H
#define TUPP_USB_DEVICE_H

class usb_configuration;
class usb_bos;
#include "usb_structs.h"
#include "usb_config.h"
#include <array>
#include <functional>

class usb_device {
public:
    usb_device();

    // No copy, no assignment
    usb_device(const usb_device &) = delete;
    usb_device & operator= (const usb_device &) = delete;

    // Methods to modify the device descriptor
    inline void set_bcdUSB(uint16_t n) {
        _descriptor.bcdUSB = n;
    }
    inline void set_bDeviceClass(USB::bDeviceClass_t n) {
        _descriptor.bDeviceClass = n;
    }
    inline void set_bDeviceSubClass(uint8_t n) {
        _descriptor.bDeviceSubClass = n;
    }
    inline void set_bDeviceProtocol(uint8_t n) {
        _descriptor.bDeviceProtocol = n;
    }
    inline void set_bMaxPacketSize0(uint8_t n) {
        _descriptor.bMaxPacketSize0 = n;
    }
    inline void set_idVendor(uint16_t n) {
        _descriptor.idVendor = n;
    }
    inline void set_idProduct(uint16_t n) {
        _descriptor.idProduct = n;
    }
    inline void set_bcdDevice(uint16_t n) {
        _descriptor.bcdDevice = n;
    }
    void set_Manufacturer(const char *);
    void set_Product(const char *);
    void set_SerialNumber(const char *);

    // Add a new configuration to this device
    void add_configuration(usb_configuration * config);

    // Add a BOS descriptor
    void add_bos(usb_bos * bos);

    // Find a configuration based on its bConfigurationValue
    // Return nullptr if not found
    usb_configuration * find_configuration(uint8_t i);

    // Read-only version of our descriptor
    const USB::device_descriptor_t & descriptor;

    // Read-only version of our configurations
    const std::array<usb_configuration *, TUPP_MAX_CONF_PER_DEVICE> & configurations;

    // Read-only version of our BOS pointer
    const usb_bos * const & bos;

    // The setup message handler which handles all
    // non-standard commands directed to this device.
    // Will be called by the usb_device_controller.
    std::function<void(USB::setup_packet_t * packet)> setup_handler;

private:
    // The device descriptor
    USB::device_descriptor_t _descriptor;

    // Array of pointers to our configurations
    std::array<usb_configuration *, TUPP_MAX_CONF_PER_DEVICE> _configurations;

    // Optional BOS descriptor
    usb_bos * _bos;
};

#endif // TUPP_USB_DEVICE_H
