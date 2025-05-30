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
#include "usb_dcd.h"
#include "usb_endpoint_rp2350.h"
#include "usb_log.h"
#include "RP2350.h"
#include <cassert>

using namespace _USB_;

uint8_t * usb_endpoint_rp2350::_next_free_buffer = (uint8_t *)&USB_DPRAM + 0x180;

usb_endpoint_rp2350::usb_endpoint_rp2350(uint8_t  addr,
                                         ep_attributes_t transfer_type,
                                         uint16_t packet_size,
                                         uint8_t  interval,
                                         usb_interface * interface)
: usb_endpoint(addr, transfer_type, packet_size, interval, interface)
{
    // Align packet size to multiples of 64 byte
    if (packet_size & 0x3f) packet_size += 64;
    packet_size &= 0xffc0;
    set_wMaxPacketSize(packet_size);

    // Set Hardware registers
    uint8_t offset = (addr & 0x0f) << 1;

    if (offset) {
        _endp_ctrl = (EP_CONTROL_t *)&USB_DPRAM + offset;
        if (!is_IN()) _endp_ctrl++;
        // Set buffer address in DPRAM
        _hw_buffer = _next_free_buffer;
        _next_free_buffer += packet_size;
        _hw_buffer_size = packet_size;
        assert(_next_free_buffer <= (uint8_t *)&USB_DPRAM + 0x1000);
    } else {
        // Handle special case EP0
        _endp_ctrl = nullptr;
        _hw_buffer = (uint8_t *)&USB_DPRAM + 0x100;
        _hw_buffer_size = packet_size;
        assert(_hw_buffer_size == 64);
    }
    _buff_ctrl = (EP_BUFFER_CONTROL_t *)&USB_DPRAM.EP0_IN_BUFFER_CONTROL + offset;
    if (!is_IN()) _buff_ctrl++;

    // Set endpoint control register
    if (_endp_ctrl) {
        _endp_ctrl->BUFFER_ADDRESS     = (uint32_t)_hw_buffer & 0xffff;
        _endp_ctrl->INTERRUPT_PER_BUFF = 1;
        _endp_ctrl->ENDPOINT_TYPE      = (uint32_t)descriptor.bmAttributes;
        _endp_ctrl->ENABLE             = 0;
    }

    // Initial PID value
    _next_pid = 0;

    // Initialize bitmask (used for some registers)
    _mask = 1 << offset;
    if (!is_IN()) _mask <<= 1;

    // Store this endpoint in lookup table
    usb_dcd::inst()._endpoints[addr & 0x0f][addr >> 7] = this;
}

void usb_endpoint_rp2350::_process_buffer() {
    // Dispatch request according endpoint direction
    if (is_IN()) {
        usb_dcd::inst().check_address();
        handle_buffer_in(_buff_ctrl->LENGTH_0);
    } else {
        handle_buffer_out(_buff_ctrl->LENGTH_0);
    }
}

void usb_endpoint_rp2350::enable_endpoint(bool b) {
    TUPP_LOG(LOG_INFO, "Endpoint 0x%x enabled: %b", descriptor.bEndpointAddress, b);
    if (_endp_ctrl) {
        _endp_ctrl->ENABLE = b;
    }
}

void usb_endpoint_rp2350::send_NAK(bool b) {
    if (b) {
        USB_SET.EP_ABORT = _mask;
    } else {
        USB_CLR.EP_ABORT = _mask;
    }
}

void usb_endpoint_rp2350::send_stall(bool b) {
    if (b) {
        if ((descriptor.bEndpointAddress & 0xf) == 0) {
            USB_SET.EP_STALL_ARM = _mask;
        }
        _buff_ctrl->STALL = 1;
        _next_pid = 0;
    } else {
        _next_pid = 0;
        _buff_ctrl->STALL = 0;
        _buff_ctrl->AVAILABLE_0 = 0;
    }
}

bool usb_endpoint_rp2350::is_stalled() const {
    return _buff_ctrl->STALL;
}

void usb_endpoint_rp2350::trigger_transfer(uint16_t len) {
    assert(_buff_ctrl->AVAILABLE_0 == 0);
    // Set pid and flip for next transfer
    _buff_ctrl->PID_0       = _next_pid;
    _buff_ctrl->FULL_0      = is_IN() ? 1 : 0;
    _buff_ctrl->LENGTH_0    = len;
    _next_pid ^= 1;
    // Finally mark this buffer as ready
    _buff_ctrl->AVAILABLE_0 = 1;
}
