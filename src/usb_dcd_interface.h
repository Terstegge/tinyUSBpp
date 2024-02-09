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
#ifndef _USB_DCD_INTERFACE_H
#define _USB_DCD_INTERFACE_H

#include <functional>
using std::function;

#include "usb_common.h"
#include "usb_config.h"

class usb_endpoint;
class usb_interface;

using namespace USB;

class usb_dcd_interface {
public:

    // Enable/Disable pullup resistors
    virtual void pullup_enable(bool e) = 0;

    // Enable/Disable USB interrupts
    virtual void irq_enable(bool e) = 0;

    // Set new USB address
    virtual void set_address(uint8_t addr) = 0;
    virtual void reset_address() = 0;

    // Set handler for setup packets
    function<void(USB::setup_packet_t * packet)> setup_handler;

    // Set handler for bus reset
    function<void()> bus_reset_handler;

    // Create a new endpoint based on its address.
    virtual usb_endpoint * create_endpoint(
                                 uint8_t         addr,
                                 ep_attributes_t type,
                                 uint16_t        packet_size = TUPP_DEFAULT_PAKET_SIZE,
                                 uint8_t         interval    = TUPP_DEFAULT_POLL_INTERVAL,
                                 usb_interface * interface = nullptr) = 0;

    // Create a new endpoint based on its direction.
    // The next free available address is used.
    virtual usb_endpoint * create_endpoint(
                                 direction_t     direction,
                                 ep_attributes_t type,
                                 uint16_t        packet_size = TUPP_DEFAULT_PAKET_SIZE,
                                 uint8_t         interval    = TUPP_DEFAULT_POLL_INTERVAL,
                                 usb_interface * interface = nullptr) = 0;

    virtual usb_endpoint * addr_to_ep(uint8_t addr) = 0;

protected:
    virtual ~usb_dcd_interface() = default;
};

#endif // _USB_DCD_INTERFACE_H
