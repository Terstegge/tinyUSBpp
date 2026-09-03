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
// Implementation of the USB Host Controller Driver (HCD)
// using the YAHAL OS. Mirror image of usb_dcd.h (device side).
//
#ifndef TUPP_USB_HCD_H
#define TUPP_USB_HCD_H

#include <functional>
#include "usb_hcd_interface.h"
#include "RP2350.h"

extern "C" {
    void USBCTRL_IRQ_Handler(void);
};

class usb_hcd : public usb_hcd_interface {
public:
    std::function<void()> connect_handler;
    std::function<void()> disconnect_handler;
    friend void USBCTRL_IRQ_Handler(void);

    static usb_hcd& inst() {
        static usb_hcd _inst;
        return _inst;
    }

    void port_reset(uint8_t delay_ms = 50) override;
    bool port_connected() override;
    void assign_address(uint8_t addr) override;
    void irq_enable(bool e) override;

    bool control_xfer(uint8_t daddr,
        TUPP::setup_packet_t& request,
        uint8_t* buffer,
        std::function<void(hcd_xfer_result_t)> complete_cb) override;

    bool set_int_ep_handler(uint8_t daddr, uint8_t ep_addr,
        std::function<void(uint8_t*, uint16_t)> handler) override;

    usb_endpoint* create_endpoint(
        uint8_t         daddr,
        uint8_t         ep_addr,
        ep_attributes_t type,
        uint16_t        packet_size,
        uint8_t         interval) override;

private:
    usb_hcd();
    std::function<void(hcd_xfer_result_t)> _xfer_complete_cb;
    uint8_t* _xfer_buffer;
    volatile bool _trans_triggered{ false };
    uint8_t _pending_address{ 0 };
    uint16_t  _xfer_length{ 0 };
    enum class xfer_stage_t { SETUP, DATA, STATUS };
    uint16_t _xfer_offset{ 0 };
    bool          _status_out_pending{ false };
    bool          _status_in_only{ false };
    xfer_stage_t _xfer_stage{ xfer_stage_t::SETUP };

    // Bookkeeping for the 15 hardware "interrupt endpoint" slots
    // (INT_EP_CTRL bits 1..15). These are auto-polled by hardware
    // once active - used for reading e.g. HID mouse/keyboard reports.
    struct int_ep_slot_t {
        bool      used{ false };
        uint8_t   daddr{ 0 };
        uint8_t   ep_addr{ 0 };
        uint8_t* hw_buffer{ nullptr };
        uint16_t  packet_size{ 0 };
        std::function<void(uint8_t*, uint16_t)> data_handler;
    };
    int_ep_slot_t _int_eps[15];
    uint32_t _saved_int_ep_ctrl{ 0 };
};

#endif // TUPP_USB_HCD_H