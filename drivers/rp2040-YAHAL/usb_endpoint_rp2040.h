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
// Implementation of the USB endpoint class for
// the RP2040 using the YAHAL OS
//
#ifndef TUPP_USB_ENDPOINT_RP2040_H
#define TUPP_USB_ENDPOINT_RP2040_H

class usb_interface;

#include "usb_endpoint.h"
#include "RP2040.h"

using namespace _USBCTRL_DPRAM_;

extern "C" {
void USBCTRL_IRQ_Handler(void);
};

class usb_endpoint_rp2040 : public usb_endpoint {
public:
    friend class usb_dcd;
    friend void USBCTRL_IRQ_Handler(void);

    void enable_endpoint(bool b) override;

    void send_stall(bool b) override;
    bool is_stalled() const override;

    void send_NAK(bool b) override;

private:
    usb_endpoint_rp2040(uint8_t  addr,
                        TUPP::ep_attributes_t  type,
                        uint16_t packet_size = 64,
                        uint8_t  interval    = 0,
                        usb_interface * interface = nullptr);

    void _process_buffer();

    EP_CONTROL_t *        _endp_ctrl;
    EP_BUFFER_CONTROL_t * _buff_ctrl;

    uint16_t              _hw_buffer_size;
    static uint8_t *      _next_free_buffer;

    uint32_t              _mask;

    void trigger_transfer(uint16_t len) override;
};

#endif  // TUPP_USB_ENDPOINT_RP2040_H
