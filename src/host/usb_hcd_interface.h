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
// Abstract interface for a USB Host Controller Driver (HCD).
// This is the mirror image of usb_dcd_interface.h:
//   - usb_dcd_interface  REACTS to requests sent by a PC host.
//   - usb_hcd_interface  ACTIVELY SENDS requests to a connected device.
//
#ifndef TUPP_USB_HCD_INTERFACE_H
#define TUPP_USB_HCD_INTERFACE_H

#include <functional>
using std::function;
#include "usb_structs.h"
#include "usb_config.h"

class usb_endpoint;

using namespace TUPP;

// Result of a control transfer sent to a device
struct hcd_xfer_result_t {
    bool     success;
    uint16_t actual_len;
};

class usb_hcd_interface {
public:
    // Reset the USB bus / port (electrical reset of the connected device)
    virtual void port_reset(uint8_t delay_ms = 50) = 0;

    // Check if a device is currently connected to the port
    virtual bool port_connected() = 0;

    // Assign a new address to the device currently being enumerated
    // (the device starts at address 0, like in the USB spec)
    virtual void assign_address(uint8_t addr) = 0;

    // Enable/Disable USB host-mode interrupts
    virtual void irq_enable(bool e) = 0;

    // Called when a new device is detected on the bus
    function<void()> connect_handler;

    // Called when a device is unplugged
    function<void()> disconnect_handler;

    // Send a control transfer (e.g. GET_DESCRIPTOR, SET_ADDRESS) to a
    // device at the given address. The callback is invoked when the
    // transfer is complete.
    virtual bool control_xfer(uint8_t                 daddr,
                               TUPP::setup_packet_t  & request,
                               uint8_t                * buffer,
                               function<void(hcd_xfer_result_t)> complete_cb) = 0;

    // Register a data handler for an already-configured interrupt
    // endpoint (set up via create_endpoint). Called from the IRQ context
    // whenever new report data arrives.
    virtual bool set_int_ep_handler(uint8_t daddr, uint8_t ep_addr,
                        std::function<void(uint8_t*, uint16_t)> handler) = 0;

    // Create an endpoint towards a specific connected device
    // (daddr identifies which device this endpoint talks to)
    virtual usb_endpoint * create_endpoint(
                                 uint8_t         daddr,
                                 uint8_t         ep_addr,
                                 ep_attributes_t type,
                                 uint16_t        packet_size = TUPP_DEFAULT_PAKET_SIZE,
                                 uint8_t         interval    = TUPP_DEFAULT_POLL_INTERVAL) = 0;

protected:
    virtual ~usb_hcd_interface() = default;
};

#endif // TUPP_USB_HCD_INTERFACE_H
