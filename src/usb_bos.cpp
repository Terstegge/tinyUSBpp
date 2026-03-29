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
#include <cassert>
#include <cstring>

#include "usb_bos.h"
#include "usb_device_controller.h"
#include "usb_device.h"
#include "usb_log.h"
#include "usb_ms_header.h"
#include "usb_ms_structs.h"
#include "usb_strings.h"

using namespace TUPP;
using enum usb_log::log_level;

usb_bos::usb_bos(usb_device_controller & controller, usb_device & device)
: descriptor(_descriptor), _descriptor{}, _capabilities{nullptr}
{
    TUPP_LOG(LOG_DEBUG, "usb_bos() @%x", this);
    // Set descriptor length
    _descriptor.bLength = sizeof(bos_descriptor_t);
    // Set descriptor type
    _descriptor.bDescriptorType = bDescriptorType_t::DESC_BOS;
    // No device capabilities so far...
    _descriptor.bNumDeviceCaps = 0;
    // Update total length
    set_total_length();
    // Add this configuration to our parent device
    device.add_bos(this);

    // Set up USB request handler for this device
    device.setup_handler = [&] (TUPP::setup_packet_t * pkt) {
        TUPP_LOG(LOG_DEBUG, "device setup_handler()");

        if ( (pkt->bRequest == GET_MS_OS20_DESC) &&
             (pkt->wIndex   == MS_OS20_INDEX) &&
             (pkt->wValue   == 0)) {
            TUPP_LOG(LOG_INFO, "Getting MS OS 2.0 descriptor");
            if (_ms_header) {
                _ms_header->desc_begin();
                uint8_t * _buffer_ptr = _buffer;
                size_t size = _ms_header->desc_total_size();
                for (size_t i=0; i < size; ++i) {
                    *_buffer_ptr++ = _ms_header->desc_getNext();
                }
                controller._ep0_in->start_transfer(_buffer, size);
            } else {
                controller._ep0_in->send_stall(true);
                controller._ep0_out->send_stall(true);
            }
        } else if ( (pkt->bRequest == GET_MS_WEBUSB) &&
                    (pkt->wIndex   == WEBUSB_INDEX)) {
            TUPP_LOG(LOG_INFO, "Getting MS WebUSB URL");
            // Make sure we have a landing page (string index > 0)!
            if (pkt->wValue) {
                uint8_t len = usb_strings::inst.prepare_url_utf8(pkt->wValue, _url_format, _buffer);
                if (len > pkt->wLength) len = pkt->wLength;
                controller._ep0_in->start_transfer(_buffer, len);
            } else {
                controller._ep0_in->send_stall(true);
                controller._ep0_out->send_stall(true);
            }
        } else {
            TUPP_LOG(LOG_ERROR, "Unknown device request %d %d %d",
                     pkt->bRequest, pkt->wValue, pkt->wIndex);
            controller._ep0_in->send_stall(true);
            controller._ep0_out->send_stall(true);
        }
    };
}

void usb_bos::add_capability(usb_ms_descriptor_base & cap) {
    TUPP_LOG(LOG_DEBUG, "usb_bos_dev_cap()");
    int i=0;
    // Find an empty slot
    for (i=0; i < TUPP_MAX_BOS_CAPABILITIES; ++i) {
        if (!_capabilities[i]) {
             _capabilities[i] = &cap;
            break;
        }
    }
    assert(i != TUPP_MAX_BOS_CAPABILITIES);
    // Update number of device capabilities
    _descriptor.bNumDeviceCaps = i+1;
    // Update total length
    set_total_length();
}

void usb_bos::set_total_length() {
    TUPP_LOG(LOG_DEBUG, "set_total_length()");
    // Start with our own descriptor
    uint16_t len = sizeof(bos_descriptor_t);
    for(int i=0; i < _descriptor.bNumDeviceCaps; ++i) {
        len += _capabilities[i]->desc_total_size();
    }
    _descriptor.wTotalLength = len;
}

uint16_t usb_bos::prepare_descriptor(uint8_t * buffer, uint16_t size) const {
    TUPP_LOG(LOG_DEBUG, "prepare_descriptor(size=%d)", size);
    uint8_t * tmp_ptr = buffer;
    memcpy(tmp_ptr, &_descriptor, sizeof(TUPP::bos_descriptor_t));
    tmp_ptr += sizeof(TUPP::bos_descriptor_t);
    assert((tmp_ptr-buffer) <= size);
    for (int i=0; i < _descriptor.bNumDeviceCaps; ++i) {
        _capabilities[i]->desc_begin();
        uint16_t cap_len = _capabilities[i]->desc_total_size();
        for (size_t j=0; j < cap_len; ++j) {
            *tmp_ptr++ = _capabilities[i]->desc_getNext();
        }
        assert((tmp_ptr-buffer) <= size);
    }
    return tmp_ptr-buffer;
}
