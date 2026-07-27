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
// Minimal USB Host enumeration state machine. Mirrors the steps used
// by tinyusb's process_enumeration() in usbh.c, but written using
// blocking task::sleep_ms() calls for simplicity (no async callbacks
// yet) since YAHAL already provides a cooperative task scheduler.
//
#ifndef TUPP_USB_HOST_ENUM_H
#define TUPP_USB_HOST_ENUM_H

#include "usb_hcd_interface.h"
#include "usb_structs.h"
#include "task.h"
#include "usb_log.h"
#include <cstring>
#include <cstdio>
#include <functional>

// How long we wait for a control transfer to complete before giving
// up. Without this, a non-responsive/disconnected device would hang
// the whole system forever in the blocking wait loop.
#define TUPP_HOST_XFER_TIMEOUT_MS 500

struct usb_host_found_endpoint_t {
    bool    found        {false};
    uint8_t daddr         {0};
    uint8_t ep_addr       {0};   // bit7 set = IN
    uint16_t max_packet   {0};
    uint8_t interval      {0};
    uint8_t interface_class    {0};
    uint8_t interface_subclass {0};
    uint8_t interface_protocol {0};
};

class usb_host_enum {
public:
    explicit usb_host_enum(usb_hcd_interface & hcd) : _hcd(hcd) {}

    // Run the enumeration sequence for whatever device is currently
    // attached: reset -> address-0 8-byte descriptor -> SET_ADDRESS ->
    // full device descriptor -> configuration descriptor -> SET_CONFIGURATION
    // -> find a HID interrupt endpoint -> create the endpoint and
    // register a report handler. Only handles a single device at
    // address 1 for now - no hub support, no multi-device tracking yet.
    bool enumerate(usb_host_found_endpoint_t & out_ep,
                   std::function<void(uint8_t*, uint16_t)> report_handler) {
        using enum usb_log::log_level;
        out_ep = {};
        TUPP_LOG(LOG_INFO, "usb_host_enum: sending bus reset");
        _hcd.port_reset(50); // 50ms bus reset
        task::sleep_ms(10); // recovery delay
        task::sleep_ms(500); // extended stability delay after reset

        if (!_hcd.port_connected()) {
            TUPP_LOG(LOG_WARNING, "usb_host_enum: device disappeared during reset");
            return false;
        }

        if (!get_descriptor_8(0)) {
            TUPP_LOG(LOG_WARNING, "usb_host_enum: failed/timed out reading "
                      "8-byte device descriptor");
            return false;
        }

        const uint8_t new_addr = 1;
        if (!set_address(new_addr)) {
            TUPP_LOG(LOG_WARNING, "usb_host_enum: SET_ADDRESS failed/timed out");
            return false;
        }
        task::sleep_ms(10);
        _hcd.assign_address(new_addr);

        if (!get_descriptor_full(new_addr)) {
            TUPP_LOG(LOG_WARNING, "usb_host_enum: failed/timed out reading "
                      "full device descriptor");
            return false;
        }

        if (!get_config_descriptor(new_addr, 9)) {
            TUPP_LOG(LOG_WARNING, "usb_host_enum: failed/timed out reading "
                      "config descriptor header");
            return false;
        }
        uint16_t total_len = (uint16_t)_buffer[2] | ((uint16_t)_buffer[3] << 8);
        if (total_len > sizeof(_buffer)) {
            TUPP_LOG(LOG_WARNING, "usb_host_enum: configuration too large (%d > %d)",
                      total_len, (int)sizeof(_buffer));
            return false;
        }

        if (!get_config_descriptor(new_addr, total_len)) {
            TUPP_LOG(LOG_WARNING, "usb_host_enum: failed/timed out reading "
                      "full config descriptor");
            return false;
        }

        if (!parse_config_for_hid_endpoint(total_len, out_ep)) {
            TUPP_LOG(LOG_WARNING, "usb_host_enum: no HID interrupt endpoint found");
            return false;
        }
        out_ep.daddr = new_addr;

        // --- SET_CONFIGURATION: REQUIRED by the USB spec before the
        //     device will respond on any endpoint other than control! ---
        uint8_t config_value = _buffer[5]; // bConfigurationValue
        if (!set_configuration(new_addr, config_value)) {
            TUPP_LOG(LOG_WARNING, "usb_host_enum: SET_CONFIGURATION failed/timed out");
            return false;
        }
        TUPP_LOG(LOG_INFO, "usb_host_enum: SET_CONFIGURATION(%d) done", config_value);

        TUPP_LOG(LOG_INFO, "usb_host_enum: found HID endpoint 0x%02x, creating "
                  "hardware endpoint and registering report handler",
                  out_ep.ep_addr);

        _hcd.create_endpoint(out_ep.daddr, out_ep.ep_addr,
                              TUPP::ep_attributes_t::TRANS_INTERRUPT,
                              out_ep.max_packet, out_ep.interval);
        bool registered = _hcd.set_int_ep_handler(out_ep.daddr, out_ep.ep_addr,
                                                   report_handler);
        if (!registered) {
            TUPP_LOG(LOG_WARNING, "usb_host_enum: failed to register report handler");
            return false;
        }

        TUPP_LOG(LOG_INFO, "usb_host_enum: enumeration complete, waiting for reports");
        return true;
    }

private:
    usb_hcd_interface & _hcd;
    uint8_t _buffer[256] {0};

