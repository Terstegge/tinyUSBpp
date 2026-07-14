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
// This class implements a generic HID (Human Interface Device).
// The user provides a HID Report Descriptor in the application,
// which defines the exact type of device (keyboard, mouse, etc.)
//
#ifndef TUPP_USB_HID_DEVICE_H
#define TUPP_USB_HID_DEVICE_H

#include "usb_hid_structs.h"
#include "usb_hid_defines.h"
#include "usb_fd_hid.h"
#include "usb_configuration.h"
#include "usb_interface.h"
#include "usb_endpoint.h"
#include "usb_device_controller.h"
#include <functional>
#include <cstdint>

class usb_hid_device {
public:
    usb_hid_device(usb_device_controller & controller,
                   usb_configuration     & configuration,
                   const uint8_t         * report_desc,
                   uint16_t                report_desc_len);

    // Send a keyboard report to the host
    bool send_keyboard_report(uint8_t modifier,
                              const uint8_t keycode[6]);

    // Send a mouse report to the host
    bool send_mouse_report(uint8_t buttons,
                           int8_t  x,
                           int8_t  y,
                           int8_t  wheel);

    // Send a raw HID report to the host
    bool send_report(const uint8_t * report, uint16_t len);

    // Callback: called when host sends a report (e.g. LED state)
    std::function<void(const uint8_t * data, uint16_t len)> set_report_handler;

    // Callback: called when report has been sent successfully
    std::function<void()> report_complete_handler;

private:
    // USB descriptor tree
    usb_configuration &  _configuration;
    usb_interface        _interface {_configuration};
    usb_fd_hid           _fd_hid    {_interface, 0};

    // USB endpoint (HID only needs one INTERRUPT IN endpoint)
    usb_endpoint *       _ep_in {nullptr};

    // HID Report Descriptor (provided by user application)
    const uint8_t *      _report_desc;
    uint16_t             _report_desc_len;

    // Internal data buffer
    uint8_t              _buffer_in[64] {0};
};

#endif // TUPP_USB_HID_DEVICE_H
