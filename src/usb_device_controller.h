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
#ifndef TUPP_USB_DEVICE_CONTROLLER_H
#define TUPP_USB_DEVICE_CONTROLLER_H

class usb_device;
#include "usb_config.h"
#include "usb_dcd_interface.h"
#include "usb_endpoint.h"
#include <functional>

class usb_device_controller {
public:
    usb_device_controller(usb_dcd_interface & driver, usb_device & device);

    // Create a new endpoint based on its address.
    usb_endpoint * create_endpoint(
            usb_interface & interface,
            uint8_t         addr,
            ep_attributes_t type,
            uint16_t        packet_size = TUPP_DEFAULT_PAKET_SIZE,
            uint8_t         interval    = TUPP_DEFAULT_POLL_INTERVAL) {
        return _driver.create_endpoint(addr, type, packet_size, interval, &interface);
    }

    // Create a new endpoint based on its address.
    usb_endpoint * create_endpoint(
            usb_interface & interface,
            direction_t     direction,
            ep_attributes_t type,
            uint16_t        packet_size = TUPP_DEFAULT_PAKET_SIZE,
            uint8_t         interval    = TUPP_DEFAULT_POLL_INTERVAL) {
        return _driver.create_endpoint(direction, type, packet_size, interval, &interface);
    }

    const volatile uint8_t & active_configuration;

    // Standard endpoints 0
    usb_endpoint * _ep0_in;
    usb_endpoint * _ep0_out;

    std::function<void(uint8_t *, uint16_t)> handler;

private:

    void handle_set_address      (USB::setup_packet_t * pkt);
    void handle_get_descriptor   (USB::setup_packet_t * pkt);
    void handle_set_descriptor   (USB::setup_packet_t * pkt);
    void handle_get_configuration(USB::setup_packet_t * pkt);
    void handle_set_configuration(USB::setup_packet_t * pkt);
    void handle_get_interface    (USB::setup_packet_t * pkt);
    void handle_synch_frame      (USB::setup_packet_t * pkt);
    void handle_set_interface    (USB::setup_packet_t * pkt);
    void handle_get_status       (USB::setup_packet_t * pkt);
    void handle_clear_feature    (USB::setup_packet_t * pkt);
    void handle_set_feature      (USB::setup_packet_t * pkt);

    usb_dcd_interface & _driver;
    usb_device &        _device;
    volatile uint8_t    _active_configuration;
    // Buffer for device descriptors
    uint8_t             _buf[TUPP_MAX_DESC_SIZE];
};

#endif  // TUPP_USB_DEVICE_CONTROLLER_H