    // Blocking helper with a timeout, so a non-responsive device can't
    // hang the whole system forever.
    bool do_control_xfer(uint8_t daddr, TUPP::setup_packet_t & req) {
        volatile bool done = false;
        volatile bool ok   = false;
        _hcd.control_xfer(daddr, req, _buffer,
            [&](hcd_xfer_result_t result) {
                ok   = result.success;
                done = true;
            });
        uint32_t waited_ms = 0;
        while (!done && waited_ms < TUPP_HOST_XFER_TIMEOUT_MS) {
            task::sleep_ms(1);
            // Poll SIE_STATUS for completion (RX_SHORT_PACKET bit12 or TRANS_COMPLETE bit18)
            uint32_t sie_st = *((volatile uint32_t*)&_USB_::USB.SIE_STATUS);
            if (waited_ms % 100 == 50) printf("  ST=0x%08lx BS=0x%08lx\n", sie_st, *((volatile uint32_t*)&_USB_::USB.BUFF_STATUS));
            if (!done && (sie_st & 0x1000u)) { // RX_SHORT_PACKET
                *((volatile uint32_t*)&_USB_::USB_CLR.SIE_STATUS) = 0x1000u;
                ok = true; done = true;
            }
            waited_ms++;
        }
        if (!done) {
            TUPP_LOG(usb_log::log_level::LOG_WARNING,
                      "usb_host_enum: control transfer timed out after %d ms",
                      TUPP_HOST_XFER_TIMEOUT_MS);
            return false;
        }
        return ok;
    }

    bool get_descriptor_8(uint8_t daddr) {
        TUPP::setup_packet_t req {};
        req.direction = TUPP::direction_t::DIR_IN;
        req.type      = TUPP::type_t::TYPE_STANDARD;
        req.recipient = TUPP::recipient_t::REC_DEVICE;
        req.bRequest  = TUPP::bRequest_t::REQ_GET_DESCRIPTOR;
        req.wValue    = (uint16_t)(TUPP::bDescriptorType_t::DESC_DEVICE) << 8;
        req.wIndex    = 0;
        req.wLength   = 8;
        return do_control_xfer(daddr, req) && _buffer[0] >= 8;
    }

