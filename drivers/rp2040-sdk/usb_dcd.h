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
// Implementation of the USB Device Controller Driver (DCD)
// using the RP2040 pico-sdk
//
#ifndef TUPP_USB_DCD_H
#define TUPP_USB_DCD_H

#include "usb_dcd_interface.h"
#include "usb_endpoint_rp2040.h"
#include "usb_interface.h"

extern "C" {
void isr_irq5(void);
};

class usb_dcd : public usb_dcd_interface {
public:

    friend class usb_endpoint_rp2040;
    friend void isr_irq5(void);

    static usb_dcd & inst() {
        static usb_dcd _inst;
        return _inst;
    }

    void pullup_enable(bool e) override;
    void irq_enable(bool e) override;
    void set_address(uint8_t addr) override;
    void reset_address() override;

    // Create a new endpoint based on its address.
    usb_endpoint * create_endpoint(
                         uint8_t         addr,
                         ep_attributes_t type,
                         uint16_t        packet_size,
                         uint8_t         interval,
                         usb_interface * interface) override;

    // Create a new endpoint based on its direction.
    // The next free available address is used.
    usb_endpoint * create_endpoint(
                         direction_t     direction,
                         ep_attributes_t type,
                         uint16_t        packet_size,
                         uint8_t         interval,
                         usb_interface * interface) override;

    void check_address();

    inline usb_endpoint * addr_to_ep(uint8_t addr) override {
        return _endpoints[addr & 0x0f][addr >> 7];
    }

private:
    usb_dcd();

    usb_endpoint_rp2040 * _endpoints[16][2];

    uint8_t             _new_addr;
    bool                _should_set_address;
};

#endif // TUPP_USB_DCD_H
