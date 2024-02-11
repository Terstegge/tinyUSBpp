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
// This class is the virtual base class for a concrete
// endpoint implementation. The virtual methods will have
// to be implemented by the hardware-specific class.
//
#ifndef TUPP_USB_ENDPOINT_H
#define TUPP_USB_ENDPOINT_H

class usb_interface;
#include "usb_structs.h"
#include "usb_config.h"
#include <cassert>
#include <cstdint>
#include <functional>

class usb_endpoint {
public:
    // No copy, no assignment
    usb_endpoint(const usb_endpoint &) = delete;
    usb_endpoint & operator= (const usb_endpoint &) = delete;

    // Methods to set the descriptor elements
    inline void set_bEndpointAddress(uint8_t n) {
        _descriptor.bEndpointAddress = n;
    }
    inline void set_bmAttributes(USB::ep_attributes_t n) {
        _descriptor.bmAttributes = n;
    }
    inline void set_wMaxPacketSize(uint16_t n) {
        _descriptor.wMaxPacketSize = n;
    }
    inline void set_bInterval(uint8_t n) {
        _descriptor.bInterval = n;
    }

    // Return true if this endpoint is an IN-endpoint
    inline bool is_IN() const {
        return descriptor.bEndpointAddress & 0x80;
    }
    // Return true if this endpoint is in a transaction
    inline bool is_active() const {
        return _active;
    }
    // Send ZLP (Zero-Length Packet) with DATA1 pid
    inline void send_zlp_data1() {
        assert (_next_pid == 1);
        start_transfer(nullptr, 0);
    }

    // Read-only version of our descriptor
    const USB::endpoint_descriptor_t & descriptor;

    // The data handler which is called when a transaction
    // to/from the host has finished.
    std::function<void(uint8_t * buffer, uint16_t len)> data_handler;

    // The setup message handler which handles all
    // commands directed to this endpoint. Will be
    // called by the usb_device_controller.
    std::function<void(USB::setup_packet_t * packet)> setup_handler;

    // Reset this endpoint to its default state
    // (no stall, no NAK, not active, next PID = 1)
    void reset();

    // Start a transfer on this endpoint (data stage).
    void start_transfer(uint8_t * buffer, uint16_t len);

    // (De-)Activate this endpoint
    virtual void enable_endpoint(bool b) = 0;

    // If parameter is true, reply with NAK from
    // next packet on, until this method is called
    // again with 'false'.
    virtual void send_NAK(bool b) = 0;

    // Stall/Un-Stall this endpoint
    virtual void send_stall(bool b) = 0;

protected:
    // The endpoint CTOR is only accessible from the
    // derived implementations of this class
    usb_endpoint(
            uint8_t  addr,
            USB::ep_attributes_t  transfer_type,
            uint16_t packet_size = TUPP_DEFAULT_PAKET_SIZE,
            uint8_t  interval    = TUPP_DEFAULT_POLL_INTERVAL,
            usb_interface * interface = nullptr);

    void handle_buffer_in (uint16_t len);
    void handle_buffer_out(uint16_t len);

    virtual void trigger_transfer(uint16_t len) = 0;

    // PID used for next transfer
    uint8_t         _next_pid {0};
    uint8_t *       _data_ptr {};
    uint16_t        _data_len {};
    uint8_t *       _current_ptr {};
    uint16_t        _current_len {};
    uint16_t        _bytes_left {};

    volatile bool   _active {false};

    uint8_t *       _hw_buffer {};

    virtual ~usb_endpoint() = default;

private:
    // The endpoint descriptor
    USB::endpoint_descriptor_t _descriptor {};
};

#endif // TUPP_USB_ENDPOINT_H
