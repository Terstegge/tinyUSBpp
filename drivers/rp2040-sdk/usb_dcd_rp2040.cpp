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
#include "usb_dcd_rp2040.h"
#include "usb_endpoint_rp2040.h"
#include "usb_log.h"
#include <cstring>
#include <cassert>

#include "hardware/irq.h"
#include "hardware/resets.h"
#include "hardware/structs/usb.h"

#define usb_hw_set   ((usb_hw_t *)hw_set_alias_untyped(usb_hw))
#define usb_hw_clear ((usb_hw_t *)hw_clear_alias_untyped(usb_hw))

usb_dcd_rp2040::usb_dcd_rp2040()
: _endpoints({nullptr},{nullptr}),
  _new_addr(0), _should_set_address(false)
{
    // Reset usb controller
    reset_block(RESETS_RESET_USBCTRL_BITS);
    unreset_block_wait(RESETS_RESET_USBCTRL_BITS);

    // Clear any previous state in dpram just in case
    memset((void *)USBCTRL_DPRAM_BASE, 0, 4096);

    // Mux the controller to the onboard usb phy
    usb_hw_set->muxing = USB_USB_MUXING_SOFTCON_BITS;
    usb_hw_set->muxing = USB_USB_MUXING_TO_PHY_BITS;

    // Force VBUS detect so the device thinks it is plugged into a host
    usb_hw_set->pwr = USB_USB_PWR_VBUS_DETECT_OVERRIDE_EN_BITS;
    usb_hw_set->pwr = USB_USB_PWR_VBUS_DETECT_BITS;

    // Enable the USB controller in device mode.
    usb_hw_set->main_ctrl = USB_MAIN_CTRL_CONTROLLER_EN_BITS;

    // Enable an interrupt per EP0 transaction
    usb_hw_clear->sie_ctrl = USB_SIE_CTRL_EP0_DOUBLE_BUF_BITS;
    usb_hw_set->sie_ctrl   = USB_SIE_CTRL_EP0_INT_1BUF_BITS;

    // Enable interrupts ...
    // ... when a setup packet is received
    // ... when the bus is reset, and
    // ... when a buffer is done,
    usb_hw_set->inte = USB_INTE_SETUP_REQ_BITS;
    usb_hw_set->inte = USB_INTE_BUS_RESET_BITS;
    usb_hw_set->inte = USB_INTE_BUFF_STATUS_BITS;

    // Enable USB interrupt
    irq_set_enabled(USBCTRL_IRQ, true);
}

void usb_dcd_rp2040::pullup_enable(bool e) {
    usb_hw_set->sie_ctrl = USB_SIE_CTRL_PULLUP_EN_BITS;
}

void usb_dcd_rp2040::irq_enable(bool e) {
    irq_set_enabled(USBCTRL_IRQ, e);
}

void usb_dcd_rp2040::set_address(uint8_t addr) {
    _new_addr = addr;
    TUPP_LOG(LOG_INFO, "Set USB address %d", _new_addr);
    _should_set_address = true;
}

void usb_dcd_rp2040::check_address() {
    if (_should_set_address) {
        usb_hw->dev_addr_ctrl = _new_addr;
        _should_set_address = false;
    }
}

void usb_dcd_rp2040::reset_address() {
    _new_addr = 0;
    _should_set_address = false;
    usb_hw->dev_addr_ctrl = 0;
}

usb_endpoint * usb_dcd_rp2040::create_endpoint(
        uint8_t addr,
        ep_attributes_t type,
        uint16_t packet_size,
        uint8_t interval,
        usb_interface * interface) {
    return new usb_endpoint_rp2040(addr, type, packet_size, interval, interface);
}

usb_endpoint * usb_dcd_rp2040::create_endpoint(
        direction_t     direction,
        ep_attributes_t type,
        uint16_t        packet_size,
        uint8_t         interval,
        usb_interface * interface) {
    uint8_t dir = (direction == direction_t::DIR_IN) ? 1 : 0;
    for (uint8_t i = 0; i < 16; ++i) {
        if (!_endpoints[i][dir]) {
            uint8_t addr = i | (dir << 7);
            return new usb_endpoint_rp2040(addr, type, packet_size, interval, interface);
        }
    }
    assert(!"No free endpoints");
    return nullptr;
}


extern "C" {
void isr_usbctrl(void) {
    uint32_t status = usb_hw->ints;
    // Setup packet received
    if (status & USB_INTS_SETUP_REQ_BITS) {
        usb_hw_clear->sie_status = USB_SIE_STATUS_SETUP_REC_BITS;
        usb_dcd_rp2040::inst().setup_handler((USB::setup_packet_t *)USBCTRL_DPRAM_BASE);
    }
    // Bus is reset
    if (status & USB_INTS_BUS_RESET_BITS) {
        usb_hw_clear->sie_status = USB_SIE_STATUS_BUS_RESET_BITS;
        usb_dcd_rp2040::inst().bus_reset_handler();
    }
    // Buffer status, one or more buffers have completed
    if (status & USB_INTS_BUFF_STATUS_BITS) {
        uint32_t buffs = usb_hw->buf_status;
        uint32_t bit = 1;
        for(uint32_t i=0; buffs && (i < 32); ++i) {
            if (buffs & bit) {
                // Clear bits
                usb_hw_clear->buf_status = bit;
                buffs ^= bit;
                // Call internal handler
                auto ep = usb_dcd_rp2040::inst()._endpoints[i>>1][!(i&1)];
                if (ep) ep->_process_buffer();
            }
            bit <<= 1;
        }
    }
}
}
