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
#include "usb_endpoint.h"
#include "usb_interface.h"
#include <cstring>

usb_endpoint::usb_endpoint(
        uint8_t addr,
        USB::ep_attributes_t transfer_type,
        uint16_t packet_size,
        uint8_t interval,
        usb_interface *interface)
: descriptor(_descriptor)
{
    // Set descriptor length
    _descriptor.bLength = sizeof(USB::endpoint_descriptor_t);
    // Set descriptor type
    _descriptor.bDescriptorType = USB::bDescriptorType_t::DESC_ENDPOINT;
    // Set up remaining descriptor items
    set_bEndpointAddress(addr);
    set_bmAttributes    (transfer_type);
    set_wMaxPacketSize  (packet_size);
    set_bInterval       (interval);
    // Add this endpoint to its parent interface, if existing
    if (interface) interface->add_endpoint(this);
}

void usb_endpoint::start_transfer(uint8_t * buffer, uint16_t len) {
    assert(!_active) ;
    // Mark this endpoint as active
    _active = true;
    // Store transfer parameters
    _data_ptr    = buffer;
    _data_len    = len;
    _current_ptr = buffer;
    _bytes_left  = len;
    // Trigger the transfer (in or out).
    // Limit size to max packet size.
    _current_len = len > descriptor.wMaxPacketSize ?
                   descriptor.wMaxPacketSize : len;

    if (is_IN() && _current_len) {
        // Copy the data from user buffer to the HW buffer
        memcpy(_hw_buffer, _current_ptr, _current_len);
        // Update transfer parameters
        _bytes_left  -= _current_len;
        _current_ptr += _current_len;
    }
    // Trigger the transfer in HW.
    trigger_transfer(_current_len);
}

void usb_endpoint::handle_buffer_in(uint16_t) {
    assert(_active);
    // Entering this method means that the hw controller
    // has sent a packet of data to the host.
    // Check if we need to change the hw address
    // Check if the last packet has been sent
    if (!_bytes_left) {
        // Call user handler which will report the
        // complete data which was sent to the host.
        _active = false;
        if (data_handler) {
            data_handler(_data_ptr, _data_len);
        }
        return;
    }
    // We need to send more data to the host. So prepare
    // a consecutive transfer. Limit the transfer size to
    // the maximum packet size.
    _current_len = _bytes_left > descriptor.wMaxPacketSize ?
                   descriptor.wMaxPacketSize : _bytes_left;
    // Copy the data from user buffer to the HW buffer
    memcpy(_hw_buffer, _current_ptr, _current_len);
    // Update transfer parameters
    _bytes_left  -= _current_len;
    _current_ptr += _current_len;
    // Finally trigger the transfer
    trigger_transfer(_current_len);
}

void usb_endpoint::handle_buffer_out(uint16_t len) {
    assert(_active);
    // Entering this method means that the host has sent us a
    // new data packet. Copy all received bytes to the user buffer.
    memcpy(_current_ptr, _hw_buffer, len);
    // Update transfer parameters
    _bytes_left  -= len;
    _current_ptr += len;
    // We terminate the transfer if we either have received
    // all bytes or received a 'short' packet, which returned
    // fewer bytes as expected.
    if ((_bytes_left == 0) || (len < _current_len)) {
        // Let the user handler consume the complete
        // received data set.
        _active = false;
        if (data_handler) {
            data_handler(_data_ptr, _data_len - _bytes_left);
        }
        return;
    }
    // More bytes to receive, so trigger a new transfer.
    // Limit the transfer size to the maximum packet size.
    _current_len = _bytes_left > descriptor.wMaxPacketSize ?
                   descriptor.wMaxPacketSize : _bytes_left;
    // Finally trigger the transfer
    trigger_transfer(_current_len);
}