    bool get_descriptor_full(uint8_t daddr) {
        TUPP::setup_packet_t req {};
        req.direction = TUPP::direction_t::DIR_IN;
        req.type      = TUPP::type_t::TYPE_STANDARD;
        req.recipient = TUPP::recipient_t::REC_DEVICE;
        req.bRequest  = TUPP::bRequest_t::REQ_GET_DESCRIPTOR;
        req.wValue    = (uint16_t)(TUPP::bDescriptorType_t::DESC_DEVICE) << 8;
        req.wIndex    = 0;
        req.wLength   = 18;
        return do_control_xfer(daddr, req) && _buffer[0] >= 18;
    }

    bool set_address(uint8_t new_addr) {
        TUPP::setup_packet_t req {};
        req.direction = TUPP::direction_t::DIR_OUT;
        req.type      = TUPP::type_t::TYPE_STANDARD;
        req.recipient = TUPP::recipient_t::REC_DEVICE;
        req.bRequest  = TUPP::bRequest_t::REQ_SET_ADDRESS;
        req.wValue    = new_addr;
        req.wIndex    = 0;
        req.wLength   = 0;
        return do_control_xfer(0, req);
    }

    bool get_config_descriptor(uint8_t daddr, uint16_t len) {
        TUPP::setup_packet_t req {};
        req.direction = TUPP::direction_t::DIR_IN;
        req.type      = TUPP::type_t::TYPE_STANDARD;
        req.recipient = TUPP::recipient_t::REC_DEVICE;
        req.bRequest  = TUPP::bRequest_t::REQ_GET_DESCRIPTOR;
        req.wValue    = (uint16_t)(TUPP::bDescriptorType_t::DESC_CONFIGURATION) << 8;
        req.wIndex    = 0;
        req.wLength   = len;
        return do_control_xfer(daddr, req) && _buffer[0] >= 9;
    }

    bool set_configuration(uint8_t daddr, uint8_t config_value) {
        TUPP::setup_packet_t req {};
        req.direction = TUPP::direction_t::DIR_OUT;
        req.type      = TUPP::type_t::TYPE_STANDARD;
        req.recipient = TUPP::recipient_t::REC_DEVICE;
        req.bRequest  = TUPP::bRequest_t::REQ_SET_CONFIGURATION;
        req.wValue    = config_value;
        req.wIndex    = 0;
        req.wLength   = 0;
        return do_control_xfer(daddr, req);
    }

    bool parse_config_for_hid_endpoint(uint16_t total_len,
                                        usb_host_found_endpoint_t & out_ep) {
        uint8_t cur_class = 0, cur_subclass = 0, cur_protocol = 0;
        uint16_t pos = 0;
        while (pos + 2 <= total_len) {
            uint8_t b_length = _buffer[pos];
            uint8_t b_type   = _buffer[pos + 1];
            if (b_length == 0) break;

            if (b_type == 0x04 && pos + 7 <= total_len) {
                cur_class    = _buffer[pos + 5];
                cur_subclass = _buffer[pos + 6];
                cur_protocol = _buffer[pos + 7];
            } else if (b_type == 0x05 && pos + 6 <= total_len) {
                uint8_t  ep_addr     = _buffer[pos + 2];
                uint8_t  attributes  = _buffer[pos + 3];
                uint16_t max_packet  = (uint16_t)_buffer[pos + 4] |
                                        ((uint16_t)_buffer[pos + 5] << 8);
                uint8_t  interval    = _buffer[pos + 6];
                bool is_in        = (ep_addr & 0x80) != 0;
                bool is_interrupt = (attributes & 0x03) == 0x03;
                if (is_in && is_interrupt) {
                    out_ep.found               = true;
                    out_ep.ep_addr             = ep_addr;
                    out_ep.max_packet          = max_packet;
                    out_ep.interval            = interval;
                    out_ep.interface_class     = cur_class;
                    out_ep.interface_subclass  = cur_subclass;
                    out_ep.interface_protocol  = cur_protocol;
                    return true;
                }
            }
            pos += b_length;
        }
        return false;
    }
};

#endif // TUPP_USB_HOST_ENUM_H
