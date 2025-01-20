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
#include "usb_dcd.h"
#include "usb_endpoint_rp2350.h"
#include "usb_log.h"
#include <cstring>
#include <cassert>

using namespace _USB_;
using namespace _RESETS_;

usb_dcd::usb_dcd()
: _endpoints({nullptr},{nullptr}),
  _new_addr(0), _should_set_address(false)
{
    // Reset usb controller
    RESETS_CLR.RESET.USBCTRL <<= 1;
    while (!RESETS.RESET_DONE.USBCTRL);

    // Clear any previous state in dpram just in case
    memset(&USB_DPRAM, 0, 4096);

    // Mux the controller to the onboard usb phy
    USB_SET.USB_MUXING.SOFTCON <<= 1;
    USB_SET.USB_MUXING.TO_PHY <<= 1;

    // Force VBUS detect so the device thinks it is plugged into a host
    USB_SET.USB_PWR.VBUS_DETECT_OVERRIDE_EN <<= 1;
    USB_SET.USB_PWR.VBUS_DETECT <<= 1;

    // Enable the USB controller in device mode and
    // disable the physical isolation (new for RP2350)
    USB_SET.MAIN_CTRL.CONTROLLER_EN <<= 1;
    USB_CLR.MAIN_CTRL.PHY_ISO <<= 1;

    // Enable an interrupt per EP0 transaction
    USB_CLR.SIE_CTRL.EP0_DOUBLE_BUF <<= 1;
    USB_SET.SIE_CTRL.EP0_INT_1BUF <<= 1;

    // Enable interrupts ...
    // ... when a setup packet is received
    // ... when the bus is reset, and
    // ... when a buffer is done,
    USB_SET.INTE.SETUP_REQ     <<= 1;
    USB_SET.INTE.BUS_RESET     <<= 1;
    USB_SET.INTE.BUFF_STATUS   <<= 1;

    // Enable USB interrupt
    NVIC_ClearPendingIRQ(USBCTRL_IRQ_IRQn);
    NVIC_EnableIRQ(USBCTRL_IRQ_IRQn);
}

void usb_dcd::pullup_enable(bool e) {
    USB_SET.SIE_CTRL.PULLUP_EN <<= e;
}

void usb_dcd::irq_enable(bool e) {
    if (e) {
        NVIC_EnableIRQ(USBCTRL_IRQ_IRQn);
    } else {
        NVIC_DisableIRQ(USBCTRL_IRQ_IRQn);
    }
}

void usb_dcd::set_address(uint8_t addr) {
    _new_addr = addr;
    TUPP_LOG(LOG_INFO, "Set USB address %d", _new_addr);
    _should_set_address = true;
}

void usb_dcd::check_address() {
    if (_should_set_address) {
        USB.ADDR_ENDP.ADDRESS = _new_addr;
        _should_set_address = false;
    }
}

void usb_dcd::reset_address() {
    _new_addr = 0;
    _should_set_address = false;
    USB.ADDR_ENDP.ADDRESS = 0;
}

usb_endpoint * usb_dcd::create_endpoint(
        uint8_t addr,
        ep_attributes_t type,
        uint16_t packet_size,
        uint8_t interval,
        usb_interface * interface) {
    return new usb_endpoint_rp2350(addr, type, packet_size, interval, interface);
}

usb_endpoint * usb_dcd::create_endpoint(
        direction_t     direction,
        ep_attributes_t type,
        uint16_t        packet_size,
        uint8_t         interval,
        usb_interface * interface) {
    uint8_t dir = (direction == direction_t::DIR_IN) ? 1 : 0;
    for (uint8_t i = 0; i < 16; ++i) {
        if (!_endpoints[i][dir]) {
            uint8_t addr = i | (dir << 7);
            return new usb_endpoint_rp2350(addr, type, packet_size, interval, interface);
        }
    }
    assert(!"Could not find a free endpoint!");
    return nullptr;
}

extern "C" {
void USBCTRL_IRQ_Handler(void) {
    // Setup packet received
    if (USB.INTS.SETUP_REQ) {
        USB_CLR.SIE_STATUS.SETUP_REC = 1;
        usb_dcd::inst().setup_handler((TUPP::setup_packet_t *)&USB_DPRAM);
    }
    // Bus is reset
    if (USB.INTS.BUS_RESET) {
        USB_CLR.SIE_STATUS.BUS_RESET = 1;
        usb_dcd::inst().bus_reset_handler();
    }
    // Buffer status, one or more buffers have completed
    if (USB.INTS.BUFF_STATUS) {
        uint32_t buffs = USB.BUFF_STATUS;
        uint32_t bit = 1;
        for(uint32_t i=0; buffs && (i < 32); ++i) {
            if (buffs & bit) {
                // Clear bits
                USB_CLR.BUFF_STATUS = bit;
                buffs ^= bit;
                // Call internal handler
                auto ep = usb_dcd::inst()._endpoints[i>>1][!(i&1)];
                if (ep) ep->_process_buffer();
            }
            bit <<= 1;
        }
    }
}
}
