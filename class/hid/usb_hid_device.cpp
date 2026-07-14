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
#include "usb_hid_device.h"
#include "usb_structs.h"
#include "usb_log.h"
#include <cstring>

using namespace TUPP;
using enum TUPP::bInterfaceClass_t;
using enum TUPP::bInterfaceSubClass_t;
using enum TUPP::bInterfaceProtocol_t;
using enum TUPP::ep_attributes_t;
using enum TUPP::direction_t;
using enum TUPP::bRequest_t;
using enum usb_log::log_level;

usb_hid_device::usb_hid_device(
        usb_device_controller & controller,
        usb_configuration     & configuration,
        const uint8_t         * report_desc,
        uint16_t                report_desc_len)
: _configuration(configuration),
  _report_desc(report_desc),
  _report_desc_len(report_desc_len)
{
    TUPP_LOG(LOG_DEBUG, "usb_hid_device() @%x", this);

    // Update report descriptor length in HID functional descriptor
    reinterpret_cast<HID::hid_descriptor_t*>(
        _fd_hid.descriptor)->wDescriptorLength = report_desc_len;

    // USB interface descriptor config
    _interface.set_bInterfaceClass   (IF_CLASS_HID);
    _interface.set_bInterfaceSubClass(IF_SUBCLASS_HID_BOOT);
    _interface.set_bInterfaceProtocol(IF_PROTOCOL_HID_NONE);
    _interface.set_InterfaceName     ("HID Interface");

    // HID only needs one INTERRUPT IN endpoint
    _ep_in = controller.create_endpoint(_interface, DIR_IN, TRANS_INTERRUPT);

    // Handler when data has been sent to host
    _ep_in->data_handler = [&](uint8_t *, uint16_t) {
        if (report_complete_handler) {
            report_complete_handler();
        }
    };

    // Handler for HID specific setup requests
    _interface.setup_handler = [&](TUPP::setup_packet_t * pkt) {
        // Handle GET_DESCRIPTOR for HID Report Descriptor
        if (pkt->bRequest == REQ_GET_DESCRIPTOR) {
            uint8_t desc_type = (pkt->wValue >> 8) & 0xFF;
            if (desc_type == HID_DESC_TYPE_REPORT) {
                TUPP_LOG(LOG_INFO, "HID: GET_REPORT_DESCRIPTOR");
                uint16_t len = _report_desc_len;
                if (pkt->wLength < len) len = pkt->wLength;
                controller._ep0_in->start_transfer(
                    (uint8_t *)_report_desc, len);
                return;
            }
        }
        switch(pkt->bRequest) {
            case REQ_HID_GET_REPORT: {
                TUPP_LOG(LOG_INFO, "HID: GET_REPORT");
                controller._ep0_in->start_transfer(_buffer_in, 8);
                break;
            }
            case REQ_HID_SET_REPORT: {
                TUPP_LOG(LOG_INFO, "HID: SET_REPORT");
                controller.handler = [&](const uint8_t * data, uint16_t len) {
                    if (set_report_handler) {
                        set_report_handler(data, len);
                    }
                };
                controller._ep0_out->start_transfer(_buffer_in, pkt->wLength);
                break;
            }
            case REQ_HID_SET_IDLE: {
                TUPP_LOG(LOG_INFO, "HID: SET_IDLE");
                controller._ep0_in->send_zlp_data1();
                break;
            }
            case REQ_HID_SET_PROTOCOL: {
                TUPP_LOG(LOG_INFO, "HID: SET_PROTOCOL");
                controller._ep0_in->send_zlp_data1();
                break;
            }
            case REQ_HID_GET_PROTOCOL: {
                TUPP_LOG(LOG_INFO, "HID: GET_PROTOCOL");
                uint8_t protocol = HID_PROTOCOL_REPORT;
                controller._ep0_in->start_transfer(&protocol, 1);
                break;
            }
            default: {
                TUPP_LOG(LOG_ERROR,
                    "Unsupported HID command 0x%x", pkt->bRequest);
                controller._ep0_in->send_stall(true);
                controller._ep0_out->send_stall(true);
            }
        }
    };
}

bool usb_hid_device::send_report(const uint8_t * report, uint16_t len) {
    if (_ep_in->is_active()) return false;
    memcpy(_buffer_in, report, len);
    _ep_in->start_transfer(_buffer_in, len);
    return true;
}

bool usb_hid_device::send_keyboard_report(uint8_t modifier,
                                           const uint8_t keycode[6]) {
    HID::keyboard_report_t report;
    report.modifier = modifier;
    report.reserved = 0;
    memcpy(report.keycode, keycode, 6);
    return send_report((uint8_t *)&report, sizeof(HID::keyboard_report_t));
}

bool usb_hid_device::send_mouse_report(uint8_t buttons,
                                        int8_t x, int8_t y, int8_t wheel) {
    HID::mouse_report_t report;
    report.buttons = buttons;
    report.x       = x;
    report.y       = y;
    report.wheel   = wheel;
    return send_report((uint8_t *)&report, sizeof(HID::mouse_report_t));
}
