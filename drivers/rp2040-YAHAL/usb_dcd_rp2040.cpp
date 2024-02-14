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

using namespace _USBCTRL_REGS_;
using namespace _RESETS_;

usb_dcd_rp2040::usb_dcd_rp2040()
: _endpoints({nullptr},{nullptr}),
  _new_addr(0), _should_set_address(false)
{
    // Reset usb controller
    RESETS_SET.RESET.usbctrl <<= 1;
    RESETS_CLR.RESET.usbctrl <<= 1;
    while (!RESETS.RESET_DONE.usbctrl);

    // Clear any previous state in dpram just in case
    memset(&USBCTRL_DPRAM, 0, 4096);

    // Mux the controller to the onboard usb phy
    USBCTRL_REGS_SET.USB_MUXING.SOFTCON <<= 1;
    USBCTRL_REGS_SET.USB_MUXING.TO_PHY <<= 1;

    // Force VBUS detect so the device thinks it is plugged into a host
    USBCTRL_REGS_SET.USB_PWR.VBUS_DETECT_OVERRIDE_EN <<= 1;
    USBCTRL_REGS_SET.USB_PWR.VBUS_DETECT <<= 1;

    // Enable the USB controller in device mode.
    USBCTRL_REGS_SET.MAIN_CTRL.CONTROLLER_EN <<= 1;

    // Enable an interrupt per EP0 transaction
    USBCTRL_REGS_CLR.SIE_CTRL.EP0_DOUBLE_BUF <<= 1;
    USBCTRL_REGS_SET.SIE_CTRL.EP0_INT_1BUF <<= 1;

    // Enable interrupts ...
    // ... when a setup packet is received
    // ... when the bus is reset, and
    // ... when a buffer is done,
    USBCTRL_REGS_SET.INTE.SETUP_REQ     <<= 1;
    USBCTRL_REGS_SET.INTE.BUS_RESET     <<= 1;
    USBCTRL_REGS_SET.INTE.BUFF_STATUS   <<= 1;

    // Enable USB interrupt
    NVIC_ClearPendingIRQ(USBCTRL_IRQ_IRQn);
    NVIC_EnableIRQ(USBCTRL_IRQ_IRQn);
}

void usb_dcd_rp2040::pullup_enable(bool e) {
    USBCTRL_REGS_SET.SIE_CTRL.PULLUP_EN <<= e;
}

void usb_dcd_rp2040::irq_enable(bool e) {
    if (e) {
        NVIC_EnableIRQ(USBCTRL_IRQ_IRQn);
    } else {
        NVIC_DisableIRQ(USBCTRL_IRQ_IRQn);
    }
}

void usb_dcd_rp2040::set_address(uint8_t addr) {
    _new_addr = addr;
    TUPP_LOG(LOG_INFO, "Set USB address %d", _new_addr);
    _should_set_address = true;
}

void usb_dcd_rp2040::check_address() {
    if (_should_set_address) {
        USBCTRL_REGS.ADDR_ENDP.ADDRESS = _new_addr;
        _should_set_address = false;
    }
}

void usb_dcd_rp2040::reset_address() {
    _new_addr = 0;
    _should_set_address = false;
    USBCTRL_REGS.ADDR_ENDP.ADDRESS = 0;
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
    assert(!"Could not find a free endpoint!");
    return nullptr;
}

extern "C" {
void USBCTRL_IRQ_Handler(void) {
    // Setup packet received
    if (USBCTRL_REGS.INTS.SETUP_REQ) {
        USBCTRL_REGS_CLR.SIE_STATUS.SETUP_REC = 1;
        usb_dcd_rp2040::inst().setup_handler((USB::setup_packet_t *)&USBCTRL_DPRAM);
    }
    // Bus is reset
    if (USBCTRL_REGS.INTS.BUS_RESET) {
        USBCTRL_REGS_CLR.SIE_STATUS.BUS_RESET = 1;
        usb_dcd_rp2040::inst().bus_reset_handler();
    }
    // Buffer status, one or more buffers have completed
    if (USBCTRL_REGS.INTS.BUFF_STATUS) {
        uint32_t buffs = USBCTRL_REGS.BUFF_STATUS;
        uint32_t bit = 1;
        for(uint32_t i=0; buffs && (i < 32); ++i) {
            if (buffs & bit) {
                // Clear bits
                USBCTRL_REGS_CLR.BUFF_STATUS = bit;
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
