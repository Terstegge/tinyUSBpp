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
#include <cassert>

#include "hardware/structs/usb.h"

#define usb_hw_set   ((usb_hw_t *)hw_set_alias_untyped(usb_hw))
#define usb_hw_clear ((usb_hw_t *)hw_clear_alias_untyped(usb_hw))

uint8_t * usb_endpoint_rp2040::_next_free_buffer = (uint8_t *)USBCTRL_DPRAM_BASE + 0x180;

usb_endpoint_rp2040::usb_endpoint_rp2040(uint8_t  addr,
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
        _endp_ctrl = (io_rw_32 *)USBCTRL_DPRAM_BASE + offset;
        if (!is_IN()) _endp_ctrl++;
        // Set buffer address in DPRAM
        _hw_buffer = _next_free_buffer;
        _next_free_buffer += packet_size;
        _hw_buffer_size = packet_size;
        assert(_next_free_buffer <= (uint8_t *)USBCTRL_DPRAM_BASE + 0x1000);
    } else {
        // Handle special case EP0
        _endp_ctrl = nullptr;
        _hw_buffer = (uint8_t *)USBCTRL_DPRAM_BASE + 0x100;
        _hw_buffer_size = packet_size;
        assert(_hw_buffer_size == 64);
    }
    _buff_ctrl = (io_rw_32 *)&usb_dpram->ep_buf_ctrl + offset;
    if (!is_IN()) _buff_ctrl++;

    // Set endpoint control register
    if (_endp_ctrl) {
        uint32_t reg = (uint32_t)_hw_buffer & 0xffff;
        reg |= EP_CTRL_INTERRUPT_PER_BUFFER;
        reg |= (uint32_t)descriptor.bmAttributes << EP_CTRL_BUFFER_TYPE_LSB;
        reg |= EP_CTRL_ENABLE_BITS;
        *_endp_ctrl = reg;
    }

    // Initial PID value
    _next_pid = 0;

    // Initialize bitmask (used for some registers)
    _mask = 1 << offset;
    if (!is_IN()) _mask <<= 1;

    // Store this endpoint in lookup table
    usb_dcd_rp2040::inst()._endpoints[addr & 0x0f][addr >> 7] = this;
}

void usb_endpoint_rp2040::_process_buffer() {
    // Dispatch request according endpoint direction
    if (is_IN()) {
        usb_dcd_rp2040::inst().check_address();
        handle_buffer_in(*_buff_ctrl & USB_BUF_CTRL_LEN_MASK);
    } else {
        handle_buffer_out(*_buff_ctrl & USB_BUF_CTRL_LEN_MASK);
    }
}

void usb_endpoint_rp2040::enable_endpoint(bool b) {
    TUPP_LOG(LOG_INFO, "Endpoint 0x%x is enabled: %b", descriptor.bEndpointAddress, b);
    if (b) {
        *_endp_ctrl |=  EP_CTRL_ENABLE_BITS;
    } else {
        *_endp_ctrl &= ~EP_CTRL_ENABLE_BITS;
    }
}

void usb_endpoint_rp2040::send_NAK(bool b) {
    if (b) {
        usb_hw_set->abort = _mask;
    } else {
        usb_hw_clear->abort = _mask;
    }
}

void usb_endpoint_rp2040::send_stall(bool b) {
    if (b) {
        if ((descriptor.bEndpointAddress & 0xf) == 0) {
            usb_hw_set->ep_stall_arm = _mask;
        }
        *_buff_ctrl |= USB_BUF_CTRL_STALL;
        _next_pid = 0;
    } else {
        _next_pid = 0;
        *_buff_ctrl &= ~(USB_BUF_CTRL_STALL | USB_BUF_CTRL_AVAIL);
    }
}

void usb_endpoint_rp2040::trigger_transfer(uint16_t len) {
    assert((*_buff_ctrl & USB_BUF_CTRL_AVAIL) == 0);
    // Prepare buffer control value
    uint32_t reg = len | USB_BUF_CTRL_AVAIL;
    reg |= _next_pid ? USB_BUF_CTRL_DATA1_PID : USB_BUF_CTRL_DATA0_PID;
    reg |= is_IN()   ? USB_BUF_CTRL_FULL : 0;
    // Flip PID
    _next_pid ^= 1;
    // Write value to register
    *_buff_ctrl = reg;
}
